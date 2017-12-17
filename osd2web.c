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
#include "config.h"

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
      "   -l <path>,       --logopath <path>         path to channel logos\n"
      "                                              (default: <plg-conf-dir>/channellogos)\n"
      "   -L,              --logonotlower            do not search logo names in lower case\n"
      "   -i,              --logobyid                logo names by channel id instead of name\n"
      "   -e,              --epgimgpath              path to epg images\n"
      "   -b,              --browser                 start browser (via startBrowser script)\n"
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
      { "epgimgpath",     required_argument, 0, 'e' },
      { "logopath",       required_argument, 0, 'l' },
      { "logonotlower",         no_argument, 0, 'L' },
      { "logobyid",             no_argument, 0, 'i' },
      { "browser",              no_argument, 0, 'b' },
      { 0, 0, 0, 0 }
   };

   // check the arguments

   while ((c = getopt_long(argc, argv, "p:s:e:l:Lib", long_options, 0)) != -1)
   {
      switch (c)
      {
         case 'p': config.webPort = atoi(optarg);  break;
         case 's': config.setLogoSuffix(optarg);   break;
         case 'e': config.setEpgImagePath(optarg); break;
         case 'l': config.setLogoPath(optarg);     break;
         case 'L': config.logoNotLower = yes;      break;
         case 'i': config.logoById = yes;          break;
         case 'b': config.startBrowser = yes;      break;

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
   else if (strcmp(id, EPG2VDR_TIMER_UPDATED) == 0)
   {
      update->triggerTimerUpdate = yes;
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
// SVDRP Help Pages
//***************************************************************************

const char** cPluginOsd2Web::SVDRPHelpPages()
{
   static const char* HelpPages[] =
   {
      "ATTACH\n"
      "    Attach osd2web to the skin interface",
      "DETACH\n"
      "    Detach osd2web to the skin interface (if it isn't the default skin)",
      "UPDREC\n"
      "    Update Recording List",
      "PREV \n"
      "    Set the normal view to the previous section\n",
      "NEXT \n"
      "    Set the normal view to the next section\n",
      "BRSTART \n"
      "    Start the local browser\n",
      "BRSTOP \n"
      "    Stop the local browser\n",
      0
   };

   return HelpPages;
}

//***************************************************************************
// SVDRP Command
//***************************************************************************

cString cPluginOsd2Web::SVDRPCommand(const char* cmd, const char* Option, int& ReplyCode)
{
   const char* result = 0;

   // ------------------------------------
   // Attach to skin interface

   if (strcasecmp(cmd, "ATTACH") == 0)
   {
      if (update->isSkinAttached())
         result = "already attached";
      else
         result = "attached";

      // call always, independent of check states above!!

      update->setSkinAttachState(yes, yes);
   }

   // Detach from skin interface

   else if (strcasecmp(cmd, "DETACH") == 0)
   {
      if (update->isDefaultSkin())
         result = "nothing to do, i'am the default skin ;)";
      else if (!update->isSkinAttached())
         result = "already detached";
      else
         result = "detached";

      // call always, independent of check states above!!

      update->setSkinAttachState(no, yes);
   }

   else if (strcasecmp(cmd, "UPDREC") == 0)
   {
      update->triggerRecordingsUpdate = yes;
      result = "recording update triggered";
   }
   else if (strcasecmp(cmd, "PREV") == 0)
   {
      update->toggleView(no);
      result = "switched view";
   }
   else if (strcasecmp(cmd, "NEXT") == 0)
   {
      update->toggleView(yes);
      result = "switched view";
   }
   else if (strcasecmp(cmd, "BRSTART") == 0)
   {
      if (cUpdate::startBrowser(yes) == success)
         result = "browser started";
      else
         result = "stopping browser failed";
   }
   else if (strcasecmp(cmd, "BRSTOP") == 0)
   {
      if (cUpdate::stopBrowser() == success)
         result = "browser stopped";
      else
         result = "stopping browser failed";
   }

   return result;
}

//***************************************************************************

VDRPLUGINCREATOR(cPluginOsd2Web)
