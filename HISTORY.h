/*
 * ----------------------------------------------
 * osd2web Plugin - Revision History
 * ----------------------------------------------
 *
 *  (c) 2017 Dirk Hermes, Jörg Wendel
 *
 */

#define _VERSION      "0.1.4"
#define VERSION_DATE  "06.06.2017"

#ifdef GIT_REV
#  define VERSION _VERSION "-GIT" GIT_REV
#else
#  define VERSION _VERSION
#endif

/*
 * ------------------------------------

2017-06-06: version 0.1.4 (horchi)
   - Change: Further less lib dependencies
   - Bugfix: Fixed compile problem

2017-06-06: version 0.1.3 (horchi)
   - Bugfix: Fixed possible crash at replay

2017-06-06: version 0.1.2 (horchi)
   - Change: removed unused mysql dependency

2017-06-02: version 0.1.1 (horchi)
   - Bugfix: Fxed possible crash

2017-06-01: version 0.1.0 (horchi)
   - Added: First official Beta Release

2017-05-03: version 0.0.3 (horchi)
   - Change: VDR 2.3.4 porting

2017-02-17: version 0.0.2 (rechner)
   - Initial version client

2017-02-13: version 0.0.1 (horchi)
   - Initial version

 * ------------------------------------
 */
