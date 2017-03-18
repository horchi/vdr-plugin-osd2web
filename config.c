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
   epgImagePath = 0;
   logoNotLower = no;
   logoById = no;

   confPath = 0;
   logoSuffix = strdup("png");
   epgImagePath = strdup("/var/cache/vdr/epgimages");
}

cOsd2WebConfig::~cOsd2WebConfig()
{
   free(confPath);
   free(epgImagePath);
   free(logoSuffix);
}
