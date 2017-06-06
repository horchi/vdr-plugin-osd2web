/**
 *  osd2web plugin for the Video Disk Recorder
 *
 *  setup.c
 *
 *  (c) 2017 Jörg Wendel
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 **/

//***************************************************************************
// Plugin Setup Menu
//***************************************************************************

class cSetupMenuOsd2Web : public cMenuSetupPage
{
   public:

      cSetupMenuOsd2Web();
      ~cSetupMenuOsd2Web();

   protected:

      virtual eOSState ProcessKey(eKeys Key);
      virtual void Store();
      virtual void Setup();

   private:

      cEpg2VdrConfig data;
      cMenuDb* menuDb;
      long int webLoginEnabled;
      char** userList;
      int userCount;
      cStringList interfaceList;
      int interfaceIndex;
};

cSetupMenuOsd2Web::cSetupMenuOsd2Web()
{
   data = Epg2VdrConfig;
   menuDb = new cMenuDb;
   webLoginEnabled = no;
   userList = 0;
   userCount = 0;
   interfaceIndex = 0;

   Setup();
}

cSetupMenuOsd2Web::~cSetupMenuOsd2Web()
{
   if (userList)
   {
      for (int i = 0; i < userCount; i++)
         free(userList[i]);

      delete userList;
   }

   delete menuDb;
}

void cSetupMenuOsd2Web::Setup()
{
   int current = Current();

   //
   // DVB Event update mode

   static const char* masterModes[4];

   masterModes[0] = tr("auto");
   masterModes[1] = tr("yes");
   masterModes[2] = tr("no");
   masterModes[3] = 0;

   //
   // List of WEB users

   menuDb->initUserList(userList, userCount, webLoginEnabled);

   if (webLoginEnabled && userCount && !isEmpty(Epg2VdrConfig.user))
   {
      // set userIndex to the configured user

      for (int i = 0; i < userCount; i++)
      {
         if (strcmp(userList[i], data.user) == 0)
         {
            data.userIndex = i;
            break;
         }
      }
   }

   //
   // Network Interfaces

   int i = 0;
   char* interfaces;

   interfaces = strdup(getInterfaces());
   interfaceList.Clear();

   for (char* p = strtok(interfaces, " "); p; p = strtok(0, " "))
   {
      interfaceList.Append(strdup(p));

      if (!isEmpty(data.netDevice) && strncmp(p, data.netDevice, strlen(data.netDevice)) == 0)
      {
         tell(0, "Index of '%s' is %d", p, i);
         interfaceIndex = i;
      }

      i++;
   }

   free(interfaces);

   // ...

   Clear();

   Add(new cOsdItem(cString::sprintf("-------------------- %s ----------------------------------", tr("EPG Update"))));
   cList<cOsdItem>::Last()->SetSelectable(false);

   cOsdMenu::Add(new cMenuEditStraItem(tr("Update DVB EPG Database"), (int*)&data.masterMode, cUpdate::mmCount, masterModes));
   Add(new cMenuEditBoolItem(tr("Load Images"), &data.getepgimages));
   Add(new cMenuEditBoolItem(tr("Prohibit Shutdown On Busy 'epgd'"), &data.activeOnEpgd));
   Add(new cMenuEditBoolItem(tr("Schedule Boot For Update"), &data.scheduleBoot));
   Add(new cMenuEditBoolItem(tr("Blacklist not configured Channels"), &data.blacklist));
   Add(new cMenuEditBoolItem(tr("Store extended EPD Data to AUX (e.g. for Skins)"), &data.extendedEpgData2Aux));

   Add(new cOsdItem(cString::sprintf("--------------------- %s ---------------------------------", tr("Menu"))));
   cList<cOsdItem>::Last()->SetSelectable(false);
   Add(new cMenuEditBoolItem(tr("Show In Main Menu"), &data.mainmenuVisible));
   Add(new cMenuEditBoolItem(tr("Replace Program Menu"), &data.replaceScheduleMenu));
   Add(new cMenuEditBoolItem(tr("Replace Timer Menu"), &data.replaceTimerMenu));
   Add(new cMenuEditBoolItem(tr("XChange Key Ok/Blue"), &data.xchgOkBlue));
   Add(new cMenuEditBoolItem(tr("Show Channels without EPG"), &data.showEmptyChannels));

   Add(new cOsdItem(cString::sprintf("--------------------- %s ---------------------------------", tr("Web"))));
   cList<cOsdItem>::Last()->SetSelectable(false);
   Add(new cMenuEditBoolItem(tr("Share in Web"), &data.shareInWeb));
   Add(new cMenuEditBoolItem(tr("Create Timer Local"), &data.createTimerLocal));
   Add(new cMenuEditBoolItem(tr("Have Common Recordings Folder (NAS)"), &data.useCommonRecFolder));
   Add(new cMenuEditStrListItem(tr("SVDRP Interface"), &interfaceIndex, &interfaceList));
   if (userCount && webLoginEnabled)
      Add(new cMenuEditStraItem(tr("User"), (int*)&data.userIndex, userCount, userList));

   Add(new cOsdItem(cString::sprintf("--------------------- %s ---------------------------------", tr("MySQL"))));
   cList<cOsdItem>::Last()->SetSelectable(false);
   Add(new cMenuEditStrItem(tr("Host"), data.dbHost, sizeof(data.dbHost), tr(FileNameChars)));
   Add(new cMenuEditIntItem(tr("Port"), &data.dbPort, 1, 99999));
   Add(new cMenuEditStrItem(tr("Database Name"), data.dbName, sizeof(data.dbName), tr(FileNameChars)));
   Add(new cMenuEditStrItem(tr("User"), data.dbUser, sizeof(data.dbUser), tr(FileNameChars)));
   Add(new cMenuEditStrItem(tr("Password"), data.dbPass, sizeof(data.dbPass), tr(FileNameChars)));

   Add(new cOsdItem(cString::sprintf("--------------------- %s ---------------------------------", tr("Technical Stuff"))));
   cList<cOsdItem>::Last()->SetSelectable(false);
   Add(new cMenuEditIntItem(tr("Log level"), &data.loglevel, 0, 4));

   SetCurrent(Get(current));
   Display();
}

