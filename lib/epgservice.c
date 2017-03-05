/*
 * epgservice.c
 *
 * See the README file for copyright information and how to reach the author.
 *
 */

#include "epgservice.h"

//***************************************************************************
// Timer  State / Action
//***************************************************************************

const char* toName(TimerState s)
{
   switch (s)
   {
      case tsPending:  return "pending";
      case tsRunning:  return "running";
      case tsFinished: return "finished";
      case tsDeleted:  return "deleted";
      case tsError:    return "failed";
      case tsIgnore:   return "ignore";
      case tsUnknown:  return "unknown";
   }

   return "unknown";
}

const char* toName(TimerAction a, int nice)
{
   switch (a)
   {
      case taCreate:  return "create";
      case taModify:  return "modify";
      case taAdjust:  return "adjust";
      case taDelete:  return "delete";
      case taAssumed: return nice ? "-" : "assumed";
      case taFailed:  return "failed";
   }

   return nice ? "-" : "unknown";
}

//***************************************************************************
// cEpgdState
//***************************************************************************

const char* cEpgdState::states[] =
{
   "init",
   "standby",
   "stopped",

   "busy (events)",
   "busy (match)",
   "busy (scraping)",
   "busy (images)",

   0
};

const char* cEpgdState::toName(cEpgdState::State s)
{
   if (!isValid(s))
      return "unknown";

   return states[s];
}

cEpgdState::State cEpgdState::toState(const char* name)
{
   for (int i = 0; i < esCount; i++)
      if (strcmp(states[i], name) == 0)
         return (State)i;

   return esUnknown;
}

//***************************************************************************
// Field Filter
//***************************************************************************

FieldFilterDef fieldFilters[] =
{
   { ffAll,         "all"     },
   { ffEpgd,        "epgd"    },
   { ffEpgHttpd,    "httpd"   },
   { ffEpg2Vdr,     "epg2vdr" },
   { ffScraper2Vdr, "scraper" },

   { 0,             0 }
};

const char* toName(FieldFilter f)
{
   for (int i = 0; fieldFilters[i].name; i++)
      if (fieldFilters[i].filter == f)
         return fieldFilters[i].name;

   return "unknown";
}

int toFieldFilter(const char* name)
{
   for (int i = 0; fieldFilters[i].name; i++)
      if (strcasecmp(fieldFilters[i].name, name) == 0)
         return fieldFilters[i].filter;

   return ffAll;
}

//***************************************************************************
// User Rights Check
//***************************************************************************

int hasUserMask(unsigned int rights, UserMask mask)
{
   return rights & mask;
}
