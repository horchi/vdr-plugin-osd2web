/**
 *  osd2web plugin for the Video Disk Recorder
 *
 *  skin.c
 *
 *  (c) 2017 Jörg Wendel
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 **/

//***************************************************************************
// Includes
//***************************************************************************

#include "update.h"

//***************************************************************************
// cSkinOsd2WebDisplayChannel
//   - normally used to display present/following
//   - here only used for channel groups while zapping (key left/right)
//   - not used to display present/following since wie need this information
//     also when the skin is atttached to the 'real' OSD
//***************************************************************************

class cSkinOsd2WebDisplayChannel : public cSkinDisplayChannel
{
    public:

      cSkinOsd2WebDisplayChannel(bool WithInfo) { visible = no; }
      virtual ~cSkinOsd2WebDisplayChannel();

      virtual void SetChannel(const cChannel *Channel, int Number);
      virtual void SetEvents(const cEvent *Present, const cEvent *Following) {}
      virtual void SetMessage(eMessageType Type, const char *Text) {}
      virtual void Flush() {}

   private:

      int visible;
};

cSkinOsd2WebDisplayChannel::~cSkinOsd2WebDisplayChannel()
{
   if (visible)
   {
      json_t* obj = json_object();
      addToJson(obj, "name", "");
      cUpdate::pushMessage(obj, "channelgroup");
   }
}

void cSkinOsd2WebDisplayChannel::SetChannel(const cChannel* Channel, int /*Number*/)
{
   tell(4, "DEB: Skin:cSkinOsd2WebDisplayChannel::DisplayChannel(%s, %d)",
        Channel ? Channel->Name() : "<null>", Channel ? Channel->Number() : na);

   if (Channel && Channel->Number() == 0)
   {
      visible = yes;
      json_t* obj = json_object();
      addToJson(obj, "name", Channel->Name());
      cUpdate::pushMessage(obj, "channelgroup");
   }
   else if (visible)
   {
      visible = no;
      json_t* obj = json_object();
      addToJson(obj, "name", "");
      cUpdate::pushMessage(obj, "channelgroup");
   }
}

//***************************************************************************
// cSkinOsd2WebDisplayMenu
//***************************************************************************

class cSkinOsd2WebDisplayMenu : public cSkinDisplayMenu
{
   public:

      cSkinOsd2WebDisplayMenu();
      virtual ~cSkinOsd2WebDisplayMenu();

      virtual void SetMenuCategory(eMenuCategory MenuCategory);
      virtual void Scroll(bool Up, bool Page);
      virtual int MaxItems();
      virtual void Clear();
      virtual void SetTitle(const char* Title);
      virtual void SetButtons(const char* Red, const char* Green = NULL, const char* Yellow = NULL, const char* Blue = NULL);
      virtual void SetMessage(eMessageType Type, const char* Text);
      virtual void SetItem(const char* Text, int Index, bool Current, bool Selectable);
      virtual bool SetItemEvent(const cEvent* Event, int Index, bool Current, bool Selectable, const cChannel *Channel, bool WithDate, eTimerMatch TimerMatch);
      virtual bool SetItemChannel(const cChannel* Channel, int Index, bool Current, bool Selectable, bool WithProvider);
      virtual bool SetItemRecording(const cRecording* Recording, int Index, bool Current, bool Selectable, int Level, int Total, int New);
      virtual bool SetItemTimer(const cTimer* Timer, int Index, bool Current, bool Selectable);
      virtual void SetScrollbar(int Total, int Offset);
      virtual void SetEvent(const cEvent* Event);
      virtual void SetRecording(const cRecording* Recording);
      virtual void SetText(const char* Text, bool FixedFont);
      virtual int GetTextAreaWidth() const;
      virtual const cFont* GetTextAreaFont(bool FixedFont) const;
      virtual void Flush() {}

   private:

      // nothing yet
};

