/*
 * ----------------------------------------------
 * osd2web Plugin - Revision History
 * ----------------------------------------------
 *
 *  (c) 2017 Dirk Hermes, Jörg Wendel
 *
 */

#define _VERSION      "0.2.29"
#define VERSION_DATE  "30.01.2018"

#ifdef GIT_REV
#  define VERSION _VERSION "-GIT" GIT_REV
#else
#  define VERSION _VERSION
#endif

/*
 * ------------------------------------

2018-01-30: version 0.2.29 (horchi)
   - bugfix: Fixed state of channel logos
   - change: Some minor improvements
   - change: Added SVDRP command to set log level and max width of log messages at runtime

2018-01-29: version 0.2.28 (horchi)
   - change: Patch vdr-2.3.8-statusmarksmodified.diff angepasst (Danke Klaus!)
   - change: Display of running 'recording' at replay now as a red dot instead of the red title

2018-01-27: version 0.2.27 (horchi)
   - bugfix: Fixed possible crash on empty event

2018-01-27: version 0.2.26 (horchi)
   - bugfix: Fixed lock sequence issue

2018-01-27: version 0.2.25 (horchi)
   - change: Update of cutting marks during replay (patch vdr-2.3.8-marks-upd.patch is needed!)

2018-01-26: version 0.2.24 (horchi)
   - change: Show recording state during replay (horchiTft)

2018-01-25: version 0.2.23 (horchi)
   - change: backward compatibility to 2.1.0

2018-01-25: version 0.2.22 (horchi)
   - change: Reduced log messages
   - change: Improved logging of libwebsockets
   - bugfix: Fixed problem with keep alive (ping/pong) since libwebsockets 2.4.0
   - change: Minor skin changes

2018-01-24: version 0.2.21 (horchi)
   - change: Improved display channel logo for skin hochiTft

2018-01-23: version 0.2.20 (horchi)
   - change: Improved display of time at hochiTft statusbar
   - change: some minor skin hochiTft improvements

2018-01-22: version 0.2.19 (horchi)
   - added: autoscroll for event title for skin horchiTft

2018-01-15: version 0.2.18 (rechner)
   - added: autoscroll for vertical and horizontal

2018-01-15: version 0.2.17 (horchi)
   - added: Auto attch/detach to skin intertace (by detecting TV state via network)

2018-01-14: version 0.2.16 (horchi)
   - change: Minor change of timing at login sequence

2018-01-13: version 0.2.15 (horchi)
   - added: Display of 'staffel/folge' to horchiTft

2018-01-13: version 0.2.14 (rechner)
   - added: autoScroll

2018-01-13: version 0.2.13 (horchi)
   - bugfix: fixed display of replay length for skins default and horchiTft

2018-01-13: version 0.2.12 (horchi)
   - change: link for http scraper2vdr not necessary anymore

2018-01-03: version 0.2.11 (horchi)
   - added: Scraper data to client communication

2017-12-22: version 0.2.10 (horchi)
   - change: VDR 2.2.0 backporting

2017-12-22: version 0.2.9 (horchi)
   - change: snow now in foreground of all elements
   - added:  svdr commands to start and stop local browser
   - added:  svdr command to switch display of local browser
   - added:  query of recording details by epg2vdr
   - added:  deliver editing marks to client
   - added:  editing marks to replay progress-bar of skin horchiTft
   - bugfix: fixed potential crash on service call due to timing issue

2017-12-16: version 0.2.8 (horchi)
   - added: now use firefox as the default browser for the TFT display

2017-12-14: version 0.2.7 (horchi)
   - added: VDR patch to update replay progress

2017-12-14: version 0.2.6 (horchi)
   - change: Improved config og browser options
   - change: Improved update of progressbar

2017-12-10: version 0.2.5 (horchi)
   - change: Finetuning for TV view of skin horchiTft

2017-12-09: version 0.2.4 (horchi)
   - added: Added file browser.conf

2017-12-09: version 0.2.3 (horchi)
   - bugfix: Fixed start option handling

2017-12-09: version 0.2.2 (horchi)
   - added: Script for browser autostart
            to enable set plugin start option -b (or --browser)

2017-12-07: version 0.2.1 (horchi)
   - bugfix: Fixed compile with VDR 2.2.0

2017-12-07: version 0.2.0 (horchi)
   - change: merged dev branch
   - added:  snow (URL option xsnow=1) thx to Dirk!
   - bugfix: Fixed many skin issues thx to Dirk!
   - change: Updated bootstrab
   - added:  Theme anthraize to skin horchiTft
   - added:  View Mode dia show
   - change: Many changes in skin horchiTft

2017-07-10: version 0.1.22 (horchi)
   - Change: VDR 2.3.8 porting

017-07-03: version 0.1.21 (horchi)
   - change:  some skin improvements

2017-06-29: version 0.1.20 (horchi)
   - change: further skin improvements
   - change: ported default skin

2017-06-28: version 0.1.19 (horchi)
   - added: provide list of last recordings to client
   - added: display last recordings in theme horchiTft
   - added: provide epg images of recordings to client

2017-06-27: version 0.1.18 (horchi)
   - added: EPG image to replay view
   - added: 'CustomData', display all data which found in files located in <pluginconf>/customdata/...
            Line-Syntax: <name> = <value>;  // <optional comment>
   - added: Theme 'bluecd' identical to 'blue' but additional displays the custom-data below the timerlist

2017-06-27: version 0.1.17 (horchi)
   - change: Improved epg image for various browsers
   - bugfix: Fixed epg image slider
   - change: Improved replay view of skin horchiTft

2017-06-26: version 0.1.16 (horchi)
   - added: Send user-data variables of files in folder <pluginconf> to client

2017-06-26: version 0.1.15 (horchi)
   - change: Improved skin horchiTft for midori browser

2017-06-24: version 0.1.14 (rechner)
   - fixed: access from polyfill intl before loaded

2017-06-23: version 0.1.13 (horchi)
   - fixed:  lock sequence
   - fixed:  disappear of replay control
   - change: removed obsolete libarchive
   - change: 'started' to implement replay view for skin horchiTft

2017-06-21: version 0.1.12 (horchi)
   - Change: further improvements of skin horchiTft
              - menu color
              - display running timer in 'red'
              - fixed epg image scaling

2017-06-13: version 0.1.11 (rechner)
   - Change: compileroptions for skins
   - fixed:  skin horchiTft/theme blue missed

2017-06-13: version 0.1.10 (horchi)
   - Change: further improvements of skin horchiTft

2017-06-13: version 0.1.9 (horchi)
   - Bugfix: Fixed possible crash on missing event

2017-06-12: version 0.1.8 (horchi)
   - Change: Improved auto size of theme horchiTft to fit different resolutions

2017-06-09: version 0.1.7 (horchi)
   - Bugfix: Fixed lock sequence

2017-06-09: version 0.1.6 (horchi)
   - Bugfix: Fixed PATCHED switch

2017-06-07: version 0.1.5 (horchi)
   - Added: TFT skin horchi (under development)

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
