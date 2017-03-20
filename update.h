/**
 *  osd2web plugin for the Video Disk Recorder
 *
 *  update.h
 *
 *  (c) 2017 Jörg Wendel
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 **/

#ifndef __UPDATE_H
#define __UPDATE_H

//***************************************************************************
// Includes
//***************************************************************************

#include <libwebsockets.h>

#include <queue>

#include <vdr/tools.h>
#include <vdr/thread.h>
#include <vdr/status.h>
#include <vdr/skins.h>

#include "lib/json.h"
#include "lib/common.h"

#include "epg2vdr.h"

#define SKIN_NAME "osd2web"

//***************************************************************************
// Class OSD Service
//***************************************************************************

class cOsdService
{
   public:

      enum ObjectShape
      {
         osText   = 0x01,
         osSmall  = 0x02,
         osLarge  = 0x04
      };

      enum SkinMode
      {
         smManual,
         smAuto
      };

      enum Event
      {
         evUnknown,
         evTakeFocus,
         evLeaveFocus,
         evKeyPress,
         evChannels,
         evMaxLines,
         evLogin,
         evLogout,

         evCount
      };

      enum ClientType
      {
         ctInactive = na,
         ctInteractive,            // = 0 - for interactive browser session
         ctView                    // = 1 - e.g. for graphtft display
      };

      static const char* toName(Event event);
      static Event toEvent(const char* name);

      static const char* events[];
};

//***************************************************************************
// Tools
//***************************************************************************

eTimerMatch Matches(const cTimer* ti, const cEvent* event);
const cTimer* getTimerMatch(const cTimers* timers, const cEvent* event, eTimerMatch* match);

int event2Json(json_t* obj, const cEvent* event, const cChannel* channel = 0,
               eTimerMatch TimerMatch = (eTimerMatch)na, int Current = no,
               cOsdService::ObjectShape shape = cOsdService::osText);
int recording2Json(json_t* obj, const cRecording* recording);
int channel2Json(json_t* obj, const cChannel* channel);
int timer2Json(json_t* obj, const cTimer* timer);
int stream2Json(json_t* obj, const cChannel* channel);
int channels2Json(json_t* obj);

//***************************************************************************
// Class cWebSock
//***************************************************************************

class cWebSock : public cOsdService
{
   public:

      enum MsgType
      {
         mtNone = na,
         mtPing,
         mtData
      };

      enum Protokoll
      {
         sizeLwsPreFrame  = LWS_SEND_BUFFER_PRE_PADDING,
         sizeLwsPostFrame = LWS_SEND_BUFFER_POST_PADDING,
         sizeLwsFrame     = sizeLwsPreFrame + sizeLwsPostFrame
      };

      struct Client
      {
         ClientType type;
         std::queue<std::string> messagesOut;
         cMutex messagesOutMutex;
         void* wsi;

         void pushMessage(const char* p)
         {
            cMutexLock lock(&messagesOutMutex);
            messagesOut.push(p);
         }

         void cleanupMessageQueue()
         {
            cMutexLock lock(&messagesOutMutex);

            // just in case client is not connected and wasted messages are pending

            tell(0, "Info: Flushing (%ld) old 'wasted' messages of client (%p)",
                 messagesOut.size(), wsi);

            while (!messagesOut.empty())
               messagesOut.pop();
         }
      };

      cWebSock(const char* aCfgPath, const char* aEpgImagePath);
      virtual ~cWebSock();

      int init(int aPort, int aTimeout);
      int exit();

      int service(int timeoutMs);
      int performData(MsgType type);

      // status callback methods

      static int callbackHttp(lws* wsi, lws_callback_reasons reason, void* user, void* in, size_t len);
      static int callbackOsd2Vdr(lws* wsi, lws_callback_reasons reason, void* user, void* in, size_t len);

      // static interface

      static void activateAvailableClient();
      static void atLogin(lws* wsi);
      static int getClientCount();

      static void pushMessage(const char* p, lws* wsi = 0);

   private:

      static int serveFile(lws* wsi, const char* path);
      static int dispatchDataRequest(lws* wsi, const char* url);
      static int doEventImg(lws* wsi);
      static int doChannelLogo(lws* wsi);

