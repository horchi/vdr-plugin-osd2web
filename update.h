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
#include "lib/vdrlocks.h"

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
int recording2Json(json_t* obj, const cTimers* timers, const cRecording* recording,
                   cOsdService::ObjectShape shape = cOsdService::ObjectShape::osLarge);
int channel2Json(json_t* obj, const cChannel* channel);
int timer2Json(json_t* obj, const cTimer* timer);
int stream2Json(json_t* obj, const cChannel* channel);
int channels2Json(json_t* obj);

int imagePaths2Json(json_t* obj, const char* path, const char* suffixFilter = "jpg jpeg");

//***************************************************************************
// Class cWebSock
//***************************************************************************

class cWebSock : public cOsdService
{
   public:

      enum MsgType
      {
         mtNone = na,
         mtPing,    // 0
         mtData     // 1
      };

      enum Protokoll
      {
         sizeLwsPreFrame  = LWS_SEND_BUFFER_PRE_PADDING,
         sizeLwsPostFrame = LWS_SEND_BUFFER_POST_PADDING,
         sizeLwsFrame     = sizeLwsPreFrame + sizeLwsPostFrame
      };

      struct SessionData
      {
         char* buffer;
         int bufferSize;
         int payloadSize;
         int dataPending;
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

      cWebSock(const char* aHttpPath);
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
      static void atLogout(lws* wsi, const char* message);
      static int getClientCount();

      static void pushMessage(const char* p, lws* wsi = 0);

   private:

      static int serveFile(lws* wsi, const char* path);
      static int dispatchDataRequest(lws* wsi, SessionData* sessionData, const char* url);
      static int doEventImg(lws* wsi);
      static int doRecordingImg(lws* wsi);
      static int doChannelLogo(lws* wsi);
      static int doEnvironment(lws* wsi, SessionData* sessionData);

      static const char* methodOf(const char* url);
      static const char* getStrParameter(lws* wsi, const char* name, const char* def = 0);
      static int getIntParameter(lws* wsi, const char* name, int def = na);

      //

      int port;
      lws_protocols protocols[3];

      // statics

      static lws_context* context;

      static char* httpPath;
      static char* epgImagePath;
      static int timeout;
      static void* activeClient;
      static std::map<void*,Client> clients;
      static cMutex clientsMutex;
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

      enum ViewMode
      {
         vmNormal = 0,
         vmDia,
      };

      struct CategoryConfig
      {
         int maxLines;
         ObjectShape shape;
      };

      struct FileVariable
      {
         std::string file;
         std::string name;
         std::string value;
         std::string color;
      };

      // object

      cUpdate();
      ~cUpdate();

      // interface

      int init(const char* dev, int port, int startDetached);

      int initFileService();
      int exitFileService();
      int checkFileService();
      int parseVariableFiles();
      int parseVariableFile(const char* path, const char* service);

      int setSkinAttachState(int state, int bySvdrp = no);
      int isDefaultSkin()                              { return strcmp(Setup.OSDSkin, SKIN_NAME) == 0; }
      int isSkinAttached(const char* name = SKIN_NAME) { return Skins.Current() && strcmp(Skins.Current()->Name(), name) == 0; }

      int toggleView(int next);

      // thread stuff

      bool Start()        { return cThread::Start(); }
      void Stop();

      // public data

      int triggerTimerUpdate;
      time_t initialRecordingsUpdateAt;
      int triggerRecordingsUpdate;
      int triggerReplayUpdate;
      int triggerReplayControlUpdate;
      int triggerForce;

      // static message interface to web thread

      static int pushMessage(json_t* obj, const char* title, long client = 0);
      static void menuClosed() { menuCloseTrigger = yes; }
      static int isEditable(eMenuCategory category);
      static void updateMenu();

      static std::queue<std::string> messagesIn;
      static std::map<int,CategoryConfig> menuMaxLines;
      static int menuCloseTrigger;
      static eMenuCategory menuCategory;
      static std::string menuTitle;

      // static interface fpr browser start/stop

      static pid_t browserPid;
      static int startBrowser(int byUser = no);
      static int stopBrowser();
      static int forkScript(int& pid, const char* script, char* const arguments[]);  // #TODO move to lib

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
      void updateRecordings();
      void updateReplayControl(int force = no);
      void updateReplay(int force = no);
      void updateDiaShow(int force = no);
      void updateCustomData();
      void updateSkinState();

      void forceRefresh();
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
      std::string activeReplayName;
      std::string activeReplayFile;

      // trigger

      time_t nextPing;
      time_t nextPresentUpdateAt;
      int pingTime;

      // ...

      int attachedBySvdrp;
      time_t lastClientActionAt;
      int epg2vdrIsLoaded;
      cWebSock* webSock;
      bool active;
      int actualClientCount;
      ViewMode viewMode;

      // file service stuff

      int fdInotify;
      int wdInotify;

      std::list<std::map<std::string,FileVariable>> serviceVariableFiles;

      // dia show stuff

      int initDia(const char* path);

      struct ImageFile
      {
         int initialized;
         std::string path;
         unsigned int width;
         unsigned int height;
         unsigned int orientation;
         unsigned int landscape;
      };

      int getNextDia(std::vector<ImageFile>::iterator& it, ImageFile*& file);
      int scanDiaDir(const char* path, int level = 0);

      std::vector<ImageFile> diaImages;
      std::vector<ImageFile>::iterator itCurrentDiaImage;

};

//***************************************************************************
#endif // __UPDATE_H