cSkinOsd2WebDisplayMenu::cSkinOsd2WebDisplayMenu()
{
}

cSkinOsd2WebDisplayMenu::~cSkinOsd2WebDisplayMenu()
{
   tell(3, "Menu closed (%d)", MenuCategory());

   if (MenuCategory() == mcMain)
      cUpdate::menuClosed();

   cUpdate::menuCategory = mcUnknown;
}

void cSkinOsd2WebDisplayMenu::SetMenuCategory(eMenuCategory MenuCategory)
{
   tell(3, "DEB: Skin:cSkinOsd2WebDisplayMenu::SetMenuCategory(%d)", MenuCategory);
   cSkinDisplayMenu::SetMenuCategory(MenuCategory);
}

void cSkinOsd2WebDisplayMenu::Scroll(bool Up, bool Page)
{
   tell(3, "DEB: Skin:cSkinOsd2WebDisplayMenu::Scroll(%d, %d)", Up, Page);

   json_t* obj = json_object();

   addToJson(obj, "up", Up);
   addToJson(obj, "page", Page);

   cUpdate::pushMessage(obj, "textscroll");
}

int cSkinOsd2WebDisplayMenu::MaxItems()
{
   tell(3, "DEB: Skin:cSkinOsd2WebDisplayMenu::MaxItems()");

   if (MenuCategory() >= mcUnknown && MenuCategory() <= mcCam)
   {
      tell(2, "Set MaxItems() to %d", cUpdate::menuMaxLines[MenuCategory()].maxLines);
      return cUpdate::menuMaxLines[MenuCategory()].maxLines;
   }

   return 10;
}

void cSkinOsd2WebDisplayMenu::Clear()
{
   tell(4, "DEB: Skin:cSkinOsd2WebDisplayMenu::Clear()");

   json_t* obj = json_object();

   addToJson(obj, "clear", yes);
   cUpdate::pushMessage(obj, "clearmenu");
}

void cSkinOsd2WebDisplayMenu::SetTitle(const char *Title)
{
   tell(4, "DEB: Skin:cSkinOsd2WebDisplayMenu::SetTitle(%s)", Title);

   cUpdate::menuCategory = MenuCategory();
   cUpdate::menuTitle = Title;

   cUpdate::updateMenu();
}

void cSkinOsd2WebDisplayMenu::SetButtons(const char *red, const char *green,
                                         const char *yellow, const char *blue)
{
   tell(4, "DEB: Skin:cSkinOsd2WebDisplayMenu::SetButtons()");
   json_t* buttons = json_object();

   if (!isEmpty(red))    addToJson(buttons, "red", red);
   if (!isEmpty(green))  addToJson(buttons, "green", green);
   if (!isEmpty(yellow)) addToJson(buttons, "yellow", yellow);
   if (!isEmpty(blue))   addToJson(buttons, "blue", blue);

   cUpdate::pushMessage(buttons, "buttons");
}

void cSkinOsd2WebDisplayMenu::SetMessage(eMessageType Type, const char *Text)
{
   tell(4, "DEB: Skin:cSkinOsd2WebDisplayMenu::SetMessage(%d, %s)", Type, Text);

   json_t* obj = json_object();

   addToJson(obj, "type", Type);
   addToJson(obj, "message", Text);

   cUpdate::pushMessage(obj, "message");
}

void cSkinOsd2WebDisplayMenu::SetItem(const char* Text, int Index, bool Current, bool Selectable)
{
   tell(4, "DEB: Skin:cSkinOsd2WebDisplayMenu::SetItem(%s, %d, %d, %d)",
        Text, Index, Current, Selectable);

   if (cUpdate::isEditable(MenuCategory()))
      SetEditableWidth(200);

   json_t* oMenuItem = json_object();

   addToJson(oMenuItem, "text", Text);
   addToJson(oMenuItem, "index", Index);
   addToJson(oMenuItem, "current", Current);
   addToJson(oMenuItem, "selectable", Selectable);

   cUpdate::pushMessage(oMenuItem, "menuitem");
}

