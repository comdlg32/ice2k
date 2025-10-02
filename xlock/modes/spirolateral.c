/* -*- Mode: C; tab-width: 4 -*- */
/* spirolateral */

#if 0
static const char sccsid[] = "@(#)spirolateral.c	5.00 2025/05/08 xlockmore";

#endif

/*-
 * Copyright (c) 2025 by David Bagley.
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
 * 08-May-2025: To me its a cross between a spirograph and turtle
 *
 * Based on Martin Gardner's chapter on "Worm Paths" in his book
 * "Knotted Doughnuts".
 *
 */

#ifdef STANDALONE
#define MODE_spirolateral
#define DEFAULTS "*delay: 40000 \n" \
	"*count: -16 \n" \
	"*cycles: -32 \n" \
	"*ncolors: 64 \n" \

# define free_spirolateral 0
# define reshape_spirolateral 0
# define spirolateral_handle_event 0
#define UNIFORM_COLORS
#include "xlockmore.h"		/* from the xscreensaver distribution */
#else /* !STANDALONE */
#include "xlock.h"		/* from the xlockmore distribution */
#endif /* !STANDALONE */

#ifdef MODE_spirolateral

#define DEF_SHARPNESS "0"

static int sharpness;

static XrmOptionDescRec opts[] =
{
	{(char *) "-sharpness", (char *) ".spirolateral.sharpness", XrmoptionSepArg, (caddr_t) NULL}
};

static argtype vars[] =
{
	{(void *) & sharpness, (char *) "sharpness", (char *) "Sharpness", (char *) DEF_SHARPNESS, t_Int}
};

static OptionStruct desc[] =
{
	{(char *) "-sharpness num", (char *) "Sharpness"}
};

ENTRYPOINT ModeSpecOpt spirolateral_opts =
{sizeof opts / sizeof opts[0], opts, sizeof vars / sizeof vars[0], vars, desc};

#ifdef USE_MODULES
ModStruct   spirolateral_description =
{"spirolateral", "init_spirolateral", "draw_spirolateral", "release_spirolateral",
 "refresh_spirolateral", "init_spirolateral", (char *) NULL, &spirolateral_opts,
 40000, -16, -32, 1, 64, 1.0, "",
 "Shows Odds' Spirolaterals", 0, NULL};

#endif

#define MIN_DIVISION 3
#define MIN_ORDER 2
#define TWOPI (2.0*M_PI)	/* for convenience */
#define DRAW 0
#define SLEEP 1
#define ERASE 2
#define SLEEP_COUNT 64
#define DEF_DIVISION_SIZE 16
#define DEF_ORDER_SIZE 16

typedef struct {
	float x, y;
} Locations;

typedef struct {
	float sin, cos;
} Angles;

typedef struct {
	int distance, size, division, width, height, lineWidth;
	int counter, heading, order, guessedLength, maxLength;
	int mode, table, altCount;
	Locations *pos;
	Angles *angles;
	float startColors, colors;
	int *altDir;
	int nAltDir;
} spirolateralstruct;

typedef struct {
	int division, order, sharpness, length;
	int altDir[DEF_DIVISION_SIZE];
} complexstruct;

/*
  (divisions - 2) * (order - 2 * altDir) / 2 * divisions) = a / b
  if integer either does not close or closes in just one cycle
  result noninteger will close after b cycles */

