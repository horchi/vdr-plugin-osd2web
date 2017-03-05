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
//***************************************************************************

class cSkinOsd2WebDisplayChannel : public cSkinDisplayChannel
{
    public:

      cSkinOsd2WebDisplayChannel(bool WithInfo);
      virtual ~cSkinOsd2WebDisplayChannel();

      virtual void SetChannel(const cChannel *Channel, int Number);
      virtual void SetEvents(const cEvent *Present, const cEvent *Following);
      virtual void SetMessage(eMessageType Type, const char *Text);
      virtual void Flush();
};

cSkinOsd2WebDisplayChannel::cSkinOsd2WebDisplayChannel(bool WithInfo)
{
}

cSkinOsd2WebDisplayChannel::~cSkinOsd2WebDisplayChannel()
{
}

void cSkinOsd2WebDisplayChannel::SetChannel(const cChannel *Channel, int Number)
{
}

void cSkinOsd2WebDisplayChannel::SetEvents(const cEvent *Present, const cEvent *Following)
{
}

void cSkinOsd2WebDisplayChannel::SetMessage(eMessageType Type, const char *Text)
{
}

void cSkinOsd2WebDisplayChannel::Flush()
{
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
      virtual void SetTitle(const char *Title);
      virtual void SetButtons(const char *Red, const char *Green = NULL, const char *Yellow = NULL, const char *Blue = NULL);
      virtual void SetMessage(eMessageType Type, const char *Text);
      virtual void SetItem(const char *Text, int Index, bool Current, bool Selectable);
      virtual void SetScrollbar(int Total, int Offset);
      virtual void SetEvent(const cEvent *Event);
      virtual void SetRecording(const cRecording *Recording);
      virtual void SetText(const char *Text, bool FixedFont);
      virtual int GetTextAreaWidth() const;
      virtual const cFont* GetTextAreaFont(bool FixedFont) const;
      virtual void Flush();
};

cSkinOsd2WebDisplayMenu::cSkinOsd2WebDisplayMenu()
{
}

cSkinOsd2WebDisplayMenu::~cSkinOsd2WebDisplayMenu()
{
}

void cSkinOsd2WebDisplayMenu::SetMenuCategory(eMenuCategory MenuCategory)
{
   tell(0, "DEB: cSkinOsd2WebDisplayMenu::SetMenuCategory(%d)", MenuCategory);
   cSkinDisplayMenu::SetMenuCategory(MenuCategory);
}

void cSkinOsd2WebDisplayMenu::Scroll(bool Up, bool Page)
{
   tell(0, "DEB: cSkinOsd2WebDisplayMenu::Scroll(%d, %d)", Up, Page);
}

int cSkinOsd2WebDisplayMenu::MaxItems()
{
   tell(0, "DEB: cSkinOsd2WebDisplayMenu::MaxItems()");
   return 11; // count of visible lines
}

void cSkinOsd2WebDisplayMenu::Clear()
{
   tell(0, "DEB: cSkinOsd2WebDisplayMenu::Clear()");
}

void cSkinOsd2WebDisplayMenu::SetTitle(const char *Title)
{
   tell(0, "DEB: cSkinOsd2WebDisplayMenu::SetTitle(%s)", Title);
}

void cSkinOsd2WebDisplayMenu::SetButtons(const char *Red, const char *Green, const char *Yellow, const char *Blue)
{
   tell(0, "DEB: cSkinOsd2WebDisplayMenu::SetButtons()");
}

void cSkinOsd2WebDisplayMenu::SetMessage(eMessageType Type, const char *Text)
{
   tell(0, "DEB: cSkinOsd2WebDisplayMenu::SetMessage(%d, %s)", Type, Text);
}

void cSkinOsd2WebDisplayMenu::SetItem(const char *Text, int Index, bool Current, bool Selectable)
{
   tell(0, "DEB: cSkinOsd2WebDisplayMenu::SetItem(%s, %d, %d, %d)", Text, Index, Current, Selectable);
}

void cSkinOsd2WebDisplayMenu::SetScrollbar(int Total, int Offset)
{
   tell(0, "DEB: cSkinOsd2WebDisplayMenu::SetScrollbar(%d, %d)", Total, Offset);
}

void cSkinOsd2WebDisplayMenu::SetEvent(const cEvent *Event)
{
   tell(0, "DEB: cSkinOsd2WebDisplayMenu::SetEvent()");
}