bool cSkinOsd2WebDisplayMenu::SetItemEvent(const cEvent* Event, int Index, bool Current,
                                           bool Selectable, const cChannel *Channel,
                                           bool WithDate, eTimerMatch TimerMatch)
{
   tell(4, "DEB: Skin:cSkinOsd2WebDisplayMenu::SetItemEvent()");

   if (cUpdate::menuMaxLines[MenuCategory()].shape & cOsdService::osText)
      return false;

   json_t* oMenuItem = json_object();
   json_t* oEvent = json_object();

   event2Json(oEvent, Event, Channel, TimerMatch, Current, cUpdate::menuMaxLines[MenuCategory()].shape);

   addToJson(oMenuItem, "index", Index);
   addToJson(oMenuItem, "current", Current);
   addToJson(oMenuItem, "selectable", Selectable);
   addToJson(oMenuItem, "withdate", WithDate);
   addToJson(oMenuItem, "event", oEvent);

   cUpdate::pushMessage(oMenuItem, "eventitem");

   return true;
}

bool cSkinOsd2WebDisplayMenu::SetItemChannel(const cChannel* Channel, int Index, bool Current,
                                             bool Selectable, bool WithProvider)
{
   tell(4, "DEB: Skin:cSkinOsd2WebDisplayMenu::SetItemChannel()");

   if (cUpdate::menuMaxLines[MenuCategory()].shape & cOsdService::osText)
      return false;

   json_t* oMenuItem = json_object();
   json_t* oChannel = json_object();

   channel2Json(oChannel, Channel);

   addToJson(oMenuItem, "index", Index);
   addToJson(oMenuItem, "current", Current);
   addToJson(oMenuItem, "selectable", Selectable);
   addToJson(oMenuItem, "withprovider", WithProvider);
   addToJson(oMenuItem, "channel", oChannel);

   cUpdate::pushMessage(oMenuItem, "channelitem");

   return true;
}

bool cSkinOsd2WebDisplayMenu::SetItemRecording(const cRecording* Recording, int Index, bool Current,
                                               bool Selectable, int Level, int Total, int New)
{
   tell(4, "DEB: Skin:cSkinOsd2WebDisplayMenu::SetItemRecording()");

   if (cUpdate::menuMaxLines[MenuCategory()].shape & cOsdService::osText)
      return false;

   json_t* oMenuItem = json_object();
   json_t* oRecording = json_object();

   recording2Json(oRecording, Recording);

   addToJson(oMenuItem, "index", Index);
   addToJson(oMenuItem, "current", Current);
   addToJson(oMenuItem, "selectable", Selectable);
   addToJson(oMenuItem, "level", Level);
   addToJson(oMenuItem, "total", Total);
   addToJson(oMenuItem, "new", New);
   addToJson(oMenuItem, "recording", oRecording);

   cUpdate::pushMessage(oMenuItem, "recordingitem");

   return true;
}

void cSkinOsd2WebDisplayMenu::SetRecording(const cRecording *Recording)
{
   tell(4, "DEB: Skin:cSkinOsd2WebDisplayMenu::SetRecording()");

   json_t* oRecording = json_object();
   recording2Json(oRecording, Recording);

   cUpdate::pushMessage(oRecording, "recording");
}

bool cSkinOsd2WebDisplayMenu::SetItemTimer(const cTimer *Timer, int Index, bool Current, bool Selectable)
{
   tell(4, "DEB: Skin:cSkinOsd2WebDisplayMenu::SetItemTimer()");

   if (cUpdate::menuMaxLines[MenuCategory()].shape & cOsdService::osText)
      return false;

   json_t* oMenuItem = json_object();
   json_t* oTimer = json_object();

   timer2Json(oTimer, Timer);

   addToJson(oMenuItem, "index", Index);
   addToJson(oMenuItem, "current", Current);
   addToJson(oMenuItem, "selectable", Selectable);
   addToJson(oMenuItem, "timer", oTimer);

   cUpdate::pushMessage(oMenuItem, "timeritem");

   return true;
}