static complexstruct rules[] =
{
	{ /* 0: Figure 122 left */
		4, 9, 1, 36,
		{6, 0}
	},
	{ /* 1: Figure 122 right */
		4, 8, 1, 15,
		{1, 4, 8, 0}
	},
#ifdef SIMPLE
	{ /* Figure 123 (a) */
		5, 3, 2, 15,
		{0}
	},
#endif
	{ /* 2: Figure 123 (b) */
		8, 3, 3, 24,
		{1, 0}
	},
#ifdef SIMPLE
	{ /* Figure 123 (c) */
		3, 2, 1, 6,
		{0}
	},
#endif
	{ /* 3: Figure 123 (d) */
		3, 5, 1, 15,
		{1, 2, 0}
	},
#ifdef NO_CLOSE
	{ /* Figure 123 (e) */
		3, 5, 1, 99,
		{3, 0}
	},
	{ /* Figure 123 (f) */
		3, 5, 1, 99,
		{2, 0}
	},
#endif
	{ /* 4: Pg 208 first */
		4, 7, 1, 28,
		{1, 2, 3, 0}
	},
	{ /* 5: Pg 208 second */
		4, 7, 1, 28,
		{5, 6, 7, 0}
	},

	/* my junk */

	{ /* Variations on Figure 122 left */
		4, 9, 1, 36,
		{1, 0}
	},
	{
		4, 9, 1, 36,
		{2, 0}
	},
	{
		4, 9, 1, 36,
		{3, 0}
	},
	{
		4, 9, 1, 36,
		{4, 0}
	},
	{
		4, 9, 1, 36,
		{5, 0}
	},
	{
		4, 9, 1, 36,
		{7, 0}
	},
	{
		4, 9, 1, 36,
		{8, 0}
	},
	{
		4, 9, 1, 36,
		{9, 0}
	},
	{ /* Variations on Figure 123 (b) */
		8, 3, 3, 24,
		{2, 0}
	},
	{
		8, 3, 3, 24,
		{3, 0}
	},
	{ /* Variation on Figure 123 (e, f) */
		3, 5, 1, 15,
		{2, 3, 0}
	},
	/* others */
	{
		5, 12, 1, 60,
		{6, 7, 0}
	},
	{
		4, 31, 1, 124,
		{5, 11, 27, 0}
	},
	{
		4, 15, 1, 68,
		{4, 0}
	},
	{
		4, 11, 1, 44,
		{0}
	},
};

static spirolateralstruct *spirolaterals = (spirolateralstruct *) NULL;

static Bool
floatEqual(float a, float b)
{
	return (ABS(a - b) < 0.01);
}

static Bool
locationEqual(spirolateralstruct *sp, int i, int counter)
{
	return (floatEqual(sp->pos[i].x, sp->pos[counter].x)
		&& floatEqual(sp->pos[i].y, sp->pos[counter].y));
}

static int
relativelyPrimeDenom(int num, int denom)
{
	int newNum = num;
	int newDenom = denom;
	int i, min = MIN(num, denom);
	for (i = min; i > 1; i--) {
		if ((newNum % i == 0) && (newDenom % i == 0)) {
			newNum = newNum / i;
			newDenom = newDenom / i;
		}
	}
	return newDenom;
}

static int
randomHalfRelativelyPrime(int n)
{
	int i, count = 1, number;
	/* find amount to choose from */
	if (n < 5)
		return 1;
	for (i = 2; i < (n + 1) / 2; i++) {
		if (relativelyPrimeDenom(i, n) == n) {
			count++;
		}
	}
	number = count;
	if (number != 1)
		number = NRAND(number) + 1;
	if (number == 1)
		return 1;
	count = 1;
	/* loop again to get actual value */
	for (i = 2; i < (n + 1) / 2; i++) {
		if (relativelyPrimeDenom(i, n) == n) {
			count++;
			if (number == count)
				return i;
		}
	}
	return 1; /* how did it get here? */
}

static void
randomAltDir(spirolateralstruct *sp)
{
	int i, chance = 8;
	int maxCount = (sp->order - 1) / 2;
	sp->nAltDir = 0;
	for (i = 1; i <= sp->order && sp->nAltDir < maxCount; i++) {
		if (NRAND(chance) == 0) {
			sp->altDir[sp->nAltDir++] = i;
		}
	}
}

static void
free_spirolateral_screen(spirolateralstruct *sp) {
	if (sp == NULL) {
		return;
	}
	if (sp->pos)
		free(sp->pos);
	if (sp->angles)
		free(sp->angles);
	if (sp->altDir)
		free(sp->altDir);
	sp = NULL;
}

