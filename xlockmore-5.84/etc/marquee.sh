#!/bin/bash

if [ -z "$1" ]
    then message="Go ahead and log me out if you want to."
else
    message="$1"
fi

# Wrap the message at 30 chars so it doesn't run off the screen.
message="$(echo $message | fmt -w 30)
=====================
Locked by $LOGNAME at:
$(date '+%a %b %d %r')"

xlock -mode marquee \
 -size 1 \
 -messagefont "-*-courier-*-r-*-*-34-*-*-*-*-*-*-*" \
 -message "$message" \
 -logoutButtonHelp "^ LOGOUT BUTTON ^

$message" \
 -logoutButton 1
