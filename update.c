/**
 *  osd2web plugin for the Video Disk Recorder
 *
 *  update.c
 *
 *  (c) 2017 Jörg Wendel
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 **/

//***************************************************************************
// Includes
//***************************************************************************

#include <vdr/remote.h>

#include "update.h"

std::queue<std::string> cUpdate::messagesOut;
cMutex cUpdate::messagesOutMutex;
std::queue<std::string> cUpdate::messagesIn;

int cUpdate::menuMaxLines[mcCam+1] = {};

//***************************************************************************
// OSD Service
//***************************************************************************

const char* cOsdService::events[] =
{
   "unknown",
   "keypress",
   "channels",
   "maxlines",
   0
};

const char* cOsdService::toName(Event event)
{
   if (event >= evUnknown && event < evCount)
      return events[event];

   return events[evUnknown];
}

cOsdService::Event cOsdService::toEvent(const char* name)
{
   for (int e = evUnknown; e < evCount; e++)
      if (strcasecmp(name, events[e]) == 0)
         return (Event)e;

   return evUnknown;
}

//***************************************************************************
// Object
//***************************************************************************

cUpdate::cUpdate()
{
   active = no;
   currentChannelNr = 0;
   pingTime = 60;
   webPort = 4444;
   nextPing = time(0);
   nextPresentUpdateAt = time(0);

   for (int i = 0; i <= mcCam; i++)
      menuMaxLines[i] = 10;

   webSock = new cWebSock();
}

cUpdate::~cUpdate()
{
   Stop();
   delete webSock;
}

//***************************************************************************
// Push Message
//***************************************************************************

int cUpdate::pushMessage(json_t* oContents, const char* title)
{
   // json2Data(obj, data, "gzip");

   json_t* obj = json_object();

   addToJson(obj, "event", title);
   json_object_set_new(obj, "object", oContents);

   char* p = json_dumps(obj, JSON_PRESERVE_ORDER);
   json_decref(obj);

   // only push message if at least one client is connected

   if (cWebSock::getClientCount())
   {
      cMutexLock lock(&messagesOutMutex);
      messagesOut.push(p);
   }

   tell(4, "DEBUG: PushMessage [%s]", p);
   free(p);

   return done;
}

//***************************************************************************
// Stop
//***************************************************************************

void cUpdate::Stop()
{
   if (active)
   {
      active = no;
      Cancel(3);
   }
}

//***************************************************************************
// At Meanwhile
//***************************************************************************

void cUpdate::atMeanwhile()
{
   // dispatch pending triggers

   if (nextPresentUpdateAt < time(0))
      updatePresentFollowing();

   // cleanup messages in case no client connected

   if (!webSock->getClientCount() && !messagesOut.empty())
      cleanupMessages();
}

//***************************************************************************
// Action
//***************************************************************************

void cUpdate::Action()
{
   tell(0, "osd2web plugin thread started (pid=%d)", getpid());

   // init web socket

   webSock->init(webPort, pingTime);
   active = yes;

   // main loop

   while (active)
   {
      webSock->service(50);
      atMeanwhile();

      if (!messagesIn.empty())       // data from clients
         dispatchClientRequest();

      if (!messagesOut.empty())      // data to clients
         performServerData();

      performPing();
   }

   active = no;
   isyslog("osd2web plugin thread ended (pid=%d)", getpid());
}

//***************************************************************************
// Perform Client Data
//***************************************************************************

int cUpdate::dispatchClientRequest()
{
   int status = fail;
   json_error_t error;
   json_t *oData, *oObject;
   Event event = evUnknown;

   // { "event" : "keypress", "object" : { "key" : "menu", "repeat" : 1 } }

   tell(0, "DEBUG: Got '%s'", messagesIn.front().c_str());
   oData = json_loads(messagesIn.front().c_str(), 0, &error);

   if (!oData)
   {
      tell(0, "Error: Ignoring unexpeted client request [%s]", messagesIn.front().c_str());
      messagesIn.pop();
      return fail;
   }

   // get the request

   event = toEvent(getStringFromJson(oData, "event", "<null>"));
   oObject = json_object_get(oData, "object");

   // dispatch client request

   switch (event)
   {
      case evKeyPress:  status = performKeyPressRequest(oObject);  break;
      case evChannels:  status = performChannelsRequest(oObject);  break;
      case evMaxLines:  status = performMaxLineRequest(oObject);   break;

      default:
         tell(0, "Error: Received unexpected client request '%s' at [%s]",
              toName(event), messagesIn.front().c_str());
   }

   json_decref(oData);      // free the json object
   messagesIn.pop();

   return status;
}

//***************************************************************************
// Perform Key Press
//***************************************************************************

int cUpdate::performKeyPressRequest(json_t* oRequest)
{
   if (!oRequest)
      return fail;

   const char* keyName = getStringFromJson(oRequest, "key", "<null>");
   int repeat = getIntFromJson(oRequest, "repeat");
   eKeys key = cKey::FromString(keyName);

   if (key == kNone)
   {
      tell(0, "Info: Recevied unknown key '%s'", keyName);
      return fail;
   }

   for (int i = 0; i < repeat; i++)
   {
      tell(0, "DEBUG: Put key (%d) '%s'", key, keyName);

      if (!cRemote::Put(key))
         tell(0, "Info: Sending key '%s' failed", keyName);
   }

   return success;
}

//***************************************************************************
// Perform Channels Request
//***************************************************************************

int cUpdate::performChannelsRequest(json_t* oRequest)
{
   json_t* oChannels = json_array();

   if (channels2Json(oChannels) == success)
      pushMessage(oChannels, "channels");

   return done;
}

//***************************************************************************
// Perform MaxLine Request
//***************************************************************************

int cUpdate::performMaxLineRequest(json_t* oRequest)
{
   // client set maxlines for the menu categories

   // { "event" : "maxlines", "object" : { "categories" : [ { "category" : "0", "maxlines" : "12",
   //                                                       { "category" : "1", "maxlines" : "10",
   //                                                       { "category" : "2", "maxlines" : "20",
   //                                                     ] } }

   json_t* objMaxLines = json_object_get(oRequest, "categories");

   if (!objMaxLines || !json_is_array(objMaxLines))
      return fail;

   int count = json_array_size(objMaxLines);

   for (int i = 0; i < count; ++i)
   {
      json_t* obj = json_array_get(objMaxLines, i);
      int category = getIntFromJson(obj, "category");

      if (category <= mcCam)
         menuMaxLines[i] = getIntFromJson(obj, "maxlines");
   }

   return done;
}

//***************************************************************************
// Perform Server Data
//***************************************************************************

int cUpdate::performServerData()
{
   cMutexLock lock(&messagesOutMutex);

   // any data ..

   if (!messagesOut.empty())
      webSock->performData(cWebSock::mtData);

   return done;
}

//***************************************************************************
// Perform Ping
//***************************************************************************

int cUpdate::performPing()
{
   if (nextPing < time(0))
   {
      webSock->performData(cWebSock::mtPing);
      nextPing = time(0) + pingTime-5;
   }

   return done;
}

//***************************************************************************
// Cleanup Messages
//***************************************************************************

int cUpdate::cleanupMessages()
{
   cMutexLock lock(&messagesOutMutex);

   // just in case no client is connected and wasted messages are pending

   tell(0, "Info: Flushing (%ld) old 'wasted' messages", messagesOut.size());

   while (!messagesOut.empty())
      messagesOut.pop();

   return done;
}
