/*
 * epgservice.h
 *
 * See the README file for copyright information and how to reach the author.
 *
 */

#ifndef __EPGSERVICE_H
#define __EPGSERVICE_H

#include <list>

#include "common.h"

#define EPG_PLUGIN_SEM_KEY 0x3db00001

//***************************************************************************
// Globals
//***************************************************************************

typedef unsigned long long tEventId;

enum EpgServiceMisc
{
   sizeMaxParameterValue = 150   // should match the field size in parameters table
};

enum FieldFilter
{
   ffAll         = 0xFFFF,
   ffEpgd        = 1,
   ffEpgHttpd    = 2,
   ffEpg2Vdr     = 4,
   ffScraper2Vdr = 8,

   ffCount = 5
};

struct FieldFilterDef
{
   int filter;
   const char* name;
};

const char* toName(FieldFilter f);
int toFieldFilter(const char* name);

enum SearchFields
{
   sfNone               = 0,  // off
   sfTitle              = 1,
   sfFolge              = 2,
   sfDescription        = 4
};

enum SearchMode
{
   smExact = 1,
   smRegexp,
   smLike,
   smContained
};

enum TimerNamingMode
{
   tnmDefault   = 0,     // naming would done by VDR

   // naming of following modes handled by recording.py an can 'configured' there

   tnmAuto        = 1,   // autodetect if 'constabel', 'serie' or 'normal movie'
   tnmConstabel   = 2,   // naming in constabel series style with season, number, ..
   tnmSerie       = 3,   // series style, like Title/Subtitle
   tnmCategorized = 4,   // sorted in sub folders which are auto-named by category
   tnmUser        = 5    // user defined mode 'to implement in recording.py'

};

enum RecordingState
{
   rsFinished  = 'F',
   rsRecording = 'R',
   rsDeleted   = 'D'
};

enum TimerState
{
   tsUnknown  = 'U',
   tsPending  = 'P',
   tsRunning  = 'R',    // timer is recording
   tsFinished = 'F',
   tsDeleted  = 'D',
   tsError    = 'E',
   tsIgnore   = '-'     // ignore in timer menu -> already tooked 'any-VDR' timer
};

const char* toName(TimerState s);

enum TimerAction
{
   taCreate   = 'C',
   taModify   = 'M',
   taAdjust   = 'J',
   taDelete   = 'D',
   taAssumed  = 'A',
   taFailed   = 'F'
};

enum TimerType
{
   ttRecord = 'R',   // Aufnahme-Timer
   ttView   = 'V',   // Umschalt-Timer
   ttSearch = 'S'    // Such-Timer
};

const char* toName(TimerAction a, int nice = no);

enum TimerDoneState
{
   tdsTimerRequested     = 'Q',  // timer already requested by epgd/webif
   tdsTimerCreated       = 'C',  // timer created by VDR
   tdsTimerCreateFailed  = 'f',  // create/delete of timer failed by VDR

   tdsRecordingDone      = 'R',  // Recording finished successfull
   tdsRecordingFailed    = 'F',  // Recording failed

   tdsTimerDeleted       = 'D',  // timer deleted by user
   tdsTimerRejected      = 'J'   // timer rejected due to user action or timer conflict
};

enum UserMask
{
   umNone            = 0x0,

   umNologin         = 0x1,   // the right without a session

   umConfig          = 0x2,
   umConfigEdit      = 0x4,
   umConfigUsers     = 0x8,

   umFree1           = 0x10,
   umFree2           = 0x20,

   umTimer           = 0x40,
   umTimerEdit       = 0x80,
   umSearchTimer     = 0x100,
   umSearchTimerEdit = 0x200,

   umFree3           = 0x400,
   umFree4           = 0x800,

   umFsk             = 0x1000,

   umFree5           = 0x2000,
   umFree6           = 0x4000,

   umRecordings      = 0x8000,
   umRecordingsEdit  = 0x10000,

   umAll             = 0xFFFFFFFF & ~umNologin
};

int hasUserMask(unsigned int rights, UserMask mask);

//***************************************************************************
// cEpgdState
//***************************************************************************

class cEpgdState
{
   public:

      enum State
      {
         esUnknown = na,

         esInit,
         esStandby,
         esStopped,

         // handler pause on this states!

         esBusy,
         esBusyEvents = esBusy,
         esBusyMatch,
         esBusyScraping,

         // handler don't pause on this states!

         esBusyImages,

         esCount
      };

      static const char* toName(State s);
      static State toState(const char* name);
      static int isValid(State s) { return s > esUnknown && s < esCount; }

      static const char* states[];
};

typedef cEpgdState Es;

//***************************************************************************
// cEventState
//***************************************************************************

class cEventState
{
   public:

      enum State
      {
         // add to VDRs EPG => wird von allen VDR angezeigt

         usActive      = 'A', // => Aktiv
         usLink        = 'L', // => DVB Event welches auf ein externes zeigt
         usPassthrough = 'P', // => Durchgeleitetes Event: vdr:000

         // remove from VDRs EPG => Löschsignal wird an alle vdr gesendet

         usChanged     = 'C', // => war mal aktiv wurde jedoch später zum Target,seine eigene ID muss also aus dem epg gelöscht werden. C ist also eine Ausprägung von T
         usDelete      = 'D', // => Sender oder Providerseitig gelöscht
         usRemove      = 'R', // => von uns gelöscht weil wir uns für ein anderes Event entschieden haben, zB eins mit Bildern oder Serieninformationen.

