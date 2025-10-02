/* -*- Mode: C; tab-width: 4 -*- */
/* dclock --- floating digital clock or message */

#if 0
static const char sccsid[] = "@(#)dclock.c	5.00 2000/11/01 xlockmore";

#endif

/*-
 * Copyright (C) 1995 by Michael Stembera <mrbig@fc.net>.
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
 * 03-Dec-2005: Added a binary clock display, after seeing the binary clock
 *              at thinkgeek. Petey (petey_leinonen AT yahoo.com.au)
 * 01-Mar-2005: Added 24 hour clock (alex(at)shark-linux(dot)de)
 * 01-Nov-2000: Allocation checks
 * 07-May-1999: New "savers" added for y2k and second millennium countdowns.
 *              Tom Schmidt <tschmidt AT micron.com>
 * 04-Dec-1998: New "savers" added for hiv, veg, and lab.
 *              hiv one due to Kenneth Stailey <kstailey@disclosure.com>
 * 10-Aug-1998: Population Explosion and Tropical Forest Countdown stuff
 *              I tried to get precise numbers but they may be off a few
 *              percent.  Whether or not, its still pretty scary IMHO.
 *              Although I am a US citizen... I have the default for area in
 *              metric.  David Bagley <bagleyd AT verizon.net>
 * 10-May-1997: Compatible with xscreensaver
 * 29-Aug-1995: Written.
 */

/*-
 *  Some of my calculations laid bare...  (I have a little problem with
 *  the consistency of the numbers I got at the Bronx Zoo but proportions
 *  were figured to be 160.70344 people a minute increase not 180 and
 *  35.303144 hectares (87.198766 acres) a minute decrease not 247 (100 acres).
 *  So I am going with these more conservative numbers.)
 *
 *  Time 0 is 0:00:00 1 Jan 1970 at least according to hard core UNIX fans
 *  Minutes from 1  Jan 1970 to 21 Jun 1985:  8137440
 *  Minutes from 21 Jun 1985 to 12 Jul 1998:  6867360
 *                                    Total: 15004800
 *
 *  Population Explosion Saver (popex)
 *  3,535,369,000 people (figured by extrapolation) 1 Jan 1970
 *  4,843,083,596 people 21 Jun 1985 (Opening of Wild Asia in the Bronx Zoo)
 *  5,946,692,000 people 12 Jul 1998 (David Bagley visits zoo ;) )
 *  180 people a minute increase in global population (I heard 170 also)
 *  260,000 people a day increase in global population
 *
 *  Tropical Forest Countdown Saver (forest)
 *  1,184,193,000 hectares (figured by extrapolation) 1 Jan 1970
 *    (2,924,959,000 acres)
 *  896,916,700 hectares 21 Jun 1985 (Opening of Wild Asia in the Bronx Zoo)
 *    (2,215,384,320 acres)
 *  654,477,300 hectares 12 Jul 1998 (David Bagley visits zoo ;) )
 *    (1,616,559,000 acres)
 *  247 hectares a minute lost forever (1 hectare = 2.47 acres)
 *
 *  HIV Infection Counter Saver (hiv) (stats from http://www.unaids.org/)
 *  33,600,000 31 Dec 1999 living w/HIV
 *  16,300,000 31 Dec 1999 dead
 *  49,900,000 31 Dec 1999 total infections
 *   5,600,000 new infections in 1999
 *  10.6545 infections/min
 *  -118,195,407 virtual cases (figured by extrapolation) 1 Jan 1970
 *   (this is a result of applying linear tracking to a non-linear event)
 *
 *  Animal Research Counter Saver (lab)
 *  Approximately 17-22 million animals are used in research each year.
 *  OK so assume 17,000,000 / 525960 = 32.32184957 per minute
 *  http://www.fcs.uga.edu/~mhulsey/GDB.html
 *
 *  Animal Consumation Counter Saver (veg)
 *  Approximately 5 billion are consumed for food annually.
 *  OK 5,000,000,000 / 525960 = 9506.426344209 per minute
 *  http://www.fcs.uga.edu/~mhulsey/GDB.html
 */

#ifdef STANDALONE
#define MODE_dclock
#define DEFAULTS "*delay: 10000 \n" \
	"*cycles: 10000 \n" \
	"*ncolors: 64 \n" \

# define free_dclock 0
# define reshape_dclock 0
# define dclock_handle_event 0
#define BRIGHT_COLORS
#define UNIFORM_COLORS
#include "xlockmore.h"		/* in xscreensaver distribution */
#else /* STANDALONE */
#include "xlock.h"		/* in xlockmore distribution */
#include "iostuff.h"
#include "util.h"
#endif /* STANDALONE */

#ifdef MODE_dclock

#ifndef METRIC
#define METRIC 1
#endif

#ifdef METRIC
#define AREA_MIN 35.303144
#define AREA_TIME_START 1184193000.0
#else
#define AREA_MIN 87.198766
#define AREA_TIME_START 2924959000.0
#endif

#define PEOPLE_MIN 160.70344
#define PEOPLE_TIME_START 3535369000.0
#define HIV_MIN 10.6545
#define HIV_TIME_START -118195407.0
#define LAB_MIN 32.32184957
#define LAB_TIME_START 0
#define VEG_MIN 9506.426344209
#define VEG_TIME_START 0
/* epoch time at midnight 1 January 2000 UTC */
#define Y2K_TIME_START (((30 * 365) + 7) * 24 * 60 * 60)
#define Y2001_TIME_START (Y2K_TIME_START + 366 * 24 * 60 * 60)
#define MAYAN_TIME_START (((42 * 365) + 10) * 24 * 60 * 60  + (366 - 11) * 24 * 60 * 60)

#define LED_LEAN 0.2
#define LED_XS 30.0
#define LED_YS 45.0
#define LED_WIDTH (0.15 * LED_YS)
#define LED_INC (LED_LEAN * LED_XS) /* LEAN and INC do not have to be linked */

#define BINARY_WIDTH 10
#define BINARY_HEIGHT 10
#define BINARY_INC 5

#define DEF_BINARY "False"
#define DEF_LED    "False"
#define DEF_POPEX  "False"
#define DEF_FOREST "False"
#define DEF_HIV    "False"
#define DEF_LAB    "False"
#define DEF_VEG    "False"
#define DEF_TIME24 "False"
#define DEF_Y2K    "False"
#define DEF_Y2001  "False"
#define DEF_MAYAN  "False"

/*- If you remember your Electronics course...
        a
        _
    f / / b
      - g
  e /_/ c
     d
 */

