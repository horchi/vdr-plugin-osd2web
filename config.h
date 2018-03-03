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
      cOsd2WebConfig(const cOsd2WebConfig& other);
      ~cOsd2WebConfig();

      cOsd2WebConfig& operator = (const cOsd2WebConfig& other) { copy(other); return *this; }

      void copy(const cOsd2WebConfig& other);

      void setLogoSuffix(const char* s)     { free(logoSuffix); logoSuffix = sstrdup(s); }
      void setLogoPath(const char* p)       { free(logoPath); logoPath = sstrdup(p); }
      void setEpgImagePath(const char* s)   { free(epgImagePath); epgImagePath = sstrdup(s); }
      void setDiaPath(const char* p)        { free(diaPath); diaPath = sstrdup(p); }
      void setDiaPathCurrent(const char* p) { free(diaPathCurrent); diaPathCurrent = sstrdup(p); }
      void setDiaExtensions(const char* s)  { free(diaExtensions); diaExtensions = sstrdup(s); }
      void setScaper2VdrPath(const char* p) { free(scraper2VdrPath); scraper2VdrPath = sstrdup(p); }
      void setTvIp(const char* p)           { free(tvIp); tvIp = sstrdup(p); }
      void setHttpPath(const char* p)       { free(httpPath); httpPath = sstrdup(p); }
      void setConfPath(const char* p)       { free(confPath); confPath = sstrdup(p); }

      void setBrowser(const char* p, int autoStart = na);

      int webPort;
      int clientOsdTimeout;
      int logoNotLower;
      int logoById;

      char* httpPath;
      char* confPath;
      char* logoPath;
      char* logoSuffix;
      char* scraper2VdrPath;
      char* epgImagePath;
      int startBrowser;
      char* browserDisplay;
      char* tvIp;
      bool mainmenuVisible;

      char* diaPath;
      char* diaPathCurrent;
      int diaCycleTime;            // in seconds
      char* diaExtensions;
      int diaRandom;
};

extern cOsd2WebConfig config;

#endif // __O2W_CONFIG_H
