/*
 * ----------------------------------------------
 * osd2web Plugin - Revision History
 * ----------------------------------------------
 *
 *  (c) 2017-2018 Dirk Hermes, Jörg Wendel
 *
 */

#define _VERSION      "0.2.54"
#define VERSION_DATE  "20.08.2020"

#ifdef GIT_REV
#  define VERSION _VERSION "-GIT" GIT_REV
#else
#  define VERSION _VERSION
#endif

/*
 * ------------------------------------

2020-08-20: version 0.2.54 (horchi)
   - Bugfix: Fixed mem leak of version 0.2.53 and old libwebsock v2.*

2020-08-18: version 0.2.53 (horchi)
   - added: Support of libwebsock 4 and 4.1

2020-06-21: version 0.2.52 (horchi)
   - added: WS client type 'ctFB' for remote control (only) clients

2020-05-05: version 0.2.51 (horchi)
   - bugfix: fixed memory problem with json (patch by Alexander Grothe)

2019-12-22: version 0.2.50 (horchi)
   - change: backward compatibility to 2.*

2019-12-21: version 0.2.49 (horchi)
   - change: libwebsockets 3.2 porting

2018-10-13: version 0.2.48 (horchi)
   - change: Improved menu display for skin horchiTft

2018-10-12: version 0.2.47 (horchi)
   - bugfix: Fixed menu line count handling

2018-10-10: version 0.2.46 (horchi)
   - change: Improved menue handling for passive clients

2018-10-08: version 0.2.45 (rechner)
   - change: switch on/off for maxline
   - fix layout for OSD

2018-10-07: version 0.2.44 (rechner)
   - change: calculate the maxLines dynamicly

2018-10-02: version 0.2.43 (horchi)
   - change: Fixed date alignment for kiosk browser (statusbar)

2018-10-01: version 0.2.42 (horchi)
   - change: Adjusted npm dependencies

2018-09-29: version 0.2.41 (horchi)
   - change: Fixed progressbar height for kiosk browser

2018-09-29: version 0.2.40 (horchi)
   - change: Adjusted log level for debug message

2018-08-26: version 0.2.39 (horchi)
   - added: Folder 'dia' now optional

2018-06-01: version 0.2.38 (horchi)
   - added: Browser restart via menu

2018-04-03: version 0.2.37 (rechner)
   - bugfix: dia animations

2018-03-03: version 0.2.36 (horchi)
   - added: Dia settings menu

2018-02-28: version 0.2.35 (horchi)
   - added: OSD menu to select start folder of dia show

2018-02-26: version 0.2.34 (horchi)
   - change: fixed image scale of dia show

2018-02-22: version 0.2.33 (horchi)
   - added: bitrade to RDS info (Thx to Uli Eckhardt for all the radio plugin extensions)

2018-02-20: version 0.2.32 (horchi)
   - change: improved rds view

018-02-19: version 0.2.31 (horchi)
   - added: Support of RDS data provided by radio plugin

2018-02-17: version 0.2.30 (horchi)
   - change: porting to epg2vdr service interface change

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