#define MAX_LEDS 7
static unsigned char digits[][MAX_LEDS]=
{
/* a  b  c  d  e  f  g */
  {1, 1, 1, 1, 1, 1, 0},  /* 0 */
  {0, 1, 1, 0, 0, 0, 0},  /* 1 */
  {1, 1, 0, 1, 1, 0, 1},  /* 2 */
  {1, 1, 1, 1, 0, 0, 1},  /* 3 */
  {0, 1, 1, 0, 0, 1, 1},  /* 4 */
  {1, 0, 1, 1, 0, 1, 1},  /* 5 */
  {1, 0, 1, 1, 1, 1, 1},  /* 6 */
  {1, 1, 1, 0, 0, 0, 0},  /* 7 */
  {1, 1, 1, 1, 1, 1, 1},  /* 8 */
  {1, 1, 1, 1, 0, 1, 1}  /* 9 */
#if 0
  , /* Completeness, we must have completeness */
  {1, 1, 1, 0, 1, 1, 1},  /* A */
  {0, 0, 1, 1, 1, 1, 1},  /* b */
  {1, 0, 0, 1, 1, 1, 0},  /* C */
  {0, 1, 1, 1, 1, 0, 1},  /* d */
  {1, 0, 0, 1, 1, 1, 1},  /* E */
  {1, 0, 0, 0, 1, 1, 1}   /* F */
#define MAX_DIGITS 16
#else
#define MAX_DIGITS 10
#endif
};

static Bool led;

/* Create an virtual parallelogram, normal rectangle parameters plus "lean":
   the amount the start of a will be shifted to the right of the start of d.
 */

static XPoint parallelogramUnit[4] =
{
	{1, 0},
	{2, 0},
	{-1, 1},
	{-2, 0}
};

static Bool binary;
static Bool popex;
static Bool forest;
static Bool hiv;
static Bool lab;
static Bool veg;
static Bool time24;
static Bool y2k;
static Bool millennium;
static Bool mayan;

static XrmOptionDescRec opts[] =
{
	{(char *) "-binary", (char *) ".dclock.binary", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+binary", (char *) ".dclock.binary", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-led", (char *) ".dclock.led", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+led", (char *) ".dclock.led", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-popex", (char *) ".dclock.popex", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+popex", (char *) ".dclock.popex", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-forest", (char *) ".dclock.forest", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+forest", (char *) ".dclock.forest", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-hiv", (char *) ".dclock.hiv", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+hiv", (char *) ".dclock.hiv", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-lab", (char *) ".dclock.lab", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+lab", (char *) ".dclock.lab", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-veg", (char *) ".dclock.veg", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+veg", (char *) ".dclock.veg", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-time24", (char *) ".dclock.time24", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+time24", (char *) ".dclock.time24", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-y2k", (char *) ".dclock.y2k", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+y2k", (char *) ".dclock.y2k", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-millennium", (char *) ".dclock.millennium", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+millennium", (char *) ".dclock.millennium", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-mayan", (char *) ".dclock.mayan", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+mayan", (char *) ".dclock.mayan", XrmoptionNoArg, (caddr_t) "off"}
};
static argtype vars[] =
{
	{(void *) & binary, (char *) "binary", (char *) "Binary", (char *) DEF_BINARY, t_Bool},
	{(void *) & led, (char *) "led", (char *) "LED", (char *) DEF_LED, t_Bool},
	{(void *) & popex, (char *) "popex", (char *) "PopEx", (char *) DEF_POPEX, t_Bool},
	{(void *) & forest, (char *) "forest", (char *) "Forest", (char *) DEF_FOREST, t_Bool},
	{(void *) & hiv, (char *) "hiv", (char *) "Hiv", (char *) DEF_HIV, t_Bool},
	{(void *) & lab, (char *) "lab", (char *) "Lab", (char *) DEF_LAB, t_Bool},
	{(void *) & veg, (char *) "veg", (char *) "Veg", (char *) DEF_VEG, t_Bool},
	{(void *) & time24, (char *) "time24", (char *) "Time24", (char *) DEF_TIME24, t_Bool},
	{(void *) & y2k, (char *) "y2k", (char *) "Y2K", (char *) DEF_Y2K, t_Bool},
	{(void *) & millennium, (char *) "millennium", (char *) "Millennium", (char *) DEF_Y2001, t_Bool},
	{(void *) & mayan, (char *) "mayan", (char *) "Mayan", (char *) DEF_MAYAN, t_Bool}
};
static OptionStruct desc[] =
{
	{(char *) "-/+binary", (char *) "turn on/off binary clock display"},
	{(char *) "-/+led", (char *) "turn on/off Light Emitting Diode seven segment display"},
	{(char *) "-/+popex", (char *) "turn on/off population explosion counter"},
	{(char *) "-/+forest", (char *) "turn on/off tropical forest destruction counter"},
	{(char *) "-/+hiv", (char *) "turn on/off HIV infection counter"},
	{(char *) "-/+lab", (char *) "turn on/off Animal Research counter"},
	{(char *) "-/+veg", (char *) "turn on/off Animal Consumation counter"},
	{(char *) "-/+time24", (char *) "turn on/off 24 hour display"},
	{(char *) "-/+y2k", (char *) "turn on/off Year 2000 countdown"},
	{(char *) "-/+millennium", (char *) "turn on/off 3rd Millennium (1 January 2001) countdown"},
	{(char *) "-/+mayan", (char *) "turn on/off 13 bak'tun (21 December 2012) countdown"},
};

ENTRYPOINT ModeSpecOpt dclock_opts =
{sizeof opts / sizeof opts[0], opts, sizeof vars / sizeof vars[0], vars, desc};

#ifdef USE_MODULES
ModStruct dclock_description =
{"dclock", "init_dclock", "draw_dclock", "release_dclock",
 "refresh_dclock", "init_dclock", (char *) NULL, &dclock_opts,
 10000, 1, 10000, 1, 64, 0.3, "",
 "Shows a floating digital clock or message", 0, NULL};

#endif

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <time.h>

/* language depended resources */
#if defined FR
#include "dclock-msg-fr.h"
#elif defined NL
#include "dclock-msg-nl.h"
#elif defined JA
#include "dclock-msg-ja.h"
#else
#include "dclock-msg-en.h"
#endif

#ifdef USE_MB
static XFontSet mode_font = None;
static int getFontHeight(XFontSet f)
{
	XRectangle ink, log;

	if (f == None) {
		return 8;
	} else {
		XmbTextExtents(mode_font, "My", strlen("My"), &ink, &log);
		return log.height;
	}
}
static int getTextWidth(char *string)
{
	XRectangle ink, logical;

	XmbTextExtents(mode_font, string, strlen(string), &ink, &logical);
	return logical.width;
}
#define DELTA 0.2
extern XFontSet getFontSet(Display * display);
#define DrawString(d, p, gc, start, ascent, string, len) (void) XmbDrawString(\
	d, p, mode_font, gc, start, ascent, string, len)
#else
static XFontStruct *mode_font = None;
#define getFontHeight(f) ((f == None) ? 8 : f->ascent + f->descent)
static int getTextWidth(char *string)
{
	return XTextWidth(mode_font, string, strlen(string));
}	
#define DELTA 0
extern XFontStruct *getFont(Display * display);
#define DrawString(d, p, gc, start, ascent, string, len) (void) XDrawString(\
	d, p, gc, start, ascent, string, len)