ENTRYPOINT void
init_spirolateral(ModeInfo * mi)
{
	spirolateralstruct *sp;
	float heading;
	int i, hand, sharp, num;

	MI_INIT(mi, spirolaterals);
	sp = &spirolaterals[MI_SCREEN(mi)];

	sp->width = MI_WIDTH(mi);
	sp->height = MI_HEIGHT(mi);

	MI_CLEARWINDOW(mi);

	if (MI_COUNT(mi) <= -MIN_DIVISION) {
		sp->division = NRAND(-MI_COUNT(mi) - MIN_DIVISION) + MIN_DIVISION;
		if (sp->division == MIN_DIVISION) {/* try again, 3 kind of boring, can happen in other ways though */
			sp->division = NRAND(-MI_COUNT(mi) - MIN_DIVISION) + MIN_DIVISION;
		}
	} else if (MI_COUNT(mi) < MIN_DIVISION) {
		sp->division = NRAND(DEF_DIVISION_SIZE) + MIN_DIVISION; /* for a triangle and above */
	} else {
		sp->division = MI_COUNT(mi);
	}
	if (MI_CYCLES(mi) <= -MIN_ORDER) {
		sp->order = NRAND(-MI_CYCLES(mi) - MIN_ORDER) + MIN_ORDER;
	} else if (MI_CYCLES(mi) < MIN_ORDER) {
		sp->order = NRAND(DEF_ORDER_SIZE) + MIN_ORDER; /* less and we get a rectangle */
	} else {
		sp->order = MI_CYCLES(mi);
	}
	if (sp->altDir) {
		free(sp->altDir);
		sp->altDir = NULL;
	}
	if (!sp->altDir) {
		if ((sp->altDir = (int *) calloc(((sp->order + 1) / 2),
				sizeof (int))) == NULL) {
			return;
		}
	}
	randomAltDir(sp);
	sharp = sharpness;
	if (sharpness < 1 || sharpness >= (sp->division + 1) / 2) {
		if (sp->division < 5)
			sharp = 1;
		else
			sharp = randomHalfRelativelyPrime(sp->division);
	}

	while ((sp->division % ((sp->order - 2 * sp->nAltDir) * sharp) == 0) /* trivial loop */
			|| (((sp->order - 2 * sp->nAltDir) * sharp) % sp->division == 0)) /* never closes */
		sp->order++;


#if 1
	sp->table = -1;
#else
	/* helpful if you want to see a specific one */
	sp->table = 6;
#endif
	if (sp->table >= 0) {
		int i = 0;
		sp->division = rules[sp->table].division;
		sp->order = rules[sp->table].order;
		sharp = rules[sp->table].sharpness;
		if (sp->altDir) {
			free(sp->altDir);
			sp->altDir = NULL;
		}
		if (!sp->altDir) {
			if ((sp->altDir = (int *) calloc(((sp->order + 1) / 2),
					sizeof (int))) == NULL) {
				return;
			}
		}
		while (rules[sp->table].altDir[i] != 0) {
			sp->altDir[i] = rules[sp->table].altDir[i];
			i++;
		}
		sp->nAltDir = i;
	}
	/* McMahon's numerator */
	num = (sp->division - 2) * (sp->order - 2 * sp->nAltDir);
	/* guessed length sometimes double or half actual, used for colors */
	sp->guessedLength = sp->order * relativelyPrimeDenom(num, 2 * sp->division);
	/* better to match than worry about repeating colors */
	if (sp->guessedLength % 2 == 0)
		sp->guessedLength /= 2;
	sp->size = MAX(1, 5 * sharp * MIN(sp->width, sp->height)
		/ (4 * sp->order * sp->division));
	sp->lineWidth = MAX(1, sp->size / (4 * sharp));
	sp->counter = 0;
	sp->heading = 0;
	sp->altCount = 0;
	if (sp->pos) {
		free(sp->pos);
		sp->pos = NULL;
	}
	if (sp->angles) {
		free(sp->angles);
		sp->angles = NULL;
	}
	sp->maxLength = 2 * sp->division * sp->order;
	if (!sp->pos) {
		if ((sp->pos = (Locations *) malloc((sp->maxLength + 1) *
				sizeof (Locations))) == NULL) {
			return;
		}
	}
	if (!sp->angles) {
		if ((sp->angles = (Angles *) malloc(sp->division *
				sizeof (Angles))) == NULL) {
			return;
		}
	}
	if (MI_NPIXELS(mi) > 2)
		sp->startColors = (float) NRAND(MI_NPIXELS(mi));
	sp->colors = sp->startColors;

	sp->pos[0].x = (float) sp->width / 2;
	sp->pos[0].y = (float) sp->height / 2;
	heading = (TWOPI / 360.0) * NRAND(360);
	hand = (NRAND(2) ? 1 : -1);
	if (MI_IS_VERBOSE(mi)) {

		int i = 0;
		(void) printf("division %d, order %d, sharp %d, hand %d, nAltDir %d, altDir \"",
			sp->division, sp->order, sharp, hand, sp->nAltDir);
		while (sp->altDir[i] != 0) {
			(void) printf("%d", sp->altDir[i++]);
			if (sp->altDir[i] != 0)
				(void) printf(",");
		}
		(void) printf("\", guessedLength %d, size %d\n", sp->guessedLength, sp->size);

	}
	sp->distance = 1;
	sp->mode = DRAW;
	for (i = 0; i < sp->division; i++) {
		float angle;

		angle = hand * TWOPI * i * sharp
			/ sp->division + heading;
		if (angle >= TWOPI)
			angle -= TWOPI;
		if (angle < 0.0)
			angle += TWOPI;
		sp->angles[i].sin = SINF(angle);
		sp->angles[i].cos = COSF(angle);
	}
}

