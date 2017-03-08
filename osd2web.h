/*
 *  osd2web plugin for the Video Disk Recorder
 *
 * osd2web.h
 *
 * (c) 2017 Jörg Wendel
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 */

#ifndef __OSD2WEB__H__
#define __OSD2WEB__H__

//***************************************************************************
// Includes
//***************************************************************************

#include <vdr/plugin.h>
#include <vdr/config.h>

#include "lib/common.h"
#include "lib/config.h"

#include "update.h"
#include "HISTORY.h"

class cPluginOsd2Web;

//***************************************************************************
// General
//***************************************************************************

static const char* DESCRIPTION = trNOOP("osd2web plugin");

//***************************************************************************
// Config
//***************************************************************************

struct cOsd2WebConfig : public cConfigBase
{
   public:

      cOsd2WebConfig();
};

extern cOsd2WebConfig config;

//***************************************************************************
// Class cPluginOsd2Web
//***************************************************************************

class cPluginOsd2Web : public cPlugin
{
   public:

      cPluginOsd2Web();
      ~cPluginOsd2Web();

      const char* Version()                     { return VERSION; }
      const char* Description()                 { return tr(DESCRIPTION); }
      const char* CommandLineHelp()             { return ""; }
      bool ProcessArgs(int argc, char* argv[]);
      bool Service(const char* id, void* data);

      bool Initialize();
      bool Start();
      void Store();
      void Housekeeping() {}

      const char* MainMenuEntry()    { return 0; }
      cOsdObject* MainMenuAction()   { return 0; }
      cMenuSetupPage* SetupMenu()    { return 0; }

      bool SetupParse(const char* Name, const char* Value);

   private:

      cOsd2WebSkin* skin;
      cUpdate* update;
      int webPort;
};

//***************************************************************************
#endif  // __OSD2WEB__H__
