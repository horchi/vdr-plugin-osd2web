/*
 * service.h
 *
 * See the README file for copyright information
 *
 */

#ifndef __LMC_SERVICE_H
#define __LMC_SERVICE_H

#include <inttypes.h>

#include "lib/common.h"

struct PlayerState
{
   PlayerState() { memset(this, 0, sizeof(PlayerState)); }

      uint64_t updatedAt;

      char mode[20+TB];
      char version[50+TB];
      int volume;
      int muted;
      int trackTime;             // playtime of the current song

      // playlist state

      char plName[300+TB];
      int plIndex;
      int plCount;
      int plShuffle;
      int plRepeat;
};

struct TrackInfo
{
   TrackInfo() { memset(this, 0, sizeof(TrackInfo)); }

      uint64_t updatedAt;

      char genre[100+TB];
      char album[100+TB];
      char artist[100+TB];
      char title[200+TB];
      char artworkTrackId[500+TB];
      char artworkurl[500+TB];
      char remoteTitle[500+TB];
      char contentType[100+TB];
      char lyrics[10000+TB];

      unsigned int bitrate;
      unsigned short remote;
      unsigned int year;

      int index;             // index in current playlist
      int id;                // track ID
      int duration;
};

#define SQUEEZEBOX_CURRENT_TRACK    "squeezebox_current_track-v1.0"


#endif // __LMCCOM_H