#endif

#if 0
#define MAXWIDTH BUFSIZ
#else
#define MAXWIDTH 170
#endif
#if 0
#define STRSIZE 50
#define MAXLINES 2
#else
#define STRSIZE BUFSIZ
#define MAXLINES 40
#endif

typedef struct {
	int         color;
	short       height, width;
	char       *str;
	int         lines;
	char        strnew[MAXLINES][STRSIZE], strold[MAXLINES][STRSIZE];
	char       *strpta[MAXLINES], *strptb[MAXLINES];
	short       textWidth[MAXLINES], textStart[MAXLINES];
	time_t      timenew, timeold;
	int         tzoffset;
	short       maxx, maxy, clockx, clocky;
	short       text_height, text_width, text_ascent, text_descent;
	short       hour;
	short       dx, dy;
	int         pixw, pixh;
	Pixmap      pixmap;
	GC          fgGC, bgGC;
	Bool        popex, forest, hiv, lab, veg;
	Bool        time24, y2k, millennium, mayan, led, binary;
	XPoint      parallelogram[4];
} dclockstruct;

static dclockstruct *dclocks = (dclockstruct *) NULL;

extern char *message;

#ifdef STANDALONE
char * message = NULL;
#endif

static time_t
timeAtLastNewYear(time_t timeNow)
{
	struct tm *t;

	t = localtime(&timeNow);
	t->tm_mon = 0;
	t->tm_mday = 1;
	t->tm_hour = 0;
	t->tm_min = 0;
	t->tm_sec = 0;
	t->tm_isdst = -1; /* mktime shall decide itself */
	return mktime(t);
}

#ifndef HAVE_SNPRINTF
static double logbase = 0.0;
#define BASE 10.0
#define GROUP 3
#endif

static void
convert(double x, char *string)
{
#ifdef HAVE_SNPRINTF
/* Also old C compiler can not accept this syntax, but this syntax awares
   locale.  Known to work with gcc-2.95.2 and glibc-2.1.3. */
	(void) snprintf(string, STRSIZE, "%'.0f", x);
#else

	int i, j, k = 0;
	int place = (int) (log(x) / logbase);
	double divisor = 1.0;

	for (i = 0; i < place; i++)
		divisor *= BASE;

	for (i = place; i >= 0; i--) {
		j = (int) (x / divisor);
		string[k++] = (char) j + '0';
		x -= j * divisor;
		divisor /= BASE;
		if ((i > 0) && (i % GROUP == 0)) {
			string[k++] = ',';
		}
	}
	string[k] = '\0';
#endif
}

static void
dayhrminsec(time_t timeCount, int tzoffset, char *string)
{
	int days, hours, minutes, secs;
	int bufsize, i;

	/* ignore warning about useless operation if its defined unsigned */
	timeCount = ABS(timeCount);
	days = (int) (timeCount / 86400);
	hours = (int) ((timeCount / 3600) % 24);
	minutes = (int) ((timeCount / 60) % 60);
	secs = (int) (timeCount % 60);

	/* snprintf would make this easier but its not always available */
	bufsize = 16 + strlen((days==1) ? DAY : DAYS);
	if (bufsize >= STRSIZE)
		return;
	(void) sprintf(string, "%d %s", days, (days==1) ? DAY : DAYS);

	i = strlen(string);
	bufsize = 4 + strlen((hours==1) ? HOUR : HOURS);
	if (i + bufsize >= STRSIZE)
		return;
	(void) sprintf(&string[i], " %d %s", hours, (hours==1) ? HOUR : HOURS);

	i = strlen(string);
	bufsize = 4 + strlen((minutes==1) ? MINUTE : MINUTES);
	if (i + bufsize >= STRSIZE)
		return;
	(void) sprintf(&string[i], " %d %s", minutes,
		(minutes==1) ? MINUTE : MINUTES);

	i = strlen(string);
	bufsize += 4 + strlen((secs==1) ? SECOND : SECONDS);
	if (i + bufsize >= STRSIZE)
		return;
	(void) sprintf(&string[i], " %d %s", secs,
		(secs==1) ? SECOND : SECONDS);

	if (!tzoffset) {
		i = strlen(string);
		bufsize += 6; /* strlen(" (UTC)"); */
		if (i + bufsize >= STRSIZE)
			return;
		(void) strcat(string, " (UTC)");
	}
}

static void
drawaled(ModeInfo * mi, int startx, int starty, int aled)
{
	Display *display = MI_DISPLAY(mi);
	dclockstruct *dp = &dclocks[MI_SCREEN(mi)];
 	int x_1, y_1, x_2, y_2;

	int offset = (int) LED_WIDTH;
	int offset2 = (int) (LED_WIDTH / 2.0);
	int leanoffset = (int) (offset2 * LED_LEAN);

	switch (aled) {
		case 0: /* a */
			x_1 = startx + dp->parallelogram[0].x;
			y_1 = starty + dp->parallelogram[0].y;
			x_2 = x_1 + dp->parallelogram[1].x - offset;
			y_2 = y_1 + dp->parallelogram[1].y;
			x_1 = x_1 + offset;
			XDrawLine(display, dp->pixmap, dp->fgGC,
				x_1, y_1, x_2, y_2);
			break;
		case 1: /* b */
			x_1 = startx + dp->parallelogram[0].x +
				dp->parallelogram[1].x;
			y_1 = starty + dp->parallelogram[0].y +
				dp->parallelogram[1].y;
			x_2 = x_1 + dp->parallelogram[2].x / 2 + leanoffset;
			y_2 = y_1 + dp->parallelogram[2].y / 2 - offset2;
			x_1 = x_1 - leanoffset;
			y_1 = y_1 + offset2;
			XDrawLine(display, dp->pixmap, dp->fgGC,
				x_1, y_1, x_2, y_2);
			break;
		case 2: /* c */
			x_1 = startx + dp->parallelogram[0].x +
				dp->parallelogram[1].x +
				dp->parallelogram[2].x;
			y_1 = starty + dp->parallelogram[0].y +
				dp->parallelogram[1].y +
				dp->parallelogram[2].y;
			x_2 = x_1 - dp->parallelogram[2].x / 2 - leanoffset;
			y_2 = y_1 - dp->parallelogram[2].y / 2 + offset2;
			x_1 = x_1 + leanoffset;
			y_1 = y_1 - offset2;
			XDrawLine(display, dp->pixmap, dp->fgGC,
				x_1, y_1, x_2, y_2);
			break;
		case 3: /* d */
			x_1 = startx + dp->parallelogram[0].x +
				dp->parallelogram[2].x;
			y_1 = starty + dp->parallelogram[0].y +
				dp->parallelogram[2].y;
			x_2 = x_1 + dp->parallelogram[1].x - offset;
			y_2 = y_1 + dp->parallelogram[1].y;
			x_1 = x_1 + offset;
			XDrawLine(display, dp->pixmap, dp->fgGC,
				x_1, y_1, x_2, y_2);
			break;
		case 4: /* e */
			x_1 = startx + dp->parallelogram[0].x +
				dp->parallelogram[2].x;
			y_1 = starty + dp->parallelogram[0].y +
				dp->parallelogram[2].y;
			x_2 = x_1 - dp->parallelogram[2].x / 2 - leanoffset;
			y_2 = y_1 - dp->parallelogram[2].y / 2 + offset2;
			x_1 = x_1 + leanoffset;
			y_1 = y_1 - offset2;
			XDrawLine(display, dp->pixmap, dp->fgGC,
				x_1, y_1, x_2, y_2);
			break;
		case 5: /* f */
			x_1 = startx + dp->parallelogram[0].x;
			y_1 = starty + dp->parallelogram[0].y;
			x_2 = x_1 + dp->parallelogram[2].x / 2 + leanoffset;
			y_2 = y_1 + dp->parallelogram[2].y / 2 - offset2;
			x_1 = x_1 - leanoffset;
			y_1 = y_1 + offset2;
			XDrawLine(display, dp->pixmap, dp->fgGC,
				x_1, y_1, x_2, y_2);
			break;
		case 6: /* g */
			x_1 = startx + dp->parallelogram[0].x +
				dp->parallelogram[2].x / 2;
			y_1 = starty + dp->parallelogram[0].y +
				dp->parallelogram[2].y / 2;
			x_2 = x_1 + dp->parallelogram[1].x - offset;
			y_2 = y_1 + dp->parallelogram[1].y;
			x_1 = x_1 + offset;
			XDrawLine(display, dp->pixmap, dp->fgGC,
				x_1, y_1, x_2, y_2);

	}
}

