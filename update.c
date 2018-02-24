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

#include <sys/wait.h>
#include <signal.h>

#include <vdr/remote.h>
#include <vdr/plugin.h>

#include "update.h"
#include "config.h"

std::queue<std::string> cUpdate::messagesIn;
std::map<int,cUpdate::CategoryConfig> cUpdate::menuMaxLines;
int cUpdate::menuCloseTrigger = no;

eMenuCategory cUpdate::menuCategory = mcUnknown;
std::string cUpdate::menuTitle = "";

pid_t cUpdate::browserPid = 0;

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
   browserPid = 0;
   triggerTimerUpdate = no;
   triggerRadioTextUpdate = no;
   triggerRecordingsUpdate = no;
   initialRecordingsUpdateAt = 0;
   triggerReplayUpdate = no;
   triggerReplayControlUpdate = no;
   triggerForce = no;
   nextPresentUpdateAt = time(0);
   epg2vdrIsLoaded = no;
   actualClientCount = 0;
   active = no;
   currentChannelNr = 0;
   haveActualEpg = no;
   activeControl = 0;
   activeControlFps = 1;
   activeReplayName = "";
   activeReplayFile = "";
   pingTime = 60;                      // timeout
   nextPing = time(0);
   fdInotify = na;
   wdInotify = 0;
   lastClientActionAt = time(0);
   attachedBySvdrp = no;
   cuttingMarks.isSet = no;
   viewMode = vmNormal;

   for (int i = 0; i <= mcCam; i++)
   {
      menuMaxLines[i].maxLines = 10;
      menuMaxLines[i].shape = osText;
   }

   config.confPath = strdup(cPlugin::ConfigDirectory("osd2web"));
   asprintf(&config.httpPath, "%s/http", config.confPath);

   if (!config.logoPath)
      asprintf(&config.logoPath, "%s/channellogos", config.confPath);

   if (!config.diaPath)
      asprintf(&config.diaPath, "%s/dia/", config.httpPath);

   webSock = new cWebSock(config.httpPath);
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
// Is Editable Menu Category
//***************************************************************************

int cUpdate::isEditable(eMenuCategory category)
{
   return
      category >= mcPluginSetup &&
      category <= mcSetupMisc &&
      category != mcSetup;
}

//***************************************************************************
// Check Auto Attach (TV alive)
//***************************************************************************

int cUpdate::checkAutoAttach()
{
   static int lastTvState = na;
   static time_t nextAt = time(0);

   // if not already the default skin AND if iP configured
   //  => auto attach/detach is active!

   if (isDefaultSkin() || !config.tvIp || time(0) < nextAt)
      return done;

   int tvAlive = isAlive(config.tvIp);

   if (tvAlive == na)
   {
      tell(0, "Auto check of TV failed");
      nextAt = time(0) + 60;
      return fail;
   }

   if (tvAlive != lastTvState)
   {
      tell(1, "TV State change detected, TV is now '%s'", tvAlive ? "ON" : "OFF");
      setSkinAttachState(tvAlive ? no : yes, yes);
   }

   // check every 2 seconds

   nextAt = time(0) + 2;
   lastTvState = tvAlive;

   return done;
}

//***************************************************************************
// Set Skin Attach State
//***************************************************************************