eOSState cSetupMenuOsd2Web::ProcessKey(eKeys Key)
{
   eOSState state = cMenuSetupPage::ProcessKey(Key);

   switch (state)
   {
      case osContinue:
      {
         if (NORMALKEY(Key) == kUp || NORMALKEY(Key) == kDown)
         {
            cOsdItem* item = Get(Current());

            if (item)
               item->ProcessKey(kNone);
         }

         break;
      }

      default: break;
   }

   return state;
}

void cSetupMenuOsd2Web::Store()
{
   int useCommonRecFolderOptionChanged = no;

   if (Epg2VdrConfig.useCommonRecFolder != data.useCommonRecFolder)
      useCommonRecFolderOptionChanged = yes;

   if (data.hasDbLoginChanged(&Epg2VdrConfig))
      oUpdate->triggerDbReconnect();

   Epg2VdrConfig = data;

   SetupStore("LogLevel", Epg2VdrConfig.loglevel);
   SetupStore("ShowInMainMenu", Epg2VdrConfig.mainmenuVisible);
   SetupStore("Blacklist", Epg2VdrConfig.blacklist);
   SetupStore("DbHost", Epg2VdrConfig.dbHost);
   SetupStore("DbPort", Epg2VdrConfig.dbPort);
   SetupStore("DbName", Epg2VdrConfig.dbName);
   SetupStore("DbUser", Epg2VdrConfig.dbUser);
   SetupStore("DbPass", Epg2VdrConfig.dbPass);
   SetupStore("MasterMode", Epg2VdrConfig.masterMode);
   SetupStore("LoadImages", Epg2VdrConfig.getepgimages);
   SetupStore("ActiveOnEpgd", Epg2VdrConfig.activeOnEpgd);
   SetupStore("ScheduleBoot", Epg2VdrConfig.scheduleBoot);
   SetupStore("ShareInWeb", Epg2VdrConfig.shareInWeb);
   SetupStore("CreateTimerLocal", Epg2VdrConfig.createTimerLocal);
   SetupStore("XChgKeyOkBlue", Epg2VdrConfig.xchgOkBlue);
   SetupStore("ShowEmptyChannels", Epg2VdrConfig.showEmptyChannels);
   SetupStore("UseCommonRecFolder", Epg2VdrConfig.useCommonRecFolder);
   SetupStore("ReplaceScheduleMenu", Epg2VdrConfig.replaceScheduleMenu);
   SetupStore("ReplaceTimerMenu", Epg2VdrConfig.replaceTimerMenu);
   SetupStore("ExtendedEpgData2Aux", Epg2VdrConfig.extendedEpgData2Aux);

   if (userCount && Epg2VdrConfig.userIndex >= 0)
   {
      sstrcpy(Epg2VdrConfig.user, userList[Epg2VdrConfig.userIndex], sizeof(Epg2VdrConfig.user));
      SetupStore("User", Epg2VdrConfig.user);
   }

   char* device = strdup(interfaceList[interfaceIndex]);
   char* ip = strchr(device, ':');

   if (!isEmpty(ip))
   {
      *(ip++) = 0;

      if (strcmp(Epg2VdrConfig.netDevice, device) != 0)
      {
         sstrcpy(Epg2VdrConfig.netDevice, device, sizeof(Epg2VdrConfig.netDevice));

         menuDb->vdrDb->clear();
         menuDb->vdrDb->setValue("UUID", Epg2VdrConfig.uuid);
         menuDb->vdrDb->find();
         menuDb->vdrDb->setValue("IP", ip);
         menuDb->vdrDb->store();
      }
   }

   free(device);

   SetupStore("NetDevice", Epg2VdrConfig.netDevice);

   if (useCommonRecFolderOptionChanged)
      oUpdate->commonRecFolderOptionChanged();
}
