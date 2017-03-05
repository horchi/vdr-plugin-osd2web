/**
 *  osd2web plugin for the Video Disk Recorder
 *
 *  osd2web.c
 *
 *  (c) 2017 Jörg Wendel
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 **/

#include <vdr/config.h>
#include <getopt.h>

#include "lib/common.h"

#include "osd2web.h"

const char* logPrefix = LOG_PREFIX;

//***************************************************************************
// cPluginOsd2Web
//***************************************************************************

cPluginOsd2Web::cPluginOsd2Web()
{
   update = 0;
}

cPluginOsd2Web::~cPluginOsd2Web()
{
   delete update;
}

//***************************************************************************
// Initialize
//***************************************************************************

bool cPluginOsd2Web::Initialize()
{
   return true;
}

//***************************************************************************
// Start
//***************************************************************************

bool cPluginOsd2Web::Start()
{
   // init

   skin = new cOsd2WebSkin();
   update = new cUpdate();
   update->Start();                 // start plugin thread

   return true;
}

//***************************************************************************
// Store
//***************************************************************************

void cPluginOsd2Web::Store()
{
   SetupStore("LogLevel", config.logLevel);
}

//***************************************************************************
// Setup Parse
//***************************************************************************

bool cPluginOsd2Web::SetupParse(const char* name, const char* value)
{
   if      (!strcasecmp(name, "LogLevel"))     config.logLevel = atoi(value);

   else
      return false;

   return true;
}

//***************************************************************************

VDRPLUGINCREATOR(cPluginOsd2Web)
