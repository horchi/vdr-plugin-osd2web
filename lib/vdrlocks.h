/*
 * vdrlocks.h
 *
 * See the README file for copyright information and how to reach the author.
 *
 */

#ifndef __VDRLOCKS_H
#  define __VDRLOCKS_H

#  ifdef VDR_PLUGIN

#  include <vdr/config.h>
#  include <vdr/tools.h>

//***************************************************************************
// Use Loack always in the following sequence !!
//***************************************************************************

//***************************************************************************
// #1 Timer List Lock Macros
//***************************************************************************

#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
#  define GET_TIMERS_READ(name) USE_LIST_LOCK_READ(Timers);  \
   const cTimers* name = Timers;
#else
#  define GET_TIMERS_READ(name) cTimers* name = &Timers;
#endif

#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
#  define GET_TIMERS_WRITE(name) USE_LIST_LOCK_WRITE(Timers);   \
   cTimers* name = Timers;
#else
#  define GET_TIMERS_WRITE(name) cTimers* name = &Timers;
#endif

//***************************************************************************
// #2 Channel List Lock Macros
//***************************************************************************

#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
#  define GET_CHANNELS_READ(name) USE_LIST_LOCK_READ(Channels);  \
   const cChannels* name = Channels;
#else
#  define GET_CHANNELS_READ(name) cChannels* name = &Channels;
#endif

#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
#  define GET_CHANNELS_WRITE(name) USE_LIST_LOCK_WRITE(Channels);   \
   cChannels* name = Channels;
#else
#  define GET_CHANNELS_WRITE(name) cChannels* name = &Channels;
#endif

//***************************************************************************
// #3 Recording List Lock Macros
//***************************************************************************

#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
#  define GET_RECORDINGS_READ(name) USE_LIST_LOCK_READ(Recordings);  \
   const cRecordings* name = Recordings;
#else
#  define GET_RECORDINGS_READ(name) cRecordings* name = &Recordings;
#endif

#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
#  define GET_RECORDINGS_WRITE(name) USE_LIST_LOCK_WRITE(Recordings);   \
   cRecordings* name = Recordings;
#else
#  define GET_RECORDINGS_WRITE(name) cRecordings* name = &Recordings;
#endif

//***************************************************************************
// #4 Schedules List Lock Macros
//***************************************************************************

#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
#  define GET_SCHEDULES_READ(name) USE_LIST_LOCK_READ(Schedules);  \
   const cSchedules* name = Schedules;
#else
#  define GET_SCHEDULES_READ(name)  \
   cSchedulesLock schedulesLock;  \
   const cSchedules* name = (cSchedules*)cSchedules::Schedules(schedulesLock);
#endif

//***************************************************************************
#  endif // VDR_PLUGIN

#endif //___VDRLOCKS_H
