/*
 * config.h:
 *
 * See the README file for copyright information and how to reach the author.
 *
 */

#ifndef __EPG_CONFIG_H
#define __EPG_CONFIG_H

#include "common.h"

//***************************************************************************
// Config
//***************************************************************************

struct cConfigBase
{
   public:

      cConfigBase();

      static void setLogLevel(const char* logLevel);   //  "<level]:width]>"

      // static stuff


      static int logstdout;
      static int loglevel;
      static char* loglevelString;
      static int logwidth;
      static int argLoglevel;
      static int logFacility;
      static const char* logName;   // filename if we aren't a plugin
};

struct cEpgConfig : public cConfigBase
{
   public:

      cEpgConfig();

      // database connection

      int hasDbLoginChanged(cEpgConfig* old);

      char dbHost[100+TB];
      int dbPort;
      char dbName[100+TB];
      char dbUser[100+TB];
      char dbPass[100+TB];

      char netDevice[20+TB];
      char uuid[sizeUuid+TB];

      int getepgimages;
};

#endif // __EPG_CONFIG_H
