#!/bin/bash

$BASE="$1"
THEME="graycd"
MONTH=`date +%m`

URL="$BASE?theme=$THEME&onlyView"

# until xsow not working properly
$MONTH=6

if [ $MONTH -ge 11 ] || [ $MONTH -le 2 ]; then
    if [ $MONTH = 12 ]; then
        URL="$URL&xsnow=1"      # with snow
    else
        URL="$URL&xsnow=2"      # with snow and sanata
    fi
fi

DISPLAY=:1.1 /usr/bin/midori -a "$URL"

exit 0
