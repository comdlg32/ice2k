dnl Process this file with autoconf to produce a configure script.
dnl configure.in for xlockmore.  Various things were taken from xscreensaver's
dnl configure.in --- xscreensaver, Copyright (c) 1997 Jamie Zawinski.
dnl

AC_INIT
AC_CONFIG_SRCDIR([config.h.in])
AC_CANONICAL_HOST
canonical=$host

ac_original_cc="$CC"
AC_PROG_CC

###############################################################################
#
#       Query AX_PTHREAD, and figure out which compiler gets used.
#
###############################################################################

AC_DEFUN([AC_PROG_CC_PTHREAD],
 [have_pthread=no
  with_pthread_req=unspecified

  # AX_PTHREAD is from the GNU Autoconf Archive.
  # https://savannah.gnu.org/projects/autoconf-archive/
  m4_include(ax_pthread.m4)

  # This affects CC, LIBS, and CFLAGS, instead of defining new variables.

  AC_ARG_WITH([pthread],
    [  --without-pthread       do not use POSIX threads],
    [with_pthread="$withval"; with_pthread_req="$withval"],
    [with_pthread=yes])

  if test "$with_pthread" = yes; then
    # AX_PTHREAD might want a different compiler.
    AX_PTHREAD(
     [if test -n "$ac_original_cc"; then
        AC_MSG_CHECKING([if $ac_original_cc supports POSIX threads])
        if test "$CC" = "$PTHREAD_CC"; then
          have_pthread=yes
        else
          have_pthread=no
        fi
        AC_MSG_RESULT($have_pthread)
      else
        have_pthread=yes
      fi

      if test "$CC" = "$PTHREAD_CC" -o -z "$ac_original_cc"; then
        have_pthread=yes
      else
        ac_prog_cc_no_pthread=yes
      fi
    ])

    if test "$have_pthread" = yes; then
      AC_DEFINE([HAVE_PTHREAD], [1],
                [Define this if your system supports POSIX threads.])
      LIBS="$PTHREAD_LIBS $LIBS"
      CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
      CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"
      CC="$PTHREAD_CC"
    elif test "$with_pthread_req" = yes; then
      AC_MSG_ERROR([POSIX threads were requested, but were not found.])
    fi
  fi
])

AC_PROG_CC_PTHREAD # Needs ac_original_cc.

dnl Check if C++ compiler is present. If not set CXX to the C-compiler used
dnl for the other compilations.
if test "$CC" = gcc; then
  AC_CHECK_PROGS(CXX, $CCC g++ CC C++ c++ cxx cc++ xlC $CC, gcc)
else
  AC_CHECK_PROGS(CXX, $CCC CC C++ g++ c++ cxx cc++ xlC $CC, gcc)
fi
AC_PROG_CXX
if test "${CXX}" = "xlC" ; then
  CXXFLAGS="${CXXFLAGS} -+"
fi

dnl If you're using g++-2.95 or later and have old X11 includes, you may need
dnl to use "g++ -fpermissive" in the Makefiles
dnl if test "${CXX}" = "g++" ; then
dnl   gpp_major_version=`g++ --version | cut -f1 -d.`
dnl   if test "${gpp_major_version}" -gt "2" ; then
dnl     AC_MSG_RESULT([enabling -fpermissive option for g++])
dnl     CXXFLAGS="${CXXFLAGS} -fpermissive"
dnl   fi
dnl   if test "${gpp_major_version}" -eq "2" ; then
dnl     gpp_minor_version=`g++ --version | cut -f2 -d.`
dnl     if test "${gpp_minor_version}" -ge "95" ; then
dnl       AC_MSG_RESULT([enabling -fpermissive option for g++])
dnl       CXXFLAGS="${CXXFLAGS} -fpermissive"
dnl     fi
dnl   fi
dnl fi

dnl These 2 assume for now c++ exists....
AC_DEFINE([HAVE_CXX], [1], [C++ is available])
CCC=

PACKAGE=xlockmore
AC_SUBST(PACKAGE)
AC_DEFINE_UNQUOTED([PACKAGE], ["$PACKAGE"], [package])
VERSION=m4_esyscmd([grep VERSION xlock/version.h | cut -f2 -d\" | tr -d '\n'])
AC_SUBST(VERSION)
AC_DEFINE_UNQUOTED([VERSION], ["$VERSION"], [version file])

dnl Checks for programs.
dnl AC_ARG_WITH(gcc,
dnl[  --without-gcc           use CC to compile])

dnl test -n "$CC" && cc_specified=yes
dnl case ${with_gcc} in
dnl   yes ) CC=gcc ;;
dnl dnl yes ) CC=g++ ;;
dnl  no  ) CC=cc ;;
dnl  *   ) AC_PROG_CC;;
dnl  *   ) AC_PROG_CXX ;;
dnl esac

if test -n "$GCC"; then
  AC_MSG_RESULT(Turning on GNU compiler warnings.)
dnl This creates a lot of noise.
dnl CFLAGS="${CFLAGS} -W -Wshadow -Wpointer-arith -Wbad-function-cast -Wcast-align -Wwrite-strings"
dnl -Wall -Wmissing-prototypes -Wstrict-prototypes
dnl -Waggregate-return
  CC="$CC -Wnested-externs -Wno-format"
fi

###############################################################################
#
#       Functions to figure out how to disable // comments in ANSI C code.
#
#       (With recent gcc, this is done with "-std=c89".  With older gcc, this
#       is done by passing "-lang-c89" to cpp, by passing "-Wp,-lang-c89" to
#       gcc.  Old gcc doesn't support -std, and new gcc doesn't support -lang.
#       so much for compatibility!)
#
###############################################################################

AC_DEFUN([AC_GCC_ACCEPTS_STD],
 [if test -n "$GCC"; then
   AC_CACHE_CHECK([whether gcc accepts -std],
     ac_cv_gcc_accepts_std,
    [if ( gcc -E -std=c89 - </dev/null 2>&1 >/dev/null | \
          grep unrecognized >/dev/null ); then
       ac_cv_gcc_accepts_std=no
     else
       ac_cv_gcc_accepts_std=yes
     fi])
   ac_gcc_accepts_std="$ac_cv_gcc_accepts_std"
  fi
])

AC_DEFUN([AC_NO_CPLUSPLUS_COMMENTS_IN_C_CODE],
 [if test -n "$GCC"; then
   AC_GCC_ACCEPTS_STD
   AC_MSG_RESULT(Disabling C++ comments in ANSI C code.)
   #
   # The reason that // comments are banned is that gcc is
   # basically the only compiler in the world that supports them in C code.
   # All other vendors support them only in their C++ compilers, not in their
   # ANSI C compilers.  This means that it's a portability problem: every time
   # these comments have snuck into the source code, I've gotten
   # complaints about it the next day.  So we turn off support for them in gcc
   # as well to prevent them from accidentally slipping in.
   #
   if test "$ac_gcc_accepts_std" = yes ; then
     #
     # -std=c89 defines __STRICT_ANSI__, which we don't want.
     # (That appears to be the only additional preprocessor symbol
     # it defines, in addition to the syntax changes it makes.)
     #
     # -std=gnu89 is no good, because // comments were a GNU extension
     # before they were in the ANSI C 99 spec...  (gcc 2.96 permits //
     # with -std=gnu89 but not with -std=c89.)
     #
     # -U__STRICT_ANSI__ broken on Redhat 7.1
     CC="$CC -std=c89 -U__STRICT_ANSI__"
   else
     # The old way:
     CC="$CC -Wp,-lang-c89"
   fi
  fi
])


opt_purify=no
AC_ARG_WITH(purify, [  --with-purify           configure to postprocess with purify], opt_purify=$withval)
if test "$opt_purify" = yes; then
  AC_MSG_RESULT([enabling purify])
  PURIFY="purify"
else
  PURIFY=
fi

#AC_NO_CPLUSPLUS_COMMENTS_IN_C_CODE
AC_PROG_LN_S
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_MAKE_SET
ac_link_test="-L"
# test test
AC_MSG_CHECKING([test -h])
if (test \! -h /) >/dev/null 2>/dev/null ; then
  ac_link_test="-h"
  AC_MSG_RESULT($ac_link_test)
else
  AC_MSG_RESULT($ac_link_test)
fi

dnl Optional features.
dnl add copious amounts of debugging with gcc

