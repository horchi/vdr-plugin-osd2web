/*
 * radio.h: OSD2WEB plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 */

#ifndef _RADIO_SERVICE_H_
#define _RADIO_SERVICE_H_

//***************************************************************************
// RADIO - RDS Service
//***************************************************************************

#define RT_MEL 65

struct RadioTextService_v1_0 {
   int rds_info;           // 0= no / 1= Text / 2= Text + RTplus-Tags (Item,Artist)
   int rds_pty;            // 0-31
   char *rds_text;
   char *rds_title;
   char *rds_artist;
   struct tm *title_start;
};

struct RadioTextService_v1_1 {
   int rds_info;           // 0= no / 1= Text / 2= Text + RTplus-Tags (Item,Artist)
   int rds_pty;            // 0-31
   std::string rds_pty_info;
   std::string rds_text;
   std::string rds_title;
   std::string rds_artist;
   time_t title_start;
};

#define RADIO_TEXT_SERVICE   "RadioTextService-v1.1"

#endif //  _RADIO_SERVICE_H_
