/**
 *  osd2web plugin for the Video Disk Recorder
 *
 *  update.h
 *
 *  (c) 2017-2018 Jörg Wendel
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 **/

#pragma once

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
#include "squeezebox.h"
#include "websock.h"

#define SKIN_NAME "osd2web"

typedef cMutexLock cMyMutexLock;

//***************************************************************************
// Tools
//***************************************************************************

eTimerMatch Matches(const cTimer* ti, const cEvent* event);
const cTimer* getTimerMatch(const cTimers* timers, const cEvent* event, eTimerMatch* match);

int event2Json(json_t* obj, const cEvent* event, const cChannel* channel = 0,
               eTimerMatch TimerMatch = (eTimerMatch)na, int Current = no,
               cOsdService::ObjectShape shape = cOsdService::osText);
int radio2Json(json_t* obj, std::list<std::string>* rdsTextList, json_t* oEvent);
int recording2Json(json_t* obj, const cTimers* timers, const cRecording* recording,
                   cOsdService::cCuttingMarks* cuttingMarks,
                   cOsdService::ObjectShape shape = cOsdService::ObjectShape::osLarge);
int marks2Jason(const cMarks* marks, json_t* oMarks, int fps);
int channel2Json(json_t* obj, const cChannel* channel);
int timer2Json(json_t* obj, const cTimer* timer);
int stream2Json(json_t* obj, const cChannel* channel);
int channels2Json(json_t* obj);
int getRecordingDetails2Json(json_t* obj, int recId);
int imagePaths2Json(json_t* obj, const char* path, const char* suffixFilter = "jpg jpeg");
int squeezeboxTrack2Json(json_t* obj, TrackInfo* trackInfo);
int commands2Json(json_t*& obj);

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

class cUpdate : public cStatus, cThread, public cOsdService, public cWebInterface
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
         std::string vcolor;
      };

      // object

      cUpdate();
      ~cUpdate();

      // interface

      const char* myName() override  { return "osd2vdr"; }
      int init(const char* dev, int port, int startDetached);

      int initFileService();
      int exitFileService();
      int checkFileService();
      int parseVariableFiles();
      int parseVariableFile(const char* path, const char* service);

      int setSkinAttachState(int state, int bySvdrp = no);
      int isDefaultSkin()                              { return strcmp(Setup.OSDSkin, SKIN_NAME) == 0; }
      int isSkinAttached(const char* name = SKIN_NAME) { return Skins.Current() && strcmp(Skins.Current()->Name(), name) == 0; }
      int checkAutoAttach();

      int toggleView(int next);
      int setView(ViewMode view);
      ViewMode getView()  { return viewMode; }

      // thread stuff

      bool Start()        { return cThread::Start(); }
      void Stop();

      // public data

      int triggerTimerUpdate;
      int triggerRadioTextUpdate;
      time_t initialRecordingsUpdateAt;
      int triggerRecordingsUpdate;
      int triggerReplayUpdate;
      int triggerReplayControlUpdate;
      int triggerForce;

      // static message interface to web thread

      int pushInMessage(const char* data) override;

      // static int pushInMessage(const char* data);
      int pushOutMessage(json_t* obj, const char* title, long client = 0);
      static void menuClosed() { menuCloseTrigger = yes; }
      static int isEditable(eMenuCategory category);
      static void updateMenu();

      static std::queue<std::string> messagesIn;
      static cMutex messagesInMutex;

      static std::map<int,CategoryConfig> menuMaxLines;
      static int menuCloseTrigger;
      static eMenuCategory menuCategory;
      static std::string menuTitle;

      static int updateDiaList;

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
      virtual void MarksModified(const cMarks* Marks);
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
      void updateCommands();

      void forceRefresh();
      int dispatchClientRequest();
      int performLogin(json_t* oObject);
      int performKeyPressRequest(json_t* oRequest);
      int performChannelsRequest(json_t* oRequest);
      int performMaxLineRequest(json_t* oRequest);
      int performFocusRequest(json_t* oRequest, int focus);
      int performCommand(json_t* oRequest);

      // VDRs osd status data

      int currentChannelNr;
      int haveActualEpg;
      const cControl* activeControl;
      int activeControlFps;
      std::string activeReplayName;
      std::string activeReplayFile;

      cCuttingMarks cuttingMarks;

      // trigger

      time_t nextPing {0};
      time_t nextPresentUpdateAt {0};
      int pingTime {60};

      // ...

      int attachedBySvdrp {0};
      time_t lastClientActionAt {0};
      int epg2vdrIsLoaded {0};
      cWebSock* webSock {};
      bool active;
      int actualClientCount;
      ViewMode viewMode;

      // file service stuff

      int fdInotify;
      int wdInotify;

      std::list<std::map<std::string,FileVariable>> serviceVariableFiles;

      // radio RDS

      std::list<std::string> rdsTextList;

      // dia show stuff

      int initDia(const char* path);

      struct ImageFile
      {
         int initialized;
         std::string path;
         std::string name;
         unsigned int width;
         unsigned int height;
         unsigned int orientation;
         unsigned int landscape;

         bool operator < (const ImageFile& img) const { return name < img.name; }
      };

      int getNextDia(std::vector<ImageFile>::iterator& it, ImageFile*& file);
      int scanDiaDir(const char* path);

      std::vector<ImageFile> diaImages;
      std::vector<ImageFile>::iterator itCurrentDiaImage;
};

extern cUpdate* update;
