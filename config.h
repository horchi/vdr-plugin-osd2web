/**
 *  osd2web plugin for the Video Disk Recorder
 *
 *  config.h
 *
 *  (c) 2017 Jörg Wendel
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 **/

#include "lib/config.h"

//***************************************************************************
// Config
//***************************************************************************

struct cOsd2WebConfig : public cConfigBase
{
   public:

      cOsd2WebConfig();
      ~cOsd2WebConfig();

      void setLogoSuffix(const char* s)   { free(logoSuffix); logoSuffix = strdup(s); }
      void setEpgImagePath(const char* s) { free(epgImagePath); epgImagePath = strdup(s); }

      int webPort;
      char* epgImagePath;
      char* confPath;
      char* logoSuffix;
      int logoNotLower;
      int logoById;
      int clientOsdTimeout;
};

extern cOsd2WebConfig config;
