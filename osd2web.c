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
#include "service.h"

const char* logPrefix = LOG_PREFIX;

//***************************************************************************
// cPluginOsd2Web
//***************************************************************************

cPluginOsd2Web::cPluginOsd2Web()
{
   update = 0;
   webPort = 4444;
}

cPluginOsd2Web::~cPluginOsd2Web()
{
   delete update;
}

//***************************************************************************
// Process Args
//***************************************************************************

bool cPluginOsd2Web::ProcessArgs(int argc, char* argv[])
{
   int c;

   static option long_options[] =
   {
      { "port",     required_argument, 0, 'p' },
      { 0, 0, 0, 0 }
   };

   // check the arguments

   while ((c = getopt_long(argc, argv, "p", long_options, 0)) != -1)
   {
      switch (c)
      {
         case 'p': webPort = atoi(optarg);     break;
         default:  tell(0, "Ignoring unknown argument '%s'", optarg);
      }
   }

   return true;
}

//***************************************************************************
// Initialize
//***************************************************************************

bool cPluginOsd2Web::Initialize()
{
   return true;
}

//***************************************************************************
// Service
//***************************************************************************

bool cPluginOsd2Web::Service(const char* id, void* data)
{
   if (!data)
      return fail;

   if (strcmp(id, OSD2WEB_PORT_SERVICE) == 0)
   {
      Osd2Web_Port_v1_0* req = (Osd2Web_Port_v1_0*)data;

      req->webPort = webPort;

      return true;
   }

   return false;
}

//***************************************************************************
// Start
//***************************************************************************

bool cPluginOsd2Web::Start()
{
   // init

   skin = new cOsd2WebSkin();
   update = new cUpdate(webPort);
   update->Start();                 // start plugin thread

   return true;
}

//***************************************************************************
// Store
//***************************************************************************

void cPluginOsd2Web::Store()
{
   SetupStore("LogLevel", config.loglevel);
}

//***************************************************************************
// Setup Parse
//***************************************************************************

bool cPluginOsd2Web::SetupParse(const char* name, const char* value)
{
   if      (!strcasecmp(name, "LogLevel"))     config.loglevel = atoi(value);

   else
      return false;

   return true;
}

//***************************************************************************

VDRPLUGINCREATOR(cPluginOsd2Web)
