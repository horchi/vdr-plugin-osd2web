#!/bin/bash

#
# Changes settings in browser.conf instead of here
# Skript may overwritten at update!
#
# don't toch below
#

MYPATH="`dirname \"$0\"`"
MONTH=`date +%m`
BASE="$1"

SKIN="horchiTft"
THEME="graycd"

SNOW_START=11
SNOW_END=2
SANTA_MONTH=12

[ -r "$MYPATH/browser.conf" ] && . "$MYPATH/browser.conf"

URL="$BASE/$SKIN/index.html?theme=$THEME&onlyView"

if [ $MONTH -ge $SNOW_START ] || [ $MONTH -le $SNOW_END ]; then
    if [ $MONTH = $SANTA_MONTH ]; then
        URL="$URL&xsnow=1"          # with snow
    else
        URL="$URL&xsnow=1&santa=1"  # with snow and sanata
    fi
fi

DISPLAY=:1.1 /usr/bin/midori -a "$URL"

exit 0
