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

#include "update.h"

//***************************************************************************
// Osd Channel Switch
//***************************************************************************

void cUpdate::ChannelSwitch(const cDevice* device, int channelNumber, bool liveView)
{
   if (liveView && channelNumber)
   {
      tell(0, "ChannelSwitch: channelNumber: %d", channelNumber);
      currentChannelNr = channelNumber;
      updatePresentFollowing();
   }
}

//***************************************************************************
// Osd Channel
//***************************************************************************

void cUpdate::OsdChannel(const char* text)
{
}

//***************************************************************************
// Osd Set Recording
//***************************************************************************

void cUpdate::OsdSetRecording(const cRecording* recording)
{
}

//***************************************************************************
// Osd Programme
//  - only called when 'ok' is pressed to view the info on OSD
//***************************************************************************

void cUpdate::OsdProgramme(time_t PresentTime, const char* PresentTitle,
                           const char* PresentSubtitle, time_t FollowingTime,
                           const char* FollowingTitle, const char* FollowingSubtitle)
{
   tell(5, "OsdProgramme: ");
}

//***************************************************************************
// Recording
//***************************************************************************

void cUpdate::Recording(const cDevice* Device, const char* Name,
                        const char* FileName, bool On)
{
}

//***************************************************************************
// Timer Change
//***************************************************************************

void cUpdate::TimerChange(const cTimer *Timer, eTimerChange Change)
{
}

//***************************************************************************
// Replaying
//***************************************************************************

void cUpdate::Replaying(const cControl* Control, const char* Name,
                        const char* FileName, bool On)
{
}

//***************************************************************************
// Update Present / Following
//***************************************************************************

void cUpdate::updatePresentFollowing()
{
   if (!currentChannelNr)
      return;

#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
   LOCK_CHANNELS_READ;
   const cChannel* channel = Channels->GetByNumber(currentChannelNr);
#else
   const cChannel* channel = Channels.GetByNumber(currentChannelNr);
#endif

   if (channel)
   {
      tell(0, "update present/following for channel '%s'", channel->Name());

#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
      LOCK_SCHEDULES_READ;
      const cSchedules* schedules = Schedules;
#else
      cSchedulesLock schedulesLock;
      const cSchedules* schedules = (cSchedules*)cSchedules::Schedules(schedulesLock);
#endif

      const cSchedule* schedule = schedules ? schedules->GetSchedule(channel->GetChannelID()) : 0;

      if (schedule)
      {
         json_t* obj = json_object();
         json_t* oPresent = json_object();
         json_t* oFollowing = json_object();

         json_t* oStreamInfo = json_object();
         json_t* oChannel = json_object();
         const cEvent* present = schedule->GetPresentEvent();
         const cEvent* following = schedule->GetFollowingEvent();

         channel2Json(oChannel, channel);
         stream2Json(oStreamInfo, channel);
         event2Json(oPresent, present);
         event2Json(oFollowing, following);

         json_object_set_new(obj, "channel", oChannel);
         json_object_set_new(obj, "streaminfo", oStreamInfo);
         json_object_set_new(obj, "present", oPresent);
         json_object_set_new(obj, "following", oFollowing);

         cUpdate::pushMessage(obj, "actual");

         // we need a trigger on start of following event

         nextPresentUpdateAt = following->StartTime();
      }
      else
         tell(0, "Info: Can't get schedules");
   }
}

//***************************************************************************
// Event To Json
//***************************************************************************

