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
   strcpy(epgImagePath, "/var/cache/vdr/epgimages");
}
