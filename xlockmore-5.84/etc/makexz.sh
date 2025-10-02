#!/bin/sh
VERSION=`grep VERSION xlock/version.h | cut -d\" -f2` 2> /dev/null
NAME="xlockmore"
PROG="${NAME}-${VERSION}"
AUTHOR="David Bagley"

if test "$1" = "-p"; then
	PROG=${NAME}
fi
PROGZIP="${PROG}.tar.xz"
if test "${VERSION}" = ""; then
	echo "run from $NAME main directory"
	exit
fi
#make -f Makefile.win32 distclean
make -f Makefile.in distclean
mkdir -p $PROG
LIST=`ls`
EXCEPT="linux solaris cygwin $PROG"
for i in $LIST; do
	FOUND=0
	for j in $EXCEPT; do
		if test "$i" = "$j"; then
			FOUND=1
			break
		fi
	done
	if test $FOUND -eq 0; then
		cp -rp $i $PROG
	fi
done
tar cvf - $PROG | xz > $PROGZIP
rm -rf $PROG
mv $PROGZIP ..
if test "$1" != "-p"; then
	echo "run signxz.sh next"
fi
