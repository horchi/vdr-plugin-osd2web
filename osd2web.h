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
#include "config.h"
#include "HISTORY.h"

class cPluginOsd2Web;

//***************************************************************************
// General
//***************************************************************************

static const char* DESCRIPTION = trNOOP("osd2web plugin");
static const char* MAINMENUENTRY = tr("osd2web");

//***************************************************************************
// Plugin Main Menu
//***************************************************************************

class cOsd2webMenu : public cOsdMenu
{
   public:

      enum EpgMenuState
      {
         emsDiaPath = os_User,
         emsDiaStop,
         emsDiaSetup
      };

      cOsd2webMenu(const char* title, cPluginOsd2Web* aPlugin);
      virtual ~cOsd2webMenu() {};

      virtual eOSState ProcessKey(eKeys key);

   protected:

      cPluginOsd2Web* plugin;
};

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
      const char* CommandLineHelp();
      bool ProcessArgs(int argc, char* argv[]);
      bool Service(const char* id, void* data);
      const char** SVDRPHelpPages();
      cString SVDRPCommand(const char* cmd, const char* Option, int& ReplyCode);

      bool Initialize();
      bool Start();
      void Stop();
      void Store();
      void Housekeeping() {}

      const char* MainMenuEntry()
         { return config.mainmenuVisible ? MAINMENUENTRY : 0; }
      cOsdObject* MainMenuAction()   { return new cOsd2webMenu(MAINMENUENTRY, this); }
      cMenuSetupPage* SetupMenu()    { return 0; }

      bool SetupParse(const char* Name, const char* Value);

      cUpdate* update;

   private:

      cOsd2WebSkin* skin;
};

//***************************************************************************
#endif  // __OSD2WEB__H__
