#!/bin/sh
# play a sound file
#echo $1
if test -x /usr/bin/aplay; then
  /usr/bin/aplay $1 2> /dev/null
elif test -x /usr/bin/paplay; then
  /usr/bin/paplay $1 2> /dev/null
elif test -x /usr/demo/SOUND/play; then
  # SUNOS 4.1.3 */
  /usr/demo/SOUND/play $1 2> /dev/null
elif test -x /usr/sbin/sfplay; then
  # IRIX 5.3
  /usr/sbin/sfplay $1 2> /dev/null
elif test -x /usr/bin/mme/decsound; then
  # Digital Unix with Multimedia Services
  /usr/bin/mme/decsound -play $1 2> /dev/null
elif test -c /dev/audio; then
  cat $1 > /dev/audio 2> /dev/null
elif test -c /dev/dsp; then
  cat $1 > /dev/dsp 2> /dev/null
elif test -c /dev/dsp1; then
  cat $1 > /dev/dsp1 2> /dev/null
elif test -c /dev/dsp2; then
  cat $1 > /dev/dsp2 2> /dev/null
fi
