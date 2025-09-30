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
libpci-dev feh tk imagemagick libgtk2.0-dev \
|| exit


if [ "$ONLYTOOLS" -eq 1 ]; then
	exit
fi


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



simplebuild i2klibs/ini
simplebuild i2klibs/branding
simplebuild i2klibs/comctl32
simplebuild games/winmine
simplebuild mmc/devmgmt
simplebuild ice2kver
simplebuild batmeter
simplebuild shutdown
simplebuild msgina
simplebuild shell/hotplug
simplebuild shell/backmgr
simplebuild control/sysdm
simplebuild control/desk

(
cd icewm-3.3.1
libtoolize --force
aclocal
autoheader
autoupdate
automake --add-missing --force-missing --copy --foreign
autoreconf -vif
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

(
cd yad-*

aclocal
autoheader
autoupdate
automake --add-missing --force-missing --copy --foreign
autoreconf
set -e
./configure --with-gtk=gtk2
make clean
make -j$(nproc)
sudo make install
set +e
) || exit

(
cd cbatticon

make clean
make
sudo make install
) || exit



mv ~/.icewm ~/.icewm-old
cp -rf icewm-dir ~/.icewm
cp -rf .config ~/
cp -rf .foxrc ~/
cp -rf .idesktop ~/
cp -rf .icons ~/
cp -rf .themes ~/

sudo cp winvga-12.pcf /usr/share/fonts/X11/misc

# from wine
sudo cp tahoma*.ttf /usr/share/fonts/truetype
sudo cp tahoma*.pcf.gz /usr/share/fonts/X11/100dpi

sudo update-fonts-dir misc
sudo update-fonts-alias misc
sudo update-fonts-dir 100dpi
sudo update-fonts-alias 100dpi
