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

//***************************************************************************
// Tools
//***************************************************************************

eTimerMatch Matches(const cTimer* ti, const cEvent* event);
const cTimer* getTimerMatch(const cTimers* timers, const cEvent* event, eTimerMatch* match);

//***************************************************************************
// Class cWebSock
//***************************************************************************

class cWebSock
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

      cWebSock();
      virtual ~cWebSock();

      int init(int aPort, int aTimeout);
      int exit();

      int service(int timeoutMs);
      int performData(MsgType type);

      // status callback methods

      static int callbackHttp(lws* wsi, lws_callback_reasons reason, void* user, void* in, size_t len);
      static int callbackOsd2Vdr(lws* wsi, lws_callback_reasons reason, void* user, void* in, size_t len);

      static int getClientCount() { return clientCount; }

   private:

      int port;
      lws_context* context;
      lws_protocols protocols[3];

      // statics

      static int timeout;
      static int clientCount;
      static MsgType msgType;

      // only used in callback

      static char* msgBuffer;
      static int msgBufferSize;
};

//***************************************************************************
// Class OSD Service
//***************************************************************************

class cOsdService
{
   public:

      enum Event
      {
         evUnknown,
         evKeyPress,
         evChannels,

         evCount
      };

      const char* toName(Event event);
      Event toEvent(const char* name);

      static const char* events[];
};

#include <vdr/skins.h>

//***************************************************************************
// Class Web2Osd Skin
//***************************************************************************

class cOsd2WebSkin : public cSkin
{
   public:

      cOsd2WebSkin();

      virtual const char* Description()  { return tr("osd2web"); }

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

      // object

      cUpdate();
      ~cUpdate();

      // interface

      int init(const char* dev, int port, int startDetached);

      // thread stuff

      bool Start()        { return cThread::Start(); }
      void Stop();

      // static message interface to web thread

      static int pushMessage(json_t* obj, const char* title);

      // static data

      static std::queue<std::string> messagesOut;
      static cMutex messagesOutMutex;
      static std::queue<std::string> messagesIn;

   protected:

      virtual void Action();
      void atMeanwhile();

      // status interface

      virtual void ChannelSwitch(const cDevice* Device, int ChannelNumber, bool LiveView);
      virtual void OsdSetEvent(const cEvent* event);
      virtual void OsdSetRecording(const cRecording* recording);
      virtual void OsdProgramme(time_t PresentTime, const char* PresentTitle, const char *PresentSubtitle, time_t FollowingTime, const char *FollowingTitle, const char *FollowingSubtitle);
      virtual void OsdChannel(const char* text);

      virtual void SetVolume(int Volume, bool Absolute);
      virtual void Replaying(const cControl *Control, const char *Name, const char *FileName, bool On);
      virtual void Recording(const cDevice *Device, const char *Name, const char *FileName, bool On);
      virtual void TimerChange(const cTimer *Timer, eTimerChange Change);
      virtual void OsdStatusMessage(const char *Message);
      virtual void OsdTitle(const char* Title);
      virtual void OsdItem(const char* Text, int Index);
      virtual void OsdMenuReady(int Count);
      virtual void OsdRecordingItem(const cRecording* Recording, int Index, int Count);
      virtual void OsdEventItem(const cEvent* Event, const char* Text, int Index, int Count);
      virtual void OsdCurrentItemIndex(const char *Text, int Index);
      virtual void OsdClear();
      virtual void OsdHelpKeys(const char* Red, const char* Green, const char* Yellow, const char* Blue);
      virtual void OsdTextItem(const char* Text, bool Scroll);
      virtual void OsdMenuDestroy();
      virtual void OsdMenuDisplay(eMenuCategory category);

   private:

      void eventMenuReady();
      void recordingMenuReady();
      void updatePresentFollowing();
      int event2Json(json_t* obj, const cEvent* event, const cChannel* channel = 0);
      int recording2Json(json_t* obj, const cRecording* recording);
      int channel2Json(json_t* obj, const cChannel* channel);
      int stream2Json(json_t* obj, const cChannel* channel);
      int channels2Json(json_t* obj);

      int dispatchClientRequest();
      int performServerData();
      int performPing();
      int performKeyPressRequest(json_t* oRequest);
      int performChannelsRequest(json_t* oRequest);
      int cleanupMessages();

      // osd status

      eMenuCategory menuCategory;
      std::string osdTitle;
      std::queue<std::string> menuItems;
      std::queue<json_t*> menuEventItems;
      std::queue<json_t*> menuRecordingItems;
      int currentChannelNr;

      // trigger

      time_t nextPing;
      time_t nextPresentUpdateAt;
      int pingTime;

      // ...

      cWebSock* webSock;
      cOsd2WebSkin* skin;
      bool active;
      int webPort;
};

//***************************************************************************
#endif // __UPDATE_H
