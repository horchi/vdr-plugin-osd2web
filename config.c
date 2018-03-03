/**
 *  osd2web plugin for the Video Disk Recorder
 *
 *  config.c
 *
 *  (c) 2017 Jörg Wendel
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 **/

#include "config.h"

cOsd2WebConfig config;

//***************************************************************************
// cConfig
//***************************************************************************

cOsd2WebConfig::cOsd2WebConfig()
{
   webPort = 4444;
   clientOsdTimeout = 120;
   epgImagePath = 0;
   logoNotLower = no;
   logoById = no;

   httpPath = 0;
   confPath = 0;
   logoPath = 0;
   diaPath = 0;
   diaPathCurrent = 0;
   scraper2VdrPath = 0;
   logoSuffix = strdup("png");
   epgImagePath = strdup("/var/cache/vdr/epgimages");
   startBrowser = no;
   browserDisplay = strdup("");
   tvIp = 0;
   mainmenuVisible = yes;

   diaCycleTime = 20;           // seconds
   diaExtensions = strdup("jpeg:jpg");
   diaRandom = yes;
}

cOsd2WebConfig::~cOsd2WebConfig()
{
   free(confPath);
   free(httpPath);
   free(epgImagePath);
   free(logoSuffix);
   free(logoPath);
   free(diaPath);
   free(diaPathCurrent);
   free(scraper2VdrPath);
   free(browserDisplay);
   free(tvIp);
   free(diaExtensions);
}

void cOsd2WebConfig::setBrowser(const char* p, int autoStart)
{
   free(browserDisplay);
   browserDisplay = strdup(p);

   if (autoStart != na)
      startBrowser = autoStart;
}
