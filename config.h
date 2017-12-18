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
      void setLogoPath(const char* p)     { free(logoPath); logoPath = strdup(p); }
      void setEpgImagePath(const char* s) { free(epgImagePath); epgImagePath = strdup(s); }
      void setDiaPath(const char* p)      { free(diaPath); diaPath = strdup(p); }
      void setBrowser(const char* p)      { free(browserDisplay); browserDisplay = strdup(p); startBrowser = yes;}

      int webPort;
      char* httpPath;
      char* epgImagePath;
      char* confPath;
      char* logoSuffix;
      char* logoPath;
      char* diaPath;
      int logoNotLower;
      int logoById;
      int clientOsdTimeout;
      int diaCycleTime;            // in seconds
      int startBrowser;
      char* browserDisplay;
};

extern cOsd2WebConfig config;