void cSkinOsd2WebDisplayMenu::SetScrollbar(int Total, int Offset)
{
   tell(4, "DEB: Skin:cSkinOsd2WebDisplayMenu::SetScrollbar(%d, %d)", Total, Offset);

   json_t* oBar = json_object();

   addToJson(oBar, "Total", Total);
   addToJson(oBar, "Offset", Offset);

   cUpdate::pushMessage(oBar, "scrollbar");
}

void cSkinOsd2WebDisplayMenu::SetEvent(const cEvent *Event)
{
   eTimerMatch timerMatch;
   tell(4, "DEB: Skin:cSkinOsd2WebDisplayMenu::SetEvent()");

#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
   LOCK_TIMERS_READ;
   const cTimers* timers = Timers;
#else
   timers = &Timers;
#endif

   json_t* oEvent = json_object();
   getTimerMatch(timers, Event, &timerMatch);
   event2Json(oEvent, Event, 0, timerMatch, no, cOsdService::osLarge);

   cUpdate::pushMessage(oEvent, "event");
}

void cSkinOsd2WebDisplayMenu::SetText(const char *Text, bool FixedFont)
{
   tell(4, "DEB: Skin:cSkinOsd2WebDisplayMenu::SetText(%s, %d)", Text, FixedFont);

   json_t* oMenuText = json_object();

   addToJson(oMenuText, "text", Text);

   cUpdate::pushMessage(oMenuText, "menutext");
}

int cSkinOsd2WebDisplayMenu::GetTextAreaWidth() const
{
   tell(4, "DEB: Skin:cSkinOsd2WebDisplayMenu::GetTextAreaWidth() = 1024");
   return 1024;  // pixels dummy code
}

const cFont* cSkinOsd2WebDisplayMenu::GetTextAreaFont(bool FixedFont) const
{
   return cFont::GetFont(FixedFont ? fontFix : fontOsd);    // dummy code
}

//***************************************************************************
// cSkinOsd2WebDisplayReplay
//   - unused since wie need this information also when the skin
//     is atttached to the 'real' OSD
//***************************************************************************

class cSkinOsd2WebDisplayReplay : public cSkinDisplayReplay
{
    public:

      cSkinOsd2WebDisplayReplay(bool ModeOnly) {}
      virtual ~cSkinOsd2WebDisplayReplay() {}

      virtual void SetTitle(const char *Title) {}
      virtual void SetMode(bool Play, bool Forward, int Speed) {}
      virtual void SetProgress(int Current, int Total) {}
      virtual void SetCurrent(const char *Current) {}
      virtual void SetTotal(const char *Total) {}
      virtual void SetJump(const char *Jump) {}
      virtual void SetMessage(eMessageType Type, const char *Text) {}
      virtual void Flush() {}
};

//***************************************************************************
// cSkinOsd2WebDisplayVolume
//***************************************************************************

class cSkinOsd2WebDisplayVolume : public cSkinDisplayVolume
{
   public:

     cSkinOsd2WebDisplayVolume();
     virtual ~cSkinOsd2WebDisplayVolume();

     virtual void SetVolume(int Current, int Total, bool Mute);
     virtual void Flush() {}
};

cSkinOsd2WebDisplayVolume::cSkinOsd2WebDisplayVolume()
{
}

cSkinOsd2WebDisplayVolume::~cSkinOsd2WebDisplayVolume()
{
   json_t* oVolume = json_object();

   addToJson(oVolume, "display", no);

   cUpdate::pushMessage(oVolume, "volume");
}