int cUpdate::setSkinAttachState(int attach, int bySvdrp)
{
   if (isDefaultSkin())
   {
      tell(1, "Ignoring '%s' skin request, osd2web is configured as the default sikn",
           attach ? "attach" : "detach");
      return done;
   }

   if (!attach && attachedBySvdrp && !bySvdrp)
   {
      tell(1, "Ignoring 'detach' skin request, attach was performed by SVDRP");
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

   if (bySvdrp)    // change only if attach is triggert by SVDRP!
      attachedBySvdrp = attach;

   updateSkinState();

   return done;
}

//***************************************************************************
// Toggel View
//***************************************************************************

int cUpdate::toggleView(int next)
{
   viewMode = viewMode == vmNormal ? vmDia : vmNormal;
   triggerForce = yes;

   tell(0, "View mode toggled to (%d)", viewMode);

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

   if (triggerForce)
   {
      forceRefresh();
   }
   else
   {
      updateDiaShow();

      if (checkFileService() > 0)
         updateCustomData();

      if (nextPresentUpdateAt <= time(0) || triggerRadioTextUpdate)
         updatePresentFollowing();

      if (triggerTimerUpdate)
         updateTimers();

      if (triggerRecordingsUpdate)
         updateRecordings();

      if (initialRecordingsUpdateAt && time(0) >= initialRecordingsUpdateAt)
      {
         initialRecordingsUpdateAt = 0;
         updateRecordings();
      }

      if (triggerReplayUpdate)
         updateReplay();   // calls updateReplayControl()
      else
         updateReplayControl();
   }

   if (menuCloseTrigger)
   {
      if (!isDefaultSkin() && isSkinAttached())
         setSkinAttachState(no);

      menuCloseTrigger = no;
   }

   if (!attachedBySvdrp && !isDefaultSkin() && isSkinAttached() &&
       lastClientActionAt < time(0) - config.clientOsdTimeout)
   {
      tell(0, "Info: Detaching from skin interface, no client key action"
           " since (%ld) seconds", time(0) - lastClientActionAt);
      setSkinAttachState(no);
   }

   if (!webSock->getClientCount())
   {
      // detach from Skin interface?

      if (!attachedBySvdrp && isSkinAttached() && !isDefaultSkin())
      {
         tell(0, "Info: No client connected, detaching from skin interface");
         setSkinAttachState(no);
      }
   }

   if (webSock->getClientCount() != actualClientCount)
   {
      // client count changed ...
      //    nothing yet
   }

   //

   if (webSock->getClientCount() && !isDefaultSkin())
      checkAutoAttach();

   actualClientCount = webSock->getClientCount();
}

//***************************************************************************
// Fork Script
//***************************************************************************

int cUpdate::forkScript(int& pid, const char* script, char* const arguments[])
{
   pid = 0;

   tell(0, "Starting '%s'", script);

   if ((pid = fork()) < 0)
   {
      tell(0, "Error: Fork failed with %s", strerror(errno));
      pid = 0;
      return -1;
   }

   if (pid > 0)
   {
      // parent process,
      //  ... work is done

      return success;
   }

   char* argv[30]; memset(argv, 0, sizeof(argv));
   int argc = 0;

   // child code

   argv[argc++] = strdup(script);

   for (int i = 0; arguments[i]; i++)
      argv[argc++] = strdup(arguments[i]);

   argv[argc] = 0;

   tell(1, "Starting '%s' with '%s'", script, notNull(argv[1]));

   execv(script, argv);

   tell(0, "Process '%s' ended unexpectedly, reason was '%s'",
        script, strerror(errno));

   for (int i = 0; i < argc; i++)
      free(argv[i]);

   ::exit(0);

   return done;
}

//***************************************************************************
// Start Browser
//***************************************************************************

int cUpdate::startBrowser(int byUser)
{
   int res = success;

   // start browser?

   if (config.startBrowser || byUser)
   {
      char* args[30]; memset(args, 0, sizeof(args));
      char* browserScript;

      asprintf(&browserScript, "%s/%s", config.confPath, "startBrowser.sh");
      asprintf(&args[0], "http://localhost:%d/skins", config.webPort);
      asprintf(&args[1], "%s", config.browserDisplay);
      args[2] = 0;

      if (fileExists(browserScript))
      {
         res = forkScript(browserPid, browserScript, args);

         if (res == success)
            tell(0, "Started browser on display '%s', PID is (%d)", config.browserDisplay, browserPid);
      }
      else
      {
         res = fail;
         tell(0, "Error: Can't start browser '%s' not found!", browserScript);
      }

      free(browserScript);

      for (int i = 0; args[i]; i++)
         free(args[i]);
   }

   return res;
}

//***************************************************************************
// Stop Browser
//***************************************************************************

int cUpdate::stopBrowser()
{
   if (!browserPid)
      return done;

   tell(0, "Try to terminate browser with with pid (%d)", browserPid);

   if (kill(browserPid, SIGTERM) < 0)
   {
      tell(0, "Error: Terminating browser with pid (%d) failed, error was '%s'",
           browserPid, strerror(errno));
      return fail;
   }

   // wait up to 5 secons for the browser to quit

   time_t timeoutAt = time(0) + 5;
   int status = 0;

   while (waitpid(browserPid, &status, WNOHANG) == 0)
   {
      usleep(10000);

      if (time(0) > timeoutAt)
      {
         browserPid = 0;
         tell(0, "Giving up, browser seems not to be terminating");
         return fail;
      }
   }

   tell(0, "Browser exited with %d", WEXITSTATUS(status));
   browserPid = 0;

   return success;
}

//***************************************************************************
// Action
//***************************************************************************

void cUpdate::Action()
{
   int state;

   tell(0, "osd2web plugin thread started (pid=%d)", getpid());

   initialRecordingsUpdateAt = time(0) + 30;

   // init File Service

   initFileService();
   initDia(config.diaPath);

   // init web socket

   while ((state = webSock->init(config.webPort, pingTime)) != success)
   {
      tell(0, "Retrying in 2 seconds");
      sleep(2);
   }

   if (state != success)
   {
      tell(0, "osd2web plugin thread aborted (pid=%d)!", getpid());
      return ;
   }

   active = yes;

   // start browser

   if (active)
      startBrowser();

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

   stopBrowser();

   active = no;
   tell(0, "osd2web plugin thread ended (pid=%d)", getpid());
}

//***************************************************************************
// Dispatch Client Data
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

   tell(2, "DEBUG: Got '%s'", messagesIn.front().c_str());
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
      case evLogin:      status = performLogin(oObject);              break;

      default:
         tell(0, "Error: Received unexpected client request '%s' at [%s]",
              toName(event), messagesIn.front().c_str());
   }

   json_decref(oData);      // free the json object
   messagesIn.pop();

   return status;
}

