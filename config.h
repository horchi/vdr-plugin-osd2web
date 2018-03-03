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

#ifndef __O2W_CONFIG_H
#define __O2W_CONFIG_H

#include "lib/config.h"

//***************************************************************************
// Config
//***************************************************************************

struct cOsd2WebConfig : public cConfigBase
{
   public:

      cOsd2WebConfig();
      ~cOsd2WebConfig();

      void setLogoSuffix(const char* s)     { free(logoSuffix); logoSuffix = strdup(s); }
      void setLogoPath(const char* p)       { free(logoPath); logoPath = strdup(p); }
      void setEpgImagePath(const char* s)   { free(epgImagePath); epgImagePath = strdup(s); }
      void setDiaPath(const char* p)        { free(diaPath); diaPath = strdup(p); }
      void setDiaPathCurrent(const char* p)  { free(diaPathCurrent); diaPathCurrent = strdup(p); }
      void setScaper2VdrPath(const char* p) { free(scraper2VdrPath); scraper2VdrPath = strdup(p); }
      void setTvIp(const char* p)           { free(tvIp); tvIp = strdup(p); }
      void setBrowser(const char* p, int autoStart = na);

      int webPort;
      char* httpPath;
      char* epgImagePath;
      char* confPath;
      char* logoSuffix;
      char* logoPath;
      char* diaPath;
      char* diaPathCurrent;
      char* scraper2VdrPath;
      int logoNotLower;
      int logoById;
      int clientOsdTimeout;
      int startBrowser;
      char* browserDisplay;
      char* tvIp;
      bool mainmenuVisible;

      int diaCycleTime;            // in seconds
      char* diaExtensions;
      int diaRandom;
};

extern cOsd2WebConfig config;

#endif // __O2W_CONFIG_H