         // don't care for VDRs EPG => werden von den VDRs nicht beachtet und nicht mal gesehen.

         usInactive    = 'I', // => inaktiv
         usTarget      = 'T', // => verknüftes Event zum Link, wird unter seiner ID mithilfe des Link im vdr geführt
         usMergeSpare  = 'S'  // => ersatzevent welches für Multimerge ur Verfügung steht
      };

      // get lists for SQL 'in' statements

      static const char* getDeletable()      { return "'A','L','P','R','I'"; }     // epg plugins
      static const char* getNeeded()         { return "'A','L','P','C','D','R'"; } // epg2vdr
      static const char* getVisible()        { return "'A','L','P'"; }             // epghttpd

      // checks

      static int isNeeded(char c)            { return strchr("ALPCDR", c) != 0; }  // epgd2vdr
      static int isRemove(char c)            { return strchr("CDR", c) != 0; }     // epgd2vdr

};

typedef cEventState Us;   // remove later, not uses anymore

//***************************************************************************
// cUserTimes
//***************************************************************************

class cUserTimes
{
   public:

      enum Mode
      {
         mUnknown = na,
         mNow,
         mNext,
         mTime,
         mSearch
      };

      struct UserTime
      {
         UserTime(const char* strTime, const char* strTitle = 0)
         {
            highlighted = yes;
            mode = mUnknown;
            *hhmmStr = 0;
            title = 0;
            hhmm = 0;
            search = 0;
            setTime(strTime, strTitle);
         }

         UserTime(const UserTime& cp)
         {
            highlighted = cp.highlighted;
            mode = cp.mode;
            hhmm = cp.hhmm;
            strcpy(hhmmStr, cp.hhmmStr);
            title = strdup(cp.title);
            search = cp.search ? strdup(cp.search) : 0;
         }

         ~UserTime()  { free(title); free(search); }

         void setTime(const char* strTime, const char* strTitle)
         {
            hhmm = 0;
            *hhmmStr = 0;

            if (strTime[0] == '!')
            {
               highlighted = no;
               strTime++;
            }

            if (strchr(strTime, ':'))
            {
               hhmm = atoi(strTime) * 100 + atoi(strchr(strTime, ':')+1);
               sprintf(hhmmStr, "%02d:%02d", hhmm / 100, hhmm % 100);
               mode = mTime;
            }
            else if (*strTime == '@')
            {
               highlighted = no;

               if (strcmp(strTime, "@Now") == 0)
                  mode = mNow;
               else if (strcmp(strTime, "@Next") == 0)
                  mode = mNext;
               else
                  mode = mSearch;
            }

            // title

            free(title);
            title = 0;

            if (!isEmpty(strTitle))
               title = strdup(strTitle);
            else if (!isEmpty(hhmmStr))
               asprintf(&title, "%02d:%02d", hhmm / 100, hhmm % 100);

            // search

            if (*strTime == '@')
            {
               free(search);
               search = strdup(strTime+1);
            }
         }

         int getHHMM()            const { return hhmm; }
         const char* getHHMMStr() const { return hhmmStr; }
         const char* getTitle()   const { return title; }
         const char* getSearch()  const { return search; }
         int getMode()            const { return mode; }
         const char* getHelpKey() const { return title; }
         int isHighlighted()      const { return highlighted; }

         time_t getTime()
         {
            struct tm tmnow;
            time_t now = time(0);

            localtime_r(&now, &tmnow);
            tmnow.tm_hour = hhmm / 100;
            tmnow.tm_min = hhmm % 100;
            tmnow.tm_sec = 0;

            time_t ltime = mktime(&tmnow);

            if (ltime < time(0)-tmeSecondsPerHour)
               ltime += tmeSecondsPerDay;

            return ltime;
         }

         int mode;
         int highlighted;
         char* title;
         char* search;
         int hhmm;
         char hhmmStr[5+TB];
      };

      cUserTimes()
      {
         clear();
         it = times.end();
      }

      void clear()
      {
         times.clear();
      }

      void add(const char* strTime, const char* strTitle = 0)
      {
         UserTime ut(strTime, strTitle);
         times.push_back(ut);
      }

      UserTime* first()
      {
         it = times.begin();

         if (it == times.end())
            return 0;

         return &(*it);
      }

      UserTime* next()
      {
         if (it == times.end())
            it = times.begin();
         else
            it++;

         if (it == times.end())
            it = times.begin();

         return &(*it);
      }

      UserTime* getFirst()
      {
         std::list<UserTime>::iterator i;

         i = times.begin();

         return &(*i);
      }

      UserTime* getNext()
      {
         std::list<UserTime>::iterator i = it;

         if (i == times.end())
            i = times.begin();
         else
            i++;

         if (i == times.end())
            i = times.begin();

         return &(*i);
      }

      UserTime* current() { return &(*it); }

   private:

      std::list<UserTime>::iterator it;
      std::list<UserTime> times;
};

//***************************************************************************
// EPG Services
//***************************************************************************

#define EPG2VDR_UUID_SERVICE	"epg2vdr-UuidService-v1.0"

struct Epg2vdr_UUID_v1_0
{
   const char* uuid;
};

#define MYSQL_INIT_EXIT	"Mysql_Init_Exit-v1.0"

enum MysqlInitExitAction
{
   mieaInit = 0,
   mieaExit = 1
};

struct Mysql_Init_Exit_v1_0
{
   MysqlInitExitAction action;
};

#endif // __EPGSERVICE_H
