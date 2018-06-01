/**
 *  osd2web plugin for the Video Disk Recorder
 *
 *  menu.c
 *
 *  (c) 2018 Jörg Wendel
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 **/

#include <vdr/menu.h>
#include <vdr/remote.h>

#include "osd2web.h"
#include "update.h"

//***************************************************************************
// Path Item
//***************************************************************************

class cMenuPathItem : public cOsdItem
{
   public:

      cMenuPathItem(const char* aBase, const char* aPath, int aCount, int aSubCount);
      ~cMenuPathItem();

      int getSubFolderCount() { return subFolderCount; }
      int getImageCount()     { return imgCount; }
      const char* getPath()   { return path; }

   private:

      int subFolderCount;  // how menu sub folder ar directly in this folder
      int imgCount;        // how many images (recursiv) ar in this folder
      char* path;
};

cMenuPathItem::cMenuPathItem(const char* aBase, const char* aPath, int aCount, int aSubCount)
{
   imgCount = aCount;
   subFolderCount = aSubCount;
   asprintf(&path, "%s/%s", aBase, aPath);
   SetText(cString::sprintf("%s (%d)", aPath, imgCount));
}

cMenuPathItem::~cMenuPathItem()
{
   free(path);
}

//***************************************************************************
//***************************************************************************
// Path Select Menu
//***************************************************************************

class cMenuPathSelect : public cOsdMenu
{
   public:

      cMenuPathSelect(const char* aPath, cPluginOsd2Web* aPlugin);
      virtual ~cMenuPathSelect();

      virtual eOSState ProcessKey(eKeys Key);
      int refresh();

   private:

      cPluginOsd2Web* plugin;
      char* path;
};

cMenuPathSelect::cMenuPathSelect(const char* aPath, cPluginOsd2Web* aPlugin)
   : cOsdMenu("", 100)
{
   plugin = aPlugin;
   SetMenuCategory(mcMain);
   SetHelp(tr("Start Diashow"), 0, 0, 0);

   path = strdup(aPath);

   if (strlen(path) > strlen(config.diaPath))
      SetTitle(cString::sprintf(tr("Select Folder: '%s'"), path + strlen(config.diaPath) + 1));
   else
      SetTitle(tr("Select Folder:"));

   refresh();
}

cMenuPathSelect::~cMenuPathSelect()
{
   free(path);
}

eOSState cMenuPathSelect::ProcessKey(eKeys key)
{
   eOSState state = cOsdMenu::ProcessKey(key);
   cMenuPathItem* item = (cMenuPathItem*)Get(Current());

   if (state == osUnknown)
   {
      switch (key)
      {
         case kOk:
         {
            if (item && item->getSubFolderCount() && !isEmpty(item->getPath()))
               return AddSubMenu(new cMenuPathSelect(item->getPath(), plugin));

            Skins.Message(mtInfo, tr("No more sub folders below, use key [red] to start dia show from here"));
            return osContinue;
         }

         case kRed:
         {
            if (item && item->getImageCount() && !isEmpty(item->getPath()))
            {
               config.setDiaPathCurrent(item->getPath());
               cUpdate::updateDiaList = yes;
               plugin->Store();
               plugin->update->setView(cUpdate::vmDia);
               return osEnd;
            }

            Skins.Message(mtInfo, tr("No images below"));

            return osContinue;
         }

         default: break;
      }
   }

   return state;
}

//***************************************************************************
// Refresh
//***************************************************************************

int cMenuPathSelect::refresh()
{
   FileList directories;
   int count = 0;

   cSkinDisplay::Current()->SetMessage(mtInfo, tr("Collecting Data ..."));
   cSkinDisplay::Current()->Flush();
   cStatus::MsgOsdStatusMessage(tr("Collecting Data ..."));

   // Skins.Message(mtInfo, tr("Collecting Data ..."));

   if (getFileList(path, DT_DIR, 0, no, &directories, count) != success)
      return done;

   directories.sort(compareFileInfo);

   for (auto it = directories.begin(); it != directories.end(); it++)
   {
      FileInfo* fileInfo = &(*it);
      int subFolderCount = 0;
      count = 0;
      getFileList(cString::sprintf("%s/%s",path, fileInfo->name.c_str()), DT_REG, config.diaExtensions, yes, 0, count);
      getFileList(cString::sprintf("%s/%s",path, fileInfo->name.c_str()), DT_DIR, 0, no, 0, subFolderCount);
      Add(new cMenuPathItem(path, fileInfo->name.c_str(), count, subFolderCount));
   }

   cSkinDisplay::Current()->SetMessage(mtInfo, 0);
   cStatus::MsgOsdStatusMessage(0);

   Display();

   return done;
}

//***************************************************************************
// Menu Dia Setup
//***************************************************************************

class cMenuDiaSetup : public cMenuSetupPage
{
   public:

      cMenuDiaSetup(cPluginOsd2Web* aPlugin);
      virtual ~cMenuDiaSetup() { Store(); };

   protected:

      // virtual eOSState ProcessKey(eKeys Key);
      virtual void Store();

   private:

      cPluginOsd2Web* plugin;
      cOsd2WebConfig data;
};

cMenuDiaSetup::cMenuDiaSetup(cPluginOsd2Web* aPlugin)
   : cMenuSetupPage()
{
   plugin = aPlugin;
   data = config;
   SetMenuCategory(mcPluginSetup);

   Add(new cMenuEditIntItem(tr("Interval"), &data.diaCycleTime, 1, 999));
   Add(new cMenuEditBoolItem(tr("Random"), &data.diaRandom));

   Display();
}

void cMenuDiaSetup::Store()
{
   config = data;
   plugin->Store();
}

//***************************************************************************
//***************************************************************************
// Plugin Main Menu
//***************************************************************************

cOsd2webMenu::cOsd2webMenu(const char* title, cPluginOsd2Web* aPlugin)
   : cOsdMenu(title)
{
   plugin = aPlugin;
   SetMenuCategory(mcMain);
   SetHasHotkeys(yes);
   Clear();

   Add(new cOsdItem(hk(tr("Select Dia Path")), (eOSState)emsDiaPath));

   if (plugin->update->getView() == cUpdate::vmDia)
      Add(new cOsdItem(hk(tr("Stop Dia Show")), (eOSState)emsDiaStop));

   Add(new cOsdItem(hk(tr("Dia Settings")), (eOSState)emsDiaSetup));
   Add(new cOsdItem(hk(tr("Restart Local Browser")), (eOSState)emsRestartBrowser));

   SetHelp(0, 0, 0, 0);

   Display();
}

//***************************************************************************
// Process Key
//***************************************************************************

eOSState cOsd2webMenu::ProcessKey(eKeys key)
{
   eOSState state = cOsdMenu::ProcessKey(key);

   switch (state)
   {
      case emsRestartBrowser:
         if (cUpdate::stopBrowser() == success)
         {
            if (cUpdate::startBrowser(yes) == success)
               Skins.Message(mtInfo, tr("Browser (re)started"));
            else
               Skins.Message(mtInfo, tr("Browser (re)start failed"));
         }

         return osEnd;

      case emsDiaPath:
         return AddSubMenu(new cMenuPathSelect(config.diaPath, plugin));

      case emsDiaStop:
         if (plugin->update->getView() == cUpdate::vmDia)
            plugin->update->setView(cUpdate::vmNormal);
         return osEnd;

      case emsDiaSetup:
         return AddSubMenu(new cMenuDiaSetup(plugin));

      default:
         break;
   }

   return state;
}
