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
// Recording
//***************************************************************************

void cUpdate::Recording(const cDevice* Device, const char* Name,
                        const char* FileName, bool On)
{
   // #TODO - to be implemented
}

//***************************************************************************
// Replaying
//***************************************************************************

void cUpdate::Replaying(const cControl* Control, const char* Name,
                        const char* FileName, bool On)
{
   // #TODO - to be implemented
}

//***************************************************************************
// Timer Change
//***************************************************************************

void cUpdate::TimerChange(const cTimer *Timer, eTimerChange Change)
{
   // needed ?!
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