void cSkinOsd2WebDisplayVolume::SetVolume(int Current, int Total, bool Mute)
{
   tell(4, "DEB: Skin:cSkinOsd2WebDisplayVolume::SetVolume(%d, %d, %d)", Current, Total, Mute);

   json_t* oVolume = json_object();

   addToJson(oVolume, "display", yes);
   addToJson(oVolume, "current", Current);
   addToJson(oVolume, "total", Total);
   addToJson(oVolume, "mute", Mute);

   cUpdate::pushMessage(oVolume, "volume");
}

//***************************************************************************
// cSkinOsd2WebDisplayTracks
//***************************************************************************

class cSkinOsd2WebDisplayTracks : public cSkinDisplayTracks
{
   public:

      cSkinOsd2WebDisplayTracks(const char* Title, int NumTracks, const char* const* Tracks);
      virtual ~cSkinOsd2WebDisplayTracks();

      virtual void SetTrack(int Index, const char* const* Tracks);
      virtual void SetAudioChannel(int AudioChannel) {}
      virtual void Flush();
};

cSkinOsd2WebDisplayTracks::cSkinOsd2WebDisplayTracks(const char *Title, int NumTracks, const char* const* Tracks)
{
}

cSkinOsd2WebDisplayTracks::~cSkinOsd2WebDisplayTracks()
{
}

void cSkinOsd2WebDisplayTracks::SetTrack(int Index, const char * const *Tracks)
{
}

void cSkinOsd2WebDisplayTracks::Flush()
{
}

//***************************************************************************
// cSkinOsd2WebDisplayMessage
//***************************************************************************

class cSkinOsd2WebDisplayMessage : public cSkinDisplayMessage
{
    public:

      cSkinOsd2WebDisplayMessage();
      virtual ~cSkinOsd2WebDisplayMessage();

      virtual void SetMessage(eMessageType Type, const char *Text);
      virtual void Flush();
};

cSkinOsd2WebDisplayMessage::cSkinOsd2WebDisplayMessage()
{
}

cSkinOsd2WebDisplayMessage::~cSkinOsd2WebDisplayMessage()
{
   tell(4, "DEB: Skin:cSkinOsd2WebDisplayMessage~cSkinOsd2WebDisplayMessage()");

   json_t* obj = json_object();

   addToJson(obj, "message", "");

   cUpdate::pushMessage(obj, "message");
}

void cSkinOsd2WebDisplayMessage::SetMessage(eMessageType Type, const char *Text)
{
   tell(4, "DEB: Skin:cSkinOsd2WebDisplayMessage::SetMessage(%d, %s)", Type, Text);

   json_t* obj = json_object();

   addToJson(obj, "type", Type);
   addToJson(obj, "message", Text);

   cUpdate::pushMessage(obj, "message");
}

void cSkinOsd2WebDisplayMessage::Flush()
{
}

//***************************************************************************
// Class cOsd2WebSkin
//***************************************************************************

cOsd2WebSkin::cOsd2WebSkin()
   : cSkin(SKIN_NAME)
{
}

cSkinDisplayChannel* cOsd2WebSkin::DisplayChannel(bool withInfo)
{
   return new cSkinOsd2WebDisplayChannel(withInfo);
}

cSkinDisplayMenu* cOsd2WebSkin::DisplayMenu()
{
   return new cSkinOsd2WebDisplayMenu;
}

cSkinDisplayReplay* cOsd2WebSkin::DisplayReplay(bool modeOnly)
{
   return new cSkinOsd2WebDisplayReplay(modeOnly);
}

cSkinDisplayVolume* cOsd2WebSkin::DisplayVolume()
{
   return new cSkinOsd2WebDisplayVolume;
}

cSkinDisplayTracks* cOsd2WebSkin::DisplayTracks(const char* title, int numTracks, const char* const* tracks)
{
   return new cSkinOsd2WebDisplayTracks(title, numTracks, tracks);
}

cSkinDisplayMessage* cOsd2WebSkin::DisplayMessage()
{
   return new cSkinOsd2WebDisplayMessage;
}