//***************************************************************************
// Force Refresh
//***************************************************************************

void cUpdate::forceRefresh()
{
   updateDiaShow(yes);
   updateReplay(yes);

   nextPresentUpdateAt = time(0);
   updatePresentFollowing();       // trigger update of present/following

   updateCustomData();
   updateTimers();
   updateRecordings();

   if (menuCategory > mcUnknown)
      updateMenu();

   triggerForce = no;
}

//***************************************************************************
// Perform Login
//***************************************************************************

int cUpdate::performLogin(json_t* oObject)
{
   int type = getIntFromJson(oObject, "type");
   long client = getLongFromJson(oObject, "client");
   long lastClient = getLongFromJson(oObject, "lastclient");

   // passive for 'old' client ...

   if (type == ctInteractive && lastClient)
   {
      json_t* oRole = json_object();
      addToJson(oRole, "role", "passive");
      addToJson(oRole, "havelogos", folderExists(config.logoPath));
      cUpdate::pushMessage(oRole, "rolechange", lastClient);
   }

   // active for 'new' client ...

   {
      json_t* oRole = json_object();

      addToJson(oRole, "role", type == ctInteractive ? "active" : "passive");
      addToJson(oRole, "havelogos", folderExists(config.logoPath));
      cUpdate::pushMessage(oRole, "rolechange", client);
   }

   updateSkinState();
   forceRefresh();

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
      tell(3, "DEBUG: Put key (%d) '%s'", key, keyName);

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
// Update Dia Show
//***************************************************************************

void cUpdate::updateDiaShow(int force)
{
   static time_t nextAt = time(0);

   if (viewMode != vmDia)
   {
      if (force)
      {
         json_t* oDiaShow = json_object();
         addToJson(oDiaShow, "active", no);
         cUpdate::pushMessage(oDiaShow, "diashow");
      }

      return;
   }

   if (time(0) < nextAt && !force)
      return;

   ImageFile* file;

   nextAt = time(0) + config.diaCycleTime;

   if (getNextDia(itCurrentDiaImage, file) != success)
   {
      tell(0, "Info: No next image for diashow found!");
      return;
   }

   json_t* oDiaShow = json_object();

   const char* start = file->path.c_str() + strlen(config.diaPath);

   while (*start && *start == '/')
      start++;

   std::string title = strReplace("/", " - ", start);

   addToJson(oDiaShow, "active", yes);
   addToJson(oDiaShow, "title", title.c_str());
   addToJson(oDiaShow, "filename", file->path.c_str() + strlen(config.httpPath));
   addToJson(oDiaShow, "width", file->width);
   addToJson(oDiaShow, "height", file->height);
   addToJson(oDiaShow, "orientation", file->orientation);
   addToJson(oDiaShow, "landscape", file->landscape);

   tell(0, "Update diashow image to '%s'", file->path.c_str());
   cUpdate::pushMessage(oDiaShow, "diashow");
}
