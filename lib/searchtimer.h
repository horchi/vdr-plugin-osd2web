/*
 * searchtimer.h
 *
 * See the README file for copyright information
 *
 */

#ifndef __SEARCHTIMER_H
#define __SEARCHTIMER_H

#include "common.h"
#include "db.h"
#include "epgservice.h"
#include "json.h"

class Python;

//***************************************************************************
// Search Timer
//***************************************************************************

class cSearchTimer
{
   public:

      cSearchTimer();
      ~cSearchTimer();

      int init(const char* confDir);
      int initDb();
      int exitDb();

      int modified();  // check if a search timer is modified by user
      int updateSearchTimers(int force = yes, const char* reason = "");

      int getSearchMatches(cDbRow* searchTimer, json_t* obj);
      int getDoneFor(cDbRow* searchTimer, cDbRow* useevent, json_t* obj);
      // int checkTimerConflicts(std::string& mailBody);
      // int getUsedTransponderAt(time_t lStartTime, time_t lEndTime, std::string& mailBody);

      int prepareDoneSelect(cDbRow* useeventsRow, int repeatfields, cDbStatement*& select);
      cDbStatement* prepareSearchStatement(cDbRow* searchTimer, cDbTable* db);
      int matchCriterias(cDbRow* searchTimer, cDbRow* event);

   private:

      int createTimer(int id);
      int modifyCreateTimer(cDbRow* timerRow, int& newid);
      // int rejectTimer(cDbRow* timerRow);

      // data

      Python* ptyRecName;

      cDbConnection* connection;

      cDbTable* searchtimerDb;
      cDbTable* useeventsDb;
      cDbTable* timersDoneDb;
      cDbTable* timerDb;
      cDbTable* mapDb;
      cDbTable* vdrDb;

      cDbStatement* selectChannelFromMap;
      cDbStatement* selectDoneTimer;
      cDbStatement* selectActiveSearchtimers;
      cDbStatement* selectSearchtimerMaxModSp;
      cDbStatement* selectAllTimer;
      cDbStatement* selectTimerByEvent;
      // cDbStatement* selectActiveVdrs;
      // cDbStatement* selectConflictingTimers;

      cDbValue startValue;
      cDbValue endValue;

      time_t lastSearchTimerUpdate;

      static int searchField[];
      static const char* searchFieldName[];
      static int repeadCheckField[];
      static const char* repeadCheckFieldName[];

};

//***************************************************************************
#endif // __SEARCHTIMER_H
