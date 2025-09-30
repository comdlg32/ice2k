#!/bin/sh
cd "$(dirname -- "$(readlink -f -- "$0"; )")"
export INSTALLSCRDIR="$PWD"

sudo apt-get install --no-install-recommends --no-install-suggests \
autotools-dev automake libtool gettext gettext-base gettext-el gnulib autopoint \
xserver-xorg x11-apps twm xserver-xorg-dev libfontconfig-dev \
libxrender-dev libxcomposite-dev libxdamage-dev libxfixes-dev libxext-dev libx11-dev \
libsm-dev libice-dev libxrandr-dev libxft-dev libxpm-dev libjpeg-dev \
libpng-dev libfontconfig1-dev libxinerama-dev libfribidi-dev libimlib2-dev xterm \
x11-utils xinit intltool intltool-debian libxcb-util-dev libx11-xcb-dev \
build-essential libfox-1.6-dev psmisc libxtst-dev libserialport-dev \
libpci-dev feh \
|| exit


if [ "$ONLYTOOLS" -eq 1 ]; then
	exit
fi

(
cd icewm-3.3.1
libtoolize --force
aclocal
autoheader
autoupdate
automake --add-missing --force-missing --copy --foreign
autoreconf
set -e
./configure
make clean
make -j$(nproc)
sudo make install
set +e
) || exit

(
cd xfe-*

aclocal
autoheader
autoupdate
automake --add-missing --force-missing --copy --foreign
autoreconf
set -e
./configure
make clean
make -j$(nproc)
sudo make install
set +e
) || exit

simplebuild()
{
	(
	set -e
	cd "$1"
	make clean
	make -j$(nproc)
	sudo make install
	set +e
	) || exit

}

simplebuildõi2klibs/ini
simplebuildõi2klibs/branding
simplebuildõi2klibs/comctl32
simplebuildõgames/winmine
simplebuildõmmc/devmgmt
simplebuildõice2kver
simplebuildõbatmeter
simplebuildõshutdown
simplebuildõmsgina
simplebuildõshell/hotplug
simplebuildõshell/backmgr
simplebuildõcontrol/sysdm
simplebuild control/desk
