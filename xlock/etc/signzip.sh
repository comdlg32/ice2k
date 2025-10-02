#!/bin/sh
VERSION=`grep VERSION xlock/version.h | cut -d\" -f2`
NAME="xlock95"
PROG="${NAME}-${VERSION}"
PROGZIP="${PROG}.zip"

#./makezip.sh
cd ..
#gpg --print-md SHA512 $PROGZIP > $PROGZIP.sha
sha512sum --tag $PROGZIP > $PROGZIP.sha512
sha512sum -c $PROGZIP.sha512
gpg --armor --output $PROGZIP.asc --detach-sig $PROGZIP
gpg --verify $PROGZIP.asc $PROGZIP
