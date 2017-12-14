#!/bin/bash

#
# Change settings (SKIN/THEME, SONW/SANTA) in browser.conf instead here!
# Skript may overwritten at update!
#
# don't touch below
#

MYPATH="`dirname \"$0\"`"
MONTH=`date +%m`
BASE="$1"

SKIN="horchiTft"
THEME="graycd"

SNOW_MONTHS="Nov Dec Jan Feb"
SANTA_MONTHS="Dec"

[ -r "$MYPATH/browser.conf" ] && . "$MYPATH/browser.conf"

URL="$BASE/$SKIN/index.html?theme=$THEME&onlyView"

if [[ "$SNOW_MONTHS" != "${LIST/$MONTH/}" ]] ; then
    if [[ "$SANTA_MONTHS" != "${LIST/$MONTH/}" ]] ; then
        URL="$URL&xsnow=1&santa=1"  # with snow and sanata
    else
        URL="$URL&xsnow=1"          # with snow
    fi
fi

DISPLAY=:1.1 /usr/bin/midori -a "$URL"

exit 0
