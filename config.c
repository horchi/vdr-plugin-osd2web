/*
 * config.c:
 *
 * See the README file for copyright information and how to reach the author.
 *
 */

#include "osd2web.h"

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
