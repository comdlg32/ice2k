/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Andrew File System, a distributed file system */
/* #undef AFS */

/* AIX Version 3 OS */
/* #undef AIXV3 */

/* root is always allowed */
#define ALWAYS_ALLOW_ROOT 1

/* using deprecated PAM */
/* #undef BAD_PAM */

/* only blank mode will be available */
/* #undef BLANK_ONLY */

/* allow a user to close down session for lab use */
/* #undef CLOSEDOWN_LOGOUT */

/* allow customization of XResoure */
/* #undef CUSTOMIZATION */

/* use DCE passwording */
/* #undef DCE_PASSWD */

/* support reporting in German */
/* #undef DE */

/* Declared GetEnv */
#define DECLARED_GETENV 1

/* Declared SRAND48 is available */
#define DECLARED_SRAND48 1

/* default auto logout in minutes */
/* #undef DEF_AUTO_LOGOUT */

/* default button logout in minutes */
/* #undef DEF_BUTTON_LOGOUT */

/* program to play sounds */
#define DEF_PLAY "play.sh"

/* True Text Font */
#define DEF_TTFONT "/usr/lib/X11/xlock/fonts/"

/* disable interactive modes */
/* #undef DISABLE_INTERACTIVE */

/* support reporting in French */
/* #undef FR */

/* get time of day is 2 arguments */
#define GETTIMEOFDAY_TWO_ARGS 1

/* using another version of PAM */
/* #undef GOOD_PAM */

/* Athena is available */
#define HAVE_ATHENA 1

/* C++ is available */
#define HAVE_CXX 1

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#define HAVE_DIRENT_H 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Free Type */
#define HAVE_FREETYPE 1

/* GL Font Rendering Library */
#define HAVE_FTGL 1

/* Define to 1 if you have the `gethostname' function. */
#define HAVE_GETHOSTNAME 1

/* get time of day */
#define HAVE_GETTIMEOFDAY 1

/* GL Bind Texture */
#define HAVE_GLBINDTEXTURE 1

/* GL True Text */
/* #undef HAVE_GLTT */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Kerberos 4 available */
/* #undef HAVE_KRB4 */

/* Kerberos 5 available */
/* #undef HAVE_KRB5 */

/* LIBESD audio is available */
/* #undef HAVE_LIBESD */

/* PNG images */
#define HAVE_LIBPNG 1

/* neXtaw is available */
/* #undef HAVE_LIB_NEXTAW */

/* Xaw3d is available */
/* #undef HAVE_LIB_XAW3D */

/* Xaw3dxft is available */
#define HAVE_LIB_XAW3DXFT 1

/* XawPlus is available */
/* #undef HAVE_LIB_XAWPLUS */

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Motif is available */
/* #undef HAVE_MOTIF */

/* nanosecond sleep is available */
/* #undef HAVE_NANOSLEEP */

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define this if your system supports POSIX threads. */
#define HAVE_PTHREAD 1

/* Have PTHREAD_PRIO_INHERIT. */
#define HAVE_PTHREAD_PRIO_INHERIT 1

/* Low quality Random Number Generator using 32 bits is available */
/* #undef HAVE_RAND */

/* Random Number Generator using 48 bits is available */
#define HAVE_RAND48 1

/* Random Number Generator using 32 bits is available */
/* #undef HAVE_RANDOM */

/* Define to 1 if you have the `select' function. */
#define HAVE_SELECT 1

/* seteuid is available */
#define HAVE_SETEUID 1

/* setreuid is available */
/* #undef HAVE_SETREUID */

/* Shadow passwording */
#define HAVE_SHADOW 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdio.h> header file. */
#define HAVE_STDIO_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strdup' function. */
#define HAVE_STRDUP 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Struct Sigset Type */
/* #undef HAVE_STRUCT_SIGSET_T */

/* Define to 1 if you have the <syslog.h> header file. */
#define HAVE_SYSLOG_H 1

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_DIR_H */

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/param.h> header file. */
#define HAVE_SYS_PARAM_H 1

/* Define to 1 if you have the <sys/resource.h> header file. */
#define HAVE_SYS_RESOURCE_H 1

/* Define to 1 if you have the <sys/select.h> header file. */
#define HAVE_SYS_SELECT_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the `timelocal' function. */
#define HAVE_TIMELOCAL 1

/* True Text Font */
/* #undef HAVE_TTF */

/* Define to 1 if you have the `tzset' function. */
#define HAVE_TZSET 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* microsecond sleep is available */
#define HAVE_USLEEP 1

/* vfork is available */
#define HAVE_VFORK 1

/* XPM color images */
#define HAVE_XPM 1

/* support reporting in Japanese */
/* #undef JA */

/* Kerberos 5 write new credential cache */
/* #undef KRB5_WRITE_NEW_CCACHE */

/* less than AIX Version 3.2 */
/* #undef LESS_THAN_AIX3_2 */

