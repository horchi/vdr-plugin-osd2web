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
// Osd Status Message
//***************************************************************************

void cUpdate::OsdStatusMessage(const char* message)
{
   tell(3, "OsdStatusMessage: %s", message);

   json_t* obj = json_object();
   addToJson(obj, "message", message);

   pushMessage(obj, "statusmessage");
}

//***************************************************************************
// Osd Help Keys
//***************************************************************************

void cUpdate::OsdHelpKeys(const char* red, const char* green,
                          const char* yellow, const char* blue)
{
   tell(3, "OsdHelpKeys: '%s' '%s' '%s' '%s'",
        notNull(red), notNull(green), notNull(yellow), notNull(blue));

   return ;

   json_t* buttons = json_object();

   if (!isEmpty(red))    addToJson(buttons, "red", red);
   if (!isEmpty(green))  addToJson(buttons, "green", green);
   if (!isEmpty(yellow)) addToJson(buttons, "yellow", yellow);
   if (!isEmpty(blue))   addToJson(buttons, "blue", blue);

   pushMessage(buttons, "colorbuttons");
}

//***************************************************************************
// Osd Title
//***************************************************************************

void cUpdate::OsdTitle(const char* title)
{
   tell(3, "OsdTitle: %s", title);

   osdTitle = title;
}

//***************************************************************************
// Osd Text Item
//***************************************************************************

void cUpdate::OsdTextItem(const char* text, bool scroll)
{
   tell(3, "OsdTextItem: '%s' scroll: %d", text, scroll);

   json_t* menuText = json_object();

   addToJson(menuText, "category", menuCategory);
   addToJson(menuText, "text", text ? text : "");

   pushMessage(menuText, "menutext");
}

//***************************************************************************
// Osd Menu Display
//***************************************************************************

void cUpdate::OsdMenuDisplay(eMenuCategory category)
{
   tell(3, "OsdMenuDisplay: category: %d", category);
   menuCategory = category;
}

//***************************************************************************
// Osd Menu Destroy
//***************************************************************************

void cUpdate::OsdMenuDestroy()
{
   tell(3, "OsdMenuDestroy:");

   json_t* obj = json_object();
   addToJson(obj, "close", yes);
   pushMessage(obj, "closemenu");

   menuCategory = mcUndefined;
   osdTitle = "";

   while (!menuItems.empty())
      menuItems.pop();
}

//***************************************************************************
// Osd Item
//***************************************************************************

void cUpdate::OsdItem(const char* text, int index)
{
   tell(2, "OsdItem: text: %s index: %d", text, index);
   return ;
   menuItems.push(text);
}

//***************************************************************************
// Osd Current Item at Index
//***************************************************************************

void cUpdate::OsdCurrentItemIndex(const char* text, int index)
{
   tell(3, "OsdCurrentItemIndex: text %s, index: %d", text, index);

   return ;

   if (index == -1)
      return ;    // '#TODO ?? used by cMenuEditItem::SetValue()

   json_t* oCurrent = json_object();

   addToJson(oCurrent, "category", menuCategory);
   addToJson(oCurrent, "index", index);
   addToJson(oCurrent, "row", text);

   // #TODO update current at the the 'menuItems' list object

   pushMessage(oCurrent, "textmenucurrent");
}

//***************************************************************************
// Osd Event Item
//  - called after OsdItems are finished and after OsdMenuReady
//***************************************************************************

void cUpdate::OsdEventItem(const cEvent* event, const char* text,
                           int index, int count)
{
   tell(3, "OsdEventItem: '%s' at index (%d), count (%d)",
        event ? event->Title() : "<null>", index, count);

   return ;

   if (index == 0)
   {
      while (!menuEventItems.empty())
      {
         json_decref(menuEventItems.front());
         menuEventItems.pop();
      }
   }

   json_t* oEvent = json_object();

   event2Json(oEvent, event, 0);
   menuEventItems.push(oEvent);

   if (menuEventItems.size() == (unsigned int)count)
      eventMenuReady();
}

//***************************************************************************
// Event Menu Ready
//***************************************************************************

void cUpdate::eventMenuReady()
{
   return ;
   json_t* oMenu = json_object();
   json_t* oEvents = json_array();

   while (!menuEventItems.empty())
   {
      json_array_append_new(oEvents, menuEventItems.front());
      menuEventItems.pop();
   }

   addToJson(oMenu, "category", menuCategory);
   addToJson(oMenu, "title", osdTitle.c_str());
   json_object_set_new(oMenu, "events", oEvents);

   pushMessage(oMenu, "eventmenu");
}

//***************************************************************************
// Osd Menu Ready
//***************************************************************************

void cUpdate::OsdRecordingItem(const cRecording* recording, int index, int count)
{
   tell(3, "OsdRecordingItem: '%s' at index (%d), count (%d)",
        recording ? recording->Title() : "<null>", index, count);

   return ;

   if (index == 0)
   {
      while (!menuRecordingItems.empty())
      {
         json_decref(menuRecordingItems.front());
         menuRecordingItems.pop();
      }
   }

   json_t* oRecording = json_object();

   recording2Json(oRecording, recording);
   menuRecordingItems.push(oRecording);

   if (menuRecordingItems.size() == (unsigned int)count)
      recordingMenuReady();
}

//***************************************************************************
// Event Recording Ready
//***************************************************************************

void cUpdate::recordingMenuReady()
{
   json_t* oMenu = json_object();
   json_t* oRecordings = json_array();

   return ;

   while (!menuEventItems.empty())
   {
      json_array_append_new(oRecordings, menuRecordingItems.front());
      menuRecordingItems.pop();
   }

   addToJson(oMenu, "category", menuCategory);
   addToJson(oMenu, "title", osdTitle.c_str());
   json_object_set_new(oMenu, "recordings", oRecordings);

   pushMessage(oMenu, "recordingmenu");
}

//***************************************************************************
// Osd Menu Ready
//***************************************************************************

void cUpdate::OsdMenuReady(int count)
{
   tell(3, "OsdMenuReady: count: %d", count);

   return ;

   size_t colCount = 0;
   json_t* oMenu = json_object();
   json_t* oRows = json_array();

   while (!menuItems.empty())
   {
      colCount = std::max(colCount, (size_t)std::count(menuItems.front().begin(), menuItems.front().end(), '\t'));

      json_array_append_new(oRows, json_string(menuItems.front().c_str()));
      menuItems.pop();
   }

   addToJson(oMenu, "category", menuCategory);
   addToJson(oMenu, "title", osdTitle.c_str());
   addToJson(oMenu, "colcount", colCount);
   addToJson(oMenu, "rowcount", count);
   json_object_set_new(oMenu, "rows", oRows);

   pushMessage(oMenu, "textmenu");
}

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
// Osd Set Event
//***************************************************************************

void cUpdate::OsdSetEvent(const cEvent* event)
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
// Set Volume
//***************************************************************************

void cUpdate::SetVolume(int, bool)
{
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
// Osd Clear
//***************************************************************************

void cUpdate::OsdClear()
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

         pushMessage(obj, "actual");

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