void cSkinOsd2WebDisplayMenu::SetRecording(const cRecording *Recording)
{
   tell(0, "DEB: cSkinOsd2WebDisplayMenu::SetRecording()");
}

void cSkinOsd2WebDisplayMenu::SetText(const char *Text, bool FixedFont)
{
   tell(0, "DEB: cSkinOsd2WebDisplayMenu::SetText(%s, %d)", Text, FixedFont);
}

int cSkinOsd2WebDisplayMenu::GetTextAreaWidth() const
{
   tell(0, "DEB: cSkinOsd2WebDisplayMenu::GetTextAreaWidth()");
   return 600;  // pixels
}

const cFont *cSkinOsd2WebDisplayMenu::GetTextAreaFont(bool FixedFont) const
{
   return cFont::GetFont(FixedFont ? fontFix : fontOsd);
}

void cSkinOsd2WebDisplayMenu::Flush()
{
}

//***************************************************************************
// cSkinOsd2WebDisplayReplay
//***************************************************************************

class cSkinOsd2WebDisplayReplay : public cSkinDisplayReplay
{
    public:

      cSkinOsd2WebDisplayReplay(bool ModeOnly);
      virtual ~cSkinOsd2WebDisplayReplay();

      virtual void SetTitle(const char *Title);
      virtual void SetMode(bool Play, bool Forward, int Speed);
      virtual void SetProgress(int Current, int Total);
      virtual void SetCurrent(const char *Current);
      virtual void SetTotal(const char *Total);
      virtual void SetJump(const char *Jump);
      virtual void SetMessage(eMessageType Type, const char *Text);
      virtual void Flush();
};

cSkinOsd2WebDisplayReplay::cSkinOsd2WebDisplayReplay(bool ModeOnly)
{
}

cSkinOsd2WebDisplayReplay::~cSkinOsd2WebDisplayReplay()
{
}

void cSkinOsd2WebDisplayReplay::SetTitle(const char *Title)
{
}

void cSkinOsd2WebDisplayReplay::SetMode(bool Play, bool Forward, int Speed)
{
}

void cSkinOsd2WebDisplayReplay::SetProgress(int Current, int Total)
{
}

void cSkinOsd2WebDisplayReplay::SetCurrent(const char *Current)
{
}

void cSkinOsd2WebDisplayReplay::SetTotal(const char *Total)
{
}

void cSkinOsd2WebDisplayReplay::SetJump(const char *Jump)
{
}

void cSkinOsd2WebDisplayReplay::SetMessage(eMessageType Type, const char *Text)
{
}

void cSkinOsd2WebDisplayReplay::Flush()
{
}

//***************************************************************************
// cSkinOsd2WebDisplayVolume
//***************************************************************************

class cSkinOsd2WebDisplayVolume : public cSkinDisplayVolume
{
public:
  cSkinOsd2WebDisplayVolume();
  virtual ~cSkinOsd2WebDisplayVolume();
  virtual void SetVolume(int Current, int Total, bool Mute);
  virtual void Flush();
};

cSkinOsd2WebDisplayVolume::cSkinOsd2WebDisplayVolume()
{
}

cSkinOsd2WebDisplayVolume::~cSkinOsd2WebDisplayVolume()
{
}

void cSkinOsd2WebDisplayVolume::SetVolume(int Current, int Total, bool Mute)
{
}

void cSkinOsd2WebDisplayVolume::Flush()
{
}

//***************************************************************************
// cSkinOsd2WebDisplayTracks
//***************************************************************************

class cSkinOsd2WebDisplayTracks : public cSkinDisplayTracks
{
public:
  cSkinOsd2WebDisplayTracks(const char *Title, int NumTracks, const char * const *Tracks);
  virtual ~cSkinOsd2WebDisplayTracks();
  virtual void SetTrack(int Index, const char * const *Tracks);
  virtual void SetAudioChannel(int AudioChannel) {}
  virtual void Flush();
};

cSkinOsd2WebDisplayTracks::cSkinOsd2WebDisplayTracks(const char *Title, int NumTracks, const char * const *Tracks)
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
}

void cSkinOsd2WebDisplayMessage::SetMessage(eMessageType Type, const char *Text)
{
}

void cSkinOsd2WebDisplayMessage::Flush()
{
}


//***************************************************************************
// Class cOsd2WebSkin
//***************************************************************************

cOsd2WebSkin::cOsd2WebSkin()
   : cSkin("osd2web")
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
