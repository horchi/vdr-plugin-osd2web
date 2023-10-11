/**
 *  osd2web plugin for the Video Disk Recorder
 *
 *  status.c
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

#include <algorithm>

#include <vdr/plugin.h>

#include "update.h"
#include "epg2vdr.h"

//***************************************************************************
// Osd Channel Switch
//***************************************************************************

void cUpdate::ChannelSwitch(const cDevice* device, int channelNumber, bool liveView)
{
   if (liveView && channelNumber)
   {
      tell(3, "ChannelSwitch: channelNumber: %d", channelNumber);
      currentChannelNr = channelNumber;
      nextPresentUpdateAt = time(0);
      rdsTextList.clear();
      tell(3, "Cleared RDS text list");
   }
}

//***************************************************************************
// Osd Programme
//***************************************************************************

void cUpdate::OsdProgramme(time_t PresentTime, const char* PresentTitle,
                           const char* PresentSubtitle, time_t FollowingTime,
                           const char* FollowingTitle, const char* FollowingSubtitle)
{
   // haveActualEpg is NOT set if the epg not avalible at channelswich
   //   somtimes the epg is loaded later ...

   if (!haveActualEpg && (!isEmpty(PresentTitle) || !isEmpty(FollowingTitle)))
   {
      tell(3, "OsdProgramme: PresentTitle '%s', FollowingTitle '%s'",
           PresentTitle, FollowingTitle);

      nextPresentUpdateAt = time(0);
   }
}

//***************************************************************************
// Recording
//***************************************************************************

void cUpdate::Recording(const cDevice* Device, const char* Name,
                        const char* FileName, bool On)
{
   triggerRecordingsUpdate = yes;
}

//***************************************************************************
// Replaying
//***************************************************************************

void cUpdate::Replaying(const cControl* Control, const char* Name,
                        const char* FileName, bool On)
{
   tell(3, "Replaying: Replay '%s', Name '%s', FileName '%s'",
        On ? "Start" : "Stop" , notNull(Name), FileName);

   if (!On || isEmpty(FileName))
   {
      activeControl = 0;
      activeReplayFile = "";
      activeReplayName = "";

      nextPresentUpdateAt = time(0);   // update 'actual' on stop of replay
   }
   else
   {
      activeControlFps = 1;
      activeControl = Control;
      activeReplayFile = FileName;
      activeReplayName = Name;
   }

   triggerReplayUpdate = yes;
}

//***************************************************************************
// Marks Modified   -   working since vdr 2.3.9 !
//***************************************************************************

void cUpdate::MarksModified(const cMarks* Marks)
{
   cMutexLock lock(&cuttingMarks.mutex);

   while (!cuttingMarks.queue.empty())
      cuttingMarks.queue.pop();

   for (const cMark* mark = Marks->First(); mark; mark = Marks->Next(mark))
   {
      cCutMark cutMark;
      cutMark.position = mark->Position();
      cutMark.comment = mark->Comment() ? mark->Comment() : "";
      cuttingMarks.queue.push(cutMark);
   }

   cuttingMarks.isSet = yes;
   triggerReplayUpdate = yes;
}

//***************************************************************************
// Timer Change
//***************************************************************************

void cUpdate::TimerChange(const cTimer* Timer, eTimerChange Change)
{
   // update timers here only without epg2vdr
   //   with epg2vdr it is updated by a service interface trigger

   if (!epg2vdrIsLoaded)
      triggerTimerUpdate = yes;
}

//***************************************************************************
// Update Present / Following
//***************************************************************************

void cUpdate::updatePresentFollowing()
{
   triggerRadioTextUpdate = no;

   if (!currentChannelNr)
   {
      nextPresentUpdateAt = time(0) + 60;
      return;
   }

   haveActualEpg = no;

   GET_TIMERS_READ(timers);
   GET_CHANNELS_READ(channels);
   GET_SCHEDULES_READ(schedules);

   const cChannel* channel = channels->GetByNumber(currentChannelNr);

   if (!channel)
      return ;

   int isRadio = !channel->Vpid() && channel->Apid(0);

   tell(3, "update present/following for '%s' channel '%s'", isRadio ? "RADIO" : "TV", channel->Name());

   json_t* obj = json_object();
   json_t* oStreamInfo = json_object();
   json_t* oChannel = json_object();
   json_t* oPresent = json_object();
   json_t* oFollowing = 0;
   int haveSchedule = no;

   channel2Json(oChannel, channel);
   stream2Json(oStreamInfo, channel);

   json_object_set_new(obj, "channel", oChannel);
   json_object_set_new(obj, "streaminfo", oStreamInfo);

   const cSchedule* schedule = schedules ? schedules->GetSchedule(channel->GetChannelID()) : 0;

   if (schedule)
   {
      eTimerMatch timerMatch;
      const cEvent* present = schedule->GetPresentEvent();
      const cEvent* following = schedule->GetFollowingEvent();

      oFollowing = json_object();
      haveActualEpg = present != 0;
      getTimerMatch(timers, present, &timerMatch);
      event2Json(oPresent, present, 0, timerMatch, no, cOsdService::osLarge);
      getTimerMatch(timers, following, &timerMatch);
      event2Json(oFollowing, following, 0, timerMatch, no, cOsdService::osLarge);

      // we need a trigger on start of following event

      nextPresentUpdateAt = following ? following->StartTime() : time(0) + 10;

      haveSchedule = yes;
   }
   else
   {
      tell(0, "Info: Can't get schedules");
      nextPresentUpdateAt = time(0) + 60;
   }

   if (isRadio)
   {
      json_t* oRadio = json_object();

      if (radio2Json(oRadio, &rdsTextList, haveSchedule ? 0 : oPresent) == success)
         json_object_set_new(obj, "radio", oRadio);
      else
         json_decref(oRadio);
   }

   json_object_set_new(obj, "present", oPresent);

   if (oFollowing)
      json_object_set_new(obj, "following", oFollowing);

   pushOutMessage(obj, "actual");
}

//***************************************************************************
// Update Timers
//***************************************************************************

void cUpdate::updateTimers()
{
   json_t* oTimers = json_array();
   cPlugin* pEpg2Vdr = cPluginManager::GetPlugin("epg2vdr");

   if (pEpg2Vdr)
   {
      epg2vdrIsLoaded = yes;

      cEpgTimer_Service_V1 data;

      if (pEpg2Vdr->Service(EPG2VDR_TIMER_SERVICE, &data))
      {
         for (auto it = data.epgTimers.begin(); it != data.epgTimers.end(); ++it)
         {
            cEpgTimer_Interface_V1* timer = (*it);

            tell(3, "Got '%s' timer for '%s' - '%s'",
                 timer->isLocal() ? "local" : "remote",
                 timer->File(),
                 timer->hasState('R') ? "timer is recording" : "timer is pending");

            json_t* oTimer = json_object();
            timer2Json(oTimer, timer);
            json_array_append_new(oTimers, oTimer);

            delete timer;
         }

         pushOutMessage(oTimers, "timers");
      }
   }

   else
   {
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
      const cTimers* timers = 0;
      cStateKey stateKey;

      if (!(timers = cTimers::GetTimersRead(stateKey, 500)))
      {
         tell(1, "Can't get lock for updateTimers(), retrying later");
         return ;
      }
#else
      const cTimers* timers = &Timers;
#endif

      for (const cTimer* timer = timers->First(); timer; timer = timers->Next(timer))
      {
         json_t* oTimer = json_object();
         tell(3, "Got timer for '%s' - '%s'", timer->File(),
              timer->Recording() ? "timer is recording" : "timer is pending");

         timer2Json(oTimer, timer);
         json_array_append_new(oTimers, oTimer);
      }

#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
      stateKey.Remove();
#endif
      pushOutMessage(oTimers, "timers");
   }

   triggerTimerUpdate = no;
}

//***************************************************************************
// Update Recordings
//***************************************************************************

bool compareTime(const cRecording*& first, const cRecording*& second)
{
   if (first->Start() > second->Start())
      return true;

   return false;
}

void cUpdate::updateRecordings()
{
   json_t* oRecordings = json_array();
   int count = 0;

   std::list<const cRecording*> recList;

   triggerRecordingsUpdate = no;

   GET_TIMERS_READ(timers);
   GET_RECORDINGS_READ(recordings);

   // firyst add all recordings to the list

   for (const cRecording* recording = recordings->First(); recording; recording = recordings->Next(recording))
      recList.push_back(recording);

   // now sort the list by date

   recList.sort(compareTime);

   // and now use only the youngest 10

   for (auto it = recList.begin(); it != recList.end() && count < 10; it++, count++)
   {
      json_t* oRecording = json_object();
      recording2Json(oRecording, timers, *it, &cuttingMarks, cOsdService::ObjectShape::osSmall);
      json_array_append_new(oRecordings, oRecording);
   }

   pushOutMessage(oRecordings, "recordings");
}

//***************************************************************************
// Update Replay
//***************************************************************************

void cUpdate::updateReplay(int force)
{
   json_t* oRecording = json_object();

   triggerReplayUpdate = no;

   if (!activeControl)
   {
      addToJson(oRecording, "active", no);
      pushOutMessage(oRecording, "replay");
      updateReplayControl(force);
      return;
   }

   if (viewMode != vmNormal)
      return;

   GET_TIMERS_READ(timers);
   GET_RECORDINGS_READ(recordings);

   const cRecording* recording = recordings ? recordings->GetByName(activeReplayFile.c_str()) : 0;

   addToJson(oRecording, "active", yes);

   if (recording)
   {
      activeControlFps = recording->Info() ? recording->Info()->FramesPerSecond() : 1;
      recording2Json(oRecording, timers, recording, &cuttingMarks);
   }
   else
   {
      addToJson(oRecording, "name", activeReplayName.c_str());
      addToJson(oRecording, "filename", activeReplayFile.c_str());
   }

   pushOutMessage(oRecording, "replay");
   updateReplayControl(force);
}

//***************************************************************************
// Update Replay Control
//***************************************************************************

void cUpdate::updateReplayControl(int force)
{
   static time_t lastCheckAt = na;
   // static int ltotal = 0, lspeed = 0;
   // static bool lplay = false, lforward = false;
   static bool lactive = no;

   force |= triggerReplayControlUpdate;
   triggerReplayControlUpdate = no;

   if (!activeControl)
   {
      if (force || lactive)
      {
         json_t* oControl = json_object();
         lactive = no;
         addToJson(oControl, "active", lactive);
         pushOutMessage(oControl, "replaycontrol");
      }

      return;
   }

   // check only once per second for changes

   if (!force && lastCheckAt == time(0))
      return ;

   lastCheckAt = time(0);

   int total, current, speed;
   bool play, forward;
   cControl* control = (cControl*)activeControl;  // type cast only for vdr 2.2.0 needed :(

   control->GetReplayMode(play, forward, speed);
   control->GetIndex(current, total);

   // any changes ... except of 'current' position

   // if (!force && lactive && total == ltotal && speed == lspeed && play == lplay && forward == lforward)
   //    return ;

   // lspeed = speed; lplay = play; lforward = forward;
   // ltotal = total;
   lactive = yes;

   json_t* oControl = json_object();

   addToJson(oControl, "active", lactive);
   addToJson(oControl, "play", play);
   addToJson(oControl, "speed", speed);
   addToJson(oControl, "forward", forward);
   addToJson(oControl, "current", current / activeControlFps);
   addToJson(oControl, "total", total / activeControlFps);

   pushOutMessage(oControl, "replaycontrol");
}

//***************************************************************************
// Update Custom Data
//***************************************************************************

void cUpdate::updateCustomData()
{
   std::map<std::string,FileVariable> serviceVariables;
   json_t* obj = json_object();

   tell(3, "Info: Got %d serviceVariableFiles", (int)serviceVariableFiles.size());

   for (auto it = serviceVariableFiles.begin(); it != serviceVariableFiles.end(); it++)
   {
      json_t* oFile = json_array();
      std::string file = "";
      serviceVariables = *it;

      for (auto it = serviceVariables.begin(); it != serviceVariables.end(); it++)
      {
         json_t* oVariable = json_object();
         FileVariable var = it->second;

         file = var.file.c_str();
         addToJson(oVariable, "name", var.name.c_str());
         addToJson(oVariable, "value", var.value.c_str());
         addToJson(oVariable, "color", var.color.c_str());
         addToJson(oVariable, "vcolor", var.vcolor.c_str());

         json_array_append_new(oFile, oVariable);
      }

      addToJson(obj, file.c_str(), oFile);
   }

   pushOutMessage(obj, "customdata");
}

//***************************************************************************
// Update Skin State
//***************************************************************************

void cUpdate::updateSkinState()
{
   json_t* obj = json_object();
   addToJson(obj, "attached", isSkinAttached());
   pushOutMessage(obj, "skinstate");
}

//***************************************************************************
// Update Mneu Meta
//***************************************************************************

void cUpdate::updateMenu()
{
   json_t* oMenu = json_object();

   addToJson(oMenu, "category", menuCategory);
   addToJson(oMenu, "title", menuTitle.c_str());
   addToJson(oMenu, "editable", isEditable(menuCategory));

   update->pushOutMessage(oMenu, "menu");
}