int cUpdate::event2Json(json_t* obj, const cEvent* event, const cChannel* channel)
{
   if (channel && event)
   {
      json_t* oChannel = json_object();
      channel2Json(oChannel, channel);
      json_object_set_new(obj, "channel", oChannel);
   }

   if (!event)
   {
      addToJson(obj, "eventid", na);
      return done;
   }

   addToJson(obj, "eventid", event->EventID());
   addToJson(obj, "channelid", event->ChannelID().ToString());
   addToJson(obj, "title", event->Title());
   addToJson(obj, "shorttext", event->ShortText());
   addToJson(obj, "description", event->Description());
   addToJson(obj, "starttime", event->StartTime());
   addToJson(obj, "endtime", event->EndTime());
   addToJson(obj, "duration", event->Duration());
   addToJson(obj, "runningstatus", event->RunningStatus());
   addToJson(obj, "isrunning", event->IsRunning());
   addToJson(obj, "parentalrating", event->ParentalRating());
   addToJson(obj, "vps", event->Vps());
   addToJson(obj, "hastimer", event->HasTimer());
   addToJson(obj, "seen", event->Seen());

   // #TODO add components
   // const cComponents *Components(void) const { return components; }

#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
   // #TODO  - add timeout for timers lock
   cTimersLock timersLock(false);
   const cTimers* timers = timersLock.Timers();
#else
   cTimers* timers = &Timers;
#endif

   eTimerMatch timerMatch = tmNone;
   const cTimer* timer = getTimerMatch(timers, event, &timerMatch);

   if (timerMatch == tmFull)
      addToJson(obj, "timermatch", "full");
   else if (timerMatch == tmPartial && timer->StartTime() >= event->StartTime())
      addToJson(obj, "timermatch", "partialbefore");
   else if (timerMatch == tmPartial && timer->StartTime() < event->StartTime())
      addToJson(obj, "timermatch", "partialafter");

   return done;
}

//***************************************************************************
// Channel To Json
//***************************************************************************

int cUpdate::channel2Json(json_t* obj, const cChannel* channel)
{
   // { "channelid": "S19.2E-1-1019-10301", "channelname": "Das Erste HD", "channelnumber": 1 }

   addToJson(obj, "channelid", channel->GetChannelID().ToString());
   addToJson(obj, "channelname", channel->Name());
   addToJson(obj, "channelnumber", channel->Number());

   return done;
}

//***************************************************************************
// Info of current Stream To Json
//***************************************************************************

int cUpdate::stream2Json(json_t* obj, const cChannel* channel)
{
   addToJson(obj, "dolbidigital", (bool)channel->Dpid(0));
   addToJson(obj, "multilang", (bool)channel->Apid(1));
   addToJson(obj, "vtx", (bool)channel->Tpid());
   addToJson(obj, "encrypted", (bool)channel->Ca());
   addToJson(obj, "radio", channel->Vpid() == 0 || channel->Vpid() == 1 || channel->Vpid() == 0x1fff);

   if (cDevice::PrimaryDevice())
   {
      int width = 0, height = 0; double aspect;

      // only set with active output device, otherwise 0

      cDevice::PrimaryDevice()->GetVideoSize(width, height, aspect);
      addToJson(obj, "videoheight", height);
      addToJson(obj, "videowidth", width);
   }

   return done;
}

//***************************************************************************
// Channels To Json
//***************************************************************************

int cUpdate::channels2Json(json_t* obj)
{
#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
   LOCK_CHANNELS_READ;
   const cChannels* channels = Channels;
#else
   const cChannels* channels = &Channels;
#endif

   if (!channels)
      return fail;

   for (const cChannel* channel = channels->First(); channel; channel = channels->Next(channel))
   {
      json_t* oChannel = json_object();
      channel2Json(oChannel, channel);
      json_array_append_new(obj, oChannel);
   }

   return success;
}

//***************************************************************************
// Recording To Json
//***************************************************************************

int cUpdate::recording2Json(json_t* obj, const cRecording* recording)
{
   if (!recording)
   {
      addToJson(obj, "name", "<null>");
      return done;
   }

   addToJson(obj, "name", recording->Name());
   addToJson(obj, "folder", recording->Folder());
   addToJson(obj, "basename", recording->BaseName());
   addToJson(obj, "filename", recording->FileName());
   addToJson(obj, "title", recording->Title());
   addToJson(obj, "lengthinseconds", recording->LengthInSeconds());
   addToJson(obj, "filesizemb", recording->FileSizeMB());
   addToJson(obj, "isnew", recording->IsNew());
   addToJson(obj, "isedited", recording->IsEdited());
   addToJson(obj, "hasmarks", ((cRecording*)recording)->HasMarks()); // cast due to vdr 2.2.0 ' const' problem

   return success;
}