      static const char* methodOf(const char* url);
      static const char* getStrParameter(lws* wsi, const char* name, const char* def = 0);
      static int getIntParameter(lws* wsi, const char* name, int def = na);

      //

      int port;
      lws_context* context;
      lws_protocols protocols[3];

      // statics

      static char* cfgPath;
      static char* epgImagePath;
      static int timeout;
      static void* activeClient;
      static std::map<void*,Client> clients;
      static MsgType msgType;

      // only used in callback

      static char* msgBuffer;
      static int msgBufferSize;
};

#include <vdr/skins.h>

//***************************************************************************
// Class Web2Osd Skin
//***************************************************************************

class cOsd2WebSkin : public cSkin
{
   public:

      cOsd2WebSkin();

      virtual const char* Description()  { return tr(SKIN_NAME); }

      // display

      virtual cSkinDisplayChannel* DisplayChannel(bool withInfo);
      virtual cSkinDisplayMenu* DisplayMenu();
      virtual cSkinDisplayReplay* DisplayReplay(bool modeOnly);
      virtual cSkinDisplayVolume* DisplayVolume();
      virtual cSkinDisplayTracks* DisplayTracks(const char* title, int numTracks, const char* const* tracks);
      virtual cSkinDisplayMessage* DisplayMessage();
};

//***************************************************************************
// Class cUpdate
//***************************************************************************

class cUpdate : public cStatus, cThread, public cOsdService
{
   public:

      struct CategoryConfig
      {
         int maxLines;
         ObjectShape shape;
      };

      // object

      cUpdate();
      ~cUpdate();

      // interface

      int init(const char* dev, int port, int startDetached);

      int initFileService();
      int exitFileService();
      int checkFileService();

      int setSkinAttachState(int state);
      int isDefaultSkin(const char* name = SKIN_NAME)  { return strcmp(Skins.Current()->Name(), name) == 0; }
      int isSkinAttached(const char* name = SKIN_NAME) { return Skins.Current() && strcmp(Skins.Current()->Name(), name) == 0; }

      // thread stuff

      bool Start()        { return cThread::Start(); }
      void Stop();

      // public data

      int triggerTimerUpdate;

      // static message interface to web thread

      static int pushMessage(json_t* obj, const char* title, long client = 0);
      static std::queue<std::string> messagesIn;
      static std::map<int,CategoryConfig> menuMaxLines;

   protected:

      virtual void Action();
      void atMeanwhile();

      // status interface

      virtual void OsdProgramme(time_t PresentTime, const char *PresentTitle, const char *PresentSubtitle, time_t FollowingTime, const char *FollowingTitle, const char *FollowingSubtitle);
      virtual void ChannelSwitch(const cDevice* Device, int ChannelNumber, bool LiveView);
      virtual void Replaying(const cControl *Control, const char *Name, const char *FileName, bool On);
      virtual void Recording(const cDevice *Device, const char *Name, const char *FileName, bool On);
      virtual void TimerChange(const cTimer *Timer, eTimerChange Change);

   private:

      void updatePresentFollowing();
      void updateTimers();
      void updateControl();
      void updateCustomData();
      void updateSkinState();

      int dispatchClientRequest();
      int performLogin(json_t* oObject);
      int performPing();
      int performKeyPressRequest(json_t* oRequest);
      int performChannelsRequest(json_t* oRequest);
      int performMaxLineRequest(json_t* oRequest);
      int performFocusRequest(json_t* oRequest, int focus);

      // VDRs osd status data

      int currentChannelNr;
      int haveActualEpg;
      const cControl* activeControl;
      int activeControlFps;

      // trigger

      time_t nextPing;
      time_t nextPresentUpdateAt;
      int pingTime;

      // ...

      time_t lastClientActionAt;
      int epg2vdrIsLoaded;
      cWebSock* webSock;
      bool active;
      SkinMode skinMode;
      int actualClientCount;
      int fdInotify;
      int wdInotify;
};

//***************************************************************************
#endif // __UPDATE_H
