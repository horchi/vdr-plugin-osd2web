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
DISP="$2"
BROWSER="/usr/bin/firefox"

SKIN="horchiTft"
THEME="graycd"

LIST="xxx Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec"
ARRAY=($LIST)
SNOW_MONTHS="Nov Dec Jan Feb"
SANTA_MONTHS="Dec"

SEND_F11="yes"
PAUSE_BEFORE_KEY=4

#if [ "$1" == "F11" ]; then
#    sleep $PAUSE_BEFORE_KEY
#
#    for id in `xdotool search --onlyvisible --name firefox`; do
#        xdotool key --window "$id" "F11"
#    done
#
#    exit 0
#fi

[ -r "$MYPATH/browser.conf" ] && . "$MYPATH/browser.conf"

URL="$BASE/$SKIN/index.html?theme=$THEME&onlyView"

if [[ "$SNOW_MONTHS" != "${ARRAY[$MONTH]}" ]] ; then
    if [[ "$SANTA_MONTHS" != "${ARRAY[$MONTH]}" ]] ; then
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

#if [ "$SEND_F11" == "yes" ]; then
#    echo "starting $0"
#    (DISPLAY="$DISP" "$0" "F11" &)
#fi

DISPLAY="$DISP" "$BROWSER" "$URL_OPT" "$URL" > /dev/null 2>&1 &
pid=$!
trap "kill -TERM $pid" TERM

if [ "$SEND_F11" == "yes" ]; then

   sleep $PAUSE_BEFORE_KEY

   for id in `xdotool search --onlyvisible --name firefox`; do
       xdotool key --window "$id" "F11"
   done

fi

wait $pid

exit 0
