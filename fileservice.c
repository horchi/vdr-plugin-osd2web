/**
 *  osd2web plugin for the Video Disk Recorder
 *
 *  fileservice.c
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

#include <sys/inotify.h>

#include "update.h"
#include "config.h"

//***************************************************************************
//
//***************************************************************************

int cUpdate::initFileService()
{
   char* path = 0;

   // initialize inotify

   if ((fdInotify = inotify_init1(IN_NONBLOCK)) < 0)
   {
      fdInotify = na;
      tell(0, "Couldn't initialize inotify, %m");
      return fail;
   }

   asprintf(&path, "%s/%s", config.confPath, "customdata");
   chkDir(path);
   wdInotify = inotify_add_watch(fdInotify, path, IN_ALL_EVENTS);

   free(path);

   return success;
}

//***************************************************************************
//
//***************************************************************************

int cUpdate::exitFileService()
{
   if (fdInotify != na)
   {
      inotify_rm_watch(fdInotify, wdInotify);
      close(fdInotify);
   }

   return done;
}

//***************************************************************************
//
//***************************************************************************

int cUpdate::checkFileService()
{
   const int sizeBuf = 1024 * (sizeof(inotify_event) + 16);
   char buffer[sizeBuf];
   inotify_event* event = 0;
   int count = 0;

   int bytes = read(fdInotify, buffer, sizeBuf);

   for (int pos = 0; pos < bytes; pos += sizeof(inotify_event) + event->len)
   {
      event = (inotify_event*)&buffer[pos];

      if (event->mask & IN_ISDIR || strcmp(suffixOf(event->name), "data") != 0)
         continue;

      if (event->mask & IN_CREATE || event->mask & IN_MODIFY)
      {
         count++;
         tell(0, "DEBUG: File '%s' was 'created' or 'modified'", event->name);

      }
   }

   return count;
}