static void
drawacolon(ModeInfo * mi, int startx, int starty)
{
	Display *display = MI_DISPLAY(mi);
	dclockstruct *dp = &dclocks[MI_SCREEN(mi)];
 	int x_1, y_1, x_2, y_2;

	int offset2 = (int) (LED_WIDTH / 2.0);
	int leanoffset = (int) (offset2 * LED_LEAN);

	x_1 = startx + dp->parallelogram[0].x +
		(int) (dp->parallelogram[2].x / 2 - 2.0 * leanoffset);
	y_1 = starty + dp->parallelogram[0].y + dp->parallelogram[2].y / 2 +
		(int) (2.0 * offset2);
	x_2 = x_1 - (int) (2.0 * leanoffset);
	y_2 = y_1 + (int) (2.0 * offset2);
	XDrawLine(display, dp->pixmap, dp->fgGC, x_1, y_1, x_2, y_2);
	x_1 = startx + dp->parallelogram[0].x +
		dp->parallelogram[2].x / 2 + (int) (2.0 * leanoffset);
	y_1 = starty + dp->parallelogram[0].y + dp->parallelogram[2].y / 2 -
		(int) (2.0 * offset2);
	x_2 = x_1 + (int) (2.0 * leanoffset);
	y_2 = y_1 - (int) (2.0 * offset2);
	XDrawLine(display, dp->pixmap, dp->fgGC, x_1, y_1, x_2, y_2);
}

static void
drawanumber(ModeInfo * mi, int startx, int starty, int digit)
{
	int aled;

	for (aled = 0; aled < MAX_LEDS; aled++) {
		if (digits[digit][aled])
			drawaled(mi, startx, starty, aled);
	}
}

static void
drawadot(ModeInfo * mi, int startx, int starty, int filled)
{
	Display *display = MI_DISPLAY(mi);
	dclockstruct *dp = &dclocks[MI_SCREEN(mi)];

	if (filled)
		XFillArc(display, dp->pixmap, dp->fgGC, startx, starty, 
			BINARY_WIDTH, BINARY_HEIGHT, 0, 23040);
	else
		XDrawArc(display, dp->pixmap, dp->fgGC, startx, starty, 
			BINARY_WIDTH, BINARY_HEIGHT, 0, 23040);
}

static void
drawabinary(ModeInfo * mi, int startx, int starty, int dotcount, int digit)
{
	int newy;
	int filled;
	int i;

	for (i=0; i < dotcount; i++) {
		newy = starty + ((BINARY_HEIGHT + BINARY_INC) * (3 - i));
		filled = (digit >> i) & 0x01;
		drawadot(mi, startx, newy, filled);
	}
}

static void
free_dclock_screen(Display *display, dclockstruct *dp)
{
	if (dp == NULL) {
		return;
	}
	if (dp->fgGC != None) {
		XFreeGC(display, dp->fgGC);
		dp->fgGC = None;
	}
	if (dp->bgGC) {
		XFreeGC(display, dp->bgGC);
		dp->bgGC = None;
	}
	if (dp->pixmap) {
		XFreePixmap(display, dp->pixmap);
		dp->pixmap = None;
	}
	dp = NULL;
}