/* Less than Solaris 2.5 OS */
/* #undef LESS_THAN_SOLARIS2_5 */

/* Maximum random number */
/* #undef MAXRAND */

/* nice modes only */
/* #undef NICE_ONLY */

/* support reporting in Dutch */
/* #undef NL */

/* deny logout button and autologout */
/* #undef NO_NEGATIVE_LOGOUT */

/* original XPM patch for mail icon */
/* #undef ORIGINAL_XPM_PATCH */

/* OSF1 Enhanced Security */
/* #undef OSF1_ENH_SEC */

/* package */
#define PACKAGE "xlockmore"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME ""

/* Define to the full name and version of this package. */
#define PACKAGE_STRING ""

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME ""

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION ""

/* "$passwd_helper" */
/* #undef PASSWD_HELPER_PROGRAM */

/* Define to necessary symbol if this constant uses a non-standard name on
   your system. */
/* #undef PTHREAD_CREATE_JOINABLE */

/* allow a user to kill an X session for lab use */
/* #undef SESSION_LOGOUT */

/* Security Innovation Alliance */
/* #undef SIA */

/* Solaris2 OS */
/* #undef SOLARIS2 */

/* sound path */
#define SOUNDPATH "/usr/local/share/xlock/sounds"

/* file for staff exempt from auto-logout */
/* #undef STAFF_FILE */

/* netgroup of staff exempt from auto-logout */
/* #undef STAFF_NETGROUP */

/* Define to 1 if all of the C90 standard headers exist (not just the ones
   required in a freestanding environment). This macro is provided for
   backward compatibility; new code need not use it. */
#define STDC_HEADERS 1

/* SunOS4 OS */
/* #undef SUNOS4 */

/* SunOS Adjunct Passwording */
/* #undef SUNOS_ADJUNCT_PASSWD */

/* Sun OGL No Vertex Macros */
/* #undef SUN_OGL_NO_VERTEX_MACROS */

/* System 5 Release 4 */
/* #undef SVR4 */

/* System 5 */
/* #undef SYSV */

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
/* #undef TM_IN_SYS_TIME */

/* auto logout in minutes */
/* #undef USE_AUTO_LOGOUT */

/* special bomb mode, causes logout */
#define USE_BOMB 1

/* button logout in minutes */
/* #undef USE_BUTTON_LOGOUT */

/* Display Power Management System */
#define USE_DPMS 1

/* Desktop Saver */
/* #undef USE_DTSAVER */

/* using ESOUND audio */
/* #undef USE_ESOUND */

/* Extension Libraries */
#define USE_EXT 1

/* using GL */
#define USE_GL 1

/* Image Magick */
/* #undef USE_MAGICK */

/* math error */
/* #undef USE_MATHERR */

/* Mb fonts */
#define USE_MB 1

/* using modules */
/* #undef USE_MODULES */

/* multiple root users */
/* #undef USE_MULTIPLE_ROOT */

/* multiple user */
/* #undef USE_MULTIPLE_USER */

/* using NAS audio */
/* #undef USE_NAS */

/* old event loop */
/* #undef USE_OLD_EVENT_LOOP */

/* using Pluggable Authentication Module */
/* #undef USE_PAM */

/* using RPLAY audio */
/* #undef USE_RPLAY */

/* using syslog */
/* #undef USE_SYSLOG */

/* use unstable modes */
/* #undef USE_UNSTABLE */

/* virtual root window */
#define USE_VROOT 1

/* VT lock */
/* #undef USE_VTLOCK */

/* window visibility */
/* #undef USE_WINDOW_VISIBLITY */

/* Xinemara to combine separate displays */
#define USE_XINERAMA 1

/* encrypted password in .xlockrc file */
/* #undef USE_XLOCKRC */

/* xlock group */
/* #undef USE_XLOCK_GROUP */

/* using X11 miscellaneous utilities */
/* #undef USE_XMU */

/* version file */
#define VERSION "5.78"

/* Define to 1 if the X Window System is missing or not being used. */
/* #undef X_DISPLAY_MISSING */

/* BSD source */
#define _BSD_SOURCE 1

/* DEFAULT source */
#define _DEFAULT_SOURCE 1

/* GNU source */
#define _GNU_SOURCE 1

/* POSIX source */
#define _POSIX_SOURCE 1

/* wide character type defined */
/* #undef _WCHAR_T */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef gid_t */

/* GL Bind Textures */
/* #undef glBindTextures */

/* GL Delete Textures */
/* #undef glDeleteTextures */

/* GL Generic Textures */
/* #undef glGenTextures */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Linux OS */
#define linux 1

/* Define to `int' if <sys/types.h> does not define. */
/* #undef mode_t */

/* Define as a signed integer type capable of holding a process identifier. */
/* #undef pid_t */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef uid_t */
