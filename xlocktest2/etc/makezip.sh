#!/bin/sh
VERSION=`grep VERSION xlock/version.h | cut -d\" -f2` 2> /dev/null
PARENT="xlockmore"
NAME="xlock95"
PROG="${NAME}-${VERSION}"
PROGZIP="${PROG}.zip"
BINARY="${NAME}.scr"
TEXT="${NAME}.txt"
AUTHOR="David Bagley"

if test "${VERSION}" = ""; then
	echo "run from $PARENT main directory"
	exit
fi
rm -f $BINARY
make -f Makefile.win32 small
mkdir -p $PROG
mv $NAME.scr $PROG
echo "$BINARY built by $AUTHOR from $PARENT-$VERSION

To install just drop $BINARY in your C:\\Windows\\ or C:\\Windows\\System\\

Set \"$NAME\" to be your screen saver. When you enter the
Screen Saver tab in the Display Properties it may take a little
while for control to come back to you if $NAME is set.
If it does not show up the scr file is probably in the wrong directory.
Run with \"-s mode\" to run with a particular mode.

The rest is the readme file that comes with the source code...
-----------" > $PROG/$TEXT
cat win32/readme.txt >> $PROG/$TEXT
#Much care as 2 unix2dos commands that I know of.  This will work on both.
cp $PROG/$TEXT $PROG/$TEXT.old
unix2dos $PROG/$TEXT.old $PROG/$TEXT
rm -rf $PROG/$TEXT.old
zip -r $PROGZIP $PROG
rm -rf $PROG
mv $PROGZIP ..
echo "run signzip.sh next"
