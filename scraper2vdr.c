/**
 *  osd2web plugin for the Video Disk Recorder
 *
 *  scraper2vdr.c
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

#include "lib/common.h"
#include "scraper2vdr.h"

//***************************************************************************
// Get Scraper Plugin
//***************************************************************************

cPlugin* GetScraperPlugin()
{
   static cPlugin* pScraper = cPluginManager::GetPlugin("scraper2vdr");

   if (!pScraper)
      pScraper = cPluginManager::GetPlugin("tvscraper");

   return pScraper;
}

//***************************************************************************
// Get Media Path
//***************************************************************************

int getScraperMediaPath(const cEventCopy* event, const cRecording* recording,
                        std::string& bannerPath, std::string& posterPath)
{
   static cPlugin* pScraper = GetScraperPlugin();
   const cEvent* evt = 0;
   const cSchedule* s = 0;
   ScraperGetPosterBannerV2 call;

   bannerPath = "";
   posterPath = "";

   if (!pScraper)
   {
      tell(0, "Warning: Plugin scraper2vdr not found");
      return fail;
   }

#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
   LOCK_SCHEDULES_READ;
   const cSchedules* schedules = Schedules;
#else
   cSchedulesLock schedulesLock;
   const cSchedules* schedules = (cSchedules*)cSchedules::Schedules(schedulesLock);
#endif

   if (recording)
   {
      event = 0;
      call.recording = recording;
   }
   else if (event)
   {
      // lock und schedule (channel) holen

      if (!schedules ||
          !(s = (cSchedule*)schedules->GetSchedule(event->ChannelID())) ||
          !(evt = s->GetEvent(event->EventID())))
      {
         tell(0, "Error, can't get lock on schedules or cant find event, aborting!");
         return fail;
      }

      call.event = evt;
   }

   if (pScraper->Service("GetPosterBannerV2", &call))
   {
      if (call.type == tSeries && call.banner.path.size() > 0)
      {
         ScraperGetPoster callPoster;

         bannerPath = call.banner.path;

         callPoster.event = evt;                     // only one is set
         callPoster.recording = recording;           //  "    "   "  "

         if (pScraper->Service("GetPoster", &callPoster))
            posterPath = callPoster.poster.path;
      }
      else if (call.type == tMovie && call.poster.path.size() > 0 && call.poster.height > 0)
      {
         posterPath = call.poster.path;
      }
   }

   return bannerPath.size() || posterPath.size() ? success : fail;
}
