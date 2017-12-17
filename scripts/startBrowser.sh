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

BROWSER="/usr/bin/firefox"

SKIN="horchiTft"
THEME="graycd"

SNOW_MONTHS="Nov Dec Jan Feb"
SANTA_MONTHS="Dec"

SEND_F11="yes"
PAUSE_BEFORE_KEY=4

if [ "$1" == "F11" ]; then
    sleep $PAUSE_BEFORE_KEY

    for id in `xdotool search --onlyvisible --name firefox`; do
        xdotool key --window "$id" "F11"
    done

    exit 0
fi

[ -r "$MYPATH/browser.conf" ] && . "$MYPATH/browser.conf"

URL="$BASE/$SKIN/index.html?theme=$THEME&onlyView"

if [[ "$SNOW_MONTHS" != "${LIST/$MONTH/}" ]] ; then
    if [[ "$SANTA_MONTHS" != "${LIST/$MONTH/}" ]] ; then
        URL="$URL&xsnow=1&santa=1"  # with snow and sanata
    else
        URL="$URL&xsnow=1"          # with snow
    fi
fi

if [[ "$BROWSER" =~ "firefox" ]]; then
    URL_OPT="-url"
else
    URL_OPT="-a"
fi

if [ "$SEND_F11" == "yes" ]; then
    echo "starting $0"
    (DISPLAY=:1.1 "$0" "F11" &)
fi

DISPLAY=:1.1 "$BROWSER" "$URL_OPT" "$URL" > /dev/null 2>&1

#if [ "$SEND_F11" == "yes" ]; then
#
#   sleep $PAUSE_BEFORE_KEY
#
#   for id in `xdotool search --onlyvisible --name firefox`; do
#       xdotool key --window "$id" "F11"
#   done
#
#fi

exit 0
