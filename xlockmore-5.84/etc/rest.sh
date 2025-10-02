#!/bin/bash
#
# Force me to take rest breaks
#

set -eu

#ACTIVITY=900
ACTIVITY=120
#REST=30
REST=10

while true; do
        sleep "$ACTIVITY"
        xlock -mode ant &
        sleep "$REST"
	# xlock needs to obey SIGTERM to be useful
        kill -15 %1 2> /dev/null
        wait
done