dnl Checks for header files.
AC_SEARCH_LIBS([strerror],[cposix])
dnl AC_HEADER_SYS_WAIT
dnl AC_CHECK_HEADERS(poll.h sys/poll.h)
AC_CHECK_HEADERS(fcntl.h limits.h sys/resource.h sys/select.h sys/time.h syslog.h unistd.h memory.h sys/param.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
dnl AC_HEADER_SYS_WAIT
AC_TYPE_MODE_T
AC_TYPE_UID_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_STRUCT_TM

AC_MSG_CHECKING([for getenv declaration])
AC_EGREP_HEADER(getenv, stdlib.h,
	AC_DEFINE([DECLARED_GETENV], [1], [Declared GetEnv]) AC_MSG_RESULT(yes),
	AC_MSG_RESULT(no))

dnl Checks for library functions.
dnl AC_TYPE_GETGROUPS
dnl AC_CHECK_FUNCS(poll)
dnl AC_CHECK_FUNCS(signal)
AC_CHECK_FUNCS(gethostname select strdup)

AC_CHECK_FUNC(vfork, AC_DEFINE([HAVE_VFORK], [1], [vfork is available]))
AC_CHECK_FUNC(seteuid, AC_DEFINE([HAVE_SETEUID], [1], [seteuid is available]))
if test $ac_cv_func_seteuid = no; then
AC_CHECK_FUNC(setreuid, AC_DEFINE([HAVE_SETREUID], [1], [setreuid is available]))
fi

# Configure paths for GTK+
# Owen Taylor     97-11-3

dnl AC_PATH_GTK([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for GTK, and define GTK_CFLAGS and GTK_LIBS
dnl
AC_DEFUN([AC_PATH_GTK],
[dnl
dnl Get the cflags and libraries from the gtk-config script
dnl
AC_ARG_WITH(gtk-prefix,[  --with-gtk-prefix=DIR   Prefix where GTK is installed (optional)],
            gtk_config_prefix="$withval", gtk_config_prefix="")
AC_ARG_WITH(gtk-exec-prefix,[  --with-gtk-exec-prefix=DIR
                          Exec prefix where GTK is installed (optional)],
            gtk_config_exec_prefix="$withval", gtk_config_exec_prefix="")
AC_ARG_ENABLE(gtktest, [  --disable-gtktest       do not try to compile and run a test GTK program],
		    , enable_gtktest=yes)

  if test x$gtk_config_exec_prefix != x ; then
     gtk_config_args="$gtk_config_args --exec-prefix=$gtk_config_exec_prefix"
     if test x${GTK_CONFIG+set} != xset ; then
        GTK_CONFIG=$gtk_config_exec_prefix/bin/gtk-config
     fi
  fi
  if test x$gtk_config_prefix != x ; then
     gtk_config_args="$gtk_config_args --prefix=$gtk_config_prefix"
     if test x${GTK_CONFIG+set} != xset ; then
        GTK_CONFIG=$gtk_config_prefix/bin/gtk-config
     fi
  fi

  AC_PATH_PROG(GTK_CONFIG, gtk-config, no)
  min_gtk_version=ifelse([$1], ,0.99.7,$1)
  AC_MSG_CHECKING(for GTK - version >= $min_gtk_version)
  no_gtk=""
  if test "$GTK_CONFIG" = "no" ; then
    no_gtk=yes
  else
    GTK_CFLAGS=`$GTK_CONFIG $gtk_config_args --cflags`
    GTK_LIBS=`$GTK_CONFIG $gtk_config_args --libs`
    gtk_config_major_version=`$GTK_CONFIG $gtk_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    gtk_config_minor_version=`$GTK_CONFIG $gtk_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    gtk_config_micro_version=`$GTK_CONFIG $gtk_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_gtktest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $GTK_CFLAGS"
      LIBS="$LIBS $GTK_LIBS"
dnl
dnl Now check if the installed GTK is sufficiently new. (Also sanity
dnl checks the results of gtk-config to some extent
dnl
      rm -f conf.gtktest
      AC_TRY_RUN([
#include <gtk/gtk.h>
#include <stdio.h>

int
main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.gtktest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = g_strdup("$min_gtk_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_gtk_version");
     exit(1);
   }

  if ((gtk_major_version != $gtk_config_major_version) ||
      (gtk_minor_version != $gtk_config_minor_version) ||
      (gtk_micro_version != $gtk_config_micro_version))
    {
      printf("\n*** 'gtk-config --version' returned %d.%d.%d, but GTK+ (%d.%d.%d)\n",
             $gtk_config_major_version, $gtk_config_minor_version, $gtk_config_micro_version,
             gtk_major_version, gtk_minor_version, gtk_micro_version);
      printf ("*** was found! If gtk-config was correct, then it is best\n");
      printf ("*** to remove the old version of GTK+. You may also be able to fix the error\n");
      printf("*** by modifying your LD_LIBRARY_PATH environment variable, or by editing\n");
      printf("*** /etc/ld.so.conf. Make sure you have run ldconfig if that is\n");
      printf("*** required on your system.\n");
      printf("*** If gtk-config was wrong, set the environment variable GTK_CONFIG\n");
      printf("*** to point to the correct copy of gtk-config, and remove the file config.cache\n");
      printf("*** before re-running configure\n");
    }
  else
    {
      if ((gtk_major_version > major) ||
        ((gtk_major_version == major) && (gtk_minor_version > minor)) ||
        ((gtk_major_version == major) && (gtk_minor_version == minor) && (gtk_micro_version >= micro)))
      {
        return 0;
       }
     else
      {
        printf("\n*** An old version of GTK+ (%d.%d.%d) was found.\n",
               gtk_major_version, gtk_minor_version, gtk_micro_version);
        printf("*** You need a version of GTK+ newer than %d.%d.%d. The latest version of\n",
	       major, minor, micro);
        printf("*** GTK+ is always available from ftp://ftp.gtk.org.\n");
        printf("***\n");
        printf("*** If you have already installed a sufficiently new version, this error\n");
        printf("*** probably means that the wrong copy of the gtk-config shell script is\n");
        printf("*** being found. The easiest way to fix this is to remove the old version\n");
        printf("*** of GTK+, but you can also set the GTK_CONFIG environment to point to the\n");
        printf("*** correct copy of gtk-config. (In this case, you will have to\n");
        printf("*** modify your LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf\n");
        printf("*** so that the correct libraries are found at run-time))\n");
      }
    }
  return 1;
}
],, no_gtk=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_gtk" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])
  else
     AC_MSG_RESULT(no)
     if test "$GTK_CONFIG" = "no" ; then
       echo "*** The gtk-config script installed by GTK could not be found"
       echo "*** If GTK was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the GTK_CONFIG environment variable to the"
       echo "*** full path to gtk-config."
     else
       if test -f conf.gtktest ; then
        :
       else
          echo "*** Could not run GTK test program, checking why..."
          CFLAGS="$CFLAGS $GTK_CFLAGS"
          LIBS="$LIBS $GTK_LIBS"
          AC_TRY_LINK([
#include <gtk/gtk.h>
#include <stdio.h>
],      [ return ((gtk_major_version) || (gtk_minor_version) || (gtk_micro_version)); ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding GTK or finding the wrong"
          echo "*** version of GTK. If it is not finding GTK, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"
          echo "***"
          echo "*** If you have a RedHat 5.0 system, you should remove the GTK package that"
          echo "*** came with the system with the command"
          echo "***"
          echo "***    rpm --erase --nodeps gtk gtk-devel" ],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occurred. This usually means GTK was incorrectly installed"
          echo "*** or that you have moved GTK since it was installed. In the latter case, you"
          echo "*** may want to edit the gtk-config script: $GTK_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     GTK_CFLAGS=""
     GTK_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(GTK_CFLAGS)
  AC_SUBST(GTK_LIBS)
  rm -f conf.gtktest
])

dnl X Window System files.
AC_PATH_XTRA
if test "$no_x" = yes; then
  XLIBS=NONE
else
  if test -n "${x_includes}"; then
    XLOCKINC="-I${x_includes} ${XLOCKINC}"
    XMLOCKINC="-I${x_includes} ${XMLOCKINC}"
    if test "${CXX}" = "g++" ; then
dnl   Do not warn me about anything here
      XLOCKINC="-isystem ${x_includes} ${XLOCKINC}"
      XMLOCKINC="-isystem ${x_includes} ${XMLOCKINC}"
    fi
  fi
  XLIBS="${x_libraries}"
  if test -n "${x_libraries}"; then
    XLOCK_LDFLAGS="/usr/lib:${x_libraries}"
    XMLOCK_LDFLAGS="/usr/lib:${x_libraries}"
    XLOCKLIBPATHS="-L${x_libraries} ${XLOCKLIBPATHS}"
    XMLOCKLIBPATHS="-L${x_libraries} ${XMLOCKLIBPATHS}"
  fi
  XLOCKLIBS="${X_PRE_LIBS} ${X_EXTRA_LIBS} ${XLOCKLIBS}"
  case "${canonical}" in
    *-*-sco* )
      XLOCKLIBS="${X_PRE_LIBS} ${XLOCKLIBS} ${X_EXTRA_LIBS}"
    ;;
    *-*-cygwin* )
      XLOCKLIBS="${XLOCKLIBS} -lcrypt"
      XMLOCKLIBS="-lSM -lICE ${XMLOCKLIBS}"
    ;;
  esac
  XMLOCKLIBS="-lXt ${XMLOCKLIBS}"
#  if you need regex and regcmp
#  XMLOCKLIBS="-lXt ${XMLOCKLIBS} -lgen"
fi

# Try and find the app-defaults directory.
AC_DEFUN([AC_PATH_X_APP_DEFAULTS_XMKMF],
  [
  rm -fr conftestdir
  if mkdir conftestdir; then
    cd conftestdir
    # Make sure to not put "make" in the Imakefile rules, since we grep it out.
    cat > Imakefile <<'EOF'
acfindx:
  @echo 'ac_x_app_defaults="${XAPPLOADDIR}"'
EOF
    if (xmkmf) >/dev/null 2>/dev/null && test -f Makefile; then
      # GNU make sometimes prints "make[1]: Entering...", which'd confuse us.
      eval `${MAKE-make} acfindx 2>/dev/null | grep -v make`
    fi
    cd ..
    rm -fr conftestdir
  fi])

AC_DEFUN([AC_PATH_X_APP_DEFAULTS_DIRECT],
[  # Look for the directory under a standard set of common directories.
  # Check X11 before X11Rn because it's often a symlink to the current release.
  for ac_dir in         \
    /lib/X11/app-defaults \
    /lib/app-defaults \
    /lib/X11R6.5.1/app-defaults \
    /lib/X11R6.5.1/X11/app-defaults \
    /lib/X11R6.4/app-defaults \
    /lib/X11R6.4/X11/app-defaults \
    /lib/X11R6.3/app-defaults \
    /lib/X11R6.3/X11/app-defaults \
    /lib/X11R6.2/app-defaults \
    /lib/X11R6.2/X11/app-defaults \
    /lib/X11R6.1/app-defaults \
    /lib/X11R6.1/X11/app-defaults \
    /lib/X11R6/app-defaults \
    /lib/X11R6/X11/app-defaults \
    /lib/X11R5/app-defaults \
    /lib/X11R5/X11/app-defaults \
    /lib/X11R4/app-defaults \
    /lib/X11R4/X11/app-defaults \
    ; \
  do
    found=""
    if test -d "${prefix}$ac_dir"; then
      ac_x_app_defaults=\\\$\\\{prefix\\\}$ac_dir
    found="1"
      break
    fi
  done
  if test -z "$found"; then
  for ac_dir in         \
    /usr/X11/lib/app-defaults     \
    /lib/X11R6.5.1/lib/app-defaults \
    /lib/X11R6.5.1/lib/X11/app-defaults \
    /lib/X11R6.4/lib/app-defaults \
    /lib/X11R6.4/lib/X11/app-defaults \
    /usr/X11R6.3/lib/app-defaults     \
    /usr/X11R6.3/lib/X11/app-defaults   \
    /usr/X11R6.2/lib/app-defaults     \
    /usr/X11R6.2/lib/X11/app-defaults   \
    /usr/X11R6.1/lib/app-defaults     \
    /usr/X11R6.1/lib/X11/app-defaults   \
    /usr/X11R6/lib/app-defaults     \
    /usr/X11R6/lib/X11/app-defaults   \
    /usr/X11R5/lib/app-defaults     \
    /usr/X11R5/lib/X11/app-defaults   \
    /usr/X11R4/lib/app-defaults     \
    /usr/X11R4/lib/X11/app-defaults   \
            \
    /usr/lib/X11/app-defaults     \
    /usr/lib/X11R6.5.1/app-defaults \
    /usr/lib/X11R6.4/app-defaults \
    /usr/lib/X11R6.3/app-defaults     \
    /usr/lib/X11R6.2/app-defaults     \
    /usr/lib/X11R6.1/app-defaults     \
    /usr/lib/X11R6/app-defaults     \
    /usr/lib/X11R5/app-defaults     \
    /usr/lib/X11R4/app-defaults     \
            \
    /usr/local/X11/lib/app-defaults   \
    /usr/remote/X11/lib/app-defaults   \
    /usr/local/X11R6.5.1/lib/app-defaults   \
    /usr/local/X11R6.5.1/lib/X11/app-defaults   \
    /usr/local/X11R6.4/lib/app-defaults   \
    /usr/local/X11R6.4/lib/X11/app-defaults   \
    /usr/local/X11R6.3/lib/app-defaults   \
    /usr/local/X11R6.3/lib/X11/app-defaults   \
    /usr/local/X11R6.2/lib/app-defaults   \
    /usr/local/X11R6.2/lib/X11/app-defaults   \
    /usr/local/X11R6.1/lib/app-defaults   \
    /usr/local/X11R6.1/lib/X11/app-defaults   \
    /usr/local/X11R6/lib/app-defaults   \
    /usr/local/X11R6/lib/X11/app-defaults   \
    /usr/local/X11R5/lib/app-defaults   \
    /usr/local/X11R5/lib/X11/app-defaults   \
    /usr/local/X11R4/lib/app-defaults   \
    /usr/local/X11R4/lib/X11/app-defaults   \
            \
    /usr/local/lib/X11/app-defaults   \
    /usr/remote/lib/X11/app-defaults   \
    /usr/local/lib/X11R6.5.1/app-defaults   \
    /usr/local/lib/X11R6.5.1/X11/app-defaults \
    /usr/local/lib/X11R6.4/app-defaults   \
    /usr/local/lib/X11R6.4/X11/app-defaults \
    /usr/local/lib/X11R6.3/app-defaults   \
    /usr/local/lib/X11R6.3/X11/app-defaults \
    /usr/local/lib/X11R6.2/app-defaults   \
    /usr/local/lib/X11R6.2/X11/app-defaults \
    /usr/local/lib/X11R6.1/app-defaults   \
    /usr/local/lib/X11R6.1/X11/app-defaults \
    /usr/local/lib/X11R6/app-defaults   \
    /usr/local/lib/X11R6/X11/app-defaults \
    /usr/local/lib/X11R5/app-defaults   \
    /usr/local/lib/X11R5/X11/app-defaults \
    /usr/local/lib/X11R4/app-defaults   \
    /usr/local/lib/X11R4/X11/app-defaults \
            \
    /usr/X386/lib/X11/app-defaults    \
    /usr/x386/lib/X11/app-defaults    \
    /usr/XFree86/lib/X11/app-defaults   \
    /usr/pkg/lib/app-defaults          \
            \
    /usr/lib/X11/app-defaults     \
    /usr/unsupported/lib/X11/app-defaults \
    /usr/athena/lib/X11/app-defaults    \
    /usr/local/x11r5/lib/X11/app-defaults \
    /usr/lpp/Xamples/lib/X11/app-defaults \
    /lib/usr/lib/X11/app-defaults   \
    /usr/local/lib/app-defaults   \
    /usr/remote/lib/app-defaults   \
            \
    /usr/openwin/lib/app-defaults   \
    /usr/openwin/lib/X11/app-defaults   \
    /usr/openwin/share/lib/app-defaults   \
    /usr/openwin/share/lib/X11/app-defaults \
            \
    /X11R6.5.1/lib/app-defaults   \
    /X11R6.4/lib/app-defaults     \
    /X11R6.3/lib/app-defaults     \
    /X11R6.2/lib/app-defaults     \
    /X11R6.1/lib/app-defaults     \
    /X11R6/lib/app-defaults     \
    /X11R5/lib/app-defaults     \
    /X11R4/lib/app-defaults     \
    ; \
  do
    if test -d "$ac_dir"; then
      ac_x_app_defaults=$ac_dir
      break
    fi
  done
    fi
])


AC_DEFUN([AC_PATH_X_APP_DEFAULTS],
  [AC_REQUIRE_CPP()
    AC_CACHE_CHECK([for X app-defaults directory], ac_cv_x_app_defaults,
     [AC_PATH_X_APP_DEFAULTS_XMKMF
      AC_PATH_X_APP_DEFAULTS_DIRECT
      if test x"$ac_x_app_defaults" = x; then
        ac_cv_x_app_defaults="/usr/lib/X11/app-defaults"
      else
        # Record where we found app-defaults for the cache.
        ac_cv_x_app_defaults="$ac_x_app_defaults"
      fi])
    eval ac_x_app_defaults="$ac_cv_x_app_defaults"])

AC_PATH_X_APP_DEFAULTS

APPDEFAULTS=$ac_x_app_defaults

AC_ARG_WITH(includes, [  --with-includes=DIR     search include DIR for optional packages below])
case "x$withval" in
x/*|x.*)
  extra_include=$withval
  AC_MSG_RESULT([adding $extra_include to include search path for following packages])
  if test ! -d $extra_include; then
    AC_MSG_RESULT([Warning: Directory $extra_include does not exist])
  fi
  ;;
*)
  extra_include=""
  ;;
esac

AC_ARG_WITH(libraries, [  --with-libraries=DIR    search library DIR for optional packages below])
case "x$withval" in
x/*|x.*)
  extra_lib=$withval
  AC_MSG_RESULT([adding $extra_lib to library search path for following packages])
  if test ! -d $extra_lib; then
    AC_MSG_RESULT([Warning: Directory $extra_lib does not exist])
  fi
  ;;
*)
  extra_lib=""
  ;;
esac

dnl Xm MOTIF Motif motif

AC_DEFUN([AC_PATH_MOTIF_DIRECT],
[test -z "$motif_direct_test_library" && motif_direct_test_library=Xm
test -z "$motif_direct_test_function" && motif_direct_test_function=XmCreatePushButton
test -z "$motif_direct_test_include" && motif_direct_test_include=Xm/Xm.h
  for ac_dir in               \
    /usr/include/Motif1.2     \
    /usr/Motif1.2/include     \
                              \
    /usr/motif/include        \
                              \
    /usr/X11R6.5.1/include    \
    /usr/X11R6.4/include      \
    /usr/X11R6.3/include      \
    /usr/X11R6.2/include      \
    /usr/X11R6.1/include      \
    /usr/X11R6/include        \
    /usr/X11R5/include        \
    /usr/X11R4/include        \
                              \
    /usr/include/X11R6.5.1    \
    /usr/include/X11R6.4      \
    /usr/include/X11R6.3      \
    /usr/include/X11R6.2      \
    /usr/include/X11R6.1      \
    /usr/include/X11R6        \
    /usr/include/X11R5        \
    /usr/include/X11R4        \
                              \
    /usr/local/X11R6.5.1/include  \
    /usr/local/X11R6.4/include  \
    /usr/local/X11R6.3/include  \
    /usr/local/X11R6.2/include  \
    /usr/local/X11R6.1/include  \
    /usr/local/X11R6/include  \
    /usr/local/X11R5/include  \
    /usr/local/X11R4/include  \
                              \
    /usr/local/include/X11R6.5.1  \
    /usr/local/include/X11R6.4  \
    /usr/local/include/X11R6.3  \
    /usr/local/include/X11R6.2  \
    /usr/local/include/X11R6.1  \
    /usr/local/include/X11R6  \
    /usr/local/include/X11R5  \
    /usr/local/include/X11R4  \
                              \
    /usr/X11/include          \
    /usr/include/X11          \
    /usr/local/X11/include    \
    /usr/local/include/X11    \
                              \
    /usr/X386/include         \
    /usr/x386/include         \
    /usr/XFree86/include/X11  \
    /usr/pkg/include          \
                              \
    /usr/dt/include           \
                              \
    /usr/local/include        \
    /usr/remote/include       \
    /usr/include              \
    /usr/unsupported/include  \
    /usr/athena/include       \
    /usr/local/x11r5/include  \
    /usr/lpp/Xamples/include  \
    $extra_include            \
    ; \
  do
    if test -r "$ac_dir/$motif_direct_test_include"; then
      no_motif= ac_motif_includes=$ac_dir
      break
    fi
  done

# Check for the libraries.
# See if we find them without any special options.
# Do not add to $LIBS permanently.
ac_save_LIBS="$LIBS"
LIBS="-l$motif_direct_test_library $LIBS"
# First see if replacing the include by lib works.
for ac_dir in `echo "$ac_motif_includes" | sed s/include/lib/` \
    /usr/lib/Motif1.2     \
    /usr/Motif1.2/lib     \
                          \
    /usr/motif/lib        \
                          \
    /usr/X11R6.5.1/lib    \
    /usr/X11R6.4/lib      \
    /usr/X11R6.3/lib      \
    /usr/X11R6.2/lib      \
    /usr/X11R6.1/lib      \
    /usr/X11R6/lib        \
    /usr/X11R5/lib        \
    /usr/X11R4/lib        \
                          \
    /usr/lib/X11R6.5.1    \
    /usr/lib/X11R6.4      \
    /usr/lib/X11R6.3      \
    /usr/lib/X11R6.2      \
    /usr/lib/X11R6.1      \
    /usr/lib/X11R6        \
    /usr/lib/X11R5        \
    /usr/lib/X11R4        \
                          \
    /usr/local/X11R6.5.1/lib  \
    /usr/local/X11R6.4/lib  \
    /usr/local/X11R6.3/lib  \
    /usr/local/X11R6.2/lib  \
    /usr/local/X11R6.1/lib  \
    /usr/local/X11R6/lib  \
    /usr/local/X11R5/lib  \
    /usr/local/X11R4/lib  \
                          \
    /usr/local/lib/X11R6.5.1  \
    /usr/local/lib/X11R6.4  \
    /usr/local/lib/X11R6.3  \
    /usr/local/lib/X11R6.2  \
    /usr/local/lib/X11R6.1  \
    /usr/local/lib/X11R6  \
    /usr/local/lib/X11R5  \
    /usr/local/lib/X11R4  \
                          \
    /usr/X11/lib          \
    /usr/lib/X11          \
    /usr/local/X11/lib    \
                          \
    /usr/X386/lib         \
    /usr/x386/lib         \
    /usr/XFree86/lib/X11  \
    /usr/pkg/lib          \
                          \
    /usr/lib              \
    /usr/local/lib        \
    /usr/remote/lib       \
    /usr/unsupported/lib  \
    /usr/athena/lib       \
    /usr/local/x11r5/lib  \
    /usr/lpp/Xamples/lib  \
    $extra_lib            \
    ; \
do
  for ac_extension in a so sl; do
    if test -r $ac_dir/lib${motif_direct_test_library}.$ac_extension; then
      no_motif= ac_motif_libraries=$ac_dir
      break 2
    fi
  done
done
LIBS="$ac_save_LIBS"])
AC_DEFUN([AC_PATH_MOTIF],
[AC_REQUIRE_CPP()dnl

motif_includes=NONE
motif_libraries=NONE

AC_MSG_CHECKING(for Motif)
AC_ARG_WITH(motif, [  --without-motif         disable Motif (for xmlock)])
if test "x$with_motif" = xno; then
  no_motif=yes
else
  if test "x$motif_includes" != xNONE && test "x$motif_libraries" != xNONE; then
    no_motif=
  else
AC_CACHE_VAL(ac_cv_path_motif,
[# One or both of these vars are not set, and there is no cached value.
no_motif=yes
AC_PATH_MOTIF_DIRECT

if test "$no_motif" = yes; then
  ac_cv_path_motif="no_motif=yes"
else
  ac_cv_path_motif="no_motif= ac_motif_includes=$ac_motif_includes ac_motif_libraries=$ac_motif_libraries"
fi])dnl
  fi
  eval "$ac_cv_path_motif"
fi # with_motif != no

if test "$no_motif" = yes; then
  AC_MSG_RESULT(no)
  XMLOCK=""
  INSTALL_XMLOCK=""
  UNINSTALL_XMLOCK=""
else
  AC_DEFINE([HAVE_MOTIF], [1], [Motif is available])
  XMLOCKLIBS="-lXm ${XMLOCKLIBS}"
  XMLOCK="xmlock"
  INSTALL_XMLOCK="install_xmlock"
  UNINSTALL_XMLOCK="uninstall_xmlock"
  test "x$motif_includes" = xNONE && motif_includes=$ac_motif_includes
  test "x$motif_libraries" = xNONE && motif_libraries=$ac_motif_libraries
  ac_cv_path_motif="no_motif= ac_motif_includes=$motif_includes ac_motif_libraries=$motif_libraries"
  AC_MSG_RESULT([libraries $motif_libraries, headers $motif_includes])
fi
])

AC_PATH_MOTIF

if test "x$motif_libraries" != x && test "x$motif_libraries" != xNONE ; then
  XMLOCK_LDFLAGS="${XMLOCK_LDFLAGS}:$motif_libraries"
  XMLOCKLIBPATHS="${XMLOCKLIBPATHS} -L$motif_libraries"
fi
if test "x$motif_includes" != x && test "x$motif_includes" != xNONE ; then
  XMLOCKINC="${XMLOCKINC} -I$motif_includes"
fi

dnl Athena ATHENA athena

AC_ARG_WITH(athena, [  --without-athena        disable Athena (all Xaw libs)])
if test "x$with_athena" = xno; then
  with_xaw3dxft=no
  with_xaw3d=no
  with_nextaw=no
  with_xaw=no
fi

dnl Xaw3dxft XAW3DXFT xaw3dxft

AC_DEFUN([AC_PATH_XAW3DXFT_DIRECT],
[test -z "$xaw3dxft_direct_test_library" && xaw3dxft_direct_test_library=Xaw3dxft
test -z "$xaw3dxft_direct_test_function" && xaw3dxft_direct_test_function=XawDialGetValueString
test -z "$xaw3dxft_direct_test_include" && xaw3dxft_direct_test_include=X11/Xaw3dxft/Command.h
  for ac_dir in               \
    /usr/X11R6/include        \
    /usr/X11R5/include        \
    /usr/X11R4/include        \
                              \
    /usr/include/X11R6        \
    /usr/include/X11R5        \
    /usr/include/X11R4        \
                              \
    /usr/local/X11R6/include  \
    /usr/local/X11R5/include  \
    /usr/local/X11R4/include  \
                              \
    /usr/local/include/X11R6  \
    /usr/local/include/X11R5  \
    /usr/local/include/X11R4  \
                              \
    /usr/X11/include          \
    /usr/include/X11          \
    /usr/local/X11/include    \
    /usr/local/include/X11    \
                              \
    /usr/X386/include         \
    /usr/x386/include         \
    /usr/XFree86/include/X11  \
                              \
    /usr/dt/include           \
                              \
    /usr/local/include        \
    /usr/remote/include       \
    /usr/include              \
    /usr/unsupported/include  \
    /usr/local/x11r5/include  \
    /usr/lpp/Xamples/include  \
    $extra_include            \
    ; \
  do
    if test -r "$ac_dir/$xaw3dxft_direct_test_include"; then
      no_xaw3dxft= ac_xaw3dxft_includes=$ac_dir
      break
    fi
  done

# Check for the libraries.
# See if we find them without any special options.
# Don't add to $LIBS permanently.
ac_save_LIBS="$LIBS"
LIBS="-l$xaw3dxft_direct_test_library $LIBS"
# First see if replacing the include by lib works.
for ac_dir in `echo "$ac_xaw3dxft_includes" | sed s/include/lib/` \
    /usr/X11R6/lib        \
    /usr/X11R5/lib        \
    /usr/X11R4/lib        \
                          \
    /usr/lib/X11R6        \
    /usr/lib/X11R5        \
    /usr/lib/X11R4        \
                          \
    /usr/local/X11R6/lib  \
    /usr/local/X11R5/lib  \
    /usr/local/X11R4/lib  \
                          \
    /usr/local/lib/X11R6  \
    /usr/local/lib/X11R5  \
    /usr/local/lib/X11R4  \
                          \
    /usr/X11/lib          \
    /usr/lib/X11          \
    /usr/local/X11/lib    \
                          \
    /usr/X386/lib         \
    /usr/x386/lib         \
    /usr/XFree86/lib/X11  \
                          \
    /usr/lib              \
    /usr/local/lib        \
    /usr/remote/lib       \
    /usr/unsupported/lib  \
    /usr/xaw3dxft/lib       \
    /usr/local/x11r5/lib  \
    /usr/lpp/Xamples/lib  \
    $extra_lib            \
    ; \
do
  for ac_extension in a so sl; do
    if test -r $ac_dir/lib${xaw3dxft_direct_test_library}.$ac_extension; then
      no_xaw3dxft= ac_xaw3dxft_libraries=$ac_dir
      break 2
    fi
  done
done
LIBS="$ac_save_LIBS"])
AC_DEFUN([AC_PATH_XAW3DXFT],
[AC_REQUIRE_CPP()dnl

xaw3dxft_includes=NONE
xaw3dxft_libraries=NONE

AC_MSG_CHECKING(for Xaw3dxft)
AC_ARG_WITH(xaw3dxft, [  --without-xaw3dxft      disable Xaw3dxft])
if test "x$with_xaw3dxft" = xno; then
  no_xaw3dxft=yes
else
  if test "x$xaw3dxft_includes" != xNONE && test "x$xaw3dxft_libraries" != xNONE; then
    no_xaw3dxft=
  else
AC_CACHE_VAL(ac_cv_path_xaw3dxft,
[# One or both of these vars are not set, and there is no cached value.
no_xaw3dxft=yes
AC_PATH_XAW3DXFT_DIRECT

if test "$no_xaw3dxft" = yes; then
  ac_cv_path_xaw3dxft="no_xaw3dxft=yes"
else
  ac_cv_path_xaw3dxft="no_xaw3dxft= ac_xaw3dxft_includes=$ac_xaw3dxft_includes ac_xaw3dxft_libraries=$ac_xaw3dxft_libraries"
fi])dnl
  fi
  eval "$ac_cv_path_xaw3dxft"
fi # with_xaw3dxft != no

if test "$no_motif" = yes; then
  if test "$no_xaw3dxft" = yes; then
    AC_MSG_RESULT(no)
    XMLOCK=""
    INSTALL_XMLOCK=""
    UNINSTALL_XMLOCK=""
  else
    AC_DEFINE([HAVE_ATHENA], [1], [Athena is available])
    AC_DEFINE([HAVE_LIB_XAW3DXFT], [1], [Xaw3dxft is available])
    XMLOCKLIBS="-lXaw3dxft ${XMLOCKLIBS}"
    XMLOCK="xmlock"
    INSTALL_XMLOCK="install_xmlock"
    UNINSTALL_XMLOCK="uninstall_xmlock"
    test "x$xaw3dxft_includes" = xNONE && xaw3dxft_includes=$ac_xaw3dxft_includes
    test "x$xaw3dxft_libraries" = xNONE && xaw3dxft_libraries=$ac_xaw3dxft_libraries
    ac_cv_path_xaw3dxft="no_xaw3dxft= ac_xaw3dxft_includes=$xaw3dxft_includes ac_xaw3dxft_libraries=$xaw3dxft_libraries"
    AC_MSG_RESULT([libraries $xaw3dxft_libraries, headers $xaw3dxft_includes])
  fi
else
  AC_MSG_RESULT(ignored - using Motif)
fi
])

AC_PATH_XAW3DXFT

if test "x$xaw3dxft_libraries" != x ; then
  XWIDGET_LDFLAGS="${XWIDGET_LDFLAGS}:$xaw3dxft_libraries"
  XWIDGETLIBPATHS="-L$xaw3dxft_libraries ${XWIDGETLIBPATHS}"
fi
if test "x$xaw3dxft_includes" != x ; then
  XWIDGETINC="${XWIDGETINC} -I$xaw3dxft_includes"
fi

dnl Xaw3d XAW3D xaw3d

AC_DEFUN([AC_PATH_XAW3D_DIRECT],
[test -z "$xaw3d_direct_test_library" && xaw3d_direct_test_library=Xaw3d
test -z "$xaw3d_direct_test_function" && xaw3d_direct_test_function=XawDialGetValueString
test -z "$xaw3d_direct_test_include" && xaw3d_direct_test_include=X11/Xaw3d/Command.h
  for ac_dir in               \
    /usr/X11R6/include        \
    /usr/X11R5/include        \
    /usr/X11R4/include        \
                              \
    /usr/include/X11R6        \
    /usr/include/X11R5        \
    /usr/include/X11R4        \
                              \
    /usr/local/X11R6/include  \
    /usr/local/X11R5/include  \
    /usr/local/X11R4/include  \
                              \
    /usr/local/include/X11R6  \
    /usr/local/include/X11R5  \
    /usr/local/include/X11R4  \
                              \
    /usr/X11/include          \
    /usr/include/X11          \
    /usr/local/X11/include    \
    /usr/local/include/X11    \
                              \
    /usr/X386/include         \
    /usr/x386/include         \
    /usr/XFree86/include/X11  \
                              \
    /usr/dt/include           \
                              \
    /usr/local/include        \
    /usr/remote/include       \
    /usr/include              \
    /usr/unsupported/include  \
    /usr/local/x11r5/include  \
    /usr/lpp/Xamples/include  \
    $extra_include            \
    ; \
  do
    if test -r "$ac_dir/$xaw3d_direct_test_include"; then
      no_xaw3d= ac_xaw3d_includes=$ac_dir
      break
    fi
  done

# Check for the libraries.
# See if we find them without any special options.
# Don't add to $LIBS permanently.
ac_save_LIBS="$LIBS"
LIBS="-l$xaw3d_direct_test_library $LIBS"
# First see if replacing the include by lib works.
for ac_dir in `echo "$ac_xaw3d_includes" | sed s/include/lib/` \
    /usr/X11R6/lib        \
    /usr/X11R5/lib        \
    /usr/X11R4/lib        \
                          \
    /usr/lib/X11R6        \
    /usr/lib/X11R5        \
    /usr/lib/X11R4        \
                          \
    /usr/local/X11R6/lib  \
    /usr/local/X11R5/lib  \
    /usr/local/X11R4/lib  \
                          \
    /usr/local/lib/X11R6  \
    /usr/local/lib/X11R5  \
    /usr/local/lib/X11R4  \
                          \
    /usr/X11/lib          \
    /usr/lib/X11          \
    /usr/local/X11/lib    \
                          \
    /usr/X386/lib         \
    /usr/x386/lib         \
    /usr/XFree86/lib/X11  \
                          \
    /usr/lib              \
    /usr/local/lib        \
    /usr/remote/lib       \
    /usr/unsupported/lib  \
    /usr/xaw3d/lib       \
    /usr/local/x11r5/lib  \
    /usr/lpp/Xamples/lib  \
    $extra_lib            \
    ; \
do
  for ac_extension in a so sl; do
    if test -r $ac_dir/lib${xaw3d_direct_test_library}.$ac_extension; then
      no_xaw3d= ac_xaw3d_libraries=$ac_dir
      break 2
    fi
  done
done
LIBS="$ac_save_LIBS"])
AC_DEFUN([AC_PATH_XAW3D],
[AC_REQUIRE_CPP()dnl

xaw3d_includes=NONE
xaw3d_libraries=NONE

AC_MSG_CHECKING(for Xaw3d)
AC_ARG_WITH(xaw3d, [  --without-xaw3d         disable Xaw3d])
if test "x$with_xaw3d" = xno; then
  no_xaw3d=yes
else
  if test "x$xaw3d_includes" != xNONE && test "x$xaw3d_libraries" != xNONE; then
    no_xaw3d=
  else
AC_CACHE_VAL(ac_cv_path_xaw3d,
[# One or both of these vars are not set, and there is no cached value.
no_xaw3d=yes
AC_PATH_XAW3D_DIRECT

if test "$no_xaw3d" = yes; then
  ac_cv_path_xaw3d="no_xaw3d=yes"
else
  ac_cv_path_xaw3d="no_xaw3d= ac_xaw3d_includes=$ac_xaw3d_includes ac_xaw3d_libraries=$ac_xaw3d_libraries"
fi])dnl
  fi
  eval "$ac_cv_path_xaw3d"
fi # with_xaw3d != no

if test "$no_motif" = yes; then
  if test "$no_xaw3dxft" = yes; then
    if test "$no_xaw3d" = yes; then
      AC_MSG_RESULT(no)
      XMLOCK=""
      INSTALL_XMLOCK=""
      UNINSTALL_XMLOCK=""
    else
      AC_DEFINE([HAVE_ATHENA], [1], [Athena is available])
      AC_DEFINE([HAVE_LIB_XAW3D], [1], [Xaw3d is available])
      XMLOCKLIBS="-lXaw3d ${XMLOCKLIBS}"
      XMLOCK="xmlock"
      INSTALL_XMLOCK="install_xmlock"
      UNINSTALL_XMLOCK="uninstall_xmlock"
      test "x$xaw3d_includes" = xNONE && xaw3d_includes=$ac_xaw3d_includes
      test "x$xaw3d_libraries" = xNONE && xaw3d_libraries=$ac_xaw3d_libraries
      ac_cv_path_xaw3d="no_xaw3d= ac_xaw3d_includes=$xaw3d_includes ac_xaw3d_libraries=$xaw3d_libraries"
      AC_MSG_RESULT([libraries $xaw3d_libraries, headers $xaw3d_includes])
    fi
  else
    AC_MSG_RESULT(ignored - using Xaw3dxft)
  fi
else
  AC_MSG_RESULT(ignored - using Motif)
fi
])

AC_PATH_XAW3D

if test "x$xaw3d_libraries" != x ; then
  XWIDGET_LDFLAGS="${XWIDGET_LDFLAGS}:$xaw3d_libraries"
  XWIDGETLIBPATHS="-L$xaw3d_libraries ${XWIDGETLIBPATHS}"
fi
if test "x$xaw3d_includes" != x ; then
  XWIDGETINC="${XWIDGETINC} -I$xaw3d_includes"
fi

dnl neXtaw NEXTAW nextaw

AC_DEFUN([AC_PATH_NEXTAW_DIRECT],
[test -z "$nextaw_direct_test_library" && nextaw_direct_test_library=neXtaw
test -z "$nextaw_direct_test_function" && nextaw_direct_test_function=XawDialGetValueString
test -z "$nextaw_direct_test_include" && nextaw_direct_test_include=X11/neXtaw/Command.h
  for ac_dir in               \
    /usr/X11R6/include        \
    /usr/X11R5/include        \
    /usr/X11R4/include        \
                              \
    /usr/include/X11R6        \
    /usr/include/X11R5        \
    /usr/include/X11R4        \
                              \
    /usr/local/X11R6/include  \
    /usr/local/X11R5/include  \
    /usr/local/X11R4/include  \
                              \
    /usr/local/include/X11R6  \
    /usr/local/include/X11R5  \
    /usr/local/include/X11R4  \
                              \
    /usr/X11/include          \
    /usr/include/X11          \
    /usr/local/X11/include    \
    /usr/local/include/X11    \
                              \
    /usr/X386/include         \
    /usr/x386/include         \
    /usr/XFree86/include/X11  \
                              \
    /usr/dt/include           \
                              \
    /usr/local/include        \
    /usr/remote/include       \
    /usr/include              \
    /usr/unsupported/include  \
    /usr/local/x11r5/include  \
    /usr/lpp/Xamples/include  \
    $extra_include            \
    ; \
  do
    if test -r "$ac_dir/$nextaw_direct_test_include"; then
      no_nextaw= ac_nextaw_includes=$ac_dir
      break
    fi
  done

# Check for the libraries.
# See if we find them without any special options.
# Don't add to $LIBS permanently.
ac_save_LIBS="$LIBS"
LIBS="-l$nextaw_direct_test_library $LIBS"
# First see if replacing the include by lib works.
for ac_dir in `echo "$ac_nextaw_includes" | sed s/include/lib/` \
    /usr/X11R6/lib        \
    /usr/X11R5/lib        \
    /usr/X11R4/lib        \
                          \
    /usr/lib/X11R6        \
    /usr/lib/X11R5        \
    /usr/lib/X11R4        \
                          \
    /usr/local/X11R6/lib  \
    /usr/local/X11R5/lib  \
    /usr/local/X11R4/lib  \
                          \
    /usr/local/lib/X11R6  \
    /usr/local/lib/X11R5  \
    /usr/local/lib/X11R4  \
                          \
    /usr/X11/lib          \
    /usr/lib/X11          \
    /usr/local/X11/lib    \
                          \
    /usr/X386/lib         \
    /usr/x386/lib         \
    /usr/XFree86/lib/X11  \
                          \
    /usr/lib              \
    /usr/local/lib        \
    /usr/remote/lib       \
    /usr/unsupported/lib  \
    /usr/nextaw/lib       \
    /usr/local/x11r5/lib  \
    /usr/lpp/Xamples/lib  \
    $extra_lib            \
    ; \
do
  for ac_extension in a so sl; do
    if test -r $ac_dir/lib${nextaw_direct_test_library}.$ac_extension; then
      no_nextaw= ac_nextaw_libraries=$ac_dir
      break 2
    fi
  done
done
LIBS="$ac_save_LIBS"])
AC_DEFUN([AC_PATH_NEXTAW],
[AC_REQUIRE_CPP()dnl

nextaw_includes=NONE
nextaw_libraries=NONE

AC_MSG_CHECKING(for neXtaw)
AC_ARG_WITH(nextaw, [  --without-nextaw        disable neXtaw])
if test "x$with_nextaw" = xno; then
  no_nextaw=yes
else
  if test "x$nextaw_includes" != xNONE && test "x$nextaw_libraries" != xNONE; then
    no_nextaw=
  else
AC_CACHE_VAL(ac_cv_path_nextaw,
[# One or both of these vars are not set, and there is no cached value.
no_nextaw=yes
AC_PATH_NEXTAW_DIRECT

if test "$no_nextaw" = yes; then
  ac_cv_path_nextaw="no_nextaw=yes"
else
  ac_cv_path_nextaw="no_nextaw= ac_nextaw_includes=$ac_nextaw_includes ac_nextaw_libraries=$ac_nextaw_libraries"
fi])dnl
  fi
  eval "$ac_cv_path_nextaw"
fi # with_nextaw != no

if test "$no_motif" = yes; then
  if test "$no_xaw3dxft" = yes; then
    if test "$no_xaw3d" = yes; then
      if test "$no_editres" = yes; then
        no_nextaw=yes
        AC_MSG_RESULT(ignored - requires editres)
      else
        if test "$no_nextaw" = yes; then
          AC_MSG_RESULT(no)
          XMLOCK=""
          INSTALL_XMLOCK=""
          UNINSTALL_XMLOCK=""
        else
          AC_DEFINE([HAVE_ATHENA], [1], [Athena is available])
          AC_DEFINE([HAVE_LIB_NEXTAW], [1], [neXtaw is available])
          XMLOCKLIBS="-lneXtaw ${XMLOCKLIBS}"
          XMLOCK="xmlock"
          INSTALL_XMLOCK="install_xmlock"
          UNINSTALL_XMLOCK="uninstall_xmlock"
          test "x$nextaw_includes" = xNONE && nextaw_includes=$ac_nextaw_includes
          test "x$nextaw_libraries" = xNONE && nextaw_libraries=$ac_nextaw_libraries
          ac_cv_path_nextaw="no_nextaw= ac_nextaw_includes=$nextaw_includes ac_nextaw_libraries=$nextaw_libraries"
          AC_MSG_RESULT([libraries $nextaw_libraries, headers $nextaw_includes])
        fi
      fi
    else
      AC_MSG_RESULT(ignored - using Xaw3d)
    fi
  else
    AC_MSG_RESULT(ignored - using Xaw3dxft)
  fi
else
  AC_MSG_RESULT(ignored - using Motif)
fi
])

AC_PATH_NEXTAW

if test "x$nextaw_libraries" != x ; then
  XWIDGET_LDFLAGS="${XWIDGET_LDFLAGS}:$nextaw_libraries"
  XWIDGETLIBPATHS="-L$nextaw_libraries ${XWIDGETLIBPATHS}"
fi
if test "x$xaw3d_includes" != x ; then
  XWIDGETINC="${XWIDGETINC} -I$nextaw_includes"
fi

dnl XawPlus XAWPLUS xawplus

AC_DEFUN([AC_PATH_XAWPLUS_DIRECT],
[test -z "$xawplus_direct_test_library" && xawplus_direct_test_library=XawPlus
test -z "$xawplus_direct_test_function" && xawplus_direct_test_function=XawDialGetValueString
test -z "$xawplus_direct_test_include" && xawplus_direct_test_include=X11/XawPlus/Command.h
  for ac_dir in               \
    /usr/athena/include       \
                              \
    /usr/X11R6/include        \
    /usr/X11R5/include        \
    /usr/X11R4/include        \
                              \
    /usr/include/X11R6        \
    /usr/include/X11R5        \
    /usr/include/X11R4        \
                              \
    /usr/local/X11R6/include  \
    /usr/local/X11R5/include  \
    /usr/local/X11R4/include  \
                              \
    /usr/local/include/X11R6  \
    /usr/local/include/X11R5  \
    /usr/local/include/X11R4  \
                              \
    /usr/X11/include          \
    /usr/include/X11          \
    /usr/local/X11/include    \
    /usr/local/include/X11    \
                              \
    /usr/X386/include         \
    /usr/x386/include         \
    /usr/XFree86/include/X11  \
                              \
    /usr/dt/include           \
                              \
    /usr/local/include        \
    /usr/remote/include       \
    /usr/include              \
    /usr/unsupported/include  \
    /usr/local/x11r5/include  \
    /usr/lpp/Xamples/include  \
    $extra_include            \
    ; \
  do
    if test -r "$ac_dir/$xawplus_direct_test_include"; then
      no_xawplus= ac_xawplus_includes=$ac_dir
      break
    fi
  done

# Check for the libraries.
# See if we find them without any special options.
# Don't add to $LIBS permanently.
ac_save_LIBS="$LIBS"
LIBS="-l$xawplus_direct_test_library $LIBS"
# First see if replacing the include by lib works.
for ac_dir in `echo "$ac_xawplus_includes" | sed s/include/lib/` \
    /usr/athena/lib       \
                          \
    /usr/X11R6/lib        \
    /usr/X11R5/lib        \
    /usr/X11R4/lib        \
                          \
    /usr/lib/X11R6        \
    /usr/lib/X11R5        \
    /usr/lib/X11R4        \
                          \
    /usr/local/X11R6/lib  \
    /usr/local/X11R5/lib  \
    /usr/local/X11R4/lib  \
                          \
    /usr/local/lib/X11R6  \
    /usr/local/lib/X11R5  \
    /usr/local/lib/X11R4  \
                          \
    /usr/X11/lib          \
    /usr/lib/X11          \
    /usr/local/X11/lib    \
                          \
    /usr/X386/lib         \
    /usr/x386/lib         \
    /usr/XFree86/lib/X11  \
                          \
    /usr/lib              \
    /usr/local/lib        \
    /usr/remote/lib       \
    /usr/unsupported/lib  \
    /usr/local/x11r5/lib  \
    /usr/lpp/Xamples/lib  \
    $extra_lib            \
    ; \
do
  for ac_extension in a so sl; do
    if test -r $ac_dir/lib${xawplus_direct_test_library}.$ac_extension; then
      no_xawplus= ac_xawplus_libraries=$ac_dir
      break 2
    fi
  done
done
LIBS="$ac_save_LIBS"])
AC_DEFUN([AC_PATH_XAWPLUS],
[AC_REQUIRE_CPP()dnl

xawplus_includes=NONE
xawplus_libraries=NONE

AC_MSG_CHECKING(for XawPlus)
AC_ARG_WITH(xawplus, [  --without-xawplus           disable XawPlus])
if test "x$with_xawplus" = xno; then
  no_xawplus=yes
else
  if test "x$xawplus_includes" != xNONE && test "x$xawplus_libraries" != xNONE; then
    no_xawplus=
  else
AC_CACHE_VAL(ac_cv_path_xawplus,
[# One or both of these vars are not set, and there is no cached value.
no_xawplus=yes
AC_PATH_XAWPLUS_DIRECT

if test "$no_xawplus" = yes; then
  ac_cv_path_xawplus="no_xawplus=yes"
else
  ac_cv_path_xawplus="no_xawplus= ac_xawplus_includes=$ac_xawplus_includes ac_xawplus_libraries=$ac_xawplus_libraries"
fi])dnl
  fi
  eval "$ac_cv_path_xawplus"
fi # with_xawplus != no

if test "$no_motif" = yes; then
  if test "$no_xaw3dxft" = yes; then
    if test "$no_xaw3d" = yes; then
      if test "$no_nextaw" = yes; then
        if test "$no_xawplus" = yes; then
          AC_MSG_RESULT(no)
          XMLOCK=""
          INSTALL_XMLOCK=""
          UNINSTALL_XMLOCK=""
        else
          AC_DEFINE([HAVE_ATHENA], [1], [Athena is available])
          AC_DEFINE([HAVE_LIB_XAWPLUS], [1], [XawPlus is available])
          XMLOCKLIBS="-lXawPlus ${XMLOCKLIBS}"
          XMLOCK="xmlock"
          INSTALL_XMLOCK="install_xmlock"
          UNINSTALL_XMLOCK="uninstall_xmlock"
          test "x$xawplus_includes" = xNONE && xawplus_includes=$ac_xawplus_includes
          test "x$xawplus_libraries" = xNONE && xawplus_libraries=$ac_xawplus_libraries
          ac_cv_path_xawplus="no_xawplus= ac_xawplus_includes=$xawplus_includes ac_xawplus_libraries=$xawplus_libraries"
          AC_MSG_RESULT([libraries $xawplus_libraries, headers $xawplus_includes])
        fi
      else
        AC_MSG_RESULT(ignored - using neXtaw)
      fi
    else
      AC_MSG_RESULT(ignored - using Xaw3d)
    fi
  else
    AC_MSG_RESULT(ignored - using Xaw3dxft)
  fi
else
  AC_MSG_RESULT(ignored - using Motif)
fi
])

AC_PATH_XAWPLUS

if test "x$xawplus_libraries" != x ; then
  XWIDGET_LDFLAGS="${XWIDGET_LDFLAGS}:$xawplus_libraries"
  XWIDGETLIBPATHS="-L$xawplus_libraries ${XWIDGETLIBPATHS}"
fi
if test "x$xawplus_includes" != x ; then
  XWIDGETINC="${XWIDGETINC} -I$xawplus_includes"
fi

dnl Xaw XAW xaw

AC_DEFUN([AC_PATH_XAW_DIRECT],
[test -z "$xaw_direct_test_library" && xaw_direct_test_library=Xaw
test -z "$xaw_direct_test_function" && xaw_direct_test_function=XawDialGetValueString
test -z "$xaw_direct_test_include" && xaw_direct_test_include=X11/Xaw/Command.h
  for ac_dir in               \
    /usr/athena/include       \
                              \
    /usr/X11R6/include        \
    /usr/X11R5/include        \
    /usr/X11R4/include        \
                              \
    /usr/include/X11R6        \
    /usr/include/X11R5        \
    /usr/include/X11R4        \
                              \
    /usr/local/X11R6/include  \
    /usr/local/X11R5/include  \
    /usr/local/X11R4/include  \
                              \
    /usr/local/include/X11R6  \
    /usr/local/include/X11R5  \
    /usr/local/include/X11R4  \
                              \
    /usr/X11/include          \
    /usr/include/X11          \
    /usr/local/X11/include    \
    /usr/local/include/X11    \
                              \
    /usr/X386/include         \
    /usr/x386/include         \
    /usr/XFree86/include/X11  \
                              \
    /usr/dt/include           \
                              \
    /usr/local/include        \
    /usr/remote/include       \
    /usr/include              \
    /usr/unsupported/include  \
    /usr/local/x11r5/include  \
    /usr/lpp/Xamples/include  \
    $extra_include            \
    ; \
  do
    if test -r "$ac_dir/$xaw_direct_test_include"; then
      no_xaw= ac_xaw_includes=$ac_dir
      break
    fi
  done

# Check for the libraries.
# See if we find them without any special options.
# Don't add to $LIBS permanently.
ac_save_LIBS="$LIBS"
LIBS="-l$xaw_direct_test_library $LIBS"
# First see if replacing the include by lib works.
for ac_dir in `echo "$ac_xaw_includes" | sed s/include/lib/` \
    /usr/athena/lib       \
                          \
    /usr/X11R6/lib        \
    /usr/X11R5/lib        \
    /usr/X11R4/lib        \
                          \
    /usr/lib/X11R6        \
    /usr/lib/X11R5        \
    /usr/lib/X11R4        \
                          \
    /usr/local/X11R6/lib  \
    /usr/local/X11R5/lib  \
    /usr/local/X11R4/lib  \
                          \
    /usr/local/lib/X11R6  \
    /usr/local/lib/X11R5  \
    /usr/local/lib/X11R4  \
                          \
    /usr/X11/lib          \
    /usr/lib/X11          \
    /usr/local/X11/lib    \
                          \
    /usr/X386/lib         \
    /usr/x386/lib         \
    /usr/XFree86/lib/X11  \
                          \
    /usr/lib              \
    /usr/local/lib        \
    /usr/remote/lib       \
    /usr/unsupported/lib  \
    /usr/local/x11r5/lib  \
    /usr/lpp/Xamples/lib  \
    $extra_lib            \
    ; \
do
  for ac_extension in a so sl; do
    if test -r $ac_dir/lib${xaw_direct_test_library}.$ac_extension; then
      no_xaw= ac_xaw_libraries=$ac_dir
      break 2
    fi
  done
done
LIBS="$ac_save_LIBS"])
AC_DEFUN([AC_PATH_XAW],
[AC_REQUIRE_CPP()dnl

xaw_includes=NONE
xaw_libraries=NONE

AC_MSG_CHECKING(for Xaw)
AC_ARG_WITH(xaw, [  --without-xaw           disable Xaw])
if test "x$with_xaw" = xno; then
  no_xaw=yes
else
  if test "x$xaw_includes" != xNONE && test "x$xaw_libraries" != xNONE; then
    no_xaw=
  else
AC_CACHE_VAL(ac_cv_path_xaw,
[# One or both of these vars are not set, and there is no cached value.
no_xaw=yes
AC_PATH_XAW_DIRECT

if test "$no_xaw" = yes; then
  ac_cv_path_xaw="no_xaw=yes"
else
  ac_cv_path_xaw="no_xaw= ac_xaw_includes=$ac_xaw_includes ac_xaw_libraries=$ac_xaw_libraries"
fi])dnl
  fi
  eval "$ac_cv_path_xaw"
fi # with_xaw != no

if test "$no_motif" = yes; then
  if test "$no_xaw3dxft" = yes; then
    if test "$no_xaw3d" = yes; then
      if test "$no_nextaw" = yes; then
        if test "$no_xawplus" = yes; then
          if test "$no_xaw" = yes; then
            AC_MSG_RESULT(no)
            XMLOCK=""
            INSTALL_XMLOCK=""
            UNINSTALL_XMLOCK=""
          else
            AC_DEFINE([HAVE_ATHENA], [1], [Athena is available])
            XMLOCKLIBS="-lXaw ${XMLOCKLIBS}"
            XMLOCK="xmlock"
            INSTALL_XMLOCK="install_xmlock"
            UNINSTALL_XMLOCK="uninstall_xmlock"
            test "x$xaw_includes" = xNONE && xaw_includes=$ac_xaw_includes
            test "x$xaw_libraries" = xNONE && xaw_libraries=$ac_xaw_libraries
            ac_cv_path_xaw="no_xaw= ac_xaw_includes=$xaw_includes ac_xaw_libraries=$xaw_libraries"
            AC_MSG_RESULT([libraries $xaw_libraries, headers $xaw_includes])
          fi
        else
          AC_MSG_RESULT(ignored - using XawPlus)
        fi
      else
        AC_MSG_RESULT(ignored - using neXtaw)
      fi
    else
      AC_MSG_RESULT(ignored - using Xaw3d)
    fi
  else
    AC_MSG_RESULT(ignored - using Xaw3dxft)
  fi
else
  AC_MSG_RESULT(ignored - using Motif)
fi
])

AC_PATH_XAW

if test "x$xaw_libraries" != x ; then
  XWIDGET_LDFLAGS="${XWIDGET_LDFLAGS}:$xaw_libraries"
  XWIDGETLIBPATHS="-L$xaw_libraries ${XWIDGETLIBPATHS}"
fi
if test "x$xaw_includes" != x ; then
  XWIDGETINC="${XWIDGETINC} -I$xaw_includes"
fi

dnl Xmu EDITRES Editres editres

AC_DEFUN([AC_PATH_EDITRES_DIRECT],
[test -z "$editres_direct_test_library" && editres_direct_test_library=Xmu
test -z "$editres_direct_test_function" && editres_direct_test_function=_XEditResCheckMessages
test -z "$editres_direct_test_include" && editres_direct_test_include=X11/Xmu/Editres.h
  for ac_dir in               \
    /usr/include/X11R6.5.1    \
    /usr/include/X11R6.4      \
    /usr/include/X11R6.3      \
    /usr/include/X11R6.2      \
    /usr/include/X11R6.1      \
    /usr/X11R6/include        \
    /usr/X11R5/include        \
    /usr/X11R4/include        \
    /usr/X11/include          \
                              \
    /usr/include/X11          \
    /usr/include/X11R6.5.1    \
    /usr/include/X11R6.4      \
    /usr/include/X11R6.3      \
    /usr/include/X11R6.2      \
    /usr/include/X11R6.1      \
    /usr/include/X11R6        \
    /usr/include/X11R5        \
    /usr/include/X11R4        \
                              \
    /usr/local/X11/include    \
    /usr/local/X11R6.5.1/include  \
    /usr/local/X11R6.4/include  \
    /usr/local/X11R6.3/include  \
    /usr/local/X11R6.2/include  \
    /usr/local/X11R6.1/include  \
    /usr/local/X11R6/include  \
    /usr/local/X11R5/include  \
    /usr/local/X11R4/include  \
                              \
    /usr/local/include/X11    \
    /usr/local/include/X11R6.5.1  \
    /usr/local/include/X11R6.4  \
    /usr/local/include/X11R6.3  \
    /usr/local/include/X11R6.2  \
    /usr/local/include/X11R6.1  \
    /usr/local/include/X11R6  \
    /usr/local/include/X11R5  \
    /usr/local/include/X11R4  \
                              \
    /usr/X386/include         \
    /usr/x386/include         \
    /usr/XFree86/include      \
    /usr/pkg/include          \
                              \
    /usr/local/include        \
    /usr/remote/include       \
    /usr/include              \
    /usr/unsupported/include  \
    /usr/athena/include       \
    /usr/local/x11r5/include  \
    /usr/lpp/Xamples/include  \
                              \
    /usr/openwin/include      \
    /usr/openwin/share/include \
    $extra_include            \
    ; \
  do
    if test -r "$ac_dir/$editres_direct_test_include"; then
      no_editres= ac_editres_includes=$ac_dir
      break
    fi
  done

# Check for the libraries.
# See if we find them without any special options.
# Do not add to $LIBS permanently.
ac_save_LIBS="$LIBS"
LIBS="-l$editres_direct_test_library $LIBS"
# First see if replacing the include by lib works.
for ac_dir in `echo "$ac_editres_includes" | sed s/include/lib/` \
                          \
    /usr/X11/lib          \
    /usr/X11R6.5.1/lib      \
    /usr/X11R6.4/lib        \
    /usr/X11R6.3/lib        \
    /usr/X11R6.2/lib        \
    /usr/X11R6.1/lib        \
    /usr/X11R6/lib        \
    /usr/X11R5/lib        \
    /usr/X11R4/lib        \
                          \
    /usr/lib/X11          \
    /usr/lib/X11R6.5.1      \
    /usr/lib/X11R6.4        \
    /usr/lib/X11R6.3        \
    /usr/lib/X11R6.2        \
    /usr/lib/X11R6.1        \
    /usr/lib/X11R6        \
    /usr/lib/X11R5        \
    /usr/lib/X11R4        \
                          \
    /usr/local/X11/lib    \
    /usr/local/X11R6.5.1/lib  \
    /usr/local/X11R6.4/lib  \
    /usr/local/X11R6.3/lib  \
    /usr/local/X11R6.2/lib  \
    /usr/local/X11R6.1/lib  \
    /usr/local/X11R6/lib  \
    /usr/local/X11R5/lib  \
    /usr/local/X11R4/lib  \
                          \
    /usr/local/lib/X11    \
    /usr/local/lib/X11R6.5.1  \
    /usr/local/lib/X11R6.4  \
    /usr/local/lib/X11R6.3  \
    /usr/local/lib/X11R6.2  \
    /usr/local/lib/X11R6.1  \
    /usr/local/lib/X11R6  \
    /usr/local/lib/X11R5  \
    /usr/local/lib/X11R4  \
                          \
    /usr/X386/lib         \
    /usr/x386/lib         \
    /usr/XFree86/lib/X11  \
    /usr/pkg/lib          \
                          \
    /usr/lib              \
    /usr/local/lib        \
    /usr/remote/lib       \
    /usr/unsupported/lib  \
    /usr/athena/lib       \
    /usr/local/x11r5/lib  \
    /usr/lpp/Xamples/lib  \
    /lib/usr/lib/X11      \
                          \
    /usr/openwin/lib      \
    /usr/openwin/share/lib \
    $extra_lib            \
    ; \
do
  for ac_extension in a so sl; do
    if test -r $ac_dir/lib${editres_direct_test_library}.$ac_extension; then
      no_editres= ac_editres_libraries=$ac_dir
      break 2
    fi
  done
done
LIBS="$ac_save_LIBS"])
AC_DEFUN([AC_PATH_EDITRES],
[AC_REQUIRE_CPP()dnl

editres_includes=NONE
editres_libraries=NONE

AC_MSG_CHECKING(for Editres)
AC_ARG_WITH(editres, [  --without-editres       disable debugger (for x?lock)])
if test "x$with_editres" = xno; then
  no_editres=yes
else
  if test "x$editres_includes" != xNONE && test "x$editres_libraries" != xNONE; then
    no_editres=
  else
AC_CACHE_VAL(ac_cv_path_editres,
[# One or both of these vars are not set, and there is no cached value.
no_editres=yes
AC_PATH_EDITRES_DIRECT

if test "$no_editres" = yes; then
  ac_cv_path_editres="no_editres=yes"
else
  ac_cv_path_editres="no_editres= ac_editres_includes=$ac_editres_includes ac_editres_libraries=$ac_editres_libraries"
fi])dnl
  fi
  eval "$ac_cv_path_editres"
fi # with_editres != no

if test "$no_editres" = yes; then
  AC_MSG_RESULT(no)
else
  AC_DEFINE([USE_XMU], [1], [using X11 miscellaneous utilities])
  XLOCKLIBS="${XLOCKLIBS} -lXmu"
  XMLOCKLIBS="-lXmu ${XMLOCKLIBS}"
  test "x$editres_includes" = xNONE && editres_includes=$ac_editres_includes
  test "x$editres_libraries" = xNONE && editres_libraries=$ac_editres_libraries
  ac_cv_path_editres="no_editres= ac_editres_includes=$editres_includes ac_editres_libraries=$editres_libraries"
  AC_MSG_RESULT([libraries $editres_libraries, headers $editres_includes])
fi
])

AC_PATH_EDITRES

if test "x$editres_libraries" != x && test "x$editres_libraries" != xNONE ; then
  XMLOCK_LDFLAGS="${XMLOCK_LDFLAGS}:$editres_libraries"
  XMLOCKLIBPATHS="${XMLOCKLIBPATHS} -L$editres_libraries"
fi
if test "x$editres_includes" != x && test "x$editres_includes" != xNONE ; then
  XMLOCKINC="${XMLOCKINC} -I$editres_includes"
fi

xpm=no
dnl Xpm XPM xpm

AC_DEFUN([AC_PATH_XPM_DIRECT],
[test -z "$xpm_direct_test_library" && xpm_direct_test_library=Xpm
test -z "$xpm_direct_test_function" && xpm_direct_test_function=XpmCreateImageFromData
test -z "$xpm_direct_test_include" && xpm_direct_test_include=X11/xpm.h
  for ac_dir in               \
    /usr/X11R6.5.1/include    \
    /usr/X11R6.4/include      \
    /usr/X11R6.3/include      \
    /usr/X11R6.2/include      \
    /usr/X11R6.1/include      \
    /usr/X11R6/include        \
    /usr/X11R5/include        \
    /usr/X11R4/include        \
    /usr/X11/include          \
                              \
    /usr/include/X11          \
    /usr/include/X11R6.5.1    \
    /usr/include/X11R6.4      \
    /usr/include/X11R6.3      \
    /usr/include/X11R6.2      \
    /usr/include/X11R6.1      \
    /usr/include/X11R6        \
    /usr/include/X11R5        \
    /usr/include/X11R4        \
                              \
    /usr/local/X11/include    \
    /usr/local/X11R6.5.1/include  \
    /usr/local/X11R6.4/include  \
    /usr/local/X11R6.3/include  \
    /usr/local/X11R6.2/include  \
    /usr/local/X11R6.1/include  \
    /usr/local/X11R6/include  \
    /usr/local/X11R5/include  \
    /usr/local/X11R4/include  \
                              \
    /usr/local/include/X11    \
    /usr/local/include/X11R6.5.1  \
    /usr/local/include/X11R6.4  \
    /usr/local/include/X11R6.3  \
    /usr/local/include/X11R6.2  \
    /usr/local/include/X11R6.1  \
    /usr/local/include/X11R6  \
    /usr/local/include/X11R5  \
    /usr/local/include/X11R4  \
                              \
    /usr/X386/include         \
    /usr/x386/include         \
    /usr/XFree86/include/X11  \
    /usr/pkg/include          \
                              \
    /usr/local/include        \
    /usr/remote/include       \
    /usr/include              \
    /usr/unsupported/include  \
    /usr/athena/include       \
    /usr/local/x11r5/include  \
    /usr/lpp/Xamples/include  \
                              \
    /usr/openwin/include      \
    /usr/openwin/share/include \
    /usr/openwin/include      \
                              \
    /usr/include/Vk           \
    $extra_include            \
    ; \
  do
    if test -r "$ac_dir/$xpm_direct_test_include"; then
      no_xpm= ac_xpm_includes=$ac_dir
      break
    fi
  done

# Check for the libraries.
# See if we find them without any special options.
# Do not add to $LIBS permanently.
ac_save_LIBS="$LIBS"
LIBS="-l$xpm_direct_test_library $LIBS"
# First see if replacing the include by lib works.
for ac_dir in `echo "$ac_xpm_includes" | sed s%/X11$%% | sed s/include/lib/` \
                          \
    /usr/X11/lib          \
    /usr/X11R6.5.1/lib    \
    /usr/X11R6.4/lib      \
    /usr/X11R6.3/lib      \
    /usr/X11R6.2/lib      \
    /usr/X11R6.1/lib      \
    /usr/X11R6/lib        \
    /usr/X11R5/lib        \
    /usr/X11R4/lib        \
                          \
    /usr/lib/X11          \
    /usr/lib/X11R6.5.1    \
    /usr/lib/X11R6.4      \
    /usr/lib/X11R6.3      \
    /usr/lib/X11R6.2      \
    /usr/lib/X11R6.1      \
    /usr/lib/X11R6        \
    /usr/lib/X11R5        \
    /usr/lib/X11R4        \
                          \
    /usr/local/X11/lib    \
    /usr/local/X11R6/lib  \
    /usr/local/X11R5/lib  \
    /usr/local/X11R4/lib  \
                          \
    /usr/local/lib/X11    \
    /usr/local/X11R6.5.1/lib  \
    /usr/local/X11R6.4/lib  \
    /usr/local/X11R6.3/lib  \
    /usr/local/X11R6.2/lib  \
    /usr/local/X11R6.1/lib  \
    /usr/local/lib/X11R6  \
    /usr/local/lib/X11R5  \
    /usr/local/lib/X11R4  \
                          \
    /usr/X386/lib         \
    /usr/x386/lib         \
    /usr/XFree86/lib/X11  \
    /usr/pkg/lib          \
                          \
    /usr/lib              \
    /usr/local/lib        \
    /usr/remote/lib       \
    /usr/unsupported/lib  \
    /usr/athena/lib       \
    /usr/local/x11r5/lib  \
    /usr/lpp/Xamples/lib  \
    /lib/usr/lib/X11      \
                          \
    /usr/openwin/lib      \
    /usr/openwin/share/lib \
    $extra_lib            \
    ; \
do
  for ac_extension in a so sl; do
    if test -r $ac_dir/lib${xpm_direct_test_library}.$ac_extension; then
      no_xpm= ac_xpm_libraries=$ac_dir
      break 2
    fi
  done
done
LIBS="$ac_save_LIBS"])
AC_DEFUN([AC_PATH_XPM],
[AC_REQUIRE_CPP()dnl

xpm_includes=NONE
xpm_libraries=NONE

AC_MSG_CHECKING(for XPM)
AC_ARG_WITH(xpm, [  --without-xpm           disable color pixmap XPM mode(s)])
if test "x$with_xpm" = xno; then
  no_xpm=yes
else
  if test "x$xpm_includes" != xNONE && test "x$xpm_libraries" != xNONE; then
    no_xpm=
  else
AC_CACHE_VAL(ac_cv_path_xpm,
[# One or both of these vars are not set, and there is no cached value.
no_xpm=yes
AC_PATH_XPM_DIRECT

if test "$no_xpm" = yes; then
  ac_cv_path_xpm="no_xpm=yes"
else
  ac_cv_path_xpm="no_xpm= ac_xpm_includes=$ac_xpm_includes ac_xpm_libraries=$ac_xpm_libraries"
fi])dnl
  fi
  eval "$ac_cv_path_xpm"
fi # with_xpm != no

if test "$no_xpm" = yes; then
  AC_MSG_RESULT(no)
  XPM="#"
else
  xpm=yes
  AC_DEFINE([HAVE_XPM], [1], [XPM color images])
  XLOCKLIBS="${XLOCKLIBS} -lXpm"
  XMLOCKLIBS="-lXpm ${XMLOCKLIBS}"
  test "x$xpm_includes" = xNONE && xpm_includes=$ac_xpm_includes
  test "x$xpm_libraries" = xNONE && xpm_libraries=$ac_xpm_libraries
  ac_cv_path_xpm="no_xpm= ac_xpm_includes=$xpm_includes ac_xpm_libraries=$xpm_libraries"
  AC_MSG_RESULT([libraries $xpm_libraries, headers $xpm_includes])
  XPM=""
fi
])

AC_PATH_XPM

if test "x$xpm_libraries" != x && test "x$xpm_libraries" != xNONE ; then
  XLOCK_LDFLAGS="${XLOCK_LDFLAGS}:$xpm_libraries"
  XMLOCK_LDFLAGS="${XMLOCK_LDFLAGS}:$xpm_libraries"
  XLOCKLIBPATHS="${XLOCKLIBPATHS} -L$xpm_libraries"
  XMLOCKLIBPATHS="${XMLOCKLIBPATHS} -L$xpm_libraries"
fi
if test "x$xpm_includes" != x && test "x$xpm_includes" != xNONE ; then
  XLOCKINC="${XLOCKINC} -I$xpm_includes"
fi

png=no
dnl png LIBPNG

AC_DEFUN([AC_PATH_LIBPNG_DIRECT],
[test -z "$png_direct_test_library" && png_direct_test_library=png
test -z "$png_direct_test_include" && png_direct_test_include=png.h
  for ac_dir in               \
    /usr/X11R6.5.1/include    \
    /usr/X11R6.4/include      \
    /usr/X11R6.3/include      \
    /usr/X11R6.2/include      \
    /usr/X11R6.1/include      \
    /usr/X11R6/include        \
    /usr/X11R5/include        \
    /usr/X11R4/include        \
    /usr/X11/include          \
                              \
    /usr/include/X11          \
    /usr/include/X11R6.5.1    \
    /usr/include/X11R6.4      \
    /usr/include/X11R6.3      \
    /usr/include/X11R6.2      \
    /usr/include/X11R6.1      \
    /usr/include/X11R6        \
    /usr/include/X11R5        \
    /usr/include/X11R4        \
                              \
    /usr/local/X11/include    \
    /usr/local/X11R6.5.1/include  \
    /usr/local/X11R6.4/include  \
    /usr/local/X11R6.3/include  \
    /usr/local/X11R6.2/include  \
    /usr/local/X11R6.1/include  \
    /usr/local/X11R6/include  \
    /usr/local/X11R5/include  \
    /usr/local/X11R4/include  \
                              \
    /usr/local/include/X11    \
    /usr/local/include/X11R6.5.1  \
    /usr/local/include/X11R6.4  \
    /usr/local/include/X11R6.3  \
    /usr/local/include/X11R6.2  \
    /usr/local/include/X11R6.1  \
    /usr/local/include/X11R6  \
    /usr/local/include/X11R5  \
    /usr/local/include/X11R4  \
                              \
    /usr/X386/include         \
    /usr/x386/include         \
    /usr/XFree86/include/X11  \
    /usr/pkg/include          \
                              \
    /usr/local/include        \
    /usr/remote/include       \
    /usr/include              \
    /usr/unsupported/include  \
    /usr/athena/include       \
    /usr/local/x11r5/include  \
    /usr/lpp/Xamples/include  \
                              \
    /usr/openwin/include      \
    /usr/openwin/share/include \
    /usr/openwin/include      \
                              \
    /usr/include/Vk           \
    $extra_include            \
    ; \
  do
    if test -r "$ac_dir/$png_direct_test_include"; then
      no_libpng= ac_png_includes=$ac_dir
      break
    fi
  done

# Check for the libraries.
# See if we find them without any special options.  # Do not add to $LIBS permanently.
ac_save_LIBS="$LIBS"
LIBS="-l$png_direct_test_library $LIBS"
# First see if replacing the include by lib works.
for ac_dir in `echo "$ac_png_includes" | sed s/include/lib/` \
                          \
    /usr/X11/lib          \
    /usr/X11R6.5.1/lib    \
    /usr/X11R6.4/lib      \
    /usr/X11R6.3/lib      \
    /usr/X11R6.2/lib      \
    /usr/X11R6.1/lib      \
    /usr/X11R6/lib        \
    /usr/X11R5/lib        \
    /usr/X11R4/lib        \
                          \
    /usr/lib/X11          \
    /usr/lib/X11R6.5.1    \
    /usr/lib/X11R6.4      \
    /usr/lib/X11R6.3      \
    /usr/lib/X11R6.2      \
    /usr/lib/X11R6.1      \
    /usr/lib/X11R6        \
    /usr/lib/X11R5        \
    /usr/lib/X11R4        \
                          \
    /usr/local/X11/lib    \
    /usr/local/X11R6/lib  \
    /usr/local/X11R5/lib  \
    /usr/local/X11R4/lib  \
                          \
    /usr/local/lib/X11    \
    /usr/local/X11R6.5.1/lib  \
    /usr/local/X11R6.4/lib  \
    /usr/local/X11R6.3/lib  \
    /usr/local/X11R6.2/lib  \
    /usr/local/X11R6.1/lib  \
    /usr/local/lib/X11R6  \
    /usr/local/lib/X11R5  \
    /usr/local/lib/X11R4  \
                          \
    /usr/X386/lib         \
    /usr/x386/lib         \
    /usr/XFree86/lib/X11  \
    /usr/pkg/lib          \
                          \
    /usr/lib              \
    /usr/local/lib        \
    /usr/remote/lib       \
    /usr/unsupported/lib  \
    /usr/athena/lib       \
    /usr/local/x11r5/lib  \
    /usr/lpp/Xamples/lib  \
    /lib/usr/lib/X11      \
                          \
    /usr/openwin/lib      \
    /usr/openwin/share/lib \
    $extra_lib            \
    ; \
do
  for ac_extension in a so sl; do
    if test -r $ac_dir/lib${png_direct_test_library}.$ac_extension; then
      no_libpng= ac_png_libraries=$ac_dir
      break 2
    fi
  done
done
LIBS="$ac_save_LIBS"])
AC_DEFUN([AC_PATH_LIBPNG],
[AC_REQUIRE_CPP()dnl

png_includes=NONE
png_libraries=NONE

AC_MSG_CHECKING(for LIBPNG)
AC_ARG_WITH(libpng, [  --without-libpng         disable color pixmap LIBPNG mode(s)])
if test "x$with_libpng" = xno; then
  no_libpng=yes
else
  if test "x$png_includes" != xNONE && test "x$png_libraries" != xNONE; then
    no_libpng=
  else
AC_CACHE_VAL(ac_cv_path_png,
[# One or both of these vars are not set, and there is no cached value.
no_libpng=yes
AC_PATH_LIBPNG_DIRECT

if test "$no_libpng" = yes; then
  ac_cv_path_png="no_libpng=yes"
else
  ac_cv_path_png="no_libpng= ac_png_includes=$ac_png_includes ac_png_libraries=$ac_png_libraries"
fi])dnl
  fi
  eval "$ac_cv_path_png"
fi # with_libpng != no

if test "$no_libpng" = yes; then
  AC_MSG_RESULT(no)
  LIBPNG="#"
else
  libpng=yes
  AC_DEFINE([HAVE_LIBPNG], [1], [PNG images])
  XLOCKLIBS="${XLOCKLIBS} -lpng"
  test "x$png_includes" = xNONE && png_includes=$ac_png_includes
  test "x$png_libraries" = xNONE && png_libraries=$ac_png_libraries
  ac_cv_path_png="no_libpng= ac_png_includes=$png_includes ac_png_libraries=$png_libraries"
  AC_MSG_RESULT([libraries $png_libraries, headers $png_includes])
  LIBPNG=""
fi
])

AC_PATH_LIBPNG

if test "x$png_libraries" != x && test "x$png_libraries" != xNONE ; then
  XLOCK_LDFLAGS="${XLOCK_LDFLAGS}:$png_libraries"
  XLOCKLIBPATHS="${XLOCKLIBPATHS} -L$png_libraries"
fi
if test "x$png_includes" != x && test "x$png_includes" != xNONE ; then
  XLOCKINC="${XLOCKINC} -I$png_includes"
fi

dnl MAGICK ImageMagick

AC_DEFUN([AC_PATH_MAGICK],
[AC_REQUIRE_CPP()dnl

magick_includes=NONE
magick_libraries=NONE

AC_MSG_CHECKING(for MAGICK)
AC_ARG_WITH(magick,            [  --without-magick        disable ImageMagick])
AC_ARG_WITH(magick_config,     [  --with-magick-config    use this configator for ImageMagick])
AC_ARG_WITH(magick_prefix,     [  --with-magick-prefix    use this prefix for ImageMagick])
AC_ARG_WITH(magick_includes,   [  --with-magick-includes  use this dir for ImageMagick headers])
AC_ARG_WITH(magick_libraries,  [  --with-magick-libraries use this dir for ImageMagick libs])
if test "x$with_magick" = xno; then
  no_magick=yes
else
  if test "x$magick_includes" != xNONE && test "x$magick_libraries" != xNONE; then
    no_magick=
    if test "x$magick_config" = "x"; then
      if test "x$magick_prefix" != "x"; then
        magick_config="$magick_prefix/bin/Magick-config"
      else
        magick_config="Magick-config"
     fi
   else
     if test "x$magick_prefix" = "x"; then
        magick_prefix=`($magick_config --prefix) 2>/dev/null`
     fi
   fi
  else
    if test "x$magick_config" = "x"; then
      if test "x$magick_prefix" != "x"; then
        magick_config="$magick_prefix/bin/Magick-config"
      else
        magick_config="Magick-config"
     fi
   else
     if test "x$magick_prefix" = "x"; then
        magick_prefix=`($magick_config --prefix) 2>/dev/null`
     fi
   fi
dnl need magick/MagickCore.h, magick/api.h (Magick-config --version 1.3.28) is not going to work
AC_CACHE_VAL(ac_cv_path_magick,
[# One or both of these vars are not set, and there is no cached value.
no_magick=yes
no_magick_config=yes
if test "x$magick_prefix" != "x"; then
  ac_magick_prefix=$magick_prefix
  ac_magick_includes=$ac_magick_prefix/include
  ac_magick_libraries=$ac_magick_prefix/lib
  no_magick="ac_magick_config=$ac_magick_config ac_magick_prefix=$ac_magick_prefix ac_magick_includes=$ac_magick_includes ac_magick_libraries=$ac_magick_libraries"
else
  ac_magick_config="$magick_config"
  ac_magick_prefix=`($magick_config --prefix) 2>/dev/null`
  if test $? != 0; then
    no_magick=yes
    no_magick_config=yes
    ac_magick_config=""
    ac_magick_prefix=""
  else
    no_magick=
    no_magick_config=
    ac_magick_includes=$ac_magick_prefix/include
    ac_magick_libraries=$ac_magick_prefix/lib
  fi
fi
if test "$no_magick" = yes; then
  ac_cv_path_magick="no_magick=yes"
else
  ac_cv_path_magick="no_magick= ac_magick_config=\"$ac_magick_config\" ac_magick_prefix=\"$ac_magick_prefix\" ac_magick_includes=\"$ac_magick_includes\" ac_magick_libraries=\"$ac_magick_libraries\""
fi])dnl
  fi
  eval "$ac_cv_path_magick"
fi # with_magick != no

if test "$no_magick_config" = yes; then
  AC_MSG_RESULT(no Magick-config)
else
  if test "$no_magick" = yes; then
    AC_MSG_RESULT(no)
  else
    AC_DEFINE([USE_MAGICK], [1], [Image Magick])
    test "x$magick_config" = xNONE && magick_config=$ac_magick_config
    test "x$magick_prefix" = xNONE && magick_prefix=$ac_magick_prefix
    test "x$magick_includes" = xNONE && magick_includes=$ac_magick_includes
    test "x$magick_libraries" = xNONE && magick_libraries=$ac_magick_libraries
    ac_cv_path_magick="no_magick= ac_magick_config=$magick_config ac_magick_prefix=$magick_prefix ac_magick_includes=$magick_includes ac_magick_libraries=$magick_libraries"
    AC_MSG_RESULT([libraries $magick_libraries, headers $magick_includes])
  fi
fi
])

AC_PATH_MAGICK

if test "x$magick_libraries" != x && test "x$magick_libraries" != xNONE ; then
  XLOCKLIBS="${XLOCKLIBS} `$magick_config --libs`"
  XLOCK_LDFLAGS="${XLOCK_LDFLAGS}:$magick_libraries"
  XLOCKLIBPATHS="${XLOCKLIBPATHS} -L$magick_libraries"
fi
if test "x$magick_includes" != x && test "x$magick_includes" != xNONE ; then
  XLOCKINC="${XLOCKINC} -I$magick_includes `$magick_config --cppflags`"
fi

gltt=no
dnl gltt GLTT

AC_DEFUN([AC_PATH_GLTT_DIRECT],
[test -z "$gltt_direct_test_library" && gltt_direct_test_library=gltt
# test -z "$gltt_direct_test_function" && gltt_direct_test_function=glttCreateImageFromData
test -z "$gltt_direct_test_include" && gltt_direct_test_include=gltt/GLTTFont.h
  for ac_dir in               \
    /usr/X11R6.5.1/include    \
    /usr/X11R6.4/include      \
    /usr/X11R6.3/include      \
    /usr/X11R6.2/include      \
    /usr/X11R6.1/include      \
    /usr/X11R6/include        \
    /usr/X11R5/include        \
    /usr/X11R4/include        \
    /usr/X11/include          \
                              \
    /usr/include/X11          \
    /usr/include/X11R6.5.1    \
    /usr/include/X11R6.4      \
    /usr/include/X11R6.3      \
    /usr/include/X11R6.2      \
    /usr/include/X11R6.1      \
    /usr/include/X11R6        \
    /usr/include/X11R5        \
    /usr/include/X11R4        \
                              \
    /usr/local/X11/include    \
    /usr/local/X11R6.5.1/include  \
    /usr/local/X11R6.4/include  \
    /usr/local/X11R6.3/include  \
    /usr/local/X11R6.2/include  \
    /usr/local/X11R6.1/include  \
    /usr/local/X11R6/include  \
    /usr/local/X11R5/include  \
    /usr/local/X11R4/include  \
                              \
    /usr/local/include/X11    \
    /usr/local/include/X11R6.5.1  \
    /usr/local/include/X11R6.4  \
    /usr/local/include/X11R6.3  \
    /usr/local/include/X11R6.2  \
    /usr/local/include/X11R6.1  \
    /usr/local/include/X11R6  \
    /usr/local/include/X11R5  \
    /usr/local/include/X11R4  \
                              \
    /usr/X386/include         \
    /usr/x386/include         \
    /usr/XFree86/include/X11  \
    /usr/pkg/include          \
                              \
    /usr/local/include        \
    /usr/remote/include       \
    /usr/include              \
    /usr/unsupported/include  \
    /usr/athena/include       \
    /usr/local/x11r5/include  \
    /usr/lpp/Xamples/include  \
                              \
    /usr/openwin/include      \
    /usr/openwin/share/include \
    /usr/openwin/include      \
                              \
    $extra_include            \
    ; \
  do
    if test -r "$ac_dir/$gltt_direct_test_include"; then
      no_gltt= ac_gltt_includes=$ac_dir
      break
    fi
  done

# Check for the libraries.
# See if we find them without any special options.
# Do not add to $LIBS permanently.
ac_save_LIBS="$LIBS"
LIBS="-l$gltt_direct_test_library $LIBS"
# First see if replacing the include by lib works.
for ac_dir in `echo "$ac_gltt_includes" | sed s%/X11$%% | sed s/include/lib/` \
                          \
    /usr/X11/lib          \
    /usr/X11R6.5.1/lib      \
    /usr/X11R6.4/lib        \
    /usr/X11R6.3/lib        \
    /usr/X11R6.2/lib        \
    /usr/X11R6.1/lib        \
    /usr/X11R6/lib        \
    /usr/X11R5/lib        \
    /usr/X11R4/lib        \
                          \
    /usr/lib/X11          \
    /usr/lib/X11R6.5.1      \
    /usr/lib/X11R6.4        \
    /usr/lib/X11R6.3        \
    /usr/lib/X11R6.2        \
    /usr/lib/X11R6.1        \
    /usr/lib/X11R6        \
    /usr/lib/X11R5        \
    /usr/lib/X11R4        \
                          \
    /usr/local/X11/lib    \
    /usr/local/X11R6/lib  \
    /usr/local/X11R5/lib  \
    /usr/local/X11R4/lib  \
                          \
    /usr/local/lib/X11    \
    /usr/local/X11R6.5.1/lib  \
    /usr/local/X11R6.4/lib  \
    /usr/local/X11R6.3/lib  \
    /usr/local/X11R6.2/lib  \
    /usr/local/X11R6.1/lib  \
    /usr/local/lib/X11R6  \
    /usr/local/lib/X11R5  \
    /usr/local/lib/X11R4  \
                          \
    /usr/X386/lib         \
    /usr/x386/lib         \
    /usr/XFree86/lib/X11  \
    /usr/pkg/lib          \
                          \
    /usr/lib              \
    /usr/local/lib        \
    /usr/remote/lib       \
    /usr/unsupported/lib  \
    /usr/athena/lib       \
    /usr/local/x11r5/lib  \
    /usr/lpp/Xamples/lib  \
    /lib/usr/lib/X11      \
                          \
    /usr/openwin/lib      \
    /usr/openwin/share/lib \
    $extra_lib            \
    ; \
do
  for ac_extension in a so sl; do
    if test -r $ac_dir/lib${gltt_direct_test_library}.$ac_extension; then
      no_gltt= ac_gltt_libraries=$ac_dir
      break 2
   fi
  done
done
LIBS="$ac_save_LIBS"])
AC_DEFUN([AC_PATH_GLTT],
[AC_REQUIRE_CPP()dnl

gltt_includes=NONE
gltt_libraries=NONE

AC_MSG_CHECKING(for GLTT)
AC_ARG_WITH(gltt, [  --without-gltt          disable GL True Text library])
if test "x$with_gltt" = xno; then
  no_gltt=yes
else
  if test "x$gltt_includes" != xNONE && test "x$gltt_libraries" != xNONE; then
    no_gltt=
  else
AC_CACHE_VAL(ac_cv_path_gltt,
[# One or both of these vars are not set, and there is no cached value.
no_gltt=yes
AC_PATH_GLTT_DIRECT

if test "$no_gltt" = yes; then
  ac_cv_path_gltt="no_gltt=yes"
else
  ac_cv_path_gltt="no_gltt= ac_gltt_includes=$ac_gltt_includes ac_gltt_libraries=$ac_gltt_libraries"
fi])dnl
  fi
  eval "$ac_cv_path_gltt"
fi # with_gltt != no

if test "$no_gltt" = yes; then
  AC_MSG_RESULT(no)
  GLTT="#"
else
  gltt=yes
  AC_DEFINE([HAVE_GLTT], [1], [GL True Text])
  XLOCKLIBS="${XLOCKLIBS} -lgltt"
  test "x$gltt_includes" = xNONE && gltt_includes=$ac_gltt_includes
  test "x$gltt_libraries" = xNONE && gltt_libraries=$ac_gltt_libraries
  ac_cv_path_gltt="no_gltt= ac_gltt_includes=$gltt_includes ac_gltt_libraries=$gltt_libraries"
  AC_MSG_RESULT([libraries $gltt_libraries, headers $gltt_includes])
  GLTT=""
fi
])

AC_PATH_GLTT

if test "x$gltt_libraries" != x && test "x$gltt_libraries" != xNONE ; then
  XLOCK_LDFLAGS="${XLOCK_LDFLAGS}:$gltt_libraries"
  XLOCKLIBPATHS="${XLOCKLIBPATHS} -L$gltt_libraries"
fi
if test "x$gltt_includes" != x && test "x$gltt_includes" != xNONE ; then
  XLOCKINC="${XLOCKINC} -I$gltt_includes"
fi

ttf=no
dnl ttf TTF

AC_DEFUN([AC_PATH_TTF_DIRECT],
[test -z "$ttf_direct_test_library" && ttf_direct_test_library=ttf
test -z "$ttf_direct_test_function" && ttf_direct_test_function=TT_Init_FreeType
test -z "$ttf_direct_test_include" && ttf_direct_test_include=gltt/FTFace.h
  for ac_dir in               \
    /usr/X11R6.5.1/include    \
    /usr/X11R6.4/include      \
    /usr/X11R6.3/include      \
    /usr/X11R6.2/include      \
    /usr/X11R6.1/include      \
    /usr/X11R6/include        \
    /usr/X11R5/include        \
    /usr/X11R4/include        \
    /usr/X11/include          \
                              \
    /usr/include/X11          \
    /usr/include/X11R6.5.1    \
    /usr/include/X11R6.4      \
    /usr/include/X11R6.3      \
    /usr/include/X11R6.2      \
    /usr/include/X11R6.1      \
    /usr/include/X11R6        \
    /usr/include/X11R5        \
    /usr/include/X11R4        \
                              \
    /usr/local/X11/include    \
    /usr/local/X11R6.5.1/include  \
    /usr/local/X11R6.4/include  \
    /usr/local/X11R6.3/include  \
    /usr/local/X11R6.2/include  \
    /usr/local/X11R6.1/include  \
    /usr/local/X11R6/include  \
    /usr/local/X11R5/include  \
    /usr/local/X11R4/include  \
                              \
    /usr/local/include/X11    \
    /usr/local/include/X11R6.5.1  \
    /usr/local/include/X11R6.4  \
    /usr/local/include/X11R6.3  \
    /usr/local/include/X11R6.2  \
    /usr/local/include/X11R6.1  \
    /usr/local/include/X11R6  \
    /usr/local/include/X11R5  \
    /usr/local/include/X11R4  \
                              \
    /usr/X386/include         \
    /usr/x386/include         \
    /usr/XFree86/include/X11  \
    /usr/pkg/include          \
                              \
    /usr/local/include        \
    /usr/remote/include       \
    /usr/include/X11          \
    /usr/unsupported/include  \
    /usr/athena/include       \
    /usr/local/x11r5/include  \
    /usr/lpp/Xamples/include  \
                              \
    /usr/openwin/include      \
    /usr/openwin/share/include \
    /usr/openwin/include      \
                              \
    $extra_include            \
    ; \
  do
    if test -r "$ac_dir/$ttf_direct_test_include"; then
      no_ttf= ac_ttf_includes=$ac_dir
      break
    fi
  done

# Check for the libraries.
# See if we find them without any special options.
# Do not add to $LIBS permanently.
ac_save_LIBS="$LIBS"
LIBS="-l$ttf_direct_test_library $LIBS"
# First see if replacing the include by lib works.
for ac_dir in `echo "$ac_ttf_includes" | sed s%/X11$%% | sed s/include/lib/` \
                          \
    /usr/X11/lib          \
    /usr/X11R6.5.1/lib      \
    /usr/X11R6.4/lib        \
    /usr/X11R6.3/lib        \
    /usr/X11R6.2/lib        \
    /usr/X11R6.1/lib        \
    /usr/X11R6/lib        \
    /usr/X11R5/lib        \
    /usr/X11R4/lib        \
                          \
    /usr/lib/X11          \
    /usr/lib/X11R6.5.1      \
    /usr/lib/X11R6.4        \
    /usr/lib/X11R6.3        \
    /usr/lib/X11R6.2        \
    /usr/lib/X11R6.1        \
    /usr/lib/X11R6        \
    /usr/lib/X11R5        \
    /usr/lib/X11R4        \
                          \
    /usr/local/X11/lib    \
    /usr/local/X11R6/lib  \
    /usr/local/X11R5/lib  \
    /usr/local/X11R4/lib  \
                          \
    /usr/local/lib/X11    \
    /usr/local/X11R6.5.1/lib  \
    /usr/local/X11R6.4/lib  \
    /usr/local/X11R6.3/lib  \
    /usr/local/X11R6.2/lib  \
    /usr/local/X11R6.1/lib  \
    /usr/local/lib/X11R6  \
    /usr/local/lib/X11R5  \
    /usr/local/lib/X11R4  \
                          \
    /usr/X386/lib         \
    /usr/x386/lib         \
    /usr/XFree86/lib/X11  \
    /usr/pkg/lib          \
                          \
    /usr/lib              \
    /usr/local/lib        \
    /usr/remote/lib       \
    /usr/unsupported/lib  \
    /usr/athena/lib       \
    /usr/local/x11r5/lib  \
    /usr/lpp/Xamples/lib  \
    /lib/usr/lib/X11      \
                          \
    /usr/openwin/lib      \
    /usr/openwin/share/lib \
    $extra_lib            \
    ; \
do
  for ac_extension in a so sl; do
    if test -r $ac_dir/lib${ttf_direct_test_library}.$ac_extension; then
      no_ttf= ac_ttf_libraries=$ac_dir
      break 2
   fi
  done
done
LIBS="$ac_save_LIBS"])
AC_DEFUN([AC_PATH_TTF],
[AC_REQUIRE_CPP()dnl

ttf_includes=NONE
ttf_libraries=NONE

AC_MSG_CHECKING(for TTF)
AC_ARG_WITH(ttf, [  --without-ttf           disable True Text Font mode(s)])
if test "x$with_ttf" = xno; then
  no_ttf=yes
else
  if test "x$ttf_includes" != xNONE && test "x$ttf_libraries" != xNONE; then
    no_ttf=
  else
AC_CACHE_VAL(ac_cv_path_ttf,
[# One or both of these vars are not set, and there is no cached value.
no_ttf=yes
AC_PATH_TTF_DIRECT

if test "$no_ttf" = yes; then
  ac_cv_path_ttf="no_ttf=yes"
else
  ac_cv_path_ttf="no_ttf= ac_ttf_includes=$ac_ttf_includes ac_ttf_libraries=$ac_ttf_libraries"
fi])dnl
  fi
  eval "$ac_cv_path_ttf"
fi # with_ttf != no

if test "$no_ttf" = yes; then
  AC_MSG_RESULT(no)
  TTF="#"
else
  ttf=yes
  AC_DEFINE([HAVE_TTF], [1], [True Text Font])
  if test "x$prefix" = "xNONE"; then
    AC_DEFINE_UNQUOTED([DEF_TTFONT], ["/usr/lib/X11/xlock/fonts/"], [True Text Font])
  else
    AC_DEFINE_UNQUOTED([DEF_TTFONT], ["${prefix}/lib/X11/xlock/fonts/"], [True Text Font])
  fi
  XLOCKLIBS="${XLOCKLIBS} -lttf"
  test "x$ttf_includes" = xNONE && ttf_includes=$ac_ttf_includes
  test "x$ttf_libraries" = xNONE && ttf_libraries=$ac_ttf_libraries
  ac_cv_path_ttf="no_ttf= ac_ttf_includes=$ttf_includes ac_ttf_libraries=$ttf_libraries"
  AC_MSG_RESULT([libraries $ttf_libraries, headers $ttf_includes])
  TTF=""
fi
])

AC_PATH_TTF

if test "x$ttf_libraries" != x && test "x$ttf_libraries" != xNONE ; then
  XLOCK_LDFLAGS="${XLOCK_LDFLAGS}:$ttf_libraries"
  XLOCKLIBPATHS="${XLOCKLIBPATHS} -L$ttf_libraries"
fi
if test "x$ttf_includes" != x && test "x$ttf_includes" != xNONE ; then
  XLOCKINC="${XLOCKINC} -I$ttf_includes"
fi

ftgl=no
dnl ftgl FTGL

AC_DEFUN([AC_PATH_FTGL_DIRECT],
[test -z "$ftgl_direct_test_library" && ftgl_direct_test_library=ftgl
# test -z "$ftgl_direct_test_function" && ftgl_direct_test_function=ftglCreateImageFromData
test -z "$ftgl_direct_test_include" && ftgl_direct_test_include=FTGL/FTFont.h
  for ac_dir in               \
    /usr/X11R6.5.1/include    \
    /usr/X11R6.4/include      \
    /usr/X11R6.3/include      \
    /usr/X11R6.2/include      \
    /usr/X11R6.1/include      \
    /usr/X11R6/include        \
    /usr/X11R5/include        \
    /usr/X11R4/include        \
    /usr/X11/include          \
                              \
    /usr/include/X11          \
    /usr/include/X11R6.5.1    \
    /usr/include/X11R6.4      \
    /usr/include/X11R6.3      \
    /usr/include/X11R6.2      \
    /usr/include/X11R6.1      \
    /usr/include/X11R6        \
    /usr/include/X11R5        \
    /usr/include/X11R4        \
                              \
    /usr/local/X11/include    \
    /usr/local/X11R6.5.1/include  \
    /usr/local/X11R6.4/include  \
    /usr/local/X11R6.3/include  \
    /usr/local/X11R6.2/include  \
    /usr/local/X11R6.1/include  \
    /usr/local/X11R6/include  \
    /usr/local/X11R5/include  \
    /usr/local/X11R4/include  \
                              \
    /usr/local/include/X11    \
    /usr/local/include/X11R6.5.1  \
    /usr/local/include/X11R6.4  \
    /usr/local/include/X11R6.3  \
    /usr/local/include/X11R6.2  \
    /usr/local/include/X11R6.1  \
    /usr/local/include/X11R6  \
    /usr/local/include/X11R5  \
    /usr/local/include/X11R4  \
                              \
    /usr/X386/include         \
    /usr/x386/include         \
    /usr/XFree86/include/X11  \
    /usr/pkg/include          \
                              \
    /usr/local/include        \
    /usr/remote/include       \
    /usr/include              \
    /usr/unsupported/include  \
    /usr/athena/include       \
    /usr/local/x11r5/include  \
    /usr/lpp/Xamples/include  \
                              \
    /usr/openwin/include      \
    /usr/openwin/share/include \
    /usr/openwin/include      \
                              \
    $extra_include            \
    ; \
  do
    if test -r "$ac_dir/$ftgl_direct_test_include"; then
      no_ftgl= ac_ftgl_includes=$ac_dir
      break
    fi
  done

# Check for the libraries.
# See if we find them without any special options.
# Do not add to $LIBS permanently.
ac_save_LIBS="$LIBS"
LIBS="-l$ftgl_direct_test_library $LIBS"
# First see if replacing the include by lib works.
for ac_dir in `echo "$ac_ftgl_includes" | sed s%/X11$%% | sed s/include/lib/` \
                          \
    /usr/X11/lib          \
    /usr/X11R6.5.1/lib      \
    /usr/X11R6.4/lib        \
    /usr/X11R6.3/lib        \
    /usr/X11R6.2/lib        \
    /usr/X11R6.1/lib        \
    /usr/X11R6/lib        \
    /usr/X11R5/lib        \
    /usr/X11R4/lib        \
                          \
    /usr/lib/X11          \
    /usr/lib/X11R6.5.1      \
    /usr/lib/X11R6.4        \
    /usr/lib/X11R6.3        \
    /usr/lib/X11R6.2        \
    /usr/lib/X11R6.1        \
    /usr/lib/X11R6        \
    /usr/lib/X11R5        \
    /usr/lib/X11R4        \
                          \
    /usr/local/X11/lib    \
    /usr/local/X11R6/lib  \
    /usr/local/X11R5/lib  \
    /usr/local/X11R4/lib  \
                          \
    /usr/local/lib/X11    \
    /usr/local/X11R6.5.1/lib  \
    /usr/local/X11R6.4/lib  \
    /usr/local/X11R6.3/lib  \
    /usr/local/X11R6.2/lib  \
    /usr/local/X11R6.1/lib  \
    /usr/local/lib/X11R6  \
    /usr/local/lib/X11R5  \
    /usr/local/lib/X11R4  \
                          \
    /usr/X386/lib         \
    /usr/x386/lib         \
    /usr/XFree86/lib/X11  \
    /usr/pkg/lib          \
                          \
    /usr/lib              \
    /usr/local/lib        \
    /usr/remote/lib       \
    /usr/unsupported/lib  \
    /usr/athena/lib       \
    /usr/local/x11r5/lib  \
    /usr/lpp/Xamples/lib  \
    /lib/usr/lib/X11      \
                          \
    /usr/openwin/lib      \
    /usr/openwin/share/lib \
    $extra_lib            \
    ; \
do
  for ac_extension in a so sl; do
    if test -r $ac_dir/lib${ftgl_direct_test_library}.$ac_extension; then
      no_ftgl= ac_ftgl_libraries=$ac_dir
      break 2
   fi
  done
done
LIBS="$ac_save_LIBS"])
AC_DEFUN([AC_PATH_FTGL],
[AC_REQUIRE_CPP()dnl

ftgl_includes=NONE
ftgl_libraries=NONE

AC_MSG_CHECKING(for FTGL)
AC_ARG_WITH(ftgl, [  --without-ftgl          disable GL font rendering library])
if test "x$with_ftgl" = xno; then
  no_ftgl=yes
else
  if test "x$ftgl_includes" != xNONE && test "x$ftgl_libraries" != xNONE; then
    no_ftgl=
  else
AC_CACHE_VAL(ac_cv_path_ftgl,
[# One or both of these vars are not set, and there is no cached value.
no_ftgl=yes
AC_PATH_FTGL_DIRECT

if test "$no_ftgl" = yes; then
  ac_cv_path_ftgl="no_ftgl=yes"
else
  ac_cv_path_ftgl="no_ftgl= ac_ftgl_includes=$ac_ftgl_includes ac_ftgl_libraries=$ac_ftgl_libraries"
fi])dnl
  fi
  eval "$ac_cv_path_ftgl"
fi # with_ftgl != no

if test "$no_ftgl" = yes; then
  AC_MSG_RESULT(no)
  FTGL="#"
else
  ftgl=yes
  AC_DEFINE([HAVE_FTGL], [1], [GL Font Rendering Library])
  XLOCKLIBS="${XLOCKLIBS} -lftgl"
  test "x$ftgl_includes" = xNONE && ftgl_includes=$ac_ftgl_includes
  test "x$ftgl_libraries" = xNONE && ftgl_libraries=$ac_ftgl_libraries
  ac_cv_path_ftgl="no_ftgl= ac_ftgl_includes=$ftgl_includes ac_ftgl_libraries=$ftgl_libraries"
  AC_MSG_RESULT([libraries $ftgl_libraries, headers $ftgl_includes])
  FTGL=""
fi
])

AC_PATH_FTGL

if test "x$ftgl_libraries" != x && test "x$ftgl_libraries" != xNONE ; then
  XLOCK_LDFLAGS="${XLOCK_LDFLAGS}:$ftgl_libraries"
  XLOCKLIBPATHS="${XLOCKLIBPATHS} -L$ftgl_libraries"
fi
if test "x$ftgl_includes" != x && test "x$ftgl_includes" != xNONE ; then
  XLOCKINC="${XLOCKINC} -I$ftgl_includes"
fi

freetype=no
dnl freetype FREETYPE

AC_DEFUN([AC_PATH_FREETYPE_DIRECT],
[test -z "$freetype_direct_test_library" && freetype_direct_test_library=freetype
test -z "$freetype_direct_test_function" && freetype_direct_test_function=FT_Init_FreeType
test -z "$freetype_direct_test_include" && freetype_direct_test_include=freetype2/freetype.h
  for ac_dir in               \
    /usr/X11R6.5.1/include	    \
    /usr/X11R6.4/include	      \
    /usr/X11R6.3/include	      \
    /usr/X11R6.2/include	      \
    /usr/X11R6.1/include	      \
    /usr/X11R6/include	        \
    /usr/X11R5/include	        \
    /usr/X11R4/include	        \
    /usr/X11/include	          \
                              \
    /usr/include/X11	          \
    /usr/include/X11R6.5.1	    \
    /usr/include/X11R6.4	      \
    /usr/include/X11R6.3	      \
    /usr/include/X11R6.2	      \
    /usr/include/X11R6.1	      \
    /usr/include/X11R6	        \
    /usr/include/X11R5	        \
    /usr/include/X11R4	        \
                              \
    /usr/local/X11/include	    \
    /usr/local/X11R6.5.1/include	  \
    /usr/local/X11R6.4/include	  \
    /usr/local/X11R6.3/include	  \
    /usr/local/X11R6.2/include	  \
    /usr/local/X11R6.1/include	  \
    /usr/local/X11R6/include	  \
    /usr/local/X11R5/include	  \
    /usr/local/X11R4/include	  \
                              \
    /usr/local/include/X11	    \
    /usr/local/include/X11R6.5.1	  \
    /usr/local/include/X11R6.4	  \
    /usr/local/include/X11R6.3	  \
    /usr/local/include/X11R6.2	  \
    /usr/local/include/X11R6.1	  \
    /usr/local/include/X11R6	  \
    /usr/local/include/X11R5	  \
    /usr/local/include/X11R4	  \
                              \
    /usr/X386/include	         \
    /usr/x386/include	         \
    /usr/XFree86/include/X11	  \
    /usr/pkg/include          \
                              \
    /usr/include	        \
    /usr/local/include	        \
    /usr/remote/include	       \
    /usr/include/X11	          \
    /usr/unsupported/include	  \
    /usr/athena/include	       \
    /usr/local/x11r5/include	  \
    /usr/lpp/Xamples/include	  \
                              \
    /usr/openwin/include	      \
    /usr/openwin/share/include	 \
    /usr/openwin/include	      \
                              \
    $extra_include	            \
    ; \
  do
    if test -r "$ac_dir/$freetype_direct_test_include"; then
      no_freetype= ac_freetype_includes=$ac_dir/freetype2
      break
    fi
  done

# Check for the libraries.
# See if we find them without any special options.
# Do not add to $LIBS permanently.
ac_save_LIBS="$LIBS"
LIBS="-l$freetype_direct_test_library $LIBS"
# First see if replacing the include by lib works.
for ac_dir in `echo "$ac_freetype_includes" | sed s%/X11$%% | sed s/include/lib/` \
                          \
    /usr/X11/lib          \
    /usr/X11R6.5.1/lib      \
    /usr/X11R6.4/lib        \
    /usr/X11R6.3/lib        \
    /usr/X11R6.2/lib        \
    /usr/X11R6.1/lib        \
    /usr/X11R6/lib        \
    /usr/X11R5/lib        \
    /usr/X11R4/lib        \
                          \
    /usr/lib/X11          \
    /usr/lib/X11R6.5.1      \
    /usr/lib/X11R6.4        \
    /usr/lib/X11R6.3        \
    /usr/lib/X11R6.2        \
    /usr/lib/X11R6.1        \
    /usr/lib/X11R6        \
    /usr/lib/X11R5        \
    /usr/lib/X11R4        \
                          \
    /usr/local/X11/lib    \
    /usr/local/X11R6/lib  \
    /usr/local/X11R5/lib  \
    /usr/local/X11R4/lib  \
                          \
    /usr/local/lib/X11    \
    /usr/local/X11R6.5.1/lib  \
    /usr/local/X11R6.4/lib  \
    /usr/local/X11R6.3/lib  \
    /usr/local/X11R6.2/lib  \
    /usr/local/X11R6.1/lib  \
    /usr/local/lib/X11R6  \
    /usr/local/lib/X11R5  \
    /usr/local/lib/X11R4  \
                          \
    /usr/X386/lib         \
    /usr/x386/lib         \
    /usr/XFree86/lib/X11  \
    /usr/pkg/lib          \
                          \
    /usr/lib              \
    /usr/local/lib        \
    /usr/remote/lib       \
    /usr/unsupported/lib  \
    /usr/athena/lib       \
    /usr/local/x11r5/lib  \
    /usr/lpp/Xamples/lib  \
    /lib/usr/lib/X11      \
                          \
    /usr/openwin/lib      \
    /usr/openwin/share/lib \
    $extra_lib            \
    ; \
do
  for ac_extension in a so sl; do
    if test -r $ac_dir/lib${freetype_direct_test_library}.$ac_extension; then
      no_freetype= ac_freetype_libraries=$ac_dir
      break 2
   fi
  done
done
LIBS="$ac_save_LIBS"])
AC_DEFUN([AC_PATH_FREETYPE],
[AC_REQUIRE_CPP()dnl

freetype_includes=NONE
freetype_libraries=NONE

AC_MSG_CHECKING(for FREETYPE2)
AC_ARG_WITH(freetype, [  --without-freetype      disable Freetype2 mode(s)])
if test "x$with_freetype" = xno; then
  no_freetype=yes
else
  if test "x$freetype_includes" != xNONE && test "x$freetype_libraries" != xNONE; then
    no_freetype=
  else
AC_CACHE_VAL(ac_cv_path_freetype,
[# One or both of these vars are not set, and there is no cached value.
no_freetype=yes
AC_PATH_FREETYPE_DIRECT

if test "$no_freetype" = yes; then
  ac_cv_path_freetype="no_freetype=yes"
else
  ac_cv_path_freetype="no_freetype= ac_freetype_includes=$ac_freetype_includes ac_freetype_libraries=$ac_freetype_libraries"
fi])dnl
  fi
  eval "$ac_cv_path_freetype"
fi # with_freetype != no

if test "$no_freetype" = yes; then
  AC_MSG_RESULT(no)
  FREETYPE="#"
else
  freetype=yes
  AC_DEFINE([HAVE_FREETYPE], [1], [Free Type])
  if test "x$prefix" = "xNONE"; then
    AC_DEFINE_UNQUOTED([DEF_TTFONT], ["/usr/lib/X11/xlock/fonts/"], [True Text Font])
  else
    AC_DEFINE_UNQUOTED([DEF_TTFONT], ["${prefix}/lib/X11/xlock/fonts/"], [True Text Font])
  fi
  XLOCKLIBS="${XLOCKLIBS} -lfreetype"
  test "x$freetype_includes" = xNONE && freetype_includes=$ac_freetype_includes
  test "x$freetype_libraries" = xNONE && freetype_libraries=$ac_freetype_libraries
  ac_cv_path_freetype="no_freetype= ac_freetype_includes=$freetype_includes ac_freetype_libraries=$freetype_libraries"
  AC_MSG_RESULT([libraries $freetype_libraries, headers $freetype_includes])
  FREETYPE=""
fi
])

AC_PATH_FREETYPE

if test "x$freetype_libraries" != x && test "x$freetype_libraries" != xNONE ; then
  XLOCK_LDFLAGS="${XLOCK_LDFLAGS}:$freetype_libraries"
  XLOCKLIBPATHS="${XLOCKLIBPATHS} -L$freetype_libraries"
fi
if test "x$freetype_includes" != x && test "x$freetype_includes" != xNONE ; then
  XLOCKINC="${XLOCKINC} -I$freetype_includes"
fi

gl=no
dnl OpenGL opengl

dnl test -z "$opengl_direct_test_library" && opengl_direct_test_library=opengl
AC_DEFUN([AC_PATH_OPENGL_DIRECT],
[test -z "$opengl_direct_test_library" && opengl_direct_test_library=GL
test -z "$opengl_direct_test_library" && opengl_direct_test_library=GLU
test -z "$opengl_direct_test_function" && opengl_direct_test_function=glXCreateContext
test -z "$opengl_direct_test_include" && opengl_direct_test_include=GL/gl.h
  for ac_dir in               \
    /usr/include              \
    /usr/openwin/include      \
    /usr/openwin/share/include \
                              \
    /usr/X11R6.5.1/include    \
    /usr/X11R6.4/include      \
    /usr/X11R6.3/include      \
    /usr/X11R6.2/include      \
    /usr/X11R6.1/include      \
    /usr/X11R6/include        \
    /usr/X11R5/include        \
    /usr/X11/include          \
                              \
    /usr/include/X11          \
    /usr/include/X11R6.5.1    \
    /usr/include/X11R6.4      \
    /usr/include/X11R6.3      \
    /usr/include/X11R6.2      \
    /usr/include/X11R6.1      \
    /usr/include/X11R6        \
    /usr/include/X11R5        \
                              \
    /usr/local/X11/include    \
    /usr/local/X11R6.5.1/include  \
    /usr/local/X11R6.4/include  \
    /usr/local/X11R6.3/include  \
    /usr/local/X11R6.2/include  \
    /usr/local/X11R6.1/include  \
    /usr/local/X11R6/include  \
    /usr/local/X11R5/include  \
                              \
    /usr/local/include/X11    \
    /usr/local/include/X11R6.5.1  \
    /usr/local/include/X11R6.4  \
    /usr/local/include/X11R6.3  \
    /usr/local/include/X11R6.2  \
    /usr/local/include/X11R6.1  \
    /usr/local/include/X11R6  \
    /usr/local/include/X11R5  \
                              \
    /usr/X386/include         \
    /usr/x386/include         \
    /usr/XFree86/include/X11  \
    /usr/pkg/include          \
                              \
    /usr/local/include        \
    /usr/remote/include       \
    /usr/unsupported/include  \
    /usr/local/x11r5/include  \
    /usr/lpp/Xamples/include  \
                              \
    $extra_include            \
    ; \
  do
dnl # Make sure this is not Mesa
dnl    if test -r "$ac_dir/$opengl_direct_test_include" && test ! -r "$ac_dir/GL/xmesa.h"; then
    if test -r "$ac_dir/$opengl_direct_test_include"; then
      no_opengl= ac_opengl_includes=$ac_dir
      break
    fi
  done

# Check for the libraries.
# See if we find them without any special options.
# Do not add to $LIBS permanently.
ac_save_LIBS="$LIBS"
LIBS="-l$opengl_direct_test_library $LIBS"
# First see if replacing the include by lib works.
for ac_dir in `echo "$ac_opengl_includes" | sed s/include/lib/` \
                          \
    /usr/lib              \
    /usr/openwin/lib      \
    /usr/openwin/share/lib \
                          \
    /usr/X11/lib          \
    /usr/X11R6.5.1/lib    \
    /usr/X11R6.4/lib      \
    /usr/X11R6.3/lib      \
    /usr/X11R6.2/lib      \
    /usr/X11R6.1/lib      \
    /usr/X11R6/lib        \
    /usr/X11R5/lib        \
                          \
    /usr/lib/X11          \
    /usr/lib/X11R6.5.1    \
    /usr/lib/X11R6.4      \
    /usr/lib/X11R6.3      \
    /usr/lib/X11R6.2      \
    /usr/lib/X11R6.1      \
    /usr/lib/X11R6        \
    /usr/lib/X11R5        \
                          \
    /usr/local/X11/lib    \
    /usr/local/X11R6.5.1/lib  \
    /usr/local/X11R6.4/lib  \
    /usr/local/X11R6.3/lib  \
    /usr/local/X11R6.2/lib  \
    /usr/local/X11R6.1/lib  \
    /usr/local/X11R6/lib  \
    /usr/local/X11R5/lib  \
                          \
    /usr/local/lib/X11    \
    /usr/local/lib/X11R6.5.1  \
    /usr/local/lib/X11R6.4  \
    /usr/local/lib/X11R6.3  \
    /usr/local/lib/X11R6.2  \
    /usr/local/lib/X11R6.1  \
    /usr/local/lib/X11R6  \
    /usr/local/lib/X11R5  \
                          \
    /usr/X386/lib         \
    /usr/x386/lib         \
    /usr/XFree86/lib/X11  \
    /usr/pkg/lib          \
                          \
    /usr/local/lib        \
    /usr/remote/lib       \
    /usr/unsupported/lib  \
    /usr/athena/lib       \
    /usr/local/x11r5/lib  \
    /usr/lpp/Xamples/lib  \
    /lib/usr/lib/X11      \
                          \
    $extra_lib            \
    ; \
do
  for ac_extension in a so sl; do
# Make sure its not a symbolic link to MesaGL library

    if test -r $ac_dir/lib${opengl_direct_test_library}.$ac_extension && test ! $ac_link_test $ac_dir/lib${opengl_direct_test_library}.$ac_extension && test ! -r $ac_dir/libMesa${opengl_direct_test_library}.$ac_extension; then
      no_opengl= ac_opengl_libraries=$ac_dir
      break 2
    fi
  done
done
LIBS="$ac_save_LIBS"])
AC_DEFUN([AC_PATH_OPENGL],
[AC_REQUIRE_CPP()dnl

opengl_includes=NONE
opengl_libraries=NONE

AC_MSG_CHECKING(for OpenGL (OK if this is Mesa))
AC_ARG_WITH(opengl, [  --without-opengl        disable 3D OpenGL (for GL modes)])
if test "x$with_opengl" = xno; then
  no_opengl=yes
else
  if test "x$opengl_includes" != xNONE && test "x$opengl_libraries" != xNONE; then
    no_opengl=
  else
AC_CACHE_VAL(ac_cv_path_opengl,
[# One or both of these vars are not set, and there is no cached value.
no_opengl=yes
AC_PATH_OPENGL_DIRECT

if test "$no_opengl" = yes; then
  ac_cv_path_opengl="no_opengl=yes"
else
  ac_cv_path_opengl="no_opengl= ac_opengl_includes=$ac_opengl_includes ac_opengl_libraries=$ac_opengl_libraries"

fi])dnl
  fi
  eval "$ac_cv_path_opengl"
fi # with_opengl != no

if test "$no_opengl" = yes; then
  AC_MSG_RESULT(no)
else
  gl=yes
  AC_DEFINE([USE_GL], [1], [using GL])
  XLOCKLIBS="${XLOCKLIBS} -lGL -lGLU"
  test "x$opengl_includes" = xNONE && opengl_includes=$ac_opengl_includes
  test "x$opengl_libraries" = xNONE && opengl_libraries=$ac_opengl_libraries
  case "${canonical}" in
    *-*-solaris2* )
      AC_DEFINE([SUN_OGL_NO_VERTEX_MACROS], [1], [Sun OGL No Vertex Macros])
      if test "x$opengl_includes" = "x/usr/include"; then
        opengl_includes=""
      fi
    ;;
    *-*-irix5* | *-*-irix6* )
      XLOCKLIBS="${XLOCKLIBS} -lgl"
    ;;
  esac
  ac_cv_path_opengl="no_opengl= ac_opengl_includes=$opengl_includes ac_opengl_libraries=$opengl_libraries"
  AC_MSG_RESULT([libraries $opengl_libraries, headers $opengl_includes])
  GL=""
fi
])

AC_PATH_OPENGL

if test "x$opengl_libraries" != x && test "x$opengl_libraries" != xNONE ; then
  XLOCK_LDFLAGS="${XLOCK_LDFLAGS}:$opengl_libraries"
  XLOCKLIBPATHS="${XLOCKLIBPATHS} -L$opengl_libraries"
fi
if test "x$opengl_includes" != x && test "x$opengl_includes" != xNONE ; then
  XLOCKINC="${XLOCKINC} -I$opengl_includes"
fi

dnl Mesa mesa

dnl test -z "$mesa_direct_test_library" && mesa_direct_test_library=MesaGLU
AC_DEFUN([AC_PATH_MESA_DIRECT],
[test -z "$mesa_direct_test_library" && mesa_direct_test_library=MesaGL
test -z "$mesa_direct_test_library" && mesa_direct_test_library=MesaGLU
test -z "$mesa_direct_test_function" && mesa_direct_test_function=glXCreateContext
test -z "$mesa_direct_test_include" && mesa_direct_test_include=GL/gl.h
  for ac_dir in               \
    /usr/X11R6.5.1/include    \
    /usr/X11R6.4/include      \
    /usr/X11R6.3/include      \
    /usr/X11R6.2/include      \
    /usr/X11R6.1/include      \
    /usr/X11R6/include        \
    /usr/X11R5/include        \
    /usr/X11/include          \
                              \
    /usr/include/X11          \
    /usr/include/X11R6.5.1    \
    /usr/include/X11R6.4      \
    /usr/include/X11R6.3      \
    /usr/include/X11R6.2      \
    /usr/include/X11R6.1      \
    /usr/include/X11R6        \
    /usr/include/X11R5        \
                              \
    /usr/local/X11/include    \
    /usr/local/X11R6.5.1/include  \
    /usr/local/X11R6.4/include  \
    /usr/local/X11R6.3/include  \
    /usr/local/X11R6.2/include  \
    /usr/local/X11R6.1/include  \
    /usr/local/X11R6/include  \
    /usr/local/X11R5/include  \
                              \
    /usr/local/include/X11    \
    /usr/local/include/X11R6.5.1  \
    /usr/local/include/X11R6.4  \
    /usr/local/include/X11R6.3  \
    /usr/local/include/X11R6.2  \
    /usr/local/include/X11R6.1  \
    /usr/local/include/X11R6  \
    /usr/local/include/X11R5  \
                              \
    /usr/X386/include         \
    /usr/x386/include         \
    /usr/XFree86/include/X11  \
    /usr/pkg/include          \
                              \
    /usr/local/include        \
    /usr/remote/include       \
    /usr/include              \
    /usr/unsupported/include  \
    /usr/local/x11r5/include  \
    /usr/lpp/Xamples/include  \
                              \
    /usr/openwin/include      \
    /usr/openwin/share/include \
    $extra_include            \
    ; \
  do
    if test -r "$ac_dir/$mesa_direct_test_include"; then
      no_mesa= ac_mesa_includes=$ac_dir
      break
    fi
  done

# Check for the libraries.
# See if we find them without any special options.
# Do not add to $LIBS permanently.
ac_save_LIBS="$LIBS"
LIBS="-l$mesa_direct_test_library $LIBS"
# First see if replacing the include by lib works.
for ac_dir in `echo "$ac_mesa_includes" | sed s/include/lib/` \
                          \
    /usr/X11/lib          \
    /usr/X11R6.5.1/lib    \
    /usr/X11R6.4/lib      \
    /usr/X11R6.3/lib      \
    /usr/X11R6.2/lib      \
    /usr/X11R6.1/lib      \
    /usr/X11R6/lib        \
    /usr/X11R5/lib        \
                          \
    /usr/lib/X11          \
    /usr/lib/X11R6.5.1    \
    /usr/lib/X11R6.4      \
    /usr/lib/X11R6.3      \
    /usr/lib/X11R6.2      \
    /usr/lib/X11R6.1      \
    /usr/lib/X11R6        \
    /usr/lib/X11R5        \
                          \
    /usr/local/X11/lib    \
    /usr/local/X11R6.5.1/lib  \
    /usr/local/X11R6.4/lib  \
    /usr/local/X11R6.3/lib  \
    /usr/local/X11R6.2/lib  \
    /usr/local/X11R6.1/lib  \
    /usr/local/X11R6/lib  \
    /usr/local/X11R5/lib  \
                          \
    /usr/local/lib/X11    \
    /usr/local/lib/X11R6.5.1  \
    /usr/local/lib/X11R6.4  \
    /usr/local/lib/X11R6.3  \
    /usr/local/lib/X11R6.2  \
    /usr/local/lib/X11R6.1  \
    /usr/local/lib/X11R6  \
    /usr/local/lib/X11R5  \
                          \
    /usr/X386/lib         \
    /usr/x386/lib         \
    /usr/XFree86/lib/X11  \
    /usr/pkg/lib          \
                          \
    /usr/lib              \
    /usr/local/lib        \
    /usr/remote/lib       \
    /usr/unsupported/lib  \
    /usr/athena/lib       \
    /usr/local/x11r5/lib  \
    /usr/lpp/Xamples/lib  \
    /lib/usr/lib/X11      \
                          \
    /usr/openwin/lib      \
    /usr/openwin/share/lib \
    $extra_lib            \
    ; \
do
  for ac_extension in a so sl; do
    if test -r $ac_dir/lib${mesa_direct_test_library}.$ac_extension; then
      no_mesa= ac_mesa_libraries=$ac_dir
      break 2
    fi
  done
done
LIBS="$ac_save_LIBS"])
AC_DEFUN([AC_PATH_MESA],
[AC_REQUIRE_CPP()dnl

mesa_includes=NONE
mesa_libraries=NONE

AC_MSG_CHECKING(for Mesa)
AC_ARG_WITH(mesa, [  --without-mesa          disable Mesa 3D (for GL modes)])
if test "x$with_mesa" = xno; then
  no_mesa=yes
else
  if test "x$mesa_includes" != xNONE && test "x$mesa_libraries" != xNONE; then
    no_mesa=
  else
AC_CACHE_VAL(ac_cv_path_mesa,
[# One or both of these vars are not set, and there is no cached value.
no_mesa=yes
AC_PATH_MESA_DIRECT

if test "$no_mesa" = yes -a "$no_opengl" = yes ; then
  ac_cv_path_mesa="no_mesa=yes"
else
  ac_cv_path_mesa="no_mesa= ac_mesa_includes=$ac_mesa_includes ac_mesa_libraries=$ac_mesa_libraries"
fi])dnl
  fi
  eval "$ac_cv_path_mesa"
fi # with_mesa != no

if test "$no_mesa" = yes; then
  AC_MSG_RESULT(no)
  GL="#"
else
  if test "$gl" = no; then
    gl=yes
    AC_DEFINE([USE_GL], [1], [using GL])
    XLOCKLIBS="${XLOCKLIBS} -lMesaGL -lMesaGLU"
    test "x$mesa_includes" = xNONE && mesa_includes=$ac_mesa_includes
    test "x$mesa_libraries" = xNONE && mesa_libraries=$ac_mesa_libraries
    case "${canonical}" in
      *-*-solaris2* )
        if test "x$mesa_includes" = "x/usr/include"; then
          mesa_includes=""
        fi
      ;;
    esac
    ac_cv_path_mesa="no_mesa= ac_mesa_includes=$mesa_includes ac_mesa_libraries=$mesa_libraries"
    AC_MSG_RESULT([libraries $mesa_libraries, headers $mesa_includes])
    # Issue a warning if the version number of Mesa is less than 3.0 since
    # 2.6 and earlier had a security bug and 2.2 and earlier had memory leaks.
    # pre-3.4 Mesa will core dump with the -showfps option. A patch must be
    # applied to 3.4 and 3.4.1 Mesa to correct a memory leak in the molecule
    # mode and the -showfps option. This memory leak is fixed in Mesa 3.4.2
    AC_CACHE_CHECK([Mesa version number], ac_cv_mesa_version_string,
        [cat > conftest.$ac_ext <<EOF
#line __oline__ "configure"
#include "confdefs.h"
#include <GL/xmesa.h>
configure: XMESA_MAJOR_VERSION XMESA_MINOR_VERSION
EOF
         ac_save_CPPFLAGS="$CPPFLAGS"
         if test \! -z "$ac_mesa_includes" ; then
           CPPFLAGS="$CPPFLAGS -I$ac_mesa_includes"
         fi
         CPPFLAGS="$CPPFLAGS $X_CFLAGS"
         changequote(X,Y)

         mglv=`(eval "$ac_cpp conftest.$ac_ext") | sed -n \
               's/^configure:.*\([0-9][0-9]*\).*\([0-9][0-9]*\).*$/\1.\2/p'`
dnl autoconf-4.53 really hates this next line
dnl         mglv=`(eval "$ac_cpp conftest.$ac_ext") 2>&AC_FD_CC | sed -n \
dnl               's/^configure:.*\([0-9][0-9]*\).*\([0-9][0-9]*\).*$/\1.\2/p'`
         changequote([,])

         rm -f conftest.$ac_ext

         CPPFLAGS="$ac_save_CPPFLAGS"

         if test "$mglv" = ""; then
           ac_mesa_version=unknown
           ac_mesa_version_string=unknown
         else
           ac_mesa_version_string=$mglv
           maj=`echo $mglv | sed -n 's/\..*//p'`
           min=`echo $mglv | sed -n 's/.*\.//p'`
           ac_mesa_version=`echo "$maj * 1000 + $min" | bc`
           if test -z "$ac_mesa_version"; then
             ac_mesa_version=unknown
             ac_mesa_version_string=unknown
           fi
         fi
         ac_cv_mesa_version=$ac_mesa_version
         ac_cv_mesa_version_string=$ac_mesa_version_string
      ]) 
    ac_mesa_version=$ac_cv_mesa_version
    ac_mesa_version_string=$ac_cv_mesa_version_string
    preferred_mesa="3.4.2"
    if test "$ac_mesa_version" = unknown; then
      AC_MSG_RESULT(!!!WARNING!!! Unable to determine the Mesa version number!)
      AC_MSG_RESULT(!!!WARNING!!! Make sure you are using Mesa version $preferred_mesa or newer.)
    elif test \! "$ac_mesa_version" -gt 3004; then
      AC_MSG_RESULT(!!!WARNING!!! Mesa version $ac_mesa_version_string is being used.)
      AC_MSG_RESULT(!!!WARNING!!! Mesa versions 3.4.1 and earlier have a memory leak when using)
      AC_MSG_RESULT(!!!WARNING!!! molecule mode or the -showfps option. This can be corrected by)
      AC_MSG_RESULT(!!!WARNING!!! applying the Mesa.patch file to Mesa 3.4 or 3.4.1 using patch)
      AC_MSG_RESULT(!!!WARNING!!! or by using Mesa 3.4.2 or newer.)
      if test \! "$ac_mesa_version" -gt 3003; then
        AC_MSG_RESULT(!!!WARNING!!! Mesa version $ac_mesa_version_string is being used.)
        AC_MSG_RESULT(!!!WARNING!!! Mesa versions 3.3 and earlier will SEGV with the -showfps option.)
        if test \! "$ac_mesa_version" -gt 2006; then
          AC_MSG_RESULT(!!!WARNING!!! Mesa versions 2.6 and earlier have a security bug.)
          if test \! "$ac_mesa_version" -gt 2002; then
            AC_MSG_RESULT(!!!WARNING!!! Mesa versions 2.2 and earlier have memory leaks.)
          fi
        fi
      fi
      AC_MSG_RESULT(!!!WARNING!!! It is strongly recommended that you upgrade to Mesa V$preferred_mesa or newer.)
    fi

    case "${canonical}" in
      *-*-linux* )
        # Testing if Mesa was compiled against VGA
        if test -f $mesa_libraries/libMesaGL.so; then
          have_mesa_svga=`nm $mesa_libraries/libMesaGL.so | grep SVGAMesa | wc -l`
        else
          have_mesa_svga=0
        fi
        if eval "test $have_mesa_svga -ge 1"; then
          AC_MSG_RESULT([Mesa library is linked against SVGA: adding -lvga])
          XLOCKLIBS="${XLOCKLIBS} -lvga"
        fi
        # Testing if Mesa was compiled against glide
        if test -f $mesa_libraries/libMesaGL.so; then
          have_mesa_glide=`nm $mesa_libraries/libMesaGL.so | grep grGlideInit | wc -l`
        else
          have_mesa_glide=0
        fi
        if eval "test $have_mesa_glide -ge 1"; then
          AC_MSG_RESULT([Mesa library is linked against GLIDE: adding -lglide2x])
          XLOCKLIBS="${XLOCKLIBS} -lglide2x"

        fi
#  Causes major errors for maintainer.  xlock refuses to run.
#  I am not able to reproduce the problem ? What is it ?
# Does a new glibc version fix this problem ?
        # Testing if Mesa was compiled against pthread
        if test -f $mesa_libraries/libMesaGL.so; then
          have_mesa_pthread=`nm $mesa_libraries/libMesaGL.so | grep pthread_once | wc -l`
        else
          have_mesa_pthread=0
        fi
        if eval "test $have_mesa_pthread -ge 1"; then
          AC_MSG_RESULT([Mesa library is linked against PTHREAD: adding -lpthread])
          XLOCKLIBS="${XLOCKLIBS} -lpthread"
        fi
        ;;
    esac
    GL=""
  else
    AC_MSG_RESULT(ignored - using OpenGL)
  fi

fi
])

AC_PATH_MESA

if test "x$mesa_libraries" != x && test "x$mesa_libraries" != xNONE ; then
  XLOCK_LDFLAGS="$mesa_libraries:${XLOCK_LDFLAGS}"
  XLOCKLIBPATHS="-L$mesa_libraries ${XLOCKLIBPATHS}"
fi
if test "x$mesa_includes" != x && test "x$mesa_includes" != xNONE ; then
  XLOCKINC="-I$mesa_includes ${XLOCKINC}"
fi

# Test if Mesa or OpenGL has GL1.1 features
# (Should this test be performed only if (Mesa)GL is used? In that case
# it fails anyway.)
AC_MSG_CHECKING([for GL >=1.1])
ac_save_CFLAGS="$CFLAGS"
ac_save_LIBS="$LIBS"
CFLAGS="${CFLAGS} ${XLOCKINC}"
LIBS="${XLOCKLIBPATHS} ${XLOCKLIBS} -lX11 -lXext -lm"
AC_CACHE_VAL(ac_cv_c_gl1_1,[
AC_TRY_LINK([#define _XOPEN_SOURCE 1
#include <GL/gl.h>], [
GLint texture;
glBindTexture(GL_TEXTURE_2D, texture);
], ac_cv_c_gl1_1=yes, ac_cv_c_gl1_1=no)])
dnl This does not seem to work on Sun's OpenGL
dnl if test x"$ac_cv_c_gl1_1" = xyes; then
    AC_DEFINE([HAVE_GLBINDTEXTURE], [1], [GL Bind Texture])
    AC_MSG_RESULT(yes)
dnl else
dnl    AC_MSG_RESULT(no)
dnl fi
CFLAGS="$ac_save_CFLAGS"
LIBS="$ac_save_LIBS"

dnl DTSAVER DtSaver dtsaver DtSvc

AC_DEFUN([AC_PATH_DTSAVER_DIRECT],
[test -z "$dtsaver_direct_test_library" && dtsaver_direct_test_library=DtSvc
test -z "$dtsaver_direct_test_function" && dtsaver_direct_test_function=DtSaverGetWindows
test -z "$dtsaver_direct_test_include" && dtsaver_direct_test_include=Dt/Saver.h
  for ac_dir in            \
    /usr/dt/include        \
    $extra_include         \
    ; \
  do
    if test -r "$ac_dir/$dtsaver_direct_test_include"; then
      no_dtsaver= ac_dtsaver_includes=$ac_dir
      break
    fi
  done

# Check for the libraries.
# See if we find them without any special options.
# Do not add to $LIBS permanently.
ac_save_LIBS="$LIBS"
LIBS="-l$dtsaver_direct_test_library $LIBS"
# First see if replacing the include by lib works.
for ac_dir in `echo "$ac_dtsaver_includes" | sed s/include/lib/` \
    $extra_lib         \
    ; \
do
  for ac_extension in a so sl; do
    if test -r $ac_dir/lib${dtsaver_direct_test_library}.$ac_extension; then
      no_dtsaver= ac_dtsaver_libraries=$ac_dir
      break 2
    fi
  done
done
LIBS="$ac_save_LIBS"])
AC_DEFUN([AC_PATH_DTSAVER],
[AC_REQUIRE_CPP()dnl

dtsaver_includes=NONE
dtsaver_libraries=NONE

AC_MSG_CHECKING(for DtSaver)
AC_ARG_WITH(dtsaver, [  --without-dtsaver       disable -dtsaver option])
if test "x$with_dtsaver" = xno; then
  no_dtsaver=yes
else
  if test "x$dtsaver_includes" != xNONE && test "x$dtsaver_libraries" != xNONE; then
    no_dtsaver=
  else
AC_CACHE_VAL(ac_cv_path_dtsaver,
[# One or both of these vars are not set, and there is no cached value.
no_dtsaver=yes
AC_PATH_DTSAVER_DIRECT

if test "$no_dtsaver" = yes; then
  ac_cv_path_dtsaver="no_dtsaver=yes"
else
  ac_cv_path_dtsaver="no_dtsaver= ac_dtsaver_includes=$ac_dtsaver_includes ac_dtsaver_libraries=$ac_dtsaver_libraries"
fi])dnl
  fi
  eval "$ac_cv_path_dtsaver"
fi # with_dtsaver != no

if test "$no_motif" = yes; then
  AC_MSG_RESULT(ignored - needs Motif)
else
  if test "$no_dtsaver" = yes; then
    AC_MSG_RESULT(no)
  else
    AC_DEFINE([USE_DTSAVER], [1], [Desktop Saver])
    XLOCKLIBS="${XLOCKLIBS} -lDtSvc -lDtXinerama -ltt -lXm"
    test "x$dtsaver_includes" = xNONE && dtsaver_includes=$ac_dtsaver_includes
    test "x$dtsaver_libraries" = xNONE && dtsaver_libraries=$ac_dtsaver_libraries
    ac_cv_path_dtsaver="no_dtsaver= ac_dtsaver_includes=$dtsaver_includes ac_dtsaver_libraries=$dtsaver_libraries"
    AC_MSG_RESULT([libraries $dtsaver_libraries, headers $dtsaver_includes])
  fi
fi
])

AC_PATH_DTSAVER

if test "x$dtsaver_libraries" != x && test "x$dtsaver_libraries" != xNONE ; then
  XLOCK_LDFLAGS="${XLOCK_LDFLAGS}:$dtsaver_libraries:$motif_libraries"
  XLOCKLIBPATHS="${XLOCKLIBPATHS} -L$dtsaver_libraries -L$motif_libraries"
fi
if test "x$dtsaver_includes" != x && test "x$dtsaver_includes" != xNONE ; then
  XLOCKINC="${XLOCKINC} -I$dtsaver_includes -I$motif_includes"
fi

dnl Xext

AC_DEFUN([AC_PATH_EXT_DIRECT],
[test -z "$ext_direct_test_library" && ext_direct_test_library=Xext
test -z "$ext_direct_test_function" && ext_direct_test_function=DPMSSetTimeouts
test -z "$dpms_direct_test_include" && ext_direct_test_include=X11/extensions/dpms.h
  for ac_dir in               \
    /usr/X11R6.5.1/include    \
    /usr/X11R6.4/include      \
    /usr/X11R6.3/include      \
    /usr/X11R6.2/include      \
    /usr/X11R6.1/include      \
    /usr/X11R6/include        \
    /usr/X11R5/include        \
    /usr/X11R4/include        \
                              \
    /usr/include/X11R6.5.1    \
    /usr/include/X11R6.4      \
    /usr/include/X11R6.3      \
    /usr/include/X11R6.2      \
    /usr/include/X11R6.1      \
    /usr/include/X11R6        \
    /usr/include/X11R5        \
    /usr/include/X11R4        \
                              \
    /usr/local/X11R6.5.1/include  \
    /usr/local/X11R6.4/include  \
    /usr/local/X11R6.3/include  \
    /usr/local/X11R6.2/include  \
    /usr/local/X11R6.1/include  \
    /usr/local/X11R6/include  \
    /usr/local/X11R5/include  \
    /usr/local/X11R4/include  \
                              \
    /usr/local/include/X11R6.5.1  \
    /usr/local/include/X11R6.4  \
    /usr/local/include/X11R6.3  \
    /usr/local/include/X11R6.2  \
    /usr/local/include/X11R6.1  \
    /usr/local/include/X11R6  \
    /usr/local/include/X11R5  \
    /usr/local/include/X11R4  \
                              \
    /usr/X11/include          \
    /usr/include/X11          \
    /usr/local/X11/include    \
    /usr/local/include/X11    \
                              \
    /usr/X386/include         \
    /usr/x386/include         \
    /usr/XFree86/include/X11  \
    /usr/pkg/include          \
                              \
    /usr/dt/include           \
                              \
    /usr/local/include        \
    /usr/remote/include       \
    /usr/include              \
    /usr/unsupported/include  \
    /usr/dpms/include         \
    /usr/local/x11r5/include  \
    /usr/lpp/Xamples/include  \
                              \
    /usr/openwin/include      \
    /usr/openwin/share/include \
    $extra_include            \
    ; \
  do
    if test -r "$ac_dir/$ext_direct_test_include"; then
      no_ext= ac_ext_includes=$ac_dir
      break
    fi
  done

# Check for the libraries.
# See if we find them without any special options.
# Do not add to $LIBS permanently.
ac_save_LIBS="$LIBS"
LIBS="-l$ext_direct_test_library $LIBS"
# First see if replacing the include by lib works.
for ac_dir in `echo "$ac_ext_includes" | sed s/include/lib/` \
                          \
    /usr/X11R6.5.1/lib    \
    /usr/X11R6.4/lib      \
    /usr/X11R6.3/lib      \
    /usr/X11R6.2/lib      \
    /usr/X11R6.1/lib      \
    /usr/X11R6/lib        \
    /usr/X11R5/lib        \
    /usr/X11R4/lib        \
                          \
    /usr/lib/X11R6.5.1    \
    /usr/lib/X11R6.4      \
    /usr/lib/X11R6.3      \
    /usr/lib/X11R6.2      \
    /usr/lib/X11R6.1      \
    /usr/lib/X11R6        \
    /usr/lib/X11R5        \
    /usr/lib/X11R4        \
                          \
    /usr/local/X11R6.5.1/lib  \
    /usr/local/X11R6.4/lib  \
    /usr/local/X11R6.3/lib  \
    /usr/local/X11R6.2/lib  \
    /usr/local/X11R6.1/lib  \
    /usr/local/X11R6/lib  \
    /usr/local/X11R5/lib  \
    /usr/local/X11R4/lib  \
                          \
    /usr/local/lib/X11R6.5.1  \
    /usr/local/lib/X11R6.4  \
    /usr/local/lib/X11R6.3  \
    /usr/local/lib/X11R6.2  \
    /usr/local/lib/X11R6.1  \
    /usr/local/lib/X11R6  \
    /usr/local/lib/X11R5  \
    /usr/local/lib/X11R4  \
                          \
    /usr/X11/lib          \
    /usr/lib/X11          \
    /usr/local/X11/lib    \
                          \
    /usr/X386/lib         \
    /usr/x386/lib         \
    /usr/XFree86/lib/X11  \
    /usr/pkg/lib          \
                          \
    /usr/lib              \
    /usr/local/lib        \
    /usr/remote/lib       \
    /usr/unsupported/lib  \
    /usr/dpms/lib       \
    /usr/local/x11r5/lib  \
    /usr/lpp/Xamples/lib  \
                          \
    /usr/openwin/lib      \
    /usr/openwin/share/lib \
    /shlib                \
    /usr/shlib            \
    /usr/shlib/X11        \
    $extra_lib            \
    ; \
do
  for ac_extension in a so sl; do
    if test -r $ac_dir/lib${ext_direct_test_library}.$ac_extension; then
      no_ext= ac_ext_libraries=$ac_dir
      break 2
    fi
  done
done
LIBS="$ac_save_LIBS"])
AC_DEFUN([AC_PATH_EXT],
[AC_REQUIRE_CPP()dnl

ext_includes=NONE
ext_libraries=NONE

AC_MSG_CHECKING(for EXT)
AC_ARG_WITH(ext, [  --without-ext           disable EXT])
extlib=no
if test "x$with_ext" = xno; then
  no_ext=yes
else
  if test "x$ext_includes" != xNONE && test "x$ext_libraries" != xNONE; then
    no_ext=
  else
AC_CACHE_VAL(ac_cv_path_ext,
[# One or both of these vars are not set, and there is no cached value.
no_EXT=yes
AC_PATH_EXT_DIRECT

if test "$no_ext" = yes; then
  ac_cv_path_ext="no_ext=yes"
else
  ac_cv_path_ext="no_ext= ac_ext_includes=$ac_ext_includes ac_ext_libraries=$ac_ext_libraries"
fi])dnl
  fi
  eval "$ac_cv_path_ext"
fi # with_ext != no

if test "$no_ext" = yes; then
  AC_MSG_RESULT(no)
else
  AC_DEFINE([USE_EXT], [1], [Extension Libraries])
dnl test needed for Digital Unix machines
  if test "x$ac_ext_libraries" != x && test "x$ac_ext_libraries" != xNONE ; then
    XLOCKLIBS="${XLOCKLIBS} -lXext"
  fi
  test "x$ext_includes" = xNONE && ext_includes=$ac_ext_includes
  test "x$ext_libraries" = xNONE && ext_libraries=$ac_ext_libraries
  ac_cv_path_ext="no_ext= ac_ext_includes=$ext_includes ac_ext_libraries=$ext_libraries"
  AC_MSG_RESULT([libraries $ext_libraries, headers $ext_includes])
fi
])

AC_PATH_EXT

dnl Hard coded to bring Xext
dnl if test "x$ext_libraries" != x && test "x$ext_libraries" != xNONE ; then
dnl   XLOCK_LDFLAGS="${XLOCK_LDFLAGS}:$ext_libraries"
dnl   XLOCKLIBPATHS="${XLOCKLIBPATHS} -L$ext_libraries"
dnl fi
if test "x$ext_includes" != x && test "x$ext_includes" != xNONE ; then
  XLOCKINC="${XLOCKINC} -I$ext_includes"
fi

dnl Xdpms DPMS

AC_DEFUN([AC_PATH_DPMS_DIRECT],
[test -z "$dpms_direct_test_library" && dpms_direct_test_library=Xdpms
test -z "$dpms_direct_test_function" && dpms_direct_test_function=DPMSSetTimeouts
test -z "$dpms_direct_test_include" && dpms_direct_test_include=X11/extensions/dpms.h
  for ac_dir in               \
    /usr/X11R6.5.1/include    \
    /usr/X11R6.4/include      \
    /usr/X11R6.3/include      \
    /usr/X11R6.2/include      \
    /usr/X11R6.1/include      \
    /usr/X11R6/include        \
    /usr/X11R5/include        \
    /usr/X11R4/include        \
                              \
    /usr/include/X11R6.5.1    \
    /usr/include/X11R6.4      \
    /usr/include/X11R6.3      \
    /usr/include/X11R6.2      \
    /usr/include/X11R6.1      \
    /usr/include/X11R6        \
    /usr/include/X11R5        \
    /usr/include/X11R4        \
                              \
    /usr/local/X11R6.5.1/include  \
    /usr/local/X11R6.4/include  \
    /usr/local/X11R6.3/include  \
    /usr/local/X11R6.2/include  \
    /usr/local/X11R6.1/include  \
    /usr/local/X11R6/include  \
    /usr/local/X11R5/include  \
    /usr/local/X11R4/include  \
                              \
    /usr/local/include/X11R6.5.1  \
    /usr/local/include/X11R6.4  \
    /usr/local/include/X11R6.3  \
    /usr/local/include/X11R6.2  \
    /usr/local/include/X11R6.1  \
    /usr/local/include/X11R6  \
    /usr/local/include/X11R5  \
    /usr/local/include/X11R4  \
                              \
    /usr/X11/include          \
    /usr/include/X11          \
    /usr/local/X11/include    \
    /usr/local/include/X11    \
                              \
    /usr/X386/include         \
    /usr/x386/include         \
    /usr/XFree86/include/X11  \
    /usr/pkg/include          \
                              \
    /usr/dt/include           \
                              \
    /usr/local/include        \
    /usr/remote/include       \
    /usr/include              \
    /usr/unsupported/include  \
    /usr/dpms/include         \
    /usr/local/x11r5/include  \
    /usr/lpp/Xamples/include  \
                              \
    /usr/openwin/include      \
    /usr/openwin/share/include \
    $extra_include            \
    ; \
  do
    if test -r "$ac_dir/$dpms_direct_test_include"; then
      no_dpms= ac_dpms_includes=$ac_dir
      break
    fi
  done

# Check for the libraries.
# See if we find them without any special options.
# Do not add to $LIBS permanently.
ac_save_LIBS="$LIBS"
LIBS="-l$dpms_direct_test_library $LIBS"
# First see if replacing the include by lib works.
for ac_dir in `echo "$ac_dpms_includes" | sed s/include/lib/` \
                          \
    /usr/X11R6.5.1/lib    \
    /usr/X11R6.4/lib      \
    /usr/X11R6.3/lib      \
    /usr/X11R6.2/lib      \
    /usr/X11R6.1/lib      \
    /usr/X11R6/lib        \
    /usr/X11R5/lib        \
    /usr/X11R4/lib        \
                          \
    /usr/lib/X11R6.5.1    \
    /usr/lib/X11R6.4      \
    /usr/lib/X11R6.3      \
    /usr/lib/X11R6.2      \
    /usr/lib/X11R6.1      \
    /usr/lib/X11R6        \
    /usr/lib/X11R5        \
    /usr/lib/X11R4        \
                          \
    /usr/local/X11R6.5.1/lib  \
    /usr/local/X11R6.4/lib  \
    /usr/local/X11R6.3/lib  \
    /usr/local/X11R6.2/lib  \
    /usr/local/X11R6.1/lib  \
    /usr/local/X11R6/lib  \
    /usr/local/X11R5/lib  \
    /usr/local/X11R4/lib  \
                          \
    /usr/local/lib/X11R6.5.1  \
    /usr/local/lib/X11R6.4  \
    /usr/local/lib/X11R6.3  \
    /usr/local/lib/X11R6.2  \
    /usr/local/lib/X11R6.1  \
    /usr/local/lib/X11R6  \
    /usr/local/lib/X11R5  \
    /usr/local/lib/X11R4  \
                          \
    /usr/X11/lib          \
    /usr/lib/X11          \
    /usr/local/X11/lib    \
                          \
    /usr/X386/lib         \
    /usr/x386/lib         \
    /usr/XFree86/lib/X11  \
    /usr/pkg/lib          \
                          \
    /usr/lib              \
    /usr/local/lib        \
    /usr/remote/lib       \
    /usr/unsupported/lib  \
    /usr/dpms/lib       \
    /usr/local/x11r5/lib  \
    /usr/lpp/Xamples/lib  \
                          \
    /usr/openwin/lib      \
    /usr/openwin/share/lib \
    /shlib                \
    /usr/shlib            \
    /usr/shlib/X11        \
    $extra_lib            \
    ; \
do
  for ac_extension in a so sl; do
    if test -r $ac_dir/lib${dpms_direct_test_library}.$ac_extension; then
      no_dpms= ac_dpms_libraries=$ac_dir
      break 2
    fi
  done
done
LIBS="$ac_save_LIBS"])
AC_DEFUN([AC_PATH_DPMS],
[AC_REQUIRE_CPP()dnl

dpms_includes=NONE
dpms_libraries=NONE

AC_MSG_CHECKING(for DPMS)
AC_ARG_WITH(dpms, [  --without-dpms          disable DPMS])
dpmslib=no
if test "x$with_dpms" = xno; then
  no_dpms=yes
else
  if test "x$dpms_includes" != xNONE && test "x$dpms_libraries" != xNONE; then
    no_dpms=
  else
AC_CACHE_VAL(ac_cv_path_dpms,
[# One or both of these vars are not set, and there is no cached value.
no_dpms=yes
AC_PATH_DPMS_DIRECT

if test "$no_dpms" = yes; then
  ac_cv_path_dpms="no_dpms=yes"
else
  ac_cv_path_dpms="no_dpms= ac_dpms_includes=$ac_dpms_includes ac_dpms_libraries=$ac_dpms_libraries"
fi])dnl
  fi
  eval "$ac_cv_path_dpms"
fi # with_dpms != no

if test "$no_dpms" = yes; then
  AC_MSG_RESULT(no)
else
  AC_DEFINE([USE_DPMS], [1], [Display Power Management System])
dnl test needed for Digital Unix machines
  if test "x$ac_dpms_libraries" != x && test "x$ac_dpms_libraries" != xNONE ; then
    XLOCKLIBS="${XLOCKLIBS} -lXdpms"
  fi
  test "x$dpms_includes" = xNONE && dpms_includes=$ac_dpms_includes
  test "x$dpms_libraries" = xNONE && dpms_libraries=$ac_dpms_libraries
  ac_cv_path_dpms="no_dpms= ac_dpms_includes=$dpms_includes ac_dpms_libraries=$dpms_libraries"
  AC_MSG_RESULT([libraries $dpms_libraries, headers $dpms_includes])
fi
])

AC_PATH_DPMS

if test "x$dpms_libraries" != x && test "x$dpms_libraries" != xNONE ; then
  XLOCK_LDFLAGS="${XLOCK_LDFLAGS}:$dpms_libraries"
  XLOCKLIBPATHS="${XLOCKLIBPATHS} -L$dpms_libraries"
fi
if test "x$dpms_includes" != x && test "x$dpms_includes" != xNONE ; then
  XLOCKINC="${XLOCKINC} -I$dpms_includes"
fi

xinerama=no
dnl xinerama Xinerama extension

AC_DEFUN([AC_PATH_XINERAMA_DIRECT],
[test -z "$xinerama_direct_test_library" && xinerama_direct_test_library=Xinerama
test -z "$xinerama_direct_test_function" && xinerama_direct_test_function=XineramaQueryExtension
test -z "$xinerama_direct_test_include" && xinerama_direct_test_include=X11/extensions/Xinerama.h
  for ac_dir in               \
    /usr/X11R6.5.1/include	    \
    /usr/X11R6.4/include	      \
    /usr/X11R6.3/include	      \
    /usr/X11R6.2/include	      \
    /usr/X11R6.1/include	      \
    /usr/X11R6/include	        \
    /usr/X11R5/include	        \
    /usr/X11R4/include	        \
    /usr/X11/include	          \
                              \
    /usr/include/X11	          \
    /usr/include/X11R6.5.1	    \
    /usr/include/X11R6.4	      \
    /usr/include/X11R6.3	      \
    /usr/include/X11R6.2	      \
    /usr/include/X11R6.1	      \
    /usr/include/X11R6	        \
    /usr/include/X11R5	        \
    /usr/include/X11R4	        \
                              \
    /usr/local/X11/include	    \
    /usr/local/X11R6.5.1/include	  \
    /usr/local/X11R6.4/include	  \
    /usr/local/X11R6.3/include	  \
    /usr/local/X11R6.2/include	  \
    /usr/local/X11R6.1/include	  \
    /usr/local/X11R6/include	  \
    /usr/local/X11R5/include	  \
    /usr/local/X11R4/include	  \
                              \
    /usr/local/include/X11	    \
    /usr/local/include/X11R6.5.1	  \
    /usr/local/include/X11R6.4	  \
    /usr/local/include/X11R6.3	  \
    /usr/local/include/X11R6.2	  \
    /usr/local/include/X11R6.1	  \
    /usr/local/include/X11R6	  \
    /usr/local/include/X11R5	  \
    /usr/local/include/X11R4	  \
                              \
    /usr/X386/include	         \
    /usr/x386/include	         \
    /usr/XFree86/include/X11	  \
    /usr/pkg/include          \
                              \
    /usr/include	        \
    /usr/local/include	        \
    /usr/remote/include	       \
    /usr/include/X11	          \
    /usr/unsupported/include	  \
    /usr/athena/include	       \
    /usr/local/x11r5/include	  \
    /usr/lpp/Xamples/include	  \
                              \
    /usr/openwin/include	      \
    /usr/openwin/share/include	 \
    /usr/openwin/include	      \
                              \
    $extra_include	            \
    ; \
  do
    if test -r "$ac_dir/$xinerama_direct_test_include"; then
      no_xinerama= ac_xinerama_includes=$ac_dir
      break
    fi
  done

# Check for the libraries.
# See if we find them without any special options.
# Do not add to $LIBS permanently.
ac_save_LIBS="$LIBS"
LIBS="-l$xinerama_direct_test_library $LIBS"
# First see if replacing the include by lib works.
for ac_dir in `echo "$ac_xinerama_includes" | sed s/include/lib/` \
                          \
    /usr/X11/lib          \
    /usr/X11R6.5.1/lib      \
    /usr/X11R6.4/lib        \
    /usr/X11R6.3/lib        \
    /usr/X11R6.2/lib        \
    /usr/X11R6.1/lib        \
    /usr/X11R6/lib        \
    /usr/X11R5/lib        \
    /usr/X11R4/lib        \
                          \
    /usr/lib/X11          \
    /usr/lib/X11R6.5.1      \
    /usr/lib/X11R6.4        \
    /usr/lib/X11R6.3        \
    /usr/lib/X11R6.2        \
    /usr/lib/X11R6.1        \
    /usr/lib/X11R6        \
    /usr/lib/X11R5        \
    /usr/lib/X11R4        \
                          \
    /usr/local/X11/lib    \
    /usr/local/X11R6/lib  \
    /usr/local/X11R5/lib  \
    /usr/local/X11R4/lib  \
                          \
    /usr/local/lib/X11    \
    /usr/local/X11R6.5.1/lib  \
    /usr/local/X11R6.4/lib  \
    /usr/local/X11R6.3/lib  \
    /usr/local/X11R6.2/lib  \
    /usr/local/X11R6.1/lib  \
    /usr/local/lib/X11R6  \
    /usr/local/lib/X11R5  \
    /usr/local/lib/X11R4  \
                          \
    /usr/X386/lib         \
    /usr/x386/lib         \
    /usr/XFree86/lib/X11  \
    /usr/pkg/lib          \
                          \
    /usr/lib              \
    /usr/local/lib        \
    /usr/remote/lib       \
    /usr/unsupported/lib  \
    /usr/athena/lib       \
    /usr/local/x11r5/lib  \
    /usr/lpp/Xamples/lib  \
    /lib/usr/lib/X11      \
                          \
    /usr/openwin/lib      \
    /usr/openwin/share/lib \
    $extra_lib            \
    ; \
do
  for ac_extension in a so sl; do
    if test -r $ac_dir/lib${xinerama_direct_test_library}.$ac_extension; then
      no_xinerama= ac_xinerama_libraries=$ac_dir
      break 2
   fi
  done
done
LIBS="$ac_save_LIBS"])
AC_DEFUN([AC_PATH_XINERAMA],
[AC_REQUIRE_CPP()dnl

xinerama_includes=NONE
xinerama_libraries=NONE

AC_MSG_CHECKING(for XINERAMA)
AC_ARG_WITH(xinerama, [  --without-xinerama      disable Xinerama support])
if test "x$with_xinerama" = xno; then
  no_xinerama=yes
else
  if test "x$xinerama_includes" != xNONE && test "x$xinerama_libraries" != xNONE; then
    no_xinerama=
  else
AC_CACHE_VAL(ac_cv_path_xinerama,
[# One or both of these vars are not set, and there is no cached value.
no_xinerama=yes
AC_PATH_XINERAMA_DIRECT

if test "$no_xinerama" = yes; then
  ac_cv_path_xinerama="no_xinerama=yes"
else
  ac_cv_path_xinerama="no_xinerama= ac_xinerama_includes=$ac_xinerama_includes ac_xinerama_libraries=$ac_xinerama_libraries"
fi])dnl
  fi
  eval "$ac_cv_path_xinerama"
fi # with_xinerama != no

if test "$no_xinerama" = yes; then
  AC_MSG_RESULT(no)
  XINERAMA="#"
else
  xinerama=yes
  AC_DEFINE([USE_XINERAMA], [1], [Xinemara to combine separate displays])
  XLOCKLIBS="${XLOCKLIBS} -lXinerama"
  test "x$xinerama_includes" = xNONE && xinerama_includes=$ac_xinerama_includes
  test "x$xinerama_libraries" = xNONE && xinerama_libraries=$ac_xinerama_libraries
  ac_cv_path_xinerama="no_xinerama= ac_xinerama_includes=$xinerama_includes ac_xinerama_libraries=$xinerama_libraries"
  AC_MSG_RESULT([libraries $xinerama_libraries, headers $xinerama_includes])
  XINERAMA=""
fi
])

AC_PATH_XINERAMA

if test "x$xinerama_libraries" != x && test "x$xinerama_libraries" != xNONE ; then
  XLOCK_LDFLAGS="${XLOCK_LDFLAGS}:$xinerama_libraries"
  XLOCKLIBPATHS="${XLOCKLIBPATHS} -L$xinerama_libraries"
fi
if test "x$xinerama_includes" != x && test "x$xinerama_includes" != xNONE ; then
  XLOCKINC="${XLOCKINC} -I$xinerama_includes"
fi

sound=none

NOPLAY="#"
if test "$sound" = none; then
  AC_MSG_CHECKING(for DEF_PLAY)
  AC_ARG_ENABLE(def_play, [  --disable-def-play=program
                          set def-play to program that plays sounds],def_play=$enableval,def_play=yes)
  if test "$def_play" = "yes"; then
    def_play=play.sh # yes is a bad default here
  fi
  if test "$def_play" != no; then
    AC_MSG_RESULT([defining option DEF_PLAY = "$def_play"])
    AC_DEFINE_UNQUOTED([DEF_PLAY], ["$def_play"], [program to play sounds])
    sound=$def_play
    NOPLAY=""
  else
    AC_MSG_RESULT(no)
  fi
fi

dnl RPLAY RPlay rplay

AC_DEFUN([AC_PATH_RPLAY_DIRECT],
[test -z "$rplay_direct_test_library" && rplay_direct_test_library=rplay
test -z "$rplay_direct_test_function" && rplay_direct_test_function=rplay_open_default
test -z "$rplay_direct_test_include" && rplay_direct_test_include=rplay.h
  for ac_dir in               \
    /usr/X11R6.5.1/include    \
    /usr/X11R6.4/include      \
    /usr/X11R6.3/include      \
    /usr/X11R6.2/include      \
    /usr/X11R6.1/include      \
    /usr/X11R6/include        \
    /usr/X11R5/include        \
    /usr/X11/include          \
    /usr/local/include        \
    /usr/remote/include       \
    /usr/pkg/include          \
    /usr/include              \
    $extra_include            \
    ; \
  do
    if test -r "$ac_dir/$rplay_direct_test_include"; then
      no_rplay= ac_rplay_includes=$ac_dir
      break
    fi
  done

# Check for the libraries.
# See if we find them without any special options.
# Do not add to $LIBS permanently.
ac_save_LIBS="$LIBS"
LIBS="-l$rplay_direct_test_library $LIBS"
# First see if replacing the include by lib works.
for ac_dir in `echo "$ac_rplay_includes" | sed s/include/lib/` \
                          \
    /usr/X11R6.5.1/lib    \
    /usr/X11R6.4/lib      \
    /usr/X11R6.3/lib      \
    /usr/X11R6.2/lib      \
    /usr/X11R6.1/lib      \
    /usr/X11R6/lib        \
    /usr/X11R5/lib        \
    /usr/X11/lib          \
    /usr/lib              \
    /usr/local/lib        \
    /usr/remote/lib       \
    /usr/pkg/lib          \
    $extra_lib            \
    ; \
do
  for ac_extension in a so sl; do
    if test -r $ac_dir/lib${rplay_direct_test_library}.$ac_extension; then
      no_rplay= ac_rplay_libraries=$ac_dir
      break 2
    fi
  done
done
LIBS="$ac_save_LIBS"])
AC_DEFUN([AC_PATH_RPLAY],
[AC_REQUIRE_CPP()dnl

rplay_includes=NONE
rplay_libraries=NONE

AC_MSG_CHECKING(for RPLAY)
AC_ARG_WITH(rplay, [  --without-rplay         disable RPLAY sounds])
if test "x$with_rplay" = xno; then
  no_rplay=yes
else
  if test "x$rplay_includes" != xNONE && test "x$rplay_libraries" != xNONE; then
    no_rplay=
  else
AC_CACHE_VAL(ac_cv_path_rplay,
[# One or both of these vars are not set, and there is no cached value.
no_rplay=yes
AC_PATH_RPLAY_DIRECT

if test "$no_rplay" = yes; then
  ac_cv_path_rplay="no_rplay=yes"
else
  ac_cv_path_rplay="no_rplay= ac_rplay_includes=$ac_rplay_includes ac_rplay_libraries=$ac_rplay_libraries"
fi])dnl
  fi
  eval "$ac_cv_path_rplay"
fi # with_rplay != no

if test "$no_rplay" = yes; then
  AC_MSG_RESULT(no)
else
dnl  solaris26=no
dnl  case "${canonical}" in
dnl    *-*-solaris2.6 )
dnl    solaris26=yes
dnl    ;;
dnl  esac
dnl  if test "$CC" = gcc && test "$solaris26" = yes; then
dnl    AC_MSG_RESULT(ignored - problems here with gcc and solaris2.6)
dnl  else
  if test "$sound" = none; then
    sound=rplay
    AC_DEFINE([USE_RPLAY], [1], [using RPLAY audio])
    XLOCKLIBS="${XLOCKLIBS} -lrplay"
    test "x$rplay_includes" = xNONE && rplay_includes=$ac_rplay_includes
    test "x$rplay_libraries" = xNONE && rplay_libraries=$ac_rplay_libraries
    ac_cv_path_rplay="no_rplay= ac_rplay_includes=$rplay_includes ac_rplay_libraries=$rplay_libraries"
    AC_MSG_RESULT([libraries $rplay_libraries, headers $rplay_includes])
  else
    AC_MSG_RESULT(ignored - using previous definition for sound)
  fi
dnl  fi
fi
])

AC_PATH_RPLAY

if test "x$rplay_libraries" != x && test "x$rplay_libraries" != xNONE ; then
  XLOCK_LDFLAGS="${XLOCK_LDFLAGS}:$rplay_libraries"
  XLOCKLIBPATHS="${XLOCKLIBPATHS} -L$rplay_libraries"
fi
if test "x$rplay_includes" != x && test "x$rplay_includes" != xNONE ; then
  XLOCKINC="${XLOCKINC} -I$rplay_includes"
fi

dnl ESOUND Enlightenment sound daemon

AC_DEFUN([AC_PATH_ESOUND],
[AC_REQUIRE_CPP()dnl

esound_includes=NONE
esound_libraries=NONE

AC_MSG_CHECKING(for ESOUND)
AC_ARG_WITH(esound,            [  --with-esound           enable Enlightenment Sound Daemon sounds])
AC_ARG_WITH(esound_config,     [  --with-esound-config    use this configator for Enlightenment Sound Daemon])
AC_ARG_WITH(esound_prefix,     [  --with-esound-prefix    use this prefix for Enlightenment Sound Daemon])
AC_ARG_WITH(esound_includes,   [  --with-esound-includes  use this dir for Enlightenment Sound Daemon headers])
AC_ARG_WITH(esound_libraries,  [  --with-esound-libraries use this dir for Enlightenment Sound Daemon libs])
if test "x$with_esound" = xno; then
  no_esound=yes
else
  if test "x$esound_includes" != xNONE && test "x$esound_libraries" != xNONE; then
    no_esound=
    if test "x$esound_config" = "x"; then
      if test "x$esound_prefix" != "x"; then
        esound_config="$esound_prefix/bin/esd-config"
      else
        esound_config="esd-config"
     fi
   else
     if test "x$esound_prefix" = "x"; then
        esound_prefix=`($esound_config --prefix) 2>/dev/null`
     fi
   fi
  else
    if test "x$esound_config" = "x"; then
      if test "x$esound_prefix" != "x"; then
        esound_config="$esound_prefix/bin/esd-config"
      else
        esound_config="esd-config"
     fi
   else
     if test "x$esound_prefix" = "x"; then
        esound_prefix=`($esound_config --prefix) 2>/dev/null`
     fi
   fi
AC_CACHE_VAL(ac_cv_path_esound,
[# One or both of these vars are not set, and there is no cached value.
no_esound=yes
no_esound_config=yes
if test "x$esound_prefix" != "x"; then
  ac_esound_prefix=$esound_prefix
  ac_esound_includes=$ac_esound_prefix/include
  ac_esound_libraries=$ac_esound_prefix/lib
  no_esound="ac_esound_config=$ac_esound_config ac_esound_prefix=$ac_esound_prefix ac_esound_includes=$ac_esound_includes ac_esound_libraries=$ac_esound_libraries"
else
  ac_esound_config="$esound_config"
  ac_esound_prefix=`($esound_config --prefix) 2>/dev/null`
  if test $? != 0; then
    no_esound=yes
    no_esound_config=yes
    ac_esound_config=""
    ac_esound_prefix=""
  else
    no_esound=
    no_esound_config=
    ac_esound_includes=$ac_esound_prefix/include
    ac_esound_libraries=$ac_esound_prefix/lib
  fi
fi
if test "$no_esound" = yes; then
  ac_cv_path_esound="no_esound=yes"
else
  ac_cv_path_esound="no_esound= ac_esound_config=\"$ac_esound_config\" ac_esound_prefix=\"$ac_esound_prefix\" ac_esound_includes=\"$ac_esound_includes\" ac_esound_libraries=\"$ac_esound_libraries\""
fi])dnl
  fi
  eval "$ac_cv_path_esound"
fi # with_esound != no

if test "$no_esound_config" = yes; then
  AC_MSG_RESULT(no esd-config)
else
  if test "$no_esound" = yes; then
    AC_MSG_RESULT(no)
  else
    if test "$sound" = none; then
      sound=esound
      AC_DEFINE([USE_ESOUND], [1], [using ESOUND audio])
      AC_DEFINE([HAVE_LIBESD], [1], [LIBESD audio is available])
      test "x$esound_config" = xNONE && esound_config=$ac_esound_config
      test "x$esound_prefix" = xNONE && esound_prefix=$ac_esound_prefix
      test "x$esound_includes" = xNONE && esound_includes=$ac_esound_includes
      test "x$esound_libraries" = xNONE && esound_libraries=$ac_esound_libraries
      ac_cv_path_esound="no_esound= ac_esound_config=$esound_config ac_esound_prefix=$esound_prefix ac_esound_includes=$esound_includes ac_esound_libraries=$esound_libraries"
      AC_MSG_RESULT([libraries $esound_libraries, headers $esound_includes])
    else
      AC_MSG_RESULT(ignored - using previous definition for sound)
    fi
  fi
fi
])

AC_PATH_ESOUND

if test "x$esound_libraries" != x && test "x$esound_libraries" != xNONE ; then
  XLOCKLIBS="${XLOCKLIBS} `$esound_config --libs`"
  XLOCK_LDFLAGS="${XLOCK_LDFLAGS}:$esound_libraries"
  XLOCKLIBPATHS="${XLOCKLIBPATHS} -L$esound_libraries"
fi
if test "x$esound_includes" != x && test "x$esound_includes" != xNONE ; then
  XLOCKINC="${XLOCKINC} -I$esound_includes `$esound_config --cflags`"
fi

dnl NAS audio

AC_DEFUN([AC_PATH_NAS_DIRECT],
[test -z "$nas_direct_test_library" && nas_direct_test_library=audio
test -z "$nas_direct_test_function" && nas_direct_test_function=AuOpenServer
test -z "$nas_direct_test_include" && nas_direct_test_include=audio/audio.h
  for ac_dir in               \
    /usr/X11R6.5.1/include    \
    /usr/X11R6.4/include      \
    /usr/X11R6.3/include      \
    /usr/X11R6.2/include      \
    /usr/X11R6.1/include      \
    /usr/X11R6/include        \
    /usr/X11R5/include        \
    /usr/X11/include          \
    /usr/local/include        \
    /usr/remote/include       \
    /usr/pkg/include          \
    /usr/include              \
    $extra_include            \
    ; \
  do
    if test -r "$ac_dir/$nas_direct_test_include"; then
      no_nas= ac_nas_includes=$ac_dir
      break
    fi
  done

# Check for the libraries.
# See if we find them without any special options.
# Do not add to $LIBS permanently.
ac_save_LIBS="$LIBS"
LIBS="-l$nas_direct_test_library $LIBS"
# First see if replacing the include by lib works.
for ac_dir in `echo "$ac_nas_includes" | sed s/include/lib/` \
                          \
    /usr/X11R6.5.1/lib    \
    /usr/X11R6.4/lib      \
    /usr/X11R6.3/lib      \
    /usr/X11R6.2/lib      \
    /usr/X11R6.1/lib      \
    /usr/X11R6/lib        \
    /usr/X11R5/lib        \
    /usr/X11/lib          \
    /usr/lib              \
    /usr/local/lib        \
    /usr/remote/lib       \
    /usr/pkg/lib          \
    $extra_lib            \
    ; \
do
  for ac_extension in a so sl; do
    if test -r $ac_dir/lib${nas_direct_test_library}.$ac_extension; then
      no_nas= ac_nas_libraries=$ac_dir
      break 2
    fi
  done
done
LIBS="$ac_save_LIBS"])
AC_DEFUN([AC_PATH_NAS],
[AC_REQUIRE_CPP()dnl

nas_includes=NONE
nas_libraries=NONE

AC_MSG_CHECKING(for NAS)
AC_ARG_WITH(nas, [  --without-nas           disable NAS sounds])
if test "x$with_nas" = xno; then
  no_nas=yes
else
  if test "x$nas_includes" != xNONE && test "x$nas_libraries" != xNONE; then
    no_nas=
  else
AC_CACHE_VAL(ac_cv_path_nas,
[# One or both of these vars are not set, and there is no cached value.
no_nas=yes
AC_PATH_NAS_DIRECT

if test "$no_nas" = yes; then
  ac_cv_path_nas="no_nas=yes"
else
  ac_cv_path_nas="no_nas= ac_nas_includes=$ac_nas_includes ac_nas_libraries=$ac_nas_libraries"
fi])dnl
  fi
  eval "$ac_cv_path_nas"
fi # with_nas != no

if test "$no_nas" = yes; then
  AC_MSG_RESULT(no)
else
  if test "$sound" = none; then
    sgi=no
    case "${canonical}" in
      *-*-irix5* | *-*-irix6* )
        sgi=yes
      ;;
    esac
    if test "$sgi" = yes; then
      AC_MSG_RESULT(ignored - problems here with audio and nas)
    else
      sound=nas
      AC_DEFINE([USE_NAS], [1], [using NAS audio])
      XLOCKLIBS="${XLOCKLIBS} -laudio"
dnl    XLOCKLIBS="${XLOCKLIBS} -lnas"
      test "x$nas_includes" = xNONE && nas_includes=$ac_nas_includes
      test "x$nas_libraries" = xNONE && nas_libraries=$ac_nas_libraries
      ac_cv_path_nas="no_nas= ac_nas_includes=$nas_includes ac_nas_libraries=$nas_libraries"
      AC_MSG_RESULT([libraries $nas_libraries, headers $nas_includes])
    fi
  else
    AC_MSG_RESULT(ignored - using previous definition for sound)
  fi
fi
])

AC_PATH_NAS

if test "x$nas_libraries" != x && test "x$nas_libraries" != xNONE ; then
  XLOCK_LDFLAGS="${XLOCK_LDFLAGS}:$nas_libraries"
  XLOCKLIBPATHS="${XLOCKLIBPATHS} -L$nas_libraries"
fi
if test "x$nas_includes" != x && test "x$nas_includes" != xNONE ; then
  XLOCKINC="${XLOCKINC} -I$nas_includes"
fi

if test "$sound" != none; then
  if test "x$prefix" = "xNONE"; then
    AC_DEFINE_UNQUOTED([SOUNDPATH], "/usr/local/share/xlock/sounds", [sound path])
  else
    AC_DEFINE_UNQUOTED([SOUNDPATH], "${prefix}/share/xlock/sounds", [sound path])
  fi
fi

dnl crypt CRYPT

AC_DEFUN([AC_PATH_CRYPT_DIRECT],
[test -z "$crypt_direct_test_library" && crypt_direct_test_library=crypt

# Check for the libraries.
# See if we find them without any special options.
# Do not add to $LIBS permanently.
ac_save_LIBS="$LIBS"
LIBS="-l$crypt_direct_test_library $LIBS"
for ac_dir in \
    /usr/lib              \
    /usr/local/lib        \
    /usr/remote/lib       \
    /usr/pkg/lib          \
    $extra_lib            \
    ; \
do
  for ac_extension in a so sl; do
    if test -r $ac_dir/lib${crypt_direct_test_library}.$ac_extension; then
      no_crypt= ac_crypt_libraries=$ac_dir
      break 2
    fi
  done
done
LIBS="$ac_save_LIBS"])
AC_DEFUN([AC_PATH_CRYPT],
[AC_REQUIRE_CPP()dnl

crypt_libraries=NONE

AC_MSG_CHECKING(for CRYPT)
AC_ARG_WITH(crypt, [  --without-crypt         disable CRYPT])
if test "x$with_crypt" = xno; then
  no_crypt=yes
else
  if test "x$crypt_libraries" != xNONE; then
    no_crypt=
  else
AC_CACHE_VAL(ac_cv_path_crypt,
[# One or both of these vars are not set, and there is no cached value.
no_crypt=yes
AC_PATH_CRYPT_DIRECT

if test "$no_crypt" = yes; then
  ac_cv_path_crypt="no_crypt=yes"
else
  ac_cv_path_crypt="no_crypt= ac_crypt_libraries=$ac_crypt_libraries"
fi])dnl
  fi
  eval "$ac_cv_path_crypt"
fi # with_crypt != no

if test "$no_crypt" = yes; then
  AC_MSG_RESULT(no)
  # Fallback for AC_PATH_CRYPT_DIRECT.
  AC_SEARCH_LIBS([crypt], [crypt],
  [if test "x$ac_cv_search_crypt" != 'xnone required'; then
    XLOCKLIBS="${XLOCKLIBS} $ac_cv_search_crypt"
  fi])
else
  XLOCKLIBS="${XLOCKLIBS} -lcrypt"
  test "x$crypt_libraries" = xNONE && crypt_libraries=$ac_crypt_libraries
  ac_cv_path_crypt="no_crypt= ac_crypt_libraries=$crypt_libraries"
  AC_MSG_RESULT([libraries $crypt_libraries])
fi
])

AC_PATH_CRYPT

if test "x$crypt_libraries" != x && test "x$crypt_libraries" != xNONE ; then
  XLOCK_LDFLAGS="${XLOCK_LDFLAGS}:$crypt_libraries"
  XLOCKLIBPATHS="${XLOCKLIBPATHS} -L$crypt_libraries"
fi


XGLOCK=""
XGLOCKDIR=""
INSTALL_XGLOCK=""
UNINSTALL_XGLOCK=""

dnl
dnl Check pkg-config borrowed from xscreensaver
dnl

AC_PATH_PROGS(pkg_config, pkg-config,, $PATH)

if test -n "$pkg_config" ; then
    #
    # the new way...
    # run pkg-config based tests.
    #
    pkgs=''
    pkg_check_version() {
      if test "$ok" = yes ; then
        req="$1"
        min="$2"
        AC_MSG_CHECKING(for $req)
        if $pkg_config --exists "$req" 2> /dev/null ; then
          vers=`$pkg_config --modversion "$req"`
          if $pkg_config --exists "$req >= $min" ; then
            AC_MSG_RESULT($vers)
            pkgs="$pkgs $req"
            return 1
          else
            AC_MSG_RESULT($vers (wanted >= $min))
            ok=no
            return 0
          fi
        else
          AC_MSG_RESULT(no)
          ok=no
          return 0
        fi
      fi
    }
   
  AC_ARG_WITH(gtk2, [  --without-gtk2          disable GTK2 (no xglock build)],)
  if test "x$with_gtk2" != "xno" ; then
    #AC_MSG_RESULT(checking for GTK 2.x with pkg-config based tests...)
    ok="yes"
    pkg_check_version       gtk+-2.0  2.0.1  ; ac_gtk_version_string="$vers"
    have_gtk="$ok"
  
    if test "$have_gtk" = yes; then
       XGLOCK="xglock"
       INSTALL_XGLOCK="install_xglock"
       UNINSTALL_XGLOCK="uninstall_xglock"
       XGLOCKINC=`$pkg_config --cflags gtk+-x11-2.0`
       XGLOCKLDFLAGS=`$pkg_config --libs gtk+-x11-2.0`" ${LDFLAGS}"
       with_gtk="no"
    fi
  fi
fi

AC_ARG_WITH(gtk, [  --without-gtk           disable GTK (no xglock build)],)
if test "x$with_gtk" != "xno" ; then
   AC_PATH_GTK(1.2.0,
               have_gtk="yes",
               have_gtk="no")
   if test "$have_gtk" = yes; then
     XGLOCK="xglock"
     INSTALL_XGLOCK="install_xglock"
     UNINSTALL_XGLOCK="uninstall_xglock"
   fi
fi

dnl Should have a tester for XHPUX Xhp

dnl Now we handle the various system dependent problems
dnl that are not addressed in the X header files, or things dealing
dnl with utility quirks on some systems.

aixv3=no
dirent=yes
FORTUNE="#"
FORTUNE_PATH="/usr/bin/fortune"
if test -x $FORTUNE_PATH; then
  FORTUNE=""
else
  FORTUNE_PATH="/usr/games/fortune"
  if test -x $FORTUNE_PATH; then
    FORTUNE=""
  fi
fi

case "${canonical}" in

  *-*-aix* )
    BITMAPTYPE="ibm"
    PIXMAPTYPE="ibm"
    AC_DEFINE([AIXV3], [1], [AIX Version 3 OS])
    aixv3=yes
    case "${canonical}" in
      *-*-aix2* | *-*-aix3.0* | *-*-aix3.1* )
        AC_DEFINE([LESS_THAN_AIX3_2], [1], [less than AIX Version 3.2])
      ;;
    esac
  ;;

  *-*-freebsd* | *-*-openbsd* | *-*-netbsd* | *-*-dragonfly* )
    BITMAPTYPE="bsd"
    PIXMAPTYPE="bsd"
    FORTUNE=""
    INSTPGMFLAGS="-s -o root -m 4111"
    cpp_name=`echo $CC | cut -f1 -d' '`
    if test "$cpp_name" = "gcc"; then
        dirname=`which gcc`
        dirname=`dirname $dirname`
        dirname=`dirname $dirname`/lib
	if test "$dirname" != "/lib"; then
  	    XLOCK_LDFLAGS="${XLOCK_LDFLAGS}:$dirname"
  	    XMLOCK_LDFLAGS="${XMLOCK_LDFLAGS}:$dirname"
	fi
    fi
    case "${canonical}" in
      *-*-freebsd* )
        FORTUNE_PATH="/usr/bin/fortune"
      ;;
    esac
    XLOCKLDFLAGS="-Wl,-R${XLOCK_LDFLAGS}"
    XMLOCKLDFLAGS="-Wl,-R${XMLOCK_LDFLAGS}"
  ;;

  *-*-dgux5* )
    BITMAPTYPE="x11"
    PIXMAPTYPE="x11"
  ;;

  *-*-hpux* )
    if test "${CC}" = "cc" ; then
dnl      CFLAGS="${CFLAGS} -O -Aa -z -D_HPUX_SOURCE" # -O sometimes problems
      CFLAGS="${CFLAGS} -Aa -z -D_HPUX_SOURCE"
      GPROF_CFLAGS="${GPROF_CFLAGS} -Aa -z -D_HPUX_SOURCE"
    fi
dnl    LDFLAGS=-Wl,-B,immediate
    XLOCKLIBS="-lXhp11 ${XLOCKLIBS}"
    AC_DEFINE([SYSV], [1], [System 5])
    AC_DEFINE([SVR4], [1], [System 5 Release 4])
    BITMAPTYPE="hp"
    PIXMAPTYPE="hp"
  ;;

  *-*-irix5* | *-*-irix6* )
    if test "${CC}" = "cc" ; then
      CFLAGS="${CFLAGS} -fullwarn"
    fi
    AC_DEFINE_UNQUOTED([glGenTextures], ["glGenTexturesEXT"], [GL Generic Textures])
    AC_DEFINE_UNQUOTED([glDeleteTextures], ["glDeleteTexturesEXT"], [GL Delete Textures])
    AC_DEFINE_UNQUOTED([glBindTextures], ["glBindTexturesEXT"], [GL Bind Textures])
    BITMAPTYPE="sgi"
    PIXMAPTYPE="sgi"
  ;;

  *-*-linux* )
    AC_CHECK_HEADER(elf.h, AC_DEFINE([HAVE_SHADOW], [1], [use Shadow passwording]))
dnl    CFLAGS="${CFLAGS} -ansi -pedantic"
    AC_DEFINE([linux], [1], [Linux OS])
dnl    AC_DEFINE([__i386__], [1], [Intel 386 machine])
    AC_DEFINE([_POSIX_SOURCE], [1], [POSIX source])
    AC_DEFINE([_BSD_SOURCE], [1], [BSD source])
    AC_DEFINE([_DEFAULT_SOURCE], [1], [DEFAULT source])
    AC_DEFINE([_GNU_SOURCE], [1], [GNU source])
dnl    AC_DEFINE([DX_LOCALE], [1], [DX Locale])
    if test -e /etc/shadow ; then
      INSTPGMFLAGS="-g shadow -m 2111"
      case `ls -l /etc/shadow` in
        -???r?????\ *\ shadow\ *\ /etc/shadow)  # group shadow can read it
          INSTPGMFLAGS="-g shadow -m 2111"
          ;;
        *)
          INSTPGMFLAGS="-o root -m 4111"
          ;;
      esac
    fi
    INSTPGMFLAGS="-s $INSTPGMFLAGS"
    BITMAPTYPE="linux"
    PIXMAPTYPE="linux"
dnl if Shadow and not elf
dnl XLOCKLIBS="${XLOCKLIBS} -lgdbm"
    XLOCKLDFLAGS="-Wl,-R${XLOCK_LDFLAGS}"
    XMLOCKLDFLAGS="-Wl,-R${XMLOCK_LDFLAGS}"
  ;;

  *-*-cygwin* )
    BITMAPTYPE="cygwin"
    PIXMAPTYPE="cygwin"
  ;;

  *-*-osf* )
    BITMAPTYPE="dec"
    PIXMAPTYPE="dec"
    AC_MSG_CHECKING([for Digital/Tru64 Unix Enhanced Security])
    # This section should check for the presence of the newer Enhanced
    # security routines, and use them if they're available.
    if test -f /usr/sbin/rcmgr && test -f /etc/rc.config ; then
      if test X`/usr/sbin/rcmgr get SECURITY` = XENHANCED ; then
        AC_MSG_RESULT(yes)
        AC_DEFINE([OSF1_ENH_SEC], [1], [OSF1 Enhanced Security])
        AC_SUBST(OSF1_ENH_SEC)
        XLOCKLIBS="${XLOCKLIBS} -lsecurity"
        # SETGID auth may not be enough for Tru64 5.x.
        INSTPGMFLAGS="-s -g auth -m 2111"
      else
        AC_MSG_RESULT(no)
      fi
    fi
  ;;

  *-*-sco* )
    BITMAPTYPE="sco"
    PIXMAPTYPE="sco"
    AC_DEFINE([HAVE_SHADOW], [1], [Shadow passwording])
    AC_DEFINE([SYSV], [1], [System 5])
  ;;

  *-*-solaris2* )
    AC_DEFINE([SOLARIS2], [1], [Solaris2 OS])
    PIXMAPTYPE="solaris"
    case "${canonical}" in
      *-*-solaris2.0* | *-*-solaris2.1* | *-*-solaris2.2* | *-*-solaris2.3* | *-*-solaris2.4* )
        AC_DEFINE([LESS_THAN_SOLARIS2_5], [1], [Less than Solaris 2.5 OS])
        PIXMAPTYPE="sol"
      ;;
      *-*-solaris2.5* | *-*-solaris2.6*)
        PIXMAPTYPE="sol"
      ;;
    esac
    INSTPGMFLAGS="-s -o root -m 4111"
    AC_DEFINE([HAVE_SHADOW], [1], [Shadow passwording])
    AC_DEFINE([SYSV], [1], [System 5])
    AC_DEFINE([SVR4], [1], [System 5 Release 4])
    BITMAPTYPE="sun"
dnl    XLOCKLIBS="${XLOCKLIBS} -lsocket -lnsl -lposix4"
    XLOCKLIBS="${XLOCKLIBS} -lposix4"
    LIBS="${LIBS} -lsocket -lnsl -lposix4"
    cpp_name=`echo $CC | cut -f1 -d' '`
    if test "$cpp_name" = "gcc"; then
        dirname=`which gcc`
        dirname=`dirname $dirname`
        dirname=`dirname $dirname`/lib
	if test "$dirname" != "/lib"; then
  	    XLOCK_LDFLAGS="${XLOCK_LDFLAGS}:$dirname"
  	    XMLOCK_LDFLAGS="${XMLOCK_LDFLAGS}:$dirname"
	fi
    fi
    XLOCKLDFLAGS="-R${XLOCK_LDFLAGS}"
    XMLOCKLDFLAGS="-R${XMLOCK_LDFLAGS}"
  ;;

  *-*-sunos4* )
    AC_DEFINE([SUNOS4], [1], [SunOS4 OS])
    BITMAPTYPE="sun"
    PIXMAPTYPE="sun"
  ;;

  *-*-sysv5* )
    INSTPGMFLAGS="-s -o root -m 4111"
    AC_DEFINE([HAVE_SHADOW], [1], [Shadow passwording])
    AC_DEFINE([SYSV], [1], [System 5])
    AC_DEFINE([SVR4], [1], [System 5 Release 4])
    XLOCKLIBS="${XLOCKLIBS} -lgen"
    XMLOCKLIBS="${XMLOCKLIBS} -lgen"
    LIBS="${LIBS} -lsocket -lnsl"
    BITMAPTYPE="x11"
    PIXMAPTYPE="x11"
  ;;

  *)
    BITMAPTYPE="x11"
    PIXMAPTYPE="x11"
  ;;
esac

if test "$dirent" = yes; then
  AC_HEADER_DIRENT
fi

dnl Rather have a usleep
AC_CHECK_FUNC(usleep, AC_DEFINE([HAVE_USLEEP], [1], [microsecond sleep is available]))
if test $ac_cv_func_usleep = no; then
AC_CHECK_FUNC(nanosleep, AC_DEFINE([HAVE_NANOSLEEP], [1], [nanosecond sleep is available]))
fi

dnl AC_CHECK_FUNCS(gettimeofday)
AC_MSG_CHECKING(how to call gettimeofday)
AC_CACHE_VAL(ac_cv_gettimeofday_args,
 [AC_TRY_COMPILE([#include <stdlib.h>
#include <sys/time.h>],
     [struct timeval tv; struct timezone tzp;
      gettimeofday(&tv, &tzp);],
     [ac_gettimeofday_args=2],
     [AC_TRY_COMPILE([#include <stdlib.h>
#include <sys/time.h>],
         [struct timeval tv; gettimeofday(&tv);],
         [ac_gettimeofday_args=1],
         [ac_gettimeofday_args=0])])
  ac_cv_gettimeofday_args=$ac_gettimeofday_args])
ac_gettimeofday_args=$ac_cv_gettimeofday_args
if test $ac_gettimeofday_args = 2 ; then
  AC_DEFINE([HAVE_GETTIMEOFDAY], [1], [get time of day])
  AC_DEFINE([GETTIMEOFDAY_TWO_ARGS], [1], [get time of day is 2 arguments])
  AC_MSG_RESULT(two arguments)
elif test $ac_gettimeofday_args = 1 ; then
  AC_DEFINE([HAVE_GETTIMEOFDAY], [1], [get time of day])
  AC_MSG_RESULT(one argument)
else
  AC_MSG_RESULT(unknown)
fi

AC_CHECK_FUNCS(tzset timelocal)

AC_C_INLINE

srand=no
AC_CHECK_FUNC(srand48, 
  [AC_DEFINE([HAVE_RAND48], [1], [Random Number Generator using 48 bits is available])
   AC_MSG_CHECKING([for srand48 declaration])
   AC_EGREP_HEADER(srand48, stdlib.h,
     AC_DEFINE([DECLARED_SRAND48], [1], [Declared SRAND48 is available]) AC_MSG_RESULT(yes),
     AC_MSG_RESULT(no))])
  if test $ac_cv_func_srand48 = no; then
    AC_CHECK_FUNC(srandom, AC_DEFINE([HAVE_RANDOM], [1], [Random Number Generator using 32 bits is available]))
    if  test $ac_cv_func_srandom = no; then
       AC_CHECK_FUNC(random, AC_DEFINE([HAVE_RAND], [1], [Low quality Random Number Generator using 32 bits is available]) srand=yes)
    fi
  fi
if test "$srand" = yes; then
  if test "$aixv3" = yes; then
    AC_DEFINE([MAXRAND], [2147483648.0], [Maximum random number])
  else
    AC_DEFINE([MAXRAND], [32768.0], [Maximum random number])
  fi
fi

AC_MSG_CHECKING([matherr support])
AC_CACHE_VAL(ac_cv_c_matherr,[
AC_TRY_COMPILE([#define _XOPEN_SOURCE 1
#include <math.h>], [
struct exception x;
x.type = DOMAIN;
x.type = SING;
], ac_cv_c_matherr=yes, ac_cv_c_matherr=no)])
if test x"$ac_cv_c_matherr" = xyes; then
    AC_DEFINE([USE_MATHERR], [1], [math error])
    AC_MSG_RESULT(yes)
else
    AC_MSG_RESULT(no)
fi

dnl Problems here if -I/usr/include used with gcc on Solaris
AC_MSG_CHECKING([struct sigset_t])
AC_CACHE_VAL(ac_cv_c_sigset_t,[
AC_TRY_COMPILE([/*#define _XOPEN_SOURCE 1*/
#include <signal.h>], [
typedef struct {unsigned long __sigbits[4];} sigset_t;
sigset_t sigmask;
], ac_cv_c_sigset_t=yes, ac_cv_c_sigset_t=no)])
if test x"$ac_cv_c_sigset_t" = xyes; then
  case "${canonical}" in
    *-*-solaris2.0* | *-*-solaris2.1* | *-*-solaris2.2* | *-*-solaris2.3* | *-*-solaris2.4* | *-*-solaris2.5*)
        AC_MSG_RESULT(yes)
        AC_DEFINE([HAVE_STRUCT_SIGSET_T], [1], [Struct Sigset Type])
      ;;
    *)
      AC_MSG_RESULT(no)
      ;;
  esac
else
  AC_MSG_RESULT(no)
fi

dnl Problems here if -I/usr/include/stddef.h used with g++ on Solaris
AC_LANG([C++])
ac_save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS -I/usr/include"
AC_MSG_CHECKING([whether wchar_t is defined internal to C++])
AC_CACHE_VAL(ac_cv_c_wchar_t,[
AC_TRY_COMPILE([
#include <stddef.h>], [
], ac_cv_c_wchar_t=no, ac_cv_c_wchar_t=yes)])
if test x"$ac_cv_c_wchar_t" = xyes; then
  AC_DEFINE([_WCHAR_T], [1], [wide character type defined])
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi
CPPFLAGS="$ac_save_CPPFLAGS"
AC_LANG([C])

AC_ARG_ENABLE(bitmapdir, [  --enable-bitmapdir=DIR  set directory for bitmaps, default is ./bitmaps],bitmap_dir=$enableval,bitmap_dir=no)
case "x$bitmap_dir" in
x/*|x.*)
  AC_MSG_RESULT([setting BITMAPDIR = $enableval])
  BITMAPDIR=$enableval
  if test ! -d $bitmap_dir; then
    AC_MSG_RESULT([Warning: Directory $enableval does not exist])
  fi
  ;;
*)
  BITMAPDIR='$(top_srcdir)/bitmaps'
  bitmap_dir='$top_srcdir/bitmaps'
  ;;
esac

AC_ARG_ENABLE(bitmaptype, [  --enable-bitmaptype=name
                          set name for bitmap type],bitmap_type=$enableval,bitmap_type=no)
case "x$bitmap_type" in
xyes*|xno*)
  ;;
*)
  AC_MSG_RESULT([setting BITMAPTYPE = $enableval])
  BITMAPTYPE=$enableval
  if test ! -f $bitmap_dir/l-$BITMAPTYPE.xbm; then
    AC_MSG_RESULT([Warning: Bitmap $BITMAPDIR/l-$BITMAPTYPE.xbm does not exist])
  fi
  ;;
esac

AC_ARG_ENABLE(pixmapdir, [  --enable-pixmapdir=DIR  set directory for pixmaps, default is ./pixmaps],pixmap_dir=$enableval,pixmap_dir=no)
case "x$pixmap_dir" in
x/*|x.*)
  AC_MSG_RESULT([setting PIXMAPDIR = $enableval])
  PIXMAPDIR=$enableval
  if test ! -d $pixmap_dir; then
    AC_MSG_RESULT([Warning: Directory $enableval does not exist])
  fi
  ;;
*)
  PIXMAPDIR='$(top_srcdir)/pixmaps'
  pixmap_dir='$top_srcdir/pixmaps'
  ;;
esac

AC_ARG_ENABLE(pixmaptype, [  --enable-pixmaptype=name
                          set name for pixmap type],pixmap_type=$enableval,pixmap_type=no)
case "x$pixmap_type" in
xyes*|xno*)
  ;;
*)
  AC_MSG_RESULT([setting PIXMAPTYPE = $enableval])
  PIXMAPTYPE=$enableval
  if test ! -f $pixmap_dir/m-$PIXMAPTYPE.xpm; then
    AC_MSG_RESULT([Warning: Pixmap $PIXMAPDIR/m-$PIXMAPTYPE.xpm does not exist])
  fi
  ;;
esac

AC_ARG_ENABLE(mapdir, [  --enable-mapdir=DIR     set directory for bitmaps and pixmaps],map_dir=$enableval,map_dir=no)
case "x$map_dir" in
x/*|x.*)
  AC_MSG_RESULT([setting BITMAPDIR = $enableval])
  BITMAPDIR=$enableval
  AC_MSG_RESULT([setting PIXMAPDIR = $enableval])
  PIXMAPDIR=$enableval
  if test ! -d $map_dir; then
    AC_MSG_RESULT([Warning: Directory $enableval does not exist])
  fi
  ;;
*)
  BITMAPDIR='$(top_srcdir)/bitmaps'
  bitmap_dir='$top_srcdir/bitmaps'
  PIXMAPDIR='$(top_srcdir)/pixmaps'
  pixmap_dir='$top_srcdir/pixmaps'
  ;;
esac

AC_ARG_ENABLE(maptype, [  --enable-maptype=name   set name for bitmap and pixmap type],map_type=$enableval,map_type=no)
case "x$map_type" in
xyes*|xno*)
  ;;
*)
  AC_MSG_RESULT([setting BITMAPTYPE = $enableval])
  BITMAPTYPE=$enableval
  if test ! -f $map_dir/l-$BITMAPTYPE.xbm; then
    AC_MSG_RESULT([Warning: Bitmap $BITMAPDIR/l-$BITMAPTYPE.xbm does not exist])
  fi
  AC_MSG_RESULT([setting PIXMAPTYPE = $enableval])
  PIXMAPTYPE=$enableval
  if test ! -f $map_dir/m-$PIXMAPTYPE.xpm; then
    AC_MSG_RESULT([Warning: Pixmap $PIXMAPDIR/m-$PIXMAPTYPE.xpm does not exist])
  fi
  ;;
esac

AC_ARG_ENABLE(vroot, [  --disable-vroot         disables xlock from being able to run in root window
                          (some window managers have problems)],use_vroot=$enableval,use_vroot=yes)
if test "$use_vroot" = yes; then
  AC_MSG_RESULT([defining option USE_VROOT])
  AC_DEFINE([USE_VROOT], [1], [virtual root window])
fi

case ${canonical} in
      *-*-cygwin* )
        ;;
      * )
        AC_ARG_ENABLE(allow-root, [  --disable-allow-root    allows users to turn off allowroot (default is to
                          always allow root to be able to unlock xlock)],allow_root=$enableval,allow_root=yes)
        if test "$allow_root" = yes; then
          AC_MSG_RESULT([defining option ALWAYS_ALLOW_ROOT])
          AC_DEFINE([ALWAYS_ALLOW_ROOT], [1], [root is always allowed])
        fi
        ;;
esac

case ${canonical} in
    *-*-linux* | *-*-freebsd* | *-*-openbsd* | *-*-netbsd* | *-*-dragonfly* )
      AC_ARG_ENABLE(vtlock, [  --enable-vtlock         allows to turn on VT switch lock (default is to be
                          able to switch to another VT)],vtlock=$enableval,vtlock=no)
      if test "$vtlock" = yes; then
        AC_MSG_RESULT([defining option USE_VTLOCK])
        AC_DEFINE([USE_VTLOCK], [1], [VT lock])
      fi
      ;;
    *)
      ;;
esac

AC_ARG_ENABLE(syslog, [  --enable-syslog         enable syslog logging],use_syslog=$enableval,use_syslog=no)
if test "$use_syslog" = yes; then
  AC_MSG_RESULT([defining option USE_SYSLOG])
  AC_DEFINE([USE_SYSLOG], [1], [using syslog])
fi

AC_ARG_ENABLE(multiple-user, [  --enable-multiple-user  enable multiple users],use_multiple_user=$enableval,use_multiple_user=no)
if test "$use_multiple_user" = yes; then
  AC_MSG_RESULT([defining option USE_MULTIPLE_USER])
  AC_DEFINE([USE_MULTIPLE_USER], [1], [multiple user])
fi

AC_ARG_ENABLE(multiple-root, [  --enable-multiple-root  enable multiple root users],use_multiple_root=$enableval,use_multiple_root=no)
if test "$use_multiple_root" = yes; then
  AC_MSG_RESULT([defining option USE_MULTIPLE_ROOT])
  AC_DEFINE([USE_MULTIPLE_ROOT], [1], [multiple root users])
fi

AC_ARG_ENABLE(xlock-group, [  --enable-xlock-group    enable all in xlock group to unlock],use_xlock_group=$enableval,use_xlock_group=no)
if test "$use_xlock_group" = yes; then
  AC_MSG_RESULT([defining option USE_XLOCK_GROUP])
  AC_DEFINE([USE_XLOCK_GROUP], [1], [xlock group])
fi

AC_ARG_ENABLE(window-visibility, [  --enable-window-visibility
                          allow a window to be displayed over xlock],use_window_visiblity=$enableval,use_window_visibility=no)
if test "$use_window_visibility" = yes; then
  AC_MSG_RESULT([defining option USE_WINDOW_VISIBILITY])
  AC_DEFINE([USE_WINDOW_VISIBLITY], [1], [window visibility])
fi

AC_ARG_ENABLE(old-event-loop, [  --enable-old-event-loop some machines may still need this
                          (fd_set errors may be a sign)],use_old_event_loop=$enableval,use_old_event_loop=no)
if test "$use_old_event_loop" = yes; then
  AC_MSG_RESULT([defining option USE_OLD_EVENT_LOOP])
  AC_DEFINE([USE_OLD_EVENT_LOOP], [1], [old event loop])
fi

dnl AC_ARG_ENABLE(vmsutils, [  --enable-vmsutils       patches up old __VMS_VER < 70000000],use_vmsutils=$enableval,use_vmsutils=no)
dnl if test "$use_vmsutils" = yes; then
dnl   AC_MSG_RESULT([defining option USE_VMSUTILS])
dnl   AC_DEFINE([USE_VMSUTILS], [1], [VMS utilities])
dnl fi

AC_ARG_ENABLE(xlockrc, [  --enable-xlockrc        for unknown shadow passwd algorithm],use_xlockrc=$enableval,use_xlockrc=maybe)
if test "$use_xlockrc" = yes; then
  AC_MSG_RESULT([defining option USE_XLOCKRC])
  AC_DEFINE([USE_XLOCKRC], [1], [encrypted password in .xlockrc file])
else
  who_am_i=`whoami`
  if ! test -f /etc/passwd || cat /etc/passwd | grep "unused" | grep $who_am_i > /dev/null 2>&1
  then
    case "${canonical}" in
      *-*-cygwin* )
        if test "$use_xlockrc" = maybe; then
          AC_MSG_RESULT([*** No password in /etc/passwd, enabling xlockrc])
          AC_MSG_RESULT([defining option USE_XLOCKRC])
          AC_DEFINE([USE_XLOCKRC], [1], [encrypted password in .xlockrc file])
        else
          AC_MSG_RESULT([*** Use --enable-xlockrc or write an encrypted password])
          AC_MSG_RESULT([***             to /etc/passwd replacing "unused" for $who_am_i])
        fi
      ;;
      esac
  fi
fi

AC_ARG_ENABLE(auto-logout, [  --enable-auto-logout=time
                          enable auto-logout code for lab use (time in minutes)],use_auto_logout=$enableval,use_auto_logout=no)
case "x$use_auto_logout" in
x|xyes*)
  AC_MSG_RESULT([defining option USE_AUTO_LOGOUT = 240 minutes])
  AC_DEFINE([USE_AUTO_LOGOUT], [240], [auto logout in minutes])
  ;;
x[[0-9]]*)
  AC_MSG_RESULT([defining option USE_AUTO_LOGOUT = $enableval minutes])
  AC_DEFINE_UNQUOTED([USE_AUTO_LOGOUT], [$enableval], [auto logout in minutes])
  ;;
xno*)
  ;;
*)
  AC_MSG_RESULT([Warning: Illegal time value "$use_auto_logout" given])
  AC_MSG_RESULT([defining option USE_AUTO_LOGOUT = 240 minutes])
  AC_DEFINE([USE_AUTO_LOGOUT], [240], [auto logout in minutes])
  ;;
esac

AC_ARG_ENABLE(default-auto-logout, [  --enable-default-auto-logout=time
                          set default-auto-logout (time in minutes)],
              def_auto_logout=$enableval,
              def_auto_logout=no)
case "x$def_auto_logout" in
x|xyes*)
  AC_MSG_RESULT([defining option DEF_AUTO_LOGOUT = 120 minutes])
  AC_DEFINE([DEF_AUTO_LOGOUT], ["120"], [default auto logout in minutes])
  ;;
x[[0-9]]*)
  AC_MSG_RESULT([defining option DEF_AUTO_LOGOUT = $enableval minutes])
  AC_DEFINE_UNQUOTED([DEF_AUTO_LOGOUT], ["$enableval"], [default auto logout in minutes])
  ;;
xno*)
  ;;
*)
  AC_MSG_RESULT([Warning: Illegal time value "$def_auto_logout" given])
  AC_MSG_RESULT([defining option DEF_AUTO_LOGOUT = 120 minutes])
  AC_DEFINE([DEF_AUTO_LOGOUT], ["120"], [default auto logout in minutes])
  ;;
esac

AC_ARG_ENABLE(button-logout, [  --enable-button-logout=time
                          enable button logout for lab use (time in minutes)],
              use_button_logout=$enableval,
              use_button_logout=no)
case "x$use_button_logout" in
x|xyes*)
  AC_MSG_RESULT([defining option USE_BUTTON_LOGOUT = 10 minutes])
  AC_DEFINE([USE_BUTTON_LOGOUT], [10], [button logout in minutes])
  ;;
x[[0-9]]*|x-[0-9]*)
  AC_MSG_RESULT([defining option USE_BUTTON_LOGOUT = $enableval minutes])
  AC_DEFINE_UNQUOTED([USE_BUTTON_LOGOUT], $enableval, [button logout in minutes])
  ;;
xno*)
  ;;
*)
  AC_MSG_RESULT([Warning: Illegal time value "$use_button_logout" given])
  AC_MSG_RESULT([defining option USE_BUTTON_LOGOUT = 10 minutes])
  AC_DEFINE([USE_BUTTON_LOGOUT], [10], [button logout in minutes])
  ;;
esac

AC_ARG_ENABLE(default-button-logout, [  --enable-default-button-logout=time
                          set default-button-logout (time in minutes)],
              def_button_logout=$enableval,
              def_button_logout=no)
case "x$def_button_logout" in
x|xyes*)
  AC_MSG_RESULT([defining option DEF_BUTTON_LOGOUT = 5 minutes])
  AC_DEFINE([DEF_BUTTON_LOGOUT], ["5"], [default button logout in minutes])
  ;;
x[[0-9]]*)
  AC_MSG_RESULT([defining option DEF_BUTTON_LOGOUT = $enableval minutes])
  AC_DEFINE_UNQUOTED([DEF_BUTTON_LOGOUT], ["$enableval"], [default button logout in minutes])
  ;;
xno*)
  ;;
*)
  AC_MSG_RESULT([Warning: Illegal time value "$def_button_logout" given])
  AC_MSG_RESULT([defining option DEF_BUTTON_LOGOUT = 5 minutes])
  AC_DEFINE([DEF_BUTTON_LOGOUT], ["5"], [default button logout in minutes])
  ;;
esac

dnl Maybe soon
dnl AC_ARG_ENABLE(button-up, [  --enable-button-up      force button password screen to stay up],
dnl              use_button_up=$enableval,
dnl               use_button_up=no)
dnl if test "$use_button_up" = yes; then
dnl   AC_MSG_RESULT([defining option USE_BUTTON_UP])
dnl   AC_DEFINE([USE_BUTTON_UP], [1], [Button password])
dnl fi

AC_ARG_ENABLE(bomb, [  --disable-bomb          disable automatic logout mode],
              use_bomb=$enableval,
              use_bomb=yes)
if test "$use_bomb" = yes; then
  AC_MSG_RESULT([defining option USE_BOMB])
  AC_DEFINE([USE_BOMB], [1], [special bomb mode, causes logout])
  BOMB=""
else
  BOMB="#"
fi

AC_ARG_ENABLE(interactive, [  --disable-interactive   disables interactive modes (useful for prod env)],
              no_interactive=$enableval,
              no_interactive=yes)
if test "$no_interactive" = no; then
  AC_MSG_RESULT([defining option DISABLE_INTERACTIVE])
  AC_DEFINE([DISABLE_INTERACTIVE], [1], [disable interactive modes])
  INTERACTIVE=""
else
  INTERACTIVE="#"
fi

AC_ARG_ENABLE(unstable, [  --enable-unstable       enables unstable (alpha) modes],
              use_unstable=$enableval,
              use_unstable=no)
if test "$use_unstable" = yes; then
  AC_MSG_RESULT([defining option USE_UNSTABLE])
  AC_DEFINE([USE_UNSTABLE], [1], [use unstable modes])
  UNSTABLE=""
else
  UNSTABLE="#"
fi

AC_ARG_ENABLE(negative-logout, [  --disable-negative-logout
                          allows users to deny logout button and autologout],
              negative-logout=$enableval,
              negative_logout=yes)
if test "$negative_logout" = no; then
  AC_MSG_RESULT([defining option NO_NEGATIVE_LOGOUT])
  AC_DEFINE([NO_NEGATIVE_LOGOUT], [1], [deny logout button and autologout])
fi

AC_ARG_ENABLE(closedown-logout, [  --enable-closedown-logout
                          define this one or next, with enable-auto-logout],
              closedown_logout=$enableval,
              closedown_logout=no)
if test "$closedown_logout" = yes; then
  AC_MSG_RESULT([defining option CLOSEDOWN_LOGOUT])
  AC_DEFINE([CLOSEDOWN_LOGOUT], [1], [allow a user to close down session for lab use])
fi

AC_ARG_ENABLE(session-logout, [  --enable-session-logout enable-button-logout, and/or enable-bomb, for xdm],
              session_logout=$enableval,
              session_logout=no)
if test "$session_logout" = yes; then
  AC_MSG_RESULT([defining option SESSION_LOGOUT])
  AC_DEFINE([SESSION_LOGOUT], [1], [allow a user to kill an X session for lab use])
fi

AC_ARG_ENABLE(staff-file, [  --enable-staff-file=filename
                          set file of staff who are exempt from auto-logout],
              use_staff_file=$enableval,
              use_staff_file=no)
case "x$use_staff_file" in
x|xyes*)
  AC_MSG_RESULT([defining option STAFF_FILE = "/usr/remote/etc/xlock.staff"])
  AC_DEFINE_UNQUOTED([STAFF_FILE], ["/usr/remote/etc/xlock.staff"], [file for staff exempt from auto-logout])
  if test ! -f /usr/remote/etc/xlock.staff; then
    AC_MSG_RESULT([Warning: File /usr/remote/etc/xlock.staff does not exist])
  fi
  ;;
x/*)
  AC_MSG_RESULT([defining option STAFF_FILE = "$enableval"])
  AC_DEFINE_UNQUOTED([STAFF_FILE], ["$enableval"], [file for staff exempt from auto-logout])
  if test ! -f $use_staff_file; then
    AC_MSG_RESULT([Warning: File $enableval does not exist])
  fi
  ;;
esac

AC_ARG_ENABLE(staff-netgroup, [  --enable-staff-netgroup=netgroup
                          set netgroup of staff who are exempt from auto-logout],
              use_staff_netgroup=$enableval,
              use_staff_netgroup=no)
if test "$use_staff_netgroup" != no; then
  AC_MSG_RESULT([defining option STAFF_NETGROUP = "$enableval"])
  AC_DEFINE_UNQUOTED([STAFF_NETGROUP], ["$enableval"], [netgroup of staff exempt from auto-logout])
fi

AC_ARG_ENABLE(kerberos4, [  --enable-kerberos4      enable Kerberos 4],
              kerberos4=$enableval,
              kerberos4=no)
if test "$kerberos4" = yes; then
  AC_MSG_RESULT([defining option HAVE_KRB4])
  AC_DEFINE([HAVE_KRB4], [1], [Kerberos 4 available])
  XLOCKINC="${XLOCKINC} -I/usr/athena/include"
  XLOCKLIBS="${XLOCKLIBS} -L/usr/athena/lib -lkrb -ldes"
fi

AC_ARG_ENABLE(kerberos5, [  --enable-kerberos5      enable Kerberos 5],
              kerberos5=$enableval,
              kerberos5=no)
if test "$kerberos5" = yes; then
  AC_MSG_RESULT([defining option HAVE_KRB5])
  AC_DEFINE([HAVE_KRB5], [1], [Kerberos 5 available])
fi

AC_ARG_WITH(krb5-config, [  --with-krb5-config=PATH Location of krb5-config script],[
  if test $withval = yes; then
    AC_PATH_PROG(krb5_config, krb5-config, not_found)
    if test X$krb5_config = Xnot_found; then
      AC_MSG_ERROR(unable to find krb5-config)
    fi
  else
    krb5_config=$withval
    if test ! -x $krb5_config; then
      AC_MSG_ERROR([unable to find krb5-config at $krb5_config])
    fi
  fi
  krb5_cflags=`$krb5_config --cflags krb5`
  retval=$?
  if test $retval -ne 0; then
    AC_MSG_ERROR($krb5_config failed with error code of $retval)
  fi
  krb5_libs=`$krb5_config --libs krb5`
  retval=$?
  if test $retval -ne 0; then
    AC_MSG_ERROR($krb5_config failed with error code of $retval)
  fi
  AC_MSG_RESULT(Adding $krb5_cflags to XLOCKINC)
  AC_MSG_RESULT(Adding $krb5_libs to XLOCKLIBS)
  XLOCKINC="$XLOCKINC $krb5_cflags"
  XLOCKLIBS="$XLOCKLIBS $krb5_libs"
])dnl

AC_ARG_ENABLE(write_krb5, [  --enable-write-krb5     write new krb5 credentials to credential cache],
              write_krb5=$enableval,
              write_krb5=no)
if test "$write_krb5" = yes; then
  AC_MSG_RESULT(Will write out krb5 credentials to credential cache)
  AC_DEFINE([KRB5_WRITE_NEW_CCACHE], [1], [Kerberos 5 write new credential cache])
fi

AC_ARG_ENABLE(dce_passwd, [  --enable-dce-passwd     enable DCE passwording],
              dce_passwd=$enableval,
              dce_passwd=no)
if test "$dce_passwd" = yes; then
  AC_MSG_RESULT([defining option DCE_PASSWD])
  AC_DEFINE([DCE_PASSWD], [1], [use DCE passwording])
  XLOCKINC="${XLOCKINC} -I/usr/include/reentrant"
  XLOCKLIBS="${XLOCKLIBS} -ldce -lc_r"
fi

AC_ARG_ENABLE(pam, [  --enable-pam            enable PAM],
              pam=$enableval,
              pam=no)
if test "$pam" = yes; then
  if test "$use_xlockrc" = yes; then
    AC_MSG_RESULT([not defining option USE_PAM (mutually exclusive with xlockrc)])
  else
    AC_MSG_RESULT([defining option USE_PAM])
    AC_DEFINE([USE_PAM], [1], [using Pluggable Authentication Module])
    AC_CHECK_FUNC(dlsym, [ XLOCKLIBS="${XLOCKLIBS} -lpam" ], [ XLOCKLIBS="${XLOCKLIBS} -lpam -ldl"])
  fi
fi

AC_ARG_ENABLE(bad_pam, [  --enable-bad-pam        enable BAD_PAM],
              bad_pam=$enableval,
              bad_pam=no)
if test "$bad_pam" = yes; then
  if test "$pam" = no; then
    AC_MSG_RESULT([not defining option BAD_PAM (need PAM)])
  else
    AC_MSG_RESULT([defining option BAD_PAM])
    AC_DEFINE([BAD_PAM], [1], [using deprecated PAM])
  fi
fi

AC_ARG_ENABLE(good_pam, [  --enable-good-pam       enable GOOD_PAM],
              good_pam=$enableval,
              good_pam=no)
if test "$good_pam" = yes; then
  if test "$pam" = no; then
    AC_MSG_RESULT([not defining option GOOD_PAM (need PAM)])
  else
    AC_MSG_RESULT([defining option GOOD_PAM])
    AC_DEFINE([GOOD_PAM], [1], [using another version of PAM])
  fi
fi

AC_ARG_ENABLE(afs, [  --enable-afs            enable AFS],
              afs=$enableval,
              afs=no)
if test "$afs" = yes; then
  AC_MSG_RESULT([defining option AFS])
  AC_DEFINE([AFS], [1], [Andrew File System, a distributed file system])
  XLOCKINC="${XLOCKINC} -I/usr/afsws/include"
  case "${canonical}" in
    *-*-solaris2.* )
      XLOCKLIBS="${XLOCKLIBS} -lucb -L/usr/afsws/lib -L/usr/afsws/lib/afs -lkauth.krb -lprot -lubik -lauth.krb -lrxkad -lsys -ldes -lrx -llwp -lcom_err -lcmd /usr/afsws/lib/afs/util.a -laudit -lsys"
    ;;
    *-*-osf* )
      XLOCKLIBS="${XLOCKLIBS} -L/usr/afsws/lib -L/usr/afsws/lib/afs -laudit -lkauth -lafsrpc -lrxkad -lubik -lprot -lrxkad -lrx -llwp -lauth -lsys -ldes -lcmd -lcom_err /usr/afsws/lib/afs/util.a -lpthread -laudit -llwp"
    ;;
    * )
      XLOCKLIBS="${XLOCKLIBS} -L/usr/afsws/lib -L/usr/afsws/lib/afs -lkauth -lubik -lprot -lrxkad -lrx -llwp -lauth -lsys -ldes -lcmd -lcom_err /usr/afsws/lib/afs/util.a -laudit"
    ;;
  esac
fi

AC_ARG_ENABLE(sia, [  --enable-sia            enable SIA],
              sia=$enableval,
              sia=no)
if test "$sia" = yes; then
  AC_MSG_RESULT([defining option SIA])
  AC_DEFINE([SIA], [1], [Security Innovation Alliance])
  XLOCKLIBS="${XLOCKLIBS} -lsecurity"
fi

AC_ARG_ENABLE(sunos_adjunct_passwd, [  --enable-sunos-adjunct-passwd
                          enable SUNOS Adjunct passwording],
              sunos_adjunct_passwd=$enableval,
              sunos_adjunct_passwd=no)
if test "$sunos_adjunct_passwd" = yes; then
  AC_MSG_RESULT([defining option SUNOS_ADJUNCT_PASSWD])
  AC_DEFINE([SUNOS_ADJUNCT_PASSWD], [1], [SunOS Adjunct Passwording])
fi

AC_ARG_WITH(passwd-helper,
	[  --with-passwd-helper=/path Use external helper program for authentication],
	passwd_helper=$withval)
if test "$passwd_helper" != ""; then
  AC_MSG_RESULT([using password helper $passwd_helper])
  AC_DEFINE_UNQUOTED([PASSWD_HELPER_PROGRAM], [1], ["$passwd_helper"])
fi

AC_ARG_ENABLE(mb, [  --disable-mb            disable Xmb function series so will only display ASCII],
              mb=$enableval,
              mb=yes)
if test "$mb" = yes; then
  AC_MSG_RESULT([defining option USE_MB])
  AC_DEFINE([USE_MB], [1], [Mb fonts])
fi

AC_ARG_ENABLE(customization, [  --enable-customization  enable customization of XResource],
              customization=$enableval,
              customization=no)
if test "$customization" = yes; then
  AC_MSG_RESULT([defining option CUSTOMIZATION])
  AC_DEFINE([CUSTOMIZATION], [1], [allow customization of XResoure])
fi

AC_ARG_ENABLE(modules, [  --enable-modules        enable modules (experimental!)],
              modules=$enableval,
              modules=no)
if test "$modules" = yes; then
  AC_MSG_RESULT([enabling xlk modules])
  MODULES=""
  NOMODULES="#"
  AC_DEFINE([USE_MODULES], [1], [using modules])
case ${canonical} in
    *-*-linux* )
      MODULEFLAGS="-rdynamic"
      CFLAGS="$CFLAGS -fPIC"
      CXXFLAGS="$CXXFLAGS -fPIC"
      ;;
    *)
      MODULEFLAGS=""
      ;;
esac
else
  MODULES="#"
  NOMODULES=""
  MODULEFLAGS=""
fi

AC_ARG_ENABLE(blank_only, [  --enable-blank-only     enable blank mode only (boring)],
              blank_only=$enableval,
              blank_only=no)
if test "$blank_only" = "yes"; then
  AC_MSG_RESULT([enabling blank mode only])
  AC_DEFINE([BLANK_ONLY], [1], [only blank mode will be available])
else
  AC_ARG_ENABLE(nice_only, [  --enable-nice-only      enable only low cpu modes ],
              nice_only=$enableval,
              nice_only=no)
  if test "$nice_only" = "yes"; then
    AC_MSG_RESULT([enabling nice modes only])
    AC_DEFINE([NICE_ONLY], [1], [nice modes only])
  fi
fi

AC_ARG_ENABLE(check, [  --enable-check          enable check (experimental)],check=$enableval,check=no)
if test "$check" = yes; then
  AC_MSG_RESULT([enabling memory debug checking])
  CHECK=""
else
  CHECK="#"
fi

AC_ARG_WITH(lang, [  --with-lang=lang        use a foreign language (nl/fr/de/ja)],lang=$withval,lang=none)
case "$lang" in
  nl|NL|Nl)
    AC_MSG_RESULT([enabling some reporting in Dutch])
    AC_DEFINE([NL], [1], [support reporting in Dutch])
    ;;
  fr|FR|Fr)
    AC_MSG_RESULT([enabling some reporting in French])
    AC_DEFINE([FR], [1], [support reporting in French])
    ;;
  de|DE|De)
    AC_MSG_RESULT([enabling some reporting in German])
    AC_DEFINE([DE], [1], [support reporting in German])
    ;;
  ja|JA|Ja)
    AC_MSG_RESULT([enabling some reporting in Japanese])
    AC_DEFINE([JA], [1], [support reporting in Japanese])
    ;;
  none)
    AC_MSG_RESULT([using default language (English)])
    ;;
  *)
    AC_MSG_RESULT([unknown language $lang. using default language (English)])
    ;;
esac

AC_ARG_ENABLE(orig_xpm_patch, [  --enable-orig-xpm-patch use the original pixmap patch for mail icon
                          (have colormap problems in 8 bits depth visuals)],
              orig_xpm_patch=$enableval,
              orig_xpm_patch=no)
if test "$orig_xpm_patch" = "yes"; then
  AC_MSG_RESULT([enabling original mail icon xpm patch])
  AC_DEFINE([ORIGINAL_XPM_PATCH], [1], [original XPM patch for mail icon])
fi

AC_ARG_ENABLE(appdefaultdir, [  --enable-appdefaultdir=DIR
                          set directory installation of appdefaults,
			  default is where X is installed],appdefault_dir=$enableval,appdefault_dir=no)
case "x$appdefault_dir" in
x/*|x.*)
  AC_MSG_RESULT([setting APPDEFAULTS = $enableval])
  APPDEFAULTS=$enableval
  if test ! -d $appdefault_dir; then
    AC_MSG_RESULT([Warning: Directory $enableval does not exist])
  fi
  ;;
esac

if test "x$INSTPGMFLAGS" != x; then
AC_ARG_ENABLE(setuid, [  --disable-setuid        disables setuid installation if automatically chosen],setuid=$enableval,setuid=yes)
case "x$setuid" in
xno*)
  case ${canonical} in
  *-*-linux*)
    if test "$vtlock" = "yes"; then
      INSTPGMFLAGS="-s -o root -g bin -m 4111"
      AC_MSG_RESULT([defining install options (setuid/setgid) = "$INSTPGMFLAGS" for vtlock])
       echo "experimental: Big buffer overrun security risk"
    else
  INSTPGMFLAGS=""
  AC_MSG_RESULT([no setuid/setgid install])
    fi
    ;;
  *)
  INSTPGMFLAGS=""
  AC_MSG_RESULT([no setuid/setgid install])
  ;;
  esac
  ;;
x|xyes*)
  case ${canonical} in
  *-*-linux*)
    if test "$vtlock" = "yes"; then
      INSTPGMFLAGS="-s -o root -g bin -m 4111"
      AC_MSG_RESULT([defining install options (setuid/setgid) = "$INSTPGMFLAGS" for vtlock])
    else
  AC_MSG_RESULT([defining install options (setuid/setgid) = "$INSTPGMFLAGS"])
    fi
    ;;
  *)
  AC_MSG_RESULT([defining install options (setuid/setgid) = "$INSTPGMFLAGS"])
  ;;
  esac
  ;;
x*)
  INSTPGMFLAGS=$enableval
  AC_MSG_RESULT([defining install options (setuid/setgid) = "$enableval"])
  ;;
esac
else
  AC_MSG_RESULT([no setuid/setgid install])
fi

DEPEND=makedepend
DEPEND_FLAGS=
DEPEND_DEFINES=

case "${canonical}" in
  *-*-irix5* | *-*-irix6* )
    XLOCKLIBPATHS="-L/usr/lib32 ${XLOCKLIBPATHS}"
    XMLOCKLIBPATHS="-L/usr/lib32 ${XMLOCKLIBPATHS}"
    XGLOCKLIBPATHS="-L/usr/lib32 ${XGLOCKLIBPATHS}"
  ;;
esac
XLOCKLIBS="${XLOCKLIBPATHS} ${XLOCKLIBS} -lX11 -lXext -lm"
XMLOCKLIBS="${XMLOCKLIBPATHS} ${XMLOCKLIBS} -lX11"
XGLOCKLIBS="${XGLOCKLIBPATHS} ${XGLOCKLIBS} ${GTK_LIBS} -lX11"
AC_SUBST(XLOCKLDFLAGS)
AC_SUBST(XMLOCKLDFLAGS)
AC_SUBST(XGLOCKLDFLAGS)
AC_SUBST(INSTPGMFLAGS)
AC_SUBST(MODULEFLAGS)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(XLOCKINC)
AC_SUBST(XMLOCKINC)
AC_SUBST(XGLOCKINC)
AC_SUBST(GTK_LIBS)
AC_SUBST(XLIBS)
AC_SUBST(BITMAPTYPE)
AC_SUBST(PIXMAPTYPE)
AC_SUBST(BITMAPDIR)
AC_SUBST(PIXMAPDIR)
AC_SUBST(XLOCKLIBS)
AC_SUBST(XMLOCKLIBS)
AC_SUBST(XGLOCKLIBS)
AC_SUBST(XMLOCK)
AC_SUBST(XGLOCK)
AC_SUBST(XGLOCKDIR)
AC_SUBST(INSTALL_XMLOCK)
AC_SUBST(INSTALL_XGLOCK)
AC_SUBST(UNINSTALL_XMLOCK)
AC_SUBST(UNINSTALL_XGLOCK)
AC_SUBST(APPDEFAULTS)
AC_SUBST(LINT)
AC_SUBST(CHECK)
AC_SUBST(PURIFY)
AC_SUBST(XPM)
AC_SUBST(GL)
AC_SUBST(GLTT)
AC_SUBST(FTGL)
AC_SUBST(TTF)
AC_SUBST(FREETYPE)
AC_SUBST(BOMB)
AC_SUBST(INTERACTIVE)
AC_SUBST(UNSTABLE)
AC_SUBST(DEPEND)
AC_SUBST(DEPEND_FLAGS)
AC_SUBST(DEPEND_DEFINES)
AC_SUBST(CXX)
AC_SUBST(CCC)
AC_SUBST(MODULES)
AC_SUBST(NOMODULES)
AC_SUBST(FORTUNE)
AC_SUBST(FORTUNE_PATH)
AC_SUBST(NOPLAY)

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile xlock/Makefile modes/Makefile modes/glx/Makefile
  xmlock/Makefile xglock/Makefile])
AC_OUTPUT
