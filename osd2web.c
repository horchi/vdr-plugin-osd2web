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
}

cPluginOsd2Web::~cPluginOsd2Web()
{
   delete update;
}

//***************************************************************************
// Process Args
//***************************************************************************

const char* cPluginOsd2Web::CommandLineHelp()
{
   return
      "   -s <logosuffix>, --logosuffix=<logosuffix> channel logo suffix\n"
      "                                              (default: png)\n"
      "   -p <port>,       --port=<port> web port\n"
      "                                              (default: 4444)\n"
      "   -l,              --logonotlower            do not search logo names in lower case\n"
      "   -i,              --logobyid                logo names by channel id instead of name\n"
      "   -e,              --epgimgpath              path to epg images\n"
      "                                              (default: /var/cache/vdr/epgimages)\n"
      ;
}

bool cPluginOsd2Web::ProcessArgs(int argc, char* argv[])
{
   int c;

   static option long_options[] =
   {
      { "port",           required_argument, 0, 'p' },
      { "logosuffix",     required_argument, 0, 's' },
      { "logonotlower",         no_argument, 0, 'l' },
      { "logobyid",             no_argument, 0, 'i' },
      { "epgimgpath",     required_argument, 0, 'e' },
      { 0, 0, 0, 0 }
   };

   // check the arguments

   while ((c = getopt_long(argc, argv, "p:s:l:i:e", long_options, 0)) != -1)
   {
      switch (c)
      {
         case 'p': config.webPort = atoi(optarg);  break;
         case 's': config.setLogoSuffix(optarg);   break;
         case 'l': config.logoNotLower = yes;      break;
         case 'i': config.logoById = yes;          break;
         case 'e': config.setEpgImagePath(optarg); break;

         default:  tell(0, "Ignoring unknown argument '%c' '%s'", c, optarg);
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

      req->webPort = config.webPort;

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
   update = new cUpdate();
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
