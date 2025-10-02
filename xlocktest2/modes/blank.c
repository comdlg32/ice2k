/* -*- Mode: C; tab-width: 4 -*- */
/* blank --- blank screen */

#if 0
static const char sccsid[] = "@(#)blank.c	5.00 2000/11/01 xlockmore";

#endif

/*-
 * Copyright (c) 1991 by Patrick J. Naughton.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 *
 * Revision History:
 * 16-May-2006: Removed screesaver stuff.  Should use settings from xlock.
 * 10-May-1997: Compatible with xscreensaver :)  OK you probably should not
 *              use this for xscreensaver but I could not resist.
 * 21-Mar-1996: Ron Hitchens <ron AT idiom.com>
 *		        Bonehead alert.  Don't blank during password prompting.
 * 19-Mar-1996: Ron Hitchens <ron AT idiom.com>
 *		        Changed to activate X server's native screensaver.
 *		        On some devices, this will result in power saving "sleep"
 *		        mode, or video blanking.
 * 31-Aug-1990: Written.
 */

#ifdef STANDALONE
# define DEFAULTS	"*delay: 1000000 \n" \

#define draw_blank 0
#define release_blank 0
#define free_blank 0
#define reshape_blank 0
#define blank_handle_event 0
# include "xlockmore.h"		/* in xscreensaver distribution */
#else /* STANDALONE */
# include "xlock.h"		/* in xlockmore distribution */
#endif /* STANDALONE */

ENTRYPOINT ModeSpecOpt blank_opts =
{0, (XrmOptionDescRec *) NULL, 0, (argtype *) NULL, (OptionStruct *) NULL};

#ifdef USE_MODULES
const ModStruct blank_description =
{"blank", "init_blank", (char *) NULL, (char *) NULL,
 (char *) NULL, "init_blank", (char *) NULL, &blank_opts,
 3000000, 1, 1, 1, 64, 1.0, "",
 "Shows nothing but a black screen", 0, NULL};

#endif

ENTRYPOINT void
init_blank(ModeInfo * mi)
{
	MI_CLEARWINDOW(mi);
}

XSCREENSAVER_MODULE ("Blank", blank)