ENTRYPOINT void
draw_spirolateral(ModeInfo * mi)
{
	Display    *display = MI_DISPLAY(mi);
	GC          gc = MI_GC(mi);
	int         angle;
	spirolateralstruct *sp;

	if (spirolaterals == NULL)
		return;
	sp = &spirolaterals[MI_SCREEN(mi)];
	if (sp->pos == NULL)
		return;

	MI_IS_DRAWN(mi) = True;
	XSetLineAttributes(display, gc, sp->lineWidth,
		LineSolid, CapRound, JoinRound);

	sp->counter++;
	if (sp->mode == ERASE) {
		int offset = sp->counter - sp->maxLength - SLEEP_COUNT;
		if (offset > sp->maxLength) {
			init_spirolateral(mi);
			return;
		}
		XDrawLine(display, MI_WINDOW(mi), gc,
			(int) sp->pos[offset - 1].x,
			(int) sp->pos[offset - 1].y,
			(int) sp->pos[offset].x,
			(int) sp->pos[offset].y);
		return;
	}
	if (sp->counter >= sp->maxLength + SLEEP_COUNT) {
		XSetForeground(display, gc, MI_BLACK_PIXEL(mi));
		if (NRAND(5) == 0 && sp->counter < 128) /* sometimes undraw if not too long  */
			sp->mode = ERASE;
		else
			init_spirolateral(mi);
		return;
	}
	if (sp->mode == SLEEP) {
		return;
	}
	if (sp->counter > 1 &&
			(sp->counter > sp->maxLength)) {
		if (MI_IS_VERBOSE(mi)) {
			(void) printf("counter %d, division %d, order %d\n",
				sp->counter, sp->division, sp->order);
		}
		sp->mode = SLEEP;
		return;
	}
	if (((sp->counter - 1) % sp->order) + 1 == sp->altDir[sp->altCount]) {
		sp->heading = (sp->division + sp->heading - 1) % sp->division;
		sp->altCount++;
	} else {
		sp->heading = (sp->heading + 1) % sp->division;
	}
	if ((sp->counter) % sp->order == 0)
		sp->altCount = 0;
	angle = (sp->heading) % sp->division;
	sp->pos[sp->counter].x = sp->pos[sp->counter - 1].x
		+ sp->angles[angle].sin
		* (float) (sp->size * sp->distance);
	sp->pos[sp->counter].y = sp->pos[sp->counter - 1].y
		+ sp->angles[angle].cos
		* (float) (sp->size * sp->distance);
	sp->distance++;
	if (sp->distance > sp->order)
		sp->distance = 1;
	/* matching just 1 point has incomplete drawings for division 4 */
	if (sp->counter > 1 &&
			(locationEqual(sp, 0, sp->counter - 1) &&
			locationEqual(sp, 1, sp->counter))) {
		if (MI_IS_VERBOSE(mi)) {
			(void) printf("counter %d, heading %d, altCount %d, division %d order %d\n",
				sp->counter - 1, sp->heading, sp->altCount, sp->division, sp->order);
		}
		sp->maxLength = sp->counter - 1;
		sp->mode = SLEEP;
		return;
	}
	if (MI_NPIXELS(mi) > 2)
		XSetForeground(display, gc, MI_PIXEL(mi, (int) sp->colors));
        else
                XSetForeground(display, gc, MI_WHITE_PIXEL(mi));

	XDrawLine(display, MI_WINDOW(mi), gc,
		(int) sp->pos[sp->counter - 1].x,
		(int) sp->pos[sp->counter - 1].y,
		(int) sp->pos[sp->counter].x,
		(int) sp->pos[sp->counter].y);
	sp->colors += (float) MI_NPIXELS(mi) / ((float) (sp->guessedLength));
        if (sp->colors >= (float) MI_NPIXELS(mi))
                sp->colors -= MI_NPIXELS(mi);
	XSetLineAttributes(display, gc, 1, LineSolid, CapButt, JoinMiter);
}

