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

struct cEpgConfig
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

      // static stuff 

      static int logstdout;
      static int loglevel;
      static int argLoglevel;
      static int logFacility;
      static const char* logName;
};

#endif // __EPG_CONFIG_H 
