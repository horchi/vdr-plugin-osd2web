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
         ctInteractive,            // = 0  - for interactive browser session
         ctView                    // = 1  - e.g. for graphtft display
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

      cWebSock();
      virtual ~cWebSock();

      int init(int aPort, int aTimeout);
      int exit();

      int service(int timeoutMs);
      int performData(MsgType type);

      // status callback methods

      static int callbackHttp(lws* wsi, lws_callback_reasons reason, void* user, void* in, size_t len);
      static int callbackOsd2Vdr(lws* wsi, lws_callback_reasons reason, void* user, void* in, size_t len);

      // static interface

      static int getClientCount()    { return clients.size(); }
      static void activateAvailableClient();
      static void pushMessage(const char* p);

   private:

      int login(json_t* oRequest);
      int logout(json_t* oRequest);

      int port;
      lws_context* context;
      lws_protocols protocols[3];

      // statics

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

      cUpdate(int aWebPort);
      ~cUpdate();

      // interface

      int init(const char* dev, int port, int startDetached);

      // thread stuff

      bool Start()        { return cThread::Start(); }
      void Stop();

      // static message interface to web thread

      static int pushMessage(json_t* obj, const char* title);

      static std::queue<std::string> messagesIn;
      static std::map<int,CategoryConfig> menuMaxLines;

   protected:

      virtual void Action();
      void atMeanwhile();

      // status interface

      virtual void ChannelSwitch(const cDevice* Device, int ChannelNumber, bool LiveView);
      virtual void Replaying(const cControl *Control, const char *Name, const char *FileName, bool On);
      virtual void Recording(const cDevice *Device, const char *Name, const char *FileName, bool On);
      virtual void TimerChange(const cTimer *Timer, eTimerChange Change);

   private:

      void updatePresentFollowing();

      int dispatchClientRequest();
      int performPing();
      int performFocusRequest(json_t* oRequest, int focus);
      int performKeyPressRequest(json_t* oRequest);
      int performChannelsRequest(json_t* oRequest);
      int performMaxLineRequest(json_t* oRequest);
      // int cleanupMessages();

      int isDefault(const char* name = SKIN_NAME)      { return strcmp(Skins.Current()->Name(), name) == 0; }
      int isSkinAttached(const char* name = SKIN_NAME) { return Skins.Current() && strcmp(Skins.Current()->Name(), name) == 0; }

      // osd status

      int currentChannelNr;

      // trigger

      time_t nextPing;
      time_t nextPresentUpdateAt;
      int pingTime;

      // ...

      cWebSock* webSock;
      bool active;
      int webPort;
      SkinMode skinMode;
      int actualClientCount;
};

//***************************************************************************
#endif // __UPDATE_H