static void
drawDclock(ModeInfo * mi)
{
	Display *display = MI_DISPLAY(mi);
	Window window = MI_WINDOW(mi);
	GC gc = MI_GC(mi);
	dclockstruct *dp = &dclocks[MI_SCREEN(mi)];
	short xold, yold;
	int i, j;

	if (!(message && *message)) {
	  if (dp->led || dp->binary) {
		dp->timeold = dp->timenew = time((time_t *) NULL);
		dp->str = ctime(&dp->timeold);
	  } else if (!dp->popex && !dp->forest && !dp->hiv &&
	    !dp->lab && !dp->veg && !dp->y2k && !dp->millennium && !dp->mayan) {
		if (dp->timeold != (dp->timenew = time((time_t *) NULL))) {
			/* only parse if time has changed */
			dp->timeold = dp->timenew;

			if (!dp->popex && !dp->forest && !dp->hiv && !dp->lab &&
			    !dp->veg && !dp->y2k && !dp->millennium && !dp->mayan) {
			  if (dp->time24)
				(void) strftime(dp->strpta[0], STRSIZE,
					"%H:%M:%S", localtime(&(dp->timeold)));
			  else
				(void) strftime(dp->strpta[0], STRSIZE,
					"%I:%M:%S %p", localtime(&(dp->timeold)));
			}
			(void) strftime(dp->strpta[1], STRSIZE,
				"%a %b %d %Y", localtime(&(dp->timeold)));
		}
	  } else {
		time_t timeNow, timeLocal;
		timeNow = seconds();
		timeLocal = timeNow + dp->tzoffset;

		if (dp->popex) {
			convert(PEOPLE_TIME_START + (PEOPLE_MIN / 60.0) * timeNow, dp->strnew[1]);
			(void) strncat(dp->strnew[1], PEOPLE_STRING,
				STRSIZE-strlen(dp->strnew[1]));
			dp->strpta[1] = dp->strnew[1];
			dp->strptb[1] = dp->strpta[1];
		} else if (dp->forest) {
			convert(AREA_TIME_START - (AREA_MIN / 60.0) * timeNow, dp->strnew[1]);
			(void) strncat(dp->strnew[1], TROPICAL_STRING,
				STRSIZE-strlen(dp->strnew[1]));
			(void) strncat(dp->strnew[1], AREA_STRING,
				STRSIZE-strlen(dp->strnew[1]));
			dp->strpta[1] = dp->strnew[1];
			dp->strptb[1] = dp->strpta[1];
		} else if (dp->hiv) {
			convert(HIV_TIME_START + (HIV_MIN / 60.0) * timeNow, dp->strnew[1]);
			(void) strncat(dp->strnew[1], CASES_STRING,
				STRSIZE-strlen(dp->strnew[1]));
			dp->strpta[1] = dp->strnew[1];
			dp->strptb[1] = dp->strpta[1];
		} else if (dp->lab) {
			convert((LAB_MIN / 60.0) * (timeNow - timeAtLastNewYear(timeNow)),
				dp->strnew[1]);
			(void) strncat(dp->strnew[1], YEAR_STRING,
				STRSIZE-strlen(dp->strnew[1]));
			dp->strpta[1] = dp->strnew[1];
			dp->strptb[1] = dp->strpta[1];
		} else if (dp->veg) {
			convert((VEG_MIN / 60.0) * (timeNow - timeAtLastNewYear(timeNow)),
				dp->strnew[1]);
			(void) strncat(dp->strnew[1], YEAR_STRING,
				STRSIZE-strlen(dp->strnew[1]));
			dp->strpta[1] = dp->strnew[1];
			dp->strptb[1] = dp->strpta[1];
		} else if (dp->y2k) {
			if (Y2K_TIME_START >= timeLocal)
				dp->strpta[0] = (char *) Y2K_STRING;
			else
				dp->strpta[0] = (char *) POST_Y2K_STRING;
			dp->strptb[0] = dp->strpta[0];
			dayhrminsec(Y2K_TIME_START - timeLocal,
				dp->tzoffset, dp->strnew[1]);
			dp->strpta[1] = dp->strnew[1];
			dp->strptb[1] = dp->strpta[1];
		} else if (dp->millennium) {
			if (Y2001_TIME_START >= timeLocal)
				dp->strpta[0] = (char *) Y2001_STRING;
			else
				dp->strpta[0] = (char *) POST_Y2001_STRING;
			dp->strptb[0] = dp->strpta[0];
			dayhrminsec(Y2001_TIME_START - timeLocal,
				dp->tzoffset, dp->strnew[1]);
			dp->strpta[1] = dp->strnew[1];
			dp->strptb[1] = dp->strpta[1];
		} else if (dp->mayan) {
			if (MAYAN_TIME_START >= timeLocal)
				dp->strpta[0] = (char *) MAYAN_STRING;
			else
				dp->strpta[0] = (char *) POST_MAYAN_STRING;
			dp->strptb[0] = dp->strpta[0];
			dayhrminsec(MAYAN_TIME_START - timeLocal,
				dp->tzoffset, dp->strnew[1]);
			dp->strpta[1] = dp->strnew[1];
			dp->strptb[1] = dp->strpta[1];
		}
	  }
	}
	/* Recalculate string width since it can change */
	xold = dp->clockx;
	yold = dp->clocky;
	if (!dp->led && !dp->binary) {
		j = 0;
		for (i = 0; i < dp->lines; i++) {
			if (dp->strpta[i] && *(dp->strpta[i]))
				dp->textWidth[i] = getTextWidth(dp->strpta[i]);
			if (dp->textWidth[i] > dp->textWidth[j]) {
				j = i;
			}
		}
		dp->text_width = dp->textWidth[j];
		for (i = 0; i < dp->lines; i++) {
			dp->textStart[i] = (dp->text_width -
				dp->textWidth[i]) / 2;
		}
	}
	dp->width = MI_WIDTH(mi);
	dp->height = MI_HEIGHT(mi);
	dp->maxx = dp->width - dp->text_width;
	dp->maxy = dp->height - (dp->lines - 1) * dp->text_height -
		dp->text_descent;
	dp->clockx += dp->dx;
	dp->clocky += dp->dy;
	if (dp->maxx < dp->textStart[0]) {
		if (dp->clockx < dp->maxx + dp->textStart[0] ||
				dp->clockx > dp->textStart[0]) {
			dp->dx = -dp->dx;
			dp->clockx += dp->dx;
		}
	} else if (dp->maxx > dp->textStart[0]) {
		if (dp->clockx >= dp->maxx + dp->textStart[0] ||
				dp->clockx <= dp->textStart[0]) {
			dp->dx = -dp->dx;
			dp->clockx += dp->dx;
		}
	}
	if (dp->maxy < dp->text_ascent) {
		if (dp->clocky > dp->text_ascent || dp->clocky < dp->maxy) {
			dp->dy = -dp->dy;
			dp->clocky += dp->dy;
		}
	} else if (dp->maxy > dp->text_ascent) {
		if (dp->clocky > dp->maxy || dp->clocky < dp->text_ascent) {
			dp->dy = -dp->dy;
			dp->clocky += dp->dy;
		}
	}
	if (dp->pixw != dp->text_width ||
			dp->pixh != dp->lines * dp->text_height) {
		XGCValues gcv;

		if (dp->pixmap)
			MI_CLEARWINDOWCOLORMAPFAST(mi, gc, MI_BLACK_PIXEL(mi));
		free_dclock_screen(display, dp);
		dp->pixw = dp->text_width;
		if (dp->led)
			dp->pixh = dp->text_height;
		else
			dp->pixh = (dp->lines + DELTA) * dp->text_height;
		if ((dp->pixmap = XCreatePixmap(display, window,
				dp->pixw, dp->pixh, 1)) == None) {
			free_dclock_screen(display, dp);
			dp->pixw = 0;
			dp->pixh = 0;
			return;
		}
#ifndef USE_MB
		gcv.font = mode_font->fid;
#endif
		gcv.background = 0;
		gcv.foreground = 1;
		gcv.graphics_exposures = False;
		if ((dp->fgGC = XCreateGC(display, dp->pixmap,
			GCForeground | GCBackground | GCGraphicsExposures
#ifndef USE_MB
			| GCFont
#endif
			, &gcv)) == None) {
			free_dclock_screen(display, dp);
			dp->pixw = 0;
			dp->pixh = 0;
			return;
		}
		gcv.foreground = 0;
		if ((dp->bgGC = XCreateGC(display, dp->pixmap,
			GCForeground | GCBackground | GCGraphicsExposures
#ifndef USE_MB
			| GCFont
#endif
			, &gcv)) == None) {
			free_dclock_screen(display, dp);
			dp->pixw = 0;
			dp->pixh = 0;
			return;
		}
		if (!dp->binary)
			XSetLineAttributes(display, dp->fgGC,
				(unsigned int) (LED_WIDTH),
				LineSolid, CapButt, JoinMiter);
	}
	XFillRectangle(display, dp->pixmap, dp->bgGC,
		0, 0, dp->pixw, dp->pixh);
	if (dp->led) {
		int startx = (int) (LED_WIDTH / 2.0);
		int starty = (int) (LED_WIDTH / 2.0);

		drawanumber(mi, startx, starty, dp->str[11] - '0');
		startx += (int) (LED_XS + LED_WIDTH + LED_INC);
		drawanumber(mi, startx, starty, dp->str[12] - '0');
		startx += (int) (LED_XS + LED_WIDTH + LED_INC);
		drawacolon(mi, startx, starty);
		startx += (int) (LED_WIDTH + LED_INC);
		drawanumber(mi, startx, starty, dp->str[14] - '0');
		startx += (int) (LED_XS + LED_WIDTH + LED_INC);
		drawanumber(mi, startx, starty, dp->str[15] - '0');
		startx += (int) (LED_XS + LED_WIDTH + LED_INC);
		drawacolon(mi, startx, starty);
		startx += (int) (LED_WIDTH + LED_INC);
		drawanumber(mi, startx, starty, dp->str[17] - '0');
		startx += (int) (LED_XS + LED_WIDTH + LED_INC);
		drawanumber(mi, startx, starty, dp->str[18] - '0');
	} else if (dp->binary) {
		int startx = BINARY_WIDTH / 2;
		int starty = BINARY_HEIGHT / 2;

		/* hour - top number, 0, 1 or 2 */
		drawabinary(mi, startx, starty, 2, dp->str[11] - '0');
		startx += (BINARY_WIDTH + BINARY_INC);
		/* hour - bottom number 0 - 9 */
		drawabinary(mi, startx, starty, 4, dp->str[12] - '0');
		startx += (BINARY_WIDTH + BINARY_INC);
		/* minute - top number, 0 - 5 */
		drawabinary(mi, startx, starty, 3, dp->str[14] - '0');
		startx += (BINARY_WIDTH + BINARY_INC);
		/* minute - bottom number, 0 - 9 */
		drawabinary(mi, startx, starty, 4, dp->str[15] - '0');
		startx += (BINARY_WIDTH + BINARY_INC);
		/* second - top number, 0 - 5 */
		drawabinary(mi, startx, starty, 3, dp->str[17] - '0');
		startx += (BINARY_WIDTH + BINARY_INC);
		/* second - bottom number, 0 - 9 */
		drawabinary(mi, startx, starty, 4, dp->str[18] - '0');
		startx += (BINARY_WIDTH + BINARY_INC);
	} else {
		for (i = 0; i < dp->lines; i++) {
			DrawString(display, dp->pixmap, dp->fgGC,
				dp->textStart[i],
				dp->text_ascent + i * dp->text_height,
				dp->strpta[i], strlen(dp->strpta[i]));
		}
	}

	XSetForeground(display, gc, MI_BLACK_PIXEL(mi));
	/* This could leave screen dust on the screen if the width changes
	   But that only happens once a day... (well maybe not for counts)
	   ... this is solved by the ClearWindow above
	 */
	ERASE_IMAGE(display, window, gc,
		(dp->clockx - dp->textStart[0]),
		(dp->clocky - dp->text_ascent),
		(xold - dp->textStart[0]), (yold - dp->text_ascent),
		dp->pixw, dp->pixh);
	if (MI_NPIXELS(mi) > 2)
		XSetForeground(display, gc, MI_PIXEL(mi, dp->color));
	else
		XSetForeground(display, gc, MI_WHITE_PIXEL(mi));
	XCopyPlane(display, dp->pixmap, window, gc,
		0, 0, dp->text_width, (dp->lines + DELTA) * dp->text_height,
		dp->clockx - dp->textStart[0], dp->clocky - dp->text_ascent,
		1L);
}