ENTRYPOINT void
release_spirolateral(ModeInfo * mi)
{
	if (spirolaterals != NULL) {
		int         screen;

		for (screen = 0; screen < MI_NUM_SCREENS(mi); screen++) {
			free_spirolateral_screen(&spirolaterals[screen]);

		}
		free(spirolaterals);
		spirolaterals = (spirolateralstruct *) NULL;
	}
}

#ifndef STANDALONE
ENTRYPOINT void
refresh_spirolateral(ModeInfo * mi)
{
	Display    *display = MI_DISPLAY(mi);
	GC          gc = MI_GC(mi);
	spirolateralstruct *sp;

	if (spirolaterals == NULL)
		return;
	sp = &spirolaterals[MI_SCREEN(mi)];
	if (sp->pos == NULL)
		return;

	MI_CLEARWINDOW(mi);
	if (sp->mode == DRAW) {
		int i;
		XSetLineAttributes(display, gc, sp->lineWidth,
			LineSolid, CapRound, JoinRound);
		sp->colors = sp->startColors;
		for (i = 0; i < sp->counter; i++) {
			if (MI_NPIXELS(mi) > 2)
				XSetForeground(display, gc, MI_PIXEL(mi, (int) sp->colors));
			else
				XSetForeground(display, gc, MI_WHITE_PIXEL(mi));
			XDrawLine(display, MI_WINDOW(mi), gc,
				(int) sp->pos[i].x,
				(int) sp->pos[i].y,
				(int) sp->pos[i + 1].x,
				(int) sp->pos[i + 1].y);
			sp->colors += (float) MI_NPIXELS(mi) / ((float) (sp->division * sp->order));
			if (sp->colors >= (float) MI_NPIXELS(mi))
				sp->colors -= MI_NPIXELS(mi);
		}
		XSetLineAttributes(display, gc, 1,
			LineSolid, CapButt, JoinMiter);
	} else
		init_spirolateral(mi);
}
#endif

XSCREENSAVER_MODULE ("Spirolateral", spirolateral)

#endif /* MODE_spirolateral */
