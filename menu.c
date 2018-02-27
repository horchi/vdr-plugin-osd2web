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

      cMenuPathItem(const char* aBase, const char* aPath, int count);
      ~cMenuPathItem();

      const char* getPath()  { return path; }

   private:

      char* path;
};

cMenuPathItem::cMenuPathItem(const char* aBase, const char* aPath, int count)
{
   asprintf(&path, "%s/%s", aBase, aPath);
   SetText(cString::sprintf("%s (%d)", aPath, count));
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
   : cOsdMenu("", 2)
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
            if (item && !isEmpty(item->getPath()))
               return AddSubMenu(new cMenuPathSelect(item->getPath(), plugin));

            return osContinue;
         }

         case kRed:
         {
            if (item && !isEmpty(item->getPath()))
            {
               config.setDiaPathCurrent(item->getPath());
               cUpdate::updateDiaList = yes;
               plugin->Store();
               plugin->update->setView(cUpdate::vmDia);
               return osEnd;
            }

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
      count = 0;
      getFileList(cString::sprintf("%s/%s",path, fileInfo->name.c_str()), DT_REG, config.diaExtensions, yes, 0, count);
      Add(new cMenuPathItem(path, fileInfo->name.c_str(), count));
   }

   cSkinDisplay::Current()->SetMessage(mtInfo, 0);
   cStatus::MsgOsdStatusMessage(0);

   Display();

   return done;
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

   cOsdMenu::Add(new cOsdItem(hk(tr("Select Dia Path")), (eOSState)emsDiaPath));

   if (plugin->update->getView() == cUpdate::vmDia)
      cOsdMenu::Add(new cOsdItem(hk(tr("Stop Dia Show")), (eOSState)emsDiaStop));

   cOsdMenu::Add(new cOsdItem(hk(tr("Dia Time")), (eOSState)emsDiaTime));

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
      case emsDiaPath:
         return AddSubMenu(new cMenuPathSelect(config.diaPath, plugin));

      case emsDiaStop:
         if (plugin->update->getView() == cUpdate::vmDia)
            plugin->update->setView(cUpdate::vmNormal);
         return osEnd;

      case emsDiaTime:
         return osContinue;

      default:
         break;
   }

   return state;
}
