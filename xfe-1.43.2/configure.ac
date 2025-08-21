#
# Script for autoconf 2.69 or later and automake 1.14 or later
#

# Initialize
AC_INIT([xfe], [1.43.2])
AC_CONFIG_SRCDIR([src/XFileExplorer.cpp])
AM_INIT_AUTOMAKE([subdir-objects -Wall])
AM_CONFIG_HEADER([config.h])

# Test if compilation variables are already set and if not, reset them
# This mechanism prevents these variables to be changed by the following AC macros
# while still allowing to use user's variables if they are defined
if test "$CXXFLAGS" = ""; then
CXXFLAGS=""
fi
if test "$CFLAGS" = ""; then
CFLAGS=""
fi

# Minimal LIBS
LIBS="$LIBS -lX11"

AC_USE_SYSTEM_EXTENSIONS

# Checks for programs
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
PKG_PROG_PKG_CONFIG


# Internationalization
IT_PROG_INTLTOOL
GETTEXT_PACKAGE=xfe
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE",[The package name, for gettext])
AM_GNU_GETTEXT

# Checks for header files.
AC_PATH_X
AC_FUNC_ALLOCA
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h mntent.h stdlib.h string.h sys/ioctl.h sys/statfs.h sys/time.h unistd.h utime.h])


# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_HEADER_TIME
AC_STRUCT_TM


# Checks for library functions.
AC_FUNC_CHOWN
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_GETGROUPS
AC_FUNC_GETMNTENT
AC_FUNC_LSTAT
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_FUNC_STAT
AC_FUNC_UTIME_NULL
AC_CHECK_FUNCS([endgrent endpwent gethostname getmntent gettimeofday lchown memset mkdir mkfifo putenv rmdir setlocale sqrt strchr strdup strerror strstr strtol strtoul strtoull utime])


# Large files support
AC_SYS_LARGEFILE


# Check for FOX 1.6
AC_CHECK_LIB(FOX-1.6,fxfindfox,,AC_MSG_ERROR("libFOX-1.6 not found"))


# Check for FOX 1.6 header files
AC_HEADER_STDC
AC_LANG_CPLUSPLUS
AC_CHECK_HEADER(fox-1.6/fx.h,,AC_MSG_ERROR("fox-1.6/fx.h not found"))


# Check if fox-config exists
AC_CHECK_PROGS(FOX_CONFIG,fox-config-1.6 fox-1.6-config fox-config)
if test no"$FOX_CONFIG" = no ; then
	AC_MSG_ERROR("fox-config not found")               
fi


# Include flags for the FOX library
FOXCFLAGS=`$FOX_CONFIG --cflags`
CXXFLAGS="${CXXFLAGS} $FOXCFLAGS"


# Check if FOX was compiled with xft support
TEST_XFT=`$FOX_CONFIG --libs | grep Xft`
if test "x$TEST_XFT" != "x" ; then
	
	echo "checking whether FOX was compiled with Xft support... yes"

	# Check for FreeType2
	PKG_CHECK_MODULES(FREETYPE, freetype2, [
		freetype_cflags="$FREETYPE_CFLAGS"
		freetype_libs="$FREETYPE_LIBS"
		LIBS="$LIBS $freetype_libs"
		CPPFLAGS="$freetype_cflags $CPPFLAGS"
	], AC_MSG_ERROR("freetype not found"))

	# Check for Xft headers
	xft_config=''
	AC_CHECK_PROGS(xft_config,xft-config,)
	if test -n "$xft_config"; then
	  xft_cflags=`$xft_config --cflags`
	  xft_libs=`$xft_config --libs`
	  LIBS="$LIBS $xft_libs"
	  CPPFLAGS="$xft_cflags $CPPFLAGS"
	  CXXFLAGS="${CXXFLAGS} -DHAVE_XFT_H"
	else
		# On some systems xft-config is deprecated and pkg-config should be used instead
		PKG_CHECK_MODULES(XFT, xft, [
			xft_cflags="$XFT_CFLAGS"
			xft_libs="$XFT_LIBS"
			LIBS="$LIBS $xft_libs"
			CPPFLAGS="$xft_cflags $CPPFLAGS"
			CXXFLAGS="$CXXFLAGS -DHAVE_XFT_H"
		], AC_MSG_ERROR("Xft not found"))
	fi
	AC_CHECK_HEADER(X11/Xft/Xft.h,,AC_MSG_ERROR("Xft.h not found"))

else
	echo "checking whether FOX was compiled with Xft support... no"
	echo ""
	echo "===================================== Error! ================================================"
	echo "Configure has detected that your FOX library was compiled without Xft support."
	echo "Since Xfe version 1.42, Xft is mandatory and FOX *must* have been compiled with Xft support."
	echo "To enable Xft support in FOX, rebuild the FOX library using the following commands:"
	echo "   ./configure --with-xft"
	echo "   make"
	echo "   sudo make install"
	echo "============================================================================================="
	echo ""
	AC_MSG_ERROR("missing Xft support in FOX")               
fi


# Check for Xlib headers
AC_CHECK_HEADER(X11/Xlib.h,,AC_MSG_ERROR("Xlib.h not found"))

