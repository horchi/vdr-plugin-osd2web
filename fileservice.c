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
// Init File Service
//***************************************************************************

int cUpdate::initFileService()
{
   char* path = 0;

   // initialize inotify

   if ((fdInotify = inotify_init1(IN_NONBLOCK)) < 0)
   {
      fdInotify = na;
      tell(0, "Couldn't initialize inotify, error was '%s'", strerror(errno));
      return fail;
   }

   asprintf(&path, "%s/%s", config.confPath, "customdata");
   chkDir(path);
   wdInotify = inotify_add_watch(fdInotify, path, IN_ALL_EVENTS);

   free(path);

   parseVariableFiles();

   return success;
}

//***************************************************************************
// Exit File Service
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
// Check File Service
//***************************************************************************

int cUpdate::checkFileService()
{
   const int sizeBuf = 1024 * (sizeof(inotify_event) + 16);
   char buffer[sizeBuf];
//   char* path = 0;
   //   int count = 0;
   inotify_event* event = 0;

   int bytes = read(fdInotify, buffer, sizeBuf);

   for (int pos = 0; pos < bytes; pos += sizeof(inotify_event) + event->len)
   {
      event = (inotify_event*)&buffer[pos];

      if (event->mask & IN_ISDIR || strcmp(suffixOf(event->name), "data") != 0)
         continue;

      if (event->mask & IN_CREATE || event->mask & IN_MODIFY)
         return parseVariableFiles();
/*
      if (event->mask & IN_CREATE || event->mask & IN_MODIFY)
      {
         asprintf(&path, "%s/%s/%s", config.confPath, "customdata", event->name);
         tell(4, "DEBUG: File '%s' was 'created' or 'modified'", path);

         if (parseVariableFile(path, event->name) > 0)
            count++;

         free(path);
         }*/
   }

   return done;
}

//***************************************************************************
// Parse Variable Files
//***************************************************************************

int cUpdate::parseVariableFiles()
{
   int count = 0;
   char* path = 0;
   DIR* dir;

   // clear all variables

   serviceVariableFiles.clear();

   // create path of custom data files

   asprintf(&path, "%s/%s", config.confPath, "customdata");

   if (!(dir = opendir(path)))
   {
      tell(1, "Can't open directory '%s', '%s'", path, strerror(errno));
      free(path);
      return fail;
   }

#ifndef HAVE_READDIR_R
   dirent* pEntry;

   while ((pEntry = readdir(dir)))
#else
   dirent entry;
   dirent* pEntry = &entry;
   dirent* res;

   // deprecated but the only reentrant with old libc!

   while (readdir_r(dir, pEntry, &res) == 0 && res)
#endif
   {
      if (dirTypeOf(path, pEntry) != DT_REG || pEntry->d_name[0] == '.')
         continue;

      tell(4, "Checking variables of '%s'", pEntry->d_name);

      char* fPath = 0;

      asprintf(&fPath, "%s/%s/%s", config.confPath, "customdata", pEntry->d_name);

      if (parseVariableFile(fPath, pEntry->d_name) > 0)
         count++;

      free(fPath);
   }

   closedir(dir);
   free(path);

   return count;
}

//***************************************************************************
// Parse Variable File
//***************************************************************************

int cUpdate::parseVariableFile(const char* path, const char* service)
{
   std::map<std::string,FileVariable> serviceVariables;

   FILE* fp;
   char line[500+TB]; *line = 0;
   char* p;
   char* value;
   FileVariable var;

   if (!(fp = fopen(path, "r")))
   {
      tell(0, "Can't open '%s', error was '%s'",  path, strerror(errno));
      return ignore;
   }

   while (fgets(line, 500, fp))
   {
      line[strlen(line)] = 0;        // cut linefeed

      if ((p = strstr(line, "//")))  // cut comments
         *p = 0;

      if ((p = strstr(line, ";")))   // cut line end
         *p = 0;

      // skip empty lines

      allTrim(line);

      if (isEmpty(line))
         continue;

      // check line, search value

      if (!(value = strchr(line, '=')) || line >= value)
      {
         tell(0, "Info: Ignoring invalid line [%s] in '%s'", line, path);
         continue;
      }

      *value = 0;
      value++;

      allTrim(line);
      allTrim(value);

      var.file = service;
      var.name = line;
      var.value = value;

      var.file = var.file.substr(0, var.file.find_last_of("."));

      serviceVariables[service + std::string(".") + line] = var;

      tell(1, "Append variable '%s.%s' with value '%s'", var.file.c_str(), var.name.c_str(), var.value.c_str());
   }

   if (serviceVariables.size())
      serviceVariableFiles.push_back(serviceVariables);

   fclose(fp);

   return serviceVariables.size();
}
