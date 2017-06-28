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
/*
  not needed - since 'timers' support all needed data

   tell(3, "Recording: Recording '%s', Name '%s', FileName '%s'",
        On ? "Start" : "Stop" , notNull(Name), FileName);

   // to be implemented finally ... add loookup of recording ..

   json_t* oRecording = json_object();

   addToJson(oRecording, "state", On ? "started" : "endet");
   addToJson(oRecording, "name", Name);
   addToJson(oRecording, "filename", FileName);

   cUpdate::pushMessage(oRecording, "recording");
*/
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
   if (activeControl)         // skip if replay is running
      return;

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

   if (channel)
   {
      tell(3, "update present/following for channel '%s'", channel->Name());

      json_t* obj = json_object();
      json_t* oStreamInfo = json_object();
      json_t* oChannel = json_object();
      json_t* oPresent = json_object();
      json_t* oFollowing = json_object();

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

         haveActualEpg = present != 0;
         getTimerMatch(timers, present, &timerMatch);
         event2Json(oPresent, present, 0, timerMatch, no, cOsdService::osLarge);
         getTimerMatch(timers, following, &timerMatch);
         event2Json(oFollowing, following, 0, timerMatch, no, cOsdService::osLarge);

         // we need a trigger on start of following event

         nextPresentUpdateAt = following ? following->StartTime() : time(0) + 10;
      }
      else
      {
         nextPresentUpdateAt = time(0) + 60;
         tell(0, "Info: Can't get schedules");
      }

      json_object_set_new(obj, "present", oPresent);
      json_object_set_new(obj, "following", oFollowing);

      cUpdate::pushMessage(obj, "actual");
   }
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

         cUpdate::pushMessage(oTimers, "timers");
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
              timer->Recording() ? "timer is regording" : "timer is pending");

         timer2Json(oTimer, timer);
         json_array_append_new(oTimers, oTimer);
      }

#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
      stateKey.Remove();
#endif
      cUpdate::pushMessage(oTimers, "timers");
   }

   triggerTimerUpdate = no;
}

//***************************************************************************
// Update Recordings
//***************************************************************************

void cUpdate::updateRecordings()
{
   json_t* oRecordings = json_array();
   int count = 0;

   triggerRecordingsUpdate = no;

   GET_TIMERS_READ(timers);
   GET_RECORDINGS_READ(recordings);

   for (const cRecording* recording = recordings->First(); recording; recording = recordings->Next(recording ))
   {
      // olny recordings of the last 8 days

      if (recording->Start() > time(0)-8*tmeSecondsPerDay)
      {
         json_t* oRecording = json_object();
         recording2Json(oRecording, timers, recording, cOsdService::ObjectShape::osSmall);
         json_array_append_new(oRecordings, oRecording);
         count++;
      }
   }

   cUpdate::pushMessage(oRecordings, "recordings");
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
      cUpdate::pushMessage(oRecording, "replay");
      updateControl(force);
      return;
   }

   GET_TIMERS_READ(timers);
   GET_RECORDINGS_READ(recordings);

   const cRecording* recording = recordings ? recordings->GetByName(activeReplayFile.c_str()) : 0;

   addToJson(oRecording, "active", yes);

   if (recording)
   {
      activeControlFps = recording->Info() ? recording->Info()->FramesPerSecond() : 1;
      recording2Json(oRecording, timers, recording);
   }
   else
   {
      addToJson(oRecording, "name", activeReplayName.c_str());
      addToJson(oRecording, "filename", activeReplayFile.c_str());
   }

   cUpdate::pushMessage(oRecording, "replay");
   updateControl(force);
}

//***************************************************************************
// Update Control
//***************************************************************************

void cUpdate::updateControl(int force)
{
   static time_t lastCheckAt = na;
   static int ltotal = 0, lspeed = 0;
   static bool lplay = false, lforward = false;
   static bool lactive = no;

   json_t* oControl = json_object();

   if (!activeControl)
   {
      if (force || lactive)
      {
         lactive = no;
         addToJson(oControl, "active", lactive);
         cUpdate::pushMessage(oControl, "replaycontrol");
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

   if (!force && lactive && total == ltotal && speed == lspeed && play == lplay && forward == lforward)
      return ;

   lspeed = speed; lplay = play; lforward = forward;
   ltotal = total;
   lactive = yes;

   addToJson(oControl, "active", lactive);
   addToJson(oControl, "play", play);
   addToJson(oControl, "speed", speed);
   addToJson(oControl, "forward", forward);
   addToJson(oControl, "current", current / activeControlFps);
   addToJson(oControl, "total", total / activeControlFps);

   cUpdate::pushMessage(oControl, "replaycontrol");
}

//***************************************************************************
// Update Custom Data
//***************************************************************************

void cUpdate::updateCustomData()
{
   std::map<std::string,FileVariable> serviceVariables;
   json_t* obj = json_array();

   tell(0, "got %d serviceVariableFiles", (int)serviceVariableFiles.size());

   for (auto it = serviceVariableFiles.begin(); it != serviceVariableFiles.end(); it++)
   {
      serviceVariables = *it;

      for (auto it = serviceVariables.begin(); it != serviceVariables.end(); it++)
      {
         json_t* oVariable = json_object();
         FileVariable var = it->second;

         addToJson(oVariable, "file", var.file.c_str());
         addToJson(oVariable, "name", var.name.c_str());
         addToJson(oVariable, "value", var.value.c_str());

         json_array_append_new(obj, oVariable);
      }
   }

   cUpdate::pushMessage(obj, "customdata");
}

//***************************************************************************
// Update Skin State
//***************************************************************************

void cUpdate::updateSkinState()
{
   json_t* obj = json_object();
   addToJson(obj, "attached", isSkinAttached());
   cUpdate::pushMessage(obj, "skinstate");
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

   cUpdate::pushMessage(oMenu, "menu");
}
