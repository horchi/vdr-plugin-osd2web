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
#include <vdr/plugin.h>

#include "update.h"
#include "config.h"

std::queue<std::string> cUpdate::messagesIn;
std::map<int,cUpdate::CategoryConfig> cUpdate::menuMaxLines;
int cUpdate::menuCloseTrigger = no;

//***************************************************************************
// OSD Service
//***************************************************************************

const char* cOsdService::events[] =
{
   "unknown",
   "takefocus",
   "leavefocus",
   "keypress",
   "channels",
   "maxlines",
   "login",
   "logout",

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
// Class cUpdate
//***************************************************************************
//***************************************************************************
// Object
//***************************************************************************

cUpdate::cUpdate()
{
   triggerTimerUpdate = no;
   epg2vdrIsLoaded = no;
   actualClientCount = 0;
   skinMode = smAuto;
   active = no;
   currentChannelNr = 0;
   activeControl = 0;
   activeControlFps = 1;
   haveActualEpg = no;
   pingTime = 60;                      // timeout
   nextPing = time(0);
   nextPresentUpdateAt = time(0);
   fdInotify = na;
   wdInotify = 0;
   lastClientActionAt = time(0);

   for (int i = 0; i <= mcCam; i++)
   {
      menuMaxLines[i].maxLines = 10;
      menuMaxLines[i].shape = osText;
   }

   config.confPath = strdup(cPlugin::ConfigDirectory("osd2web"));

   webSock = new cWebSock(config.confPath, config.epgImagePath);
}

cUpdate::~cUpdate()
{
   Stop();
   delete webSock;
}

//***************************************************************************
// Push Message
//***************************************************************************

int cUpdate::pushMessage(json_t* oContents, const char* title, long client)
{
   // json2Data(obj, data, "gzip");

   json_t* obj = json_object();

   addToJson(obj, "event", title);
   json_object_set_new(obj, "object", oContents);

   char* p = json_dumps(obj, JSON_PRESERVE_ORDER);
   json_decref(obj);

   cWebSock::pushMessage(p, (lws*)client);

   tell(4, "DEBUG: PushMessage [%s]", p);
   free(p);

   return done;
}

//***************************************************************************
// Set Skin Attach State
//***************************************************************************

int cUpdate::setSkinAttachState(int attach)
{
   if (isDefaultSkin())
   {
      tell(1, "Ignoring '%s' skin request, osd2web ist configured as the default sikn",
           attach ? "attach" : "detach");
      return done;
   }

   // attach ..

   if (attach && !isSkinAttached())
   {
      Skins.SetCurrent(SKIN_NAME);
      tell(0, "Changed skin to '%s'", Skins.Current()->Name());
   }

   // detach ..

   else if (!attach && !isSkinAttached(Setup.OSDSkin))
   {
      Skins.SetCurrent(Setup.OSDSkin);
      cThemes::Load(Skins.Current()->Name(), Setup.OSDTheme, Skins.Current()->Theme());
      tell(0, "Changed skin to '%s'", Skins.Current()->Name());
   }

   updateSkinState();

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

   if (checkFileService() > 0)
      updateCustomData();

   if (nextPresentUpdateAt < time(0))
      updatePresentFollowing();

   if (triggerTimerUpdate)
      updateTimers();

   if (activeControl)
      updateControl();

   if (menuCloseTrigger)
   {
      if (!isDefaultSkin() && isSkinAttached())
         setSkinAttachState(no);

      menuCloseTrigger = no;
   }

   if (!isDefaultSkin() && isSkinAttached() &&
       lastClientActionAt < time(0) - config.clientOsdTimeout)
   {
      tell(0, "Info: Detaching from skin interface, no client key action"
           " since (%ld) seconds", time(0) - lastClientActionAt);
      setSkinAttachState(no);
   }

   if (!webSock->getClientCount())
   {
      // detach from Skin interface?

      if (skinMode == smAuto && isSkinAttached() && !isDefaultSkin())
      {
         tell(0, "Info: No client connected, detaching from skin interface");
         setSkinAttachState(no);
      }
   }
   else if (webSock->getClientCount() != actualClientCount)
   {
      // client count changed ...
   }

   actualClientCount = webSock->getClientCount();
}

//***************************************************************************
// Action
//***************************************************************************

void cUpdate::Action()
{
   tell(0, "osd2web plugin thread started (pid=%d)", getpid());

   // init File Service

   initFileService();

   // init web socket

   webSock->init(config.webPort, pingTime);
   active = yes;

   // main loop

   while (active)
   {
      webSock->service(100);
      atMeanwhile();

      // data from clients

      dispatchClientRequest();

      // data to client(s)

      webSock->performData(cWebSock::mtData);

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

   // #TODO loop here while (!messagesIn.empty()) ?

   if (messagesIn.empty())
      return done;

   // { "event" : "keypress", "object" : { "key" : "menu", "repeat" : 1 } }

   tell(1, "DEBUG: Got '%s'", messagesIn.front().c_str());
   oData = json_loads(messagesIn.front().c_str(), 0, &error);

   // get the request

   event = cOsdService::toEvent(getStringFromJson(oData, "event", "<null>"));
   oObject = json_object_get(oData, "object");

   // dispatch client request

   switch (event)
   {
      case evTakeFocus:  status = performFocusRequest(oObject, yes);  break;
      case evLeaveFocus: status = performFocusRequest(oObject, no);   break;
      case evKeyPress:   status = performKeyPressRequest(oObject);    break;
      case evChannels:   status = performChannelsRequest(oObject);    break;
      case evMaxLines:   status = performMaxLineRequest(oObject);     break;
      case evLogin  :    status = performLogin(oObject);              break;

      default:
         tell(0, "Error: Received unexpected client request '%s' at [%s]",
              toName(event), messagesIn.front().c_str());
   }

   json_decref(oData);      // free the json object
   messagesIn.pop();

   return status;
}

//***************************************************************************
// Perform Login
//***************************************************************************

int cUpdate::performLogin(json_t* oObject)
{
   char* path;

   int type = getIntFromJson(oObject, "type");
   long client = getLongFromJson(oObject, "client");
   long lastClient = getLongFromJson(oObject, "lastclient");

   // passive for 'old' client ?

   if (type == ctInteractive && lastClient)
   {
      json_t* oRole = json_object();
      addToJson(oRole, "role", "passive");
      cUpdate::pushMessage(oRole, "rolechange", lastClient);
   }

   // active for 'new' client

   {
      json_t* oRole = json_object();

      addToJson(oRole, "role", type == ctInteractive ? "active" : "passive");
      asprintf(&path, "%s/channellogos", config.confPath);
      addToJson(oRole, "havelogos", folderExists(path));
      cUpdate::pushMessage(oRole, "rolechange", client);
      free(path);
   }

   updateSkinState();
   updatePresentFollowing();  // trigger update of present/following
   updateTimers();

   return done;
}

//***************************************************************************
// Perform Focus Request
//***************************************************************************

int cUpdate::performFocusRequest(json_t* oRequest, int focus)
{
   return setSkinAttachState(focus);
}

//***************************************************************************
// Perform Key Press Request
//***************************************************************************

int cUpdate::performKeyPressRequest(json_t* oRequest)
{
   if (!oRequest)
      return fail;

   const char* keyName = getStringFromJson(oRequest, "key", "<null>");
   int repeat = getIntFromJson(oRequest, "repeat");
   eKeys key = cKey::FromString(keyName);

   lastClientActionAt = time(0);

   if (key == kNone)
   {
      tell(1, "Info: Received unknown key '%s'", keyName);
      return fail;
   }

   // auto attach on menu key

   if (!isDefaultSkin() && !isSkinAttached() && key == kMenu)
      setSkinAttachState(yes);

   // process key press

   for (int i = 0; i < repeat; i++)
   {
      tell(2, "DEBUG: Put key (%d) '%s'", key, keyName);

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

   // { "event" : "maxlines", "object" : { "categories" : [ { "category" : "0", "maxlines" : "12", "shape" : "1" },
   //                                                       { "category" : "1", "maxlines" : "10", "shape" : "1" },
   //                                                       { "category" : "2", "maxlines" : "20", "shape" : "1" },
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
      {
         menuMaxLines[i].maxLines = getIntFromJson(obj, "maxlines");
         menuMaxLines[i].shape = (ObjectShape)getIntFromJson(obj, "shape");
      }
   }

#if defined (APIVERSNUM) && (APIVERSNUM >= 20303)  // #TODO || (PATCHED)
   cOsdProvider::TriggerRecalcAndRefresh();
#endif

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