# Check for XRandR support
AC_MSG_CHECKING(for xrandr extension)
AC_ARG_WITH(xrandr,[  --with-xrandr           compile with XRandR support])
AC_MSG_RESULT([$with_xrandr])
if test "x$with_xrandr" != "xno"; then
AC_CHECK_HEADERS(X11/extensions/Xrandr.h,CXXFLAGS="${CXXFLAGS} -DHAVE_XRANDR_H=1"; LIBS="${LIBS} -lXrandr")
fi

# Check for libPNG
AC_CHECK_LIB(png, png_read_info,,AC_MSG_ERROR("libPNG not found"))
AC_CHECK_HEADER(png.h,,AC_MSG_ERROR("png.h not found"))

# Check for fontconfig
AC_CHECK_LIB(fontconfig, FcInit,, AC_MSG_ERROR("fontconfig not found"))
AC_CHECK_HEADER(fontconfig/fontconfig.h,,AC_MSG_ERROR("fontconfig.h not found"))

# Check for startup notification support
AC_MSG_CHECKING(for startup notification)
AC_ARG_ENABLE(sn,[  --disable-sn            compile without startup notification support])
AC_MSG_RESULT([$enable_sn])
AC_SUBST(STARTUPNOTIFY,false)
if test "x$enable_sn" != "xno"; then
	CXXFLAGS="${CXXFLAGS} -DSTARTUP_NOTIFICATION"
	AC_SUBST(STARTUPNOTIFY,true)
	enable_sn=yes

	# Check for xcb libs
	PKG_CHECK_MODULES([xcb], [xcb >= 1.6],,
		[AC_MSG_ERROR([Cannot find xcb])])
	PKG_CHECK_MODULES([xcb_aux], [xcb-aux],,
		[AC_MSG_ERROR([Cannot find xcb-aux])])
	PKG_CHECK_MODULES([xcb_event], [xcb-event],,
		[AC_MSG_ERROR([Cannot find xcb-event])])
	PKG_CHECK_MODULES([x11_xcb], [x11-xcb],,
		[AC_MSG_ERROR([Cannot find x11-xcb])])
	LIBS="$LIBS $xcb_LIBS $xcb_aux_LIBS $x11_xcb_LIBS"
fi
AM_CONDITIONAL(STARTUPNOTIFY, [test x$enable_sn = xyes])


# Building for debugging
AC_MSG_CHECKING(for debugging)
AC_ARG_ENABLE(debug,[  --enable-debug          compile for debugging])
AC_MSG_RESULT([$enable_debug])

# Add debug symbols
AC_MSG_CHECKING(minimalflags)
AC_ARG_ENABLE(minimalflags,[  --enable-minimalflags   respect system flags as much as possible])
AC_MSG_RESULT([$enable_minimalflags])

# Building for release
AC_MSG_CHECKING(for release build)
AC_ARG_ENABLE(release,[  --enable-release        compile for release (advanced optimizations)])
AC_MSG_RESULT([$enable_release])


if test "x$enable_minimalflags" = "xyes" ; then
	if test "x$enable_debug" = "xyes" ; then
		CPPFLAGS="$CPPFLAGS -DDEBUG"
	else
		CPPFLAGS="$CPPFLAGS -DNDEBUG"
	fi
else
	# Setting CXXFLAGS
	if test "x$enable_debug" = "xyes" ; then
		CXXFLAGS="${CXXFLAGS} -Wall -std=gnu++98 -fno-strict-aliasing -pg -g -DDEBUG"
	elif test "x$enable_release" = "xyes" ; then
		CXXFLAGS="-DNDEBUG ${CXXFLAGS} "
		if test "${GXX}" = "yes" ; then
			CXXFLAGS="-O3 -Wall -std=gnu++98 -ffast-math -fomit-frame-pointer -fno-strict-aliasing ${CXXFLAGS}"
		fi
	else
		CXXFLAGS="-O3 -std=gnu++98 -ffast-math -Wall ${CXXFLAGS}"
	fi


	# Setting CFLAGS
	if test "x$enable_debug" = "xyes" ; then
		CFLAGS="${CFLAGS} -Wall -g -DDEBUG"
	elif test "x$enable_release" = "xyes" ; then
		CFLAGS="-DNDEBUG ${CFLAGS}"
		if test "${GCC}" = "yes" ; then
			CFLAGS="-O3 -Wall -ffast-math -fomit-frame-pointer -fno-strict-aliasing ${CFLAGS}"
		fi
	else
		CFLAGS="-O3 -ffast-math -Wall ${CFLAGS}"
	fi
fi


# Output
AC_OUTPUT(Makefile intl/Makefile m4/Makefile po/Makefile.in xfe.spec xferc xfe.desktop.in xfi.desktop.in
          xfw.desktop.in xfp.desktop.in src/Makefile
          icons/Makefile icons/xfe-theme/Makefile icons/gnome-theme/Makefile
          icons/gnomeblue-theme/Makefile icons/windows-theme/Makefile icons/windowsxp-theme/Makefile
		  icons/tango-theme/Makefile icons/kde-theme/Makefile)


# Display CXXFLAGS, CFLAGS and LIBS
echo ""
echo "======================== Compiler and linker flags ========================"
echo "CXXFLAGS=$CXXFLAGS"
echo "CFLAGS=$CFLAGS"
echo "LIBS=$LIBS"
echo "==========================================================================="

echo ""
echo "Configure finished!"
echo "              Do:  'make' to compile Xfe."
echo "            Then:  'make install' (as root) to install Xfe."
echo ""


