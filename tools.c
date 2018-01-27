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

#include <vdr/plugin.h>

#include "update.h"

#define FULLMATCH 1000

//***************************************************************************
// Matches
//***************************************************************************

eTimerMatch Matches(const cTimer* ti, const cEvent* event)
{
   if (event && ti->HasFlags(tfActive) && ti->Channel()->GetChannelID() == event->ChannelID())
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
// Get Match (by epg2vdr)
//***************************************************************************

int getMatch(cPlugin* pEpg2Vdr, const cEvent* event, eTimerMatch* match)
{
   cHas_Timer_V1 data;

   tell(3, "DEBUG: check timer match for '%s' (%d)", event ? event->Title() : "<unknown>", event->EventID());

   *match = tmNone;
   data.eventid = event->EventID();

   if (pEpg2Vdr->Service(EPG2VDR_HAS_TIMER, &data))
   {
      if (data.hastimer)
      {
         tell(3, "DEBUG: Timer match for '%s' found", event ? event->Title() : "<unknown>" );
         *match = tmFull;
      }
   }

   return done;
}

//***************************************************************************
// Get Timer Match
//***************************************************************************

const cTimer* getTimerMatch(const cTimers* timers, const cEvent* event, eTimerMatch* match)
{
   cPlugin* pEpg2Vdr = cPluginManager::GetPlugin("epg2vdr");

   // with epg2vdr ...

   if (pEpg2Vdr)
   {
      getMatch(pEpg2Vdr, event, match);
      return 0;
   }

   // without ...

   const cTimer* t = 0;
   eTimerMatch m = tmNone;

   if (match)
      *match = m;

   if (!timers || !event)
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