ENTRYPOINT void
release_dclock(ModeInfo * mi)
{
	if (dclocks != NULL) {
		int screen;

		for (screen = 0; screen < MI_NUM_SCREENS(mi); screen++)
			free_dclock_screen(MI_DISPLAY(mi), &dclocks[screen]);
		free(dclocks);
		dclocks = (dclockstruct *) NULL;
	}
	if (mode_font != None) {
#ifdef USE_MB
		XFreeFontSet(MI_DISPLAY(mi), mode_font);
#else
		XFreeFont(MI_DISPLAY(mi), mode_font);
#endif
		mode_font = None;
	}
}

#if defined(HAVE_TZSET) && !(defined(BSD) && BSD >= 199306) && !(defined(__CYGWIN__))
extern long timezone;
#endif

ENTRYPOINT void
init_dclock(ModeInfo * mi)
{
	Display *display = MI_DISPLAY(mi);
	dclockstruct *dp;
	time_t timeNow, timeLocal;
	int i, j;

	MI_INIT(mi, dclocks);
	dp = &dclocks[MI_SCREEN(mi)];

#ifndef HAVE_SNPRINTF
	if (logbase == 0.0)
		logbase = log(BASE);
#endif
	dp->width = MI_WIDTH(mi);
	dp->height = MI_HEIGHT(mi);

	MI_CLEARWINDOW(mi);

	dp->binary = False;
	dp->led = False;
	dp->popex = False;
	dp->forest = False;
	dp->hiv = False;
	dp->lab = False;
	dp->veg = False;
	dp->time24 = False;
	dp->y2k = False;
	dp->millennium = False;
	dp->mayan = False;
	dp->lines = 2;
#if defined(MODE_dclock_y2k) && defined(MODE_dclock_millennium) && defined(MODE_dclock_mayan)
#define NUM_DCLOCK_MODES 12
#endif
#if (defined(MODE_dclock_y2k) && defined(MODE_dclock_millennium) && !defined(MODE_dclock_mayan)) || (defined(MODE_dclock_y2k) && !defined(MODE_dclock_millennium) && defined(MODE_dclock_mayan)) || (!defined(MODE_dclock_y2k) && defined(MODE_dclock_millennium) && defined(MODE_dclock_mayan))
#define NUM_DCLOCK_MODES 11
#endif
#if (!defined(MODE_dclock_y2k) && !defined(MODE_dclock_millennium) && defined(MODE_dclock_mayan)) || (!defined(MODE_dclock_y2k) && defined(MODE_dclock_millennium) && !defined(MODE_dclock_mayan)) || (defined(MODE_dclock_y2k) && !defined(MODE_dclock_millennium) && !defined(MODE_dclock_mayan))
#define NUM_DCLOCK_MODES 10
#endif
#if (!defined(MODE_dclock_y2k) && !defined(MODE_dclock_millennium) && !defined(MODE_dclock_mayan))
#define NUM_DCLOCK_MODES 9
#endif
#ifdef WIN32
	if (NRAND(2)) {
				dp->led = True;
	} else {
				dp->binary = True;
	}
#else
	if (!(message && *message)) {
	    if (MI_IS_FULLRANDOM(mi)) {
		switch (NRAND(NUM_DCLOCK_MODES)) {
			case 0:
				break;
			case 1:
				dp->led = True;
				break;
			case 2:
				dp->popex = True;
				break;
			case 3:
				dp->forest = True;
				break;
			case 4:
				dp->hiv = True;
				break;
			case 5:
				dp->lab = True;
				break;
			case 6:
				dp->veg = True;
				break;
			case 7:
				dp->time24 = True;
				break;
			case 8:
				dp->binary = True;
				break;
#if defined(MODE_dclock_y2k) || defined(MODE_dclock_millennium) || defined(MODE_dclock_mayan)
			case 9:
#ifdef MODE_dclock_y2k
				dp->y2k = True;
#else
#ifdef MODE_dclock_millennium
				dp->millennium = True;
#else
				dp->mayan = True;
#endif
#endif
				break;
#if (defined(MODE_dclock_y2k) && defined(MODE_dclock_millennium)) || (defined(MODE_dclock_millennium) && defined(MODE_dclock_mayan)) || (defined(MODE_dclock_mayan) && defined(MODE_dclock_y2k))
			case 10:
#if (defined(MODE_dclock_y2k) && defined(MODE_dclock_millennium))
				dp->millennium = True;
#else
				dp->mayan = True;
#endif
				break;
#if (defined(MODE_dclock_y2k) && defined(MODE_dclock_millennium) && defined(MODE_dclock_mayan))
			case 11:
				dp->mayan = True;
				break;
#endif
#endif
#endif
			default:
				break;
		}
	    } else { /* first come, first served */
		dp->binary = binary;
		dp->led = led;
		dp->popex = popex;
		dp->forest = forest;
		dp->hiv = hiv;
		dp->lab = lab;
		dp->veg = veg;
		dp->time24 = time24;
		dp->y2k = y2k;
		dp->millennium = millennium;
		dp->mayan = mayan;
	    }
	}
#endif

	if (mode_font == None) {
#ifdef USE_MB
		mode_font = getFontSet(display);
#else
		mode_font = getFont(display);
#endif
		if (mode_font == None) {
			release_dclock(mi);
			return;
		}
	}
	/* (void)time(&dp->timenew); */
#if defined(HAVE_TZSET) && (!defined(HAVE_TIMELOCAL) || (defined(BSD) && BSD >= 199306))
	(void) tzset();
#endif
	dp->timeold = dp->timenew = time((time_t *) NULL);
#if defined(HAVE_TIMELOCAL) && !(defined(BSD) && BSD >= 199306)
	dp->tzoffset = mktime(localtime(&dp->timeold)) -
		mktime(gmtime(&dp->timeold));
#else
#if defined(HAVE_TZSET)
	dp->tzoffset = (int)timezone;
#else
	dp->tzoffset = 0;
#endif
#endif
	if (dp->tzoffset > 86400 || dp->tzoffset < -86400)
		dp->tzoffset = 0;
	dp->str = ctime(&dp->timeold);
	dp->dx = (LRAND() & 1) ? 1 : -1;
	dp->dy = (LRAND() & 1) ? 1 : -1;

	timeNow = seconds();
	timeLocal = timeNow + dp->tzoffset;
	if (dp->led) {
		dp->lines = 2;
		/*dp->text_descent = 0;
		dp->text_ascent = 0;*/
		for (i = 0; i < 4; i++) {
			if (parallelogramUnit[i].x == 1)
				dp->parallelogram[i].x = (short) (LED_XS *
					LED_LEAN);
			else if (parallelogramUnit[i].x == 2)
				dp->parallelogram[i].x = (short) LED_XS;
			else if (parallelogramUnit[i].x == -1)
				dp->parallelogram[i].x = (short) (-LED_XS *
					LED_LEAN);
			else if (parallelogramUnit[i].x == -2)
				dp->parallelogram[i].x = (short) (-LED_XS);
			else
				dp->parallelogram[i].x = 0;
			dp->parallelogram[i].y = (short) (LED_YS *
					parallelogramUnit[i].y);
		}
		dp->parallelogram[0].x = (short) ((LED_XS * LED_LEAN) +
			LED_INC);
		dp->parallelogram[0].y = (short) LED_INC;
		dp->text_width = (short) (6 * (LED_XS + LED_WIDTH + LED_INC) +
			2 * (LED_WIDTH + LED_INC) + LED_XS * LED_LEAN - LED_INC);
		dp->text_height = (short) (LED_YS + LED_WIDTH + LED_INC);
		dp->maxy = dp->height - (dp->lines - 1) * dp->text_height;
		if (dp->maxy == 0)
			dp->clocky = 0;
		else if (dp->maxy < 0)
			dp->clocky = -NRAND(-dp->maxy);
		else
			dp->clocky = NRAND(dp->maxy);
	} else if (dp->binary) {
		dp->text_width = (6 * (BINARY_WIDTH + BINARY_INC)) +
			BINARY_WIDTH;
		dp->text_height = (4 * (BINARY_HEIGHT + BINARY_INC)) +
			BINARY_HEIGHT;
		dp->maxy = dp->height - (dp->lines - 1) * dp->text_height;
		if (dp->maxy == 0)
			dp->clocky = 0;
		else if (dp->maxy < 0)
			dp->clocky = -NRAND(-dp->maxy);
		else
			dp->clocky = NRAND(dp->maxy);
	} else {
#ifdef USE_MB
		dp->text_descent = 0;
		dp->text_ascent = getFontHeight(mode_font);
#else
		dp->text_descent = mode_font->descent;
		dp->text_ascent = mode_font->ascent;
#endif
		if (message && *message) {
			char *p, *p2, *p3, buf[BUFSIZ];
			int height, w;

			p = strncpy(buf, message, BUFSIZ - 1);
			while (strlen(p) > 0) {
				p2 = p + strlen(p) - 1;
				if (*p2 == '\n')
					*p2 = 0;
				else
					break;
			}
			for (height = 0; p && height < MAXLINES; height++) {
				if (!(p2 = (char *) strchr(p, '\n')) ||
						!p2[1]) {
					p2 = p + strlen(p);
				}
				/* p2 now points to the first '\n' */
				*p2 = 0;
				for (w = 0; w < p2 - p; w++) {
					p3 = p + w;
					if (*p3 == '\t') /* this should be improved */
						*p3 = ' ';
				}
				(void) strncpy(dp->strnew[height], p,
					MAXWIDTH - 1);
				dp->strpta[height] = dp->strnew[height];
				dp->strptb[height] = dp->strpta[height];
				dp->strpta[height][MAXWIDTH - 1] = '\0';
				if (p == p2)
					break;
				p = p2 + 1;
			}
			dp->lines = height;
		} else if (dp->popex) {
			dp->strpta[0] = (char *) POPEX_STRING;
			dp->strptb[0] = dp->strpta[0];
			convert(PEOPLE_TIME_START + (PEOPLE_MIN / 60.0) * timeNow, dp->strnew[1]);
			(void) strncat(dp->strnew[1], PEOPLE_STRING, STRSIZE-strlen(dp->strnew[1]));
			dp->strpta[1] = dp->strnew[1];
			dp->strptb[1] = dp->strpta[1];
		} else if (dp->forest) {
			dp->strpta[0] = (char *) FOREST_STRING;
			dp->strptb[0] = dp->strpta[0];
			convert(AREA_TIME_START - (AREA_MIN / 60.0) * timeNow, dp->strnew[1]);
			(void) strncat(dp->strnew[1], TROPICAL_STRING, STRSIZE-strlen(dp->strnew[1]));
			(void) strncat(dp->strnew[1], AREA_STRING, STRSIZE-strlen(dp->strnew[1]));
			dp->strpta[1] = dp->strnew[1];
			dp->strptb[1] = dp->strpta[1];
		} else if (dp->hiv) {
			dp->strpta[0] = (char *) HIV_STRING;
			dp->strptb[0] = dp->strpta[0];
			convert(HIV_TIME_START + (HIV_MIN / 60.0) * timeNow, dp->strnew[1]);
			(void) strncat(dp->strnew[1], CASES_STRING, STRSIZE-strlen(dp->strnew[1]));
			dp->strpta[1] = dp->strnew[1];
			dp->strptb[1] = dp->strpta[1];
		} else if (dp->lab) {
			dp->strpta[0] = (char *) LAB_STRING;
			dp->strptb[0] = dp->strpta[0];
			convert((LAB_MIN / 60.0) * (timeNow - timeAtLastNewYear(timeNow)),
				dp->strnew[1]);
			(void) strncat(dp->strnew[1], YEAR_STRING, STRSIZE-strlen(dp->strnew[1]));
			dp->strpta[1] = dp->strnew[1];
			dp->strptb[1] = dp->strpta[1];
		} else if (dp->veg) {
			dp->strpta[0] = (char *) VEG_STRING;
			dp->strptb[0] = dp->strpta[0];
			convert((VEG_MIN / 60.0) * (timeNow - timeAtLastNewYear(timeNow)),
				dp->strnew[1]);
			(void) strncat(dp->strnew[1], YEAR_STRING, STRSIZE-strlen(dp->strnew[1]));
			dp->strpta[1] = dp->strnew[1];
		} else if (dp->y2k) {
			if (Y2K_TIME_START >= timeLocal)
				dp->strpta[0] = (char *) Y2K_STRING;
			else
				dp->strpta[0] = (char *) POST_Y2K_STRING;
			dp->strptb[0] = dp->strpta[0];
			dayhrminsec(Y2K_TIME_START - timeLocal, dp->tzoffset, dp->strnew[1]);
			dp->strpta[1] = dp->strnew[1];
		} else if (dp->millennium) {
			if (Y2001_TIME_START >= timeLocal)
				dp->strpta[0] = (char *) Y2001_STRING;
			else
				dp->strpta[0] = (char *) POST_Y2001_STRING;
			dp->strptb[0] = dp->strpta[0];
			dayhrminsec(Y2001_TIME_START - timeLocal, dp->tzoffset, dp->strnew[1]);
			dp->strpta[1] = dp->strnew[1];
		} else if (dp->mayan) {
			if (MAYAN_TIME_START >= timeLocal)
				dp->strpta[0] = (char *) MAYAN_STRING;
			else
				dp->strpta[0] = (char *) POST_MAYAN_STRING;
			dp->strptb[0] = dp->strpta[0];
			dayhrminsec(MAYAN_TIME_START - timeLocal, dp->tzoffset, dp->strnew[1]);
			dp->strpta[1] = dp->strnew[1];
		} else {
			struct tm *t = localtime(&timeLocal);

			if (dp->time24)
			  (void) strftime(dp->strnew[0], STRSIZE, "%H:%M:%S", t);
			else
			  (void) strftime(dp->strnew[0], STRSIZE, "%I:%M:%S %p", t);
			dp->hour = t->tm_hour;
			(void) strftime(dp->strnew[1], STRSIZE, "%a %b %d %Y", t);

			dp->strpta[0] = dp->strnew[0];
			dp->strptb[0] = dp->strold[0];
			dp->strpta[1] = dp->strnew[1];
			dp->strptb[1] = dp->strold[1];
		}
		dp->text_height = getFontHeight(mode_font);
		j = 0;
		for (i = 0; i < dp->lines; i++) {
			if (dp->strpta[i] && *(dp->strpta[i]))
				dp->textWidth[i] = getTextWidth(dp->strpta[i]);
			if (dp->textWidth[i] > dp->textWidth[j]) {
				j = i;
			}
		}
		dp->text_width = dp->textWidth[j];
		for (i = 0; i < dp->lines; i++) {
			dp->textStart[i] = (dp->text_width -
				dp->textWidth[i]) / 2;
		}
		dp->maxy = dp->height - (dp->lines - 1) * dp->text_height -
			dp->text_descent;
		if (dp->maxy - dp->text_ascent == 0)
			dp->clocky = dp->text_ascent;
		else if (dp->maxy - dp->text_ascent < 0)
			dp->clocky = -NRAND(dp->text_ascent - dp->maxy) +
				dp->text_ascent;
		else
			dp->clocky = NRAND(dp->maxy - dp->text_ascent) +
				dp->text_ascent;
	}
	dp->maxx = dp->width - dp->text_width;
	if (dp->maxx == 0)
		dp->clockx = dp->textStart[0];
	else if (dp->maxx < 0)
		dp->clockx = -NRAND(-dp->maxx) + dp->textStart[0];
	else
		dp->clockx = NRAND(dp->maxx) + dp->textStart[0];

	if (MI_NPIXELS(mi) > 2)
		dp->color = NRAND(MI_NPIXELS(mi));

	/* don't want any exposure events from XCopyPlane */
	XSetGraphicsExposures(display, MI_GC(mi), False);
}

ENTRYPOINT void
draw_dclock(ModeInfo * mi)
{
	dclockstruct *dp;

	if (dclocks == NULL)
		return;
	dp = &dclocks[MI_SCREEN(mi)];
	if (mode_font == None)
		return;

	MI_IS_DRAWN(mi) = True;
	drawDclock(mi);
	if (MI_NPIXELS(mi) > 2) {
		if (++dp->color >= MI_NPIXELS(mi))
			dp->color = 0;
	}
}

#ifndef STANDALONE
ENTRYPOINT void
refresh_dclock(ModeInfo * mi)
{
	MI_CLEARWINDOW(mi);
}
#endif

XSCREENSAVER_MODULE ("Dclock", dclock)

#endif /* MODE_dclock */
