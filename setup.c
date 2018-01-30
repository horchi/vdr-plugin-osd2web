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

   if (userCount && Epg2VdrConfig.userIndex >= 0)
   {
      sstrcpy(Epg2VdrConfig.user, userList[Epg2VdrConfig.userIndex], sizeof(Epg2VdrConfig.user));
      SetupStore("User", Epg2VdrConfig.user);
   }

   char* device = strdup(interfaceList[interfaceIndex]);
   char* ip = strchr(device, ':');

   free(device);

   SetupStore("NetDevice", Epg2VdrConfig.netDevice);

   if (useCommonRecFolderOptionChanged)
      oUpdate->commonRecFolderOptionChanged();
}
