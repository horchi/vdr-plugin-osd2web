/*
 * configuration.h
 *
 * See the README file for copyright information and how to reach the author.
 *
 */

#ifndef __CONFIGURATION_H
#define __CONFIGURATION_H

#include "thread.h"
#include "common.h"

extern const char* confDir;

//***************************************************************************
// Configuration
//***************************************************************************

class Configuration
{
   public:

      Configuration() {};
      virtual ~Configuration() {};

      virtual int atConfigItem(const char* Name, const char* Value) = 0;

      virtual int readConfig(const char* file = 0)
      {
         int count = 0;
         FILE* f;
         char* line = 0;
         size_t size = 0;
         char* value;
         char* name;
         char* fileName;

         if (!isEmpty(file))
            asprintf(&fileName, "%s", file);
         else
            asprintf(&fileName, "%s/epgd.conf", confDir);

         if (access(fileName, F_OK) != 0)
         {
            fprintf(stderr, "Cannot access configuration file '%s'\n", fileName);
            free(fileName);
            return fail;
         }

         f = fopen(fileName, "r");

         while (getline(&line, &size, f) > 0)
         {
            char* p = strchr(line, '#');
            if (p) *p = 0;

            allTrim(line);

            if (isEmpty(line))
               continue;

            if (!(value = strchr(line, '=')))
               continue;

            *value = 0;
            value++;
            lTrim(value);
            name = line;
            allTrim(name);

            if (atConfigItem(name, value) != success)
            {
               fprintf(stderr, "Found unexpected parameter '%s', aborting\n", name);
               free(fileName);
               return fail;
            }

            count++;
         }

         free(line);
         fclose(f);

         tell(0, "Read %d option from %s", count , fileName);

         free(fileName);

         return success;
      }
};

//***************************************************************************
// System Notification Interface (systemd, watchdog, pidfile, ...)
//***************************************************************************

class cSystemNotification : public cThread
{
   public:

      enum SystemEvent
      {
         evReady,
         evStatus,
         evKeepalive,
         evStopping
      };

      enum Misc
      {
         defaultInterval = 60
      };

      cSystemNotification();

      int __attribute__ ((format(printf, 3, 4))) notify(int event, const char* format = 0, ...);
      int getWatchdogState(int minInterval);
      void check(int force = no);

      int startNotifyThread(int timeout);
      int stopNotifyThread();

      static void setPidFile(const char* file) { pidfile = file; }

   protected:

      virtual void action();

      int interval;
      int threadTimeout;
      cCondVar waitCondition;
      int stop;

      static time_t lastWatchdogAt;
      static const char* pidfile;
};

//***************************************************************************

#endif // __CONFIGURATION_H
