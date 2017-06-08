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

#include "update.h"

#define FULLMATCH 1000

//***************************************************************************
// Matches
//***************************************************************************

eTimerMatch Matches(const cTimer* ti, const cEvent* event)
{
   if (ti->HasFlags(tfActive) && ti->Channel()->GetChannelID() == event->ChannelID())
   {
      int overlap = 0;
      bool useVps = ti->HasFlags(tfVps) && event->Vps();

      if (useVps)
         overlap = (ti->StartTime() == event->Vps()) ? FULLMATCH + (event->IsRunning() ? 200 : 100) : 0;

      if (!overlap)
      {
         if (ti->StartTime() <= event->StartTime() && event->EndTime() <= ti->StopTime())
            overlap = FULLMATCH;
         else if (ti->StopTime() <= event->StartTime() || event->EndTime() <= ti->StartTime())
            overlap = 0;
         else
            overlap = (std::min(ti->StopTime(), event->EndTime()) - std::max(ti->StartTime(), event->StartTime())) * FULLMATCH / std::max(event->Duration(), 1);
      }

      if (useVps)
         return overlap > FULLMATCH ? tmFull : tmNone;

      return overlap >= FULLMATCH ? tmFull : overlap > 0 ? tmPartial : tmNone;
   }

   return tmNone;
}

//***************************************************************************
// Get Timer Match
//***************************************************************************

const cTimer* getTimerMatch(const cTimers* timers, const cEvent* event, eTimerMatch* match)
{
   const cTimer* t = 0;
   eTimerMatch m = tmNone;

   if (match)
      *match = m;

   if (!timers)
      return 0;

   for (const cTimer* ti = timers->First(); ti; ti = timers->Next(ti))
   {
      eTimerMatch tm = Matches(ti, event);

      if (tm > m)
      {
         t = ti; m = tm;

         if (m == tmFull)
            break;
      }
   }

   if (match)
      *match = m;

   return t;
}
