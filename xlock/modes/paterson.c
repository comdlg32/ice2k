/* -*- Mode: C; tab-width: 4 -*- */
#if 0
static const char sccsid[] = "@(#)paterson.c	5.84 2025/06/08 xlockmore";

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
 * 08-May-2025: similar to ant but worms die if they are force to
 *              repeat paths
 *
 * Based on Martin Gardner's chapter on "Worm Paths" in his book
 * "Knotted Doughnuts".  Beeler builds on work from Paterson and
 * Conway called "Paterson's Worm" (worm mode already taken).
 *
 * And this was helful too: https://www.mathpuzzle.com/Worms.html
 *
 */

/*-
  Grid     Number of Neighbors
  ----     ------------------
  Triangle 3
  Square   4
  Hexagon  6
*/

#ifdef STANDALONE
# define MODE_paterson
# define DEFAULTS	"*delay: 50000 \n" \
			"*count: -3 \n" \
			"*cycles: 2000 \n" \
			"*size: -24 \n" \
			"*ncolors: 64 \n" \
			"*fullrandom: True \n" \
			"*verbose: False \n" \
			"*fpsSolid: True \n" \

# define reshape_paterson 0
# define paterson_handle_event 0
# include "xlockmore.h"		/* in xscreensaver distribution */
#else /* STANDALONE */
# include "xlock.h"		/* in xlockmore distribution */
#endif /* STANDALONE */
#include "automata.h"

#ifdef MODE_paterson

#define DEF_LABEL "True"
#define FONT_HEIGHT 19
#define FONT_WIDTH 15
/*-
 * neighbors of 0 randomizes it for 3, 4, 6
 */

#define DEF_NEIGHBORS  "0"      /* choose random value */
#define DEF_RULE "" /* All rules */
#define DEF_VERTICAL "False"

static Bool label;
static int  neighbors;
static char *rule;
static Bool vertical;

static XrmOptionDescRec opts[] =
{
	{(char *) "-label", (char *) ".paterson.label", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+label", (char *) ".paterson.label", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-neighbors", (char *) ".paterson.neighbors", XrmoptionSepArg, (caddr_t) NULL},
	{(char *) "-rule", (char *) ".paterson.rule", XrmoptionSepArg, (caddr_t) NULL},
	{(char *) "-vertical", (char *) ".paterson.vertical", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+vertical", (char *) ".paterson.vertical", XrmoptionNoArg, (caddr_t) "off"}
};
static argtype vars[] =
{
	{(void *) & label, (char *) "label", (char *) "Label", (char *) DEF_LABEL, t_Bool},
	{(void *) & neighbors, (char *) "neighbors", (char *) "Neighbors", (char *) DEF_NEIGHBORS, t_Int},
	{(void *) & rule, (char *) "rule", (char *) "Rule", (char *) DEF_RULE, t_String},
	{(void *) & vertical, (char *) "vertical", (char *) "Vertical", (char *) DEF_VERTICAL, t_Bool}
};
static OptionStruct desc[] =
{
	{(char *) "-/+label", (char *) "turn on/off rule labeling"},
	{(char *) "-neighbors num", (char *) "triangles 3, squares 4, hexagons 6"},
	{(char *) "-rule string", (char *) "string for Gardner/Beeler Notation"},
	{(char *) "-/+vertical", (char *) "change orientation for hexagons and triangles"}
};

ENTRYPOINT  ModeSpecOpt paterson_opts =
{sizeof opts / sizeof opts[0], opts, sizeof vars / sizeof vars[0], vars, desc};

#ifdef USE_MODULES
const ModStruct paterson_description =
{"paterson", "init_paterson", "draw_paterson", "release_paterson",
 "init_paterson", "init_paterson", "free_paterson", &paterson_opts,
 2000, -3, 50000, -24, 64, 1.0, "",
 "Shows Paterson's Worm Paths", 0, NULL};

#endif

#define MINWORMS 1
#define MINGRIDSIZE 24
#define MINSIZE 1
#define MINRANDOMSIZE 5
#define ANGLES 360
#define SLEEP_COUNT 32
#define RULE2_SIZE 5
#define RULE3_SIZE 10
#define RULE4_SIZE 10
#define RULE_SIZE (1 + RULE2_SIZE + RULE3_SIZE + RULE4_SIZE)

#define WHITE (MI_NPIXELS(mi))
#define RED (0)
#define ORANGE (3 * MI_NPIXELS(mi) / 32)
#define YELLOW (MI_NPIXELS(mi) / 6)
#define GREEN (23 * MI_NPIXELS(mi) / 64)
#define CYAN (MI_NPIXELS(mi) / 2)
#define BLUE (45 * MI_NPIXELS(mi) / 64)
#define MAGENTA (53 * MI_NPIXELS(mi) / 64)

int internetRule[RULE_SIZE];
Bool gardner;
int gardnerRule1;
int gardnerRule2[RULE2_SIZE]; /* 1, many possible for multiple worms */
int gardnerRule3[RULE3_SIZE]; /* 4, more possible for multiple worms */
int gardnerRule4[RULE4_SIZE]; /* 1, many possible for multiple worms */
char newGardnerRule[4 + RULE_SIZE];

typedef struct {
	int	    alive;
	int         oldcol, oldrow;
	int         newcol, newrow;
	int         newaltcol, newaltrow; // for edges
	short       direction;
	int         color;
	int         state[RULE_SIZE];
	int         pathMask2[RULE2_SIZE];
	int         pathMask3[RULE3_SIZE];
	int         pathMask4[RULE4_SIZE];
	int         positions;
	int         pathMask;
	int         internetRule[RULE_SIZE];
	char	    internetRuleString[80];
} wormstruct;

typedef struct {
	Bool        painted, vertical;
	int         neighbors, polygon;
	int         generation, mode;
	int         left;
	int         xs, ys;
	int         xb, yb;
	int         nrows, ncols;
	int         width, height;
	unsigned char ncolors;
	unsigned char colors[7];
	int         n, sleepCount;
	unsigned char *field;
	wormstruct  *worms;
	int	    labelOffsetX, labelOffsetY;
	char	    gardnerRuleString[80];
	char	    internetRuleLabel[80], gardnerRuleLabel[80];
	union {
		XPoint      triangle[2][4];
		XPoint      pentagon[4][6];
		XPoint      hexagon[7];
	} shape;
#ifdef STANDALONE
	eraser_state *eraser;
#endif
} patersonstruct;

#if 0
/* Quadrille worm */
static int quadChoice[2] = {3, 0};
/* Isometric worm */
static int field1Choice[2] = {1, 2};
#endif
static int field2Choice[5][4] =
{
	{4, 5, 0, 1},
	{4, 5, 0, 2},
	{4, 5, 1, 2},
	{4, 0, 1, 2},
	{5, 0, 1, 2}
};
/* not sure of the logic of the order in book */
static int field3Choice[10][3] =
{
#if 0
	{4, 5, 3}, /* 4.1 sharp */
	{4, 5, 2}, /* 4.2 gentle */
	/*{4, 0, 2}, impossible opposite */
	{5, 0, 2}, /* 2.2 gentle */

	{4, 5, 2}, /* 3.1 sharp */
	{4, 0, 2}, /* 3.2 gentle */
	/*{5, 0, 2}, impossible opposite */

	{4, 1, 2}, /* 2.1 sharp */
	{5, 1, 2}, /* 1.2 gentle */

	{0, 1, 2} /* 1.1 sharp */
#else
	{0, 1, 2},
	{2, 5, 1},

	{4, 1, 2},
	{0, 5, 1},

	{4, 5, 2},
	{0, 4, 2},

	{4, 5, 0},
	{4, 5, 1}
#endif
};
static int field4Choice[10][2] =
{
	{4, 5},
	{4, 0},
	{4, 1},
	{4, 2},

	{5, 0},
	{5, 1},
	{5, 2},

	{0, 1},
	{0, 2},

	{1, 2}
};
#if 0
static int mask2Choice[5] = {51, 53, 54, 23, 39};
static int mask3Choice[10] = {7, 38, 22, 35, 52, 21, 49, 50, 37, 19};
static int mask4Choice[10] = {48, 17, 18, 20, 33, 34, 36, 3, 5, 6};
#endif
/* following is inverse of other choice 2-4 */
static int invMaskChoice[64] = {
	-1, -1, -1, 7,   -1, 8, 9, 0,
	-1, -1, -1, -1,  -1, -1, -1, -1,
	-1, 1, 2, 9,  3, 5, 2, 3,
	-1, -1, -1, -1,  -1, -1, -1, -1,

	-1, 4, 5, 3,  6, 8, 1, 4,
	-1, -1, -1, -1,  -1, -1, -1, -1,
	0, 6, 7, 0,  4, 1, 2, -1,
	-1, -1, -1, -1,  -1, -1, -1, -1
};

#if 0
/* Gardner's notation */
char trimmed_triangle[] = "1a2b3cac4b";
char curlyq_maze[] = "1a2b3caca4b";
char radioactivity[] = "1b2(ac)34a"; /* "1b2(ac)3(abc)(abc)(abc)(abc)4" */
char zipper[] = "1b2d3b4"; /* "1b2d3(abc)(abc)b4" */
char three_pointed_spiral[] = "1a2c3acba4a";
char shoot_grower[] = "1a2d3caaa4b";
char long_one[] = "1a2d3cbac4b"; /* 220142 */
char unknown_length[] = "1b2a3bcaa4b";
char cloud_path[] = "1a2a3baac4(ab)"; /* "1a2abaa3c4(ab)" ? */
char superdoily[] = "1a2d3cbaa4b";
char x[] = "1a2c3b4b";

/* Internet notation */
char neighbors_3_rule[] = "1";
char neighbors_4_rule0[] = "1,0";
char neighbors_4_rule3[] = "1,3";
char x[] = "1,0,5,1";
char radioactivity[] = "2,0,0";
char same_start0[] = "1,0,4,0,1,5";
char same_start1[]= "1,0,4,0,1,0,1";
char same_start2[]= "1,0,4,0,1,0,2";
char same_start3[]= "1,0,4,0,1,0,5";
char curlyq_maze[]= "1,5,2,0,1,2,1";
char superdoily2[]=  "1,4,5,1,4,1,2"; /* 1a2a3cbca4a */
char longest_known[] = "1,0,4,2,0,2,0"; /* 5.7 * 10^13 steps */
char unknown[] = "1,0,4,2,0,1,5";
char arule[] = "1,2,2,5,4,4,2";
#endif
/*
	1. if no segments eaten, turn right.
	2. if all segments eated, die.
	3. if only one segment uneaten, take it.
	4. Otherwise follow rule.
*/
/* if hexagonal only one path (no choices)
   if square only 2 paths (only one choice),
   if triangular 299 distinct paths (less as some die before they get too far)
   (ideas: allow directed paths for triangular and paths through connected by Cairo pattern)
*/

#define NEIGHBORKINDS ((long) (sizeof plots / sizeof *plots))

static patersonstruct *patersons = (patersonstruct *) NULL;

static int
deshift(int mask)
{
	int value = mask, count = -1;
	while (value > 0) {
		value = (value >> 1);
		count++;
	}
	return count;
}

static int
heading(int n, int direction, int turn)
{
	int chgDir;

	chgDir = (2 * ANGLES - turn) % ANGLES;
	return (chgDir + direction) % ANGLES;
}

static void
nextPosition(patersonstruct *pp, int dir, int *pcol, int *prow, int *pcol2, int *prow2)
{
	int col = *pcol, row = *prow;
	int col2 = col, row2 = row;

	if (pp->neighbors == 3 || pp->neighbors == 4 || pp->neighbors == 6) {
		switch (dir) {
		case 0:
			col2 = col + 1;
			col = (col + 1 == pp->ncols) ? 0 : col + 1;
			break;
		case 60:
			if (!(row & 1)) {
				col2 = col + 1;
				col = (col + 1 == pp->ncols) ? 0 : col + 1;
			}
			row2 = row - 1;
			row = (!row) ? pp->nrows - 1 : row - 1;
			break;
		case 90:
			row2 = row - 1;
			row = (!row) ? pp->nrows - 1 : row - 1;
			break;
		case 120:
			if (row & 1) {
				col2 = col - 1;
				col = (!col) ? pp->ncols - 1 : col - 1;
			}
			row2 = row - 1;
			row = (!row) ? pp->nrows - 1 : row - 1;
			break;
		case 180:
			col2 = col - 1;
			col = (!col) ? pp->ncols - 1 : col - 1;
			break;
		case 240:
			if (row & 1) {
				col2 = col - 1;
				col = (!col) ? pp->ncols - 1 : col - 1;
			}
			row2 = row + 1;
			row = (row + 1 == pp->nrows) ? 0 : row + 1;
			break;
		case 270:
			row2 = row + 1;
			row = (row + 1 == pp->nrows) ? 0 : row + 1;
			break;
		case 300:
			if (!(row & 1)) {
				col2 = col + 1;
				col = (col + 1 == pp->ncols) ? 0 : col + 1;
			}
			row2 = row + 1;
			row = (row + 1 == pp->nrows) ? 0 : row + 1;
			break;
		default:
			(void) fprintf(stderr, "wrong direction %d\n", dir);
		}
	}
	*pcol = col;
	*prow = row;
	*pcol2 = col2;
	*prow2 = row2;
}

static int
getColor(ModeInfo * mi, int i)
{
	switch (i) {
	case 0:
		return WHITE;
	case 1:
		return MAGENTA;
	case 2:
		return RED;
	case 3:
		return ORANGE;
	case 4:
		return YELLOW;
	case 5:
		return GREEN;
	case 6:
		return CYAN;
	case 7:
	default:
		return BLUE;
		/*(void) printf("need a new color!!!!\n");*/
	}
}

static void
drawLine(ModeInfo * mi, GC gc, int oldcol, int oldrow, int newcol, int newrow)
{
	patersonstruct *pp = &patersons[MI_SCREEN(mi)];
	Display *display = MI_DISPLAY(mi);
	Window window = MI_WINDOW(mi);
	XPoint oldPt, newPt;

	if (pp->neighbors == 3 || pp->neighbors == 6) {
		int coldcol = 2 * oldcol + !(oldrow & 1), coldrow = 2 * oldrow;
		int cnewcol = 2 * newcol + !(newrow & 1), cnewrow = 2 * newrow;

		if (pp->vertical) {
			oldPt.x = pp->xb + coldcol * pp->xs;
			oldPt.y = pp->yb + coldrow * pp->ys;
			newPt.x = pp->xb + cnewcol * pp->xs;
			newPt.y = pp->yb + cnewrow * pp->ys;
		} else {
			oldPt.y = pp->xb + coldcol * pp->xs;
			oldPt.x = pp->yb + coldrow * pp->ys;
			newPt.y = pp->xb + cnewcol * pp->xs;
			newPt.x = pp->yb + cnewrow * pp->ys;
		}
		XDrawLine(display, window, gc, oldPt.x, oldPt.y,
			newPt.x, newPt.y);
	} else if (pp->neighbors == 4) {
		oldPt.x = pp->xb + oldcol * pp->xs;
		oldPt.y = pp->yb + oldrow * pp->ys;
		newPt.x = pp->xb + newcol * pp->xs;
		newPt.y = pp->yb + newrow * pp->ys;
		XDrawLine(display, window, gc, oldPt.x, oldPt.y,
			newPt.x, newPt.y);
	}
}

static void
drawWorm(ModeInfo * mi, wormstruct *worm)
{
        Display    *display = MI_DISPLAY(mi);

	if (worm->color == 0 || MI_NPIXELS(mi) == 2)
		XSetForeground(display, MI_GC(mi), MI_WHITE_PIXEL(mi));
	else
		XSetForeground(display, MI_GC(mi),
			MI_PIXEL(mi, getColor(mi, worm->color)));
#ifdef DEBUG
        (void) printf("%d %d  %d %d  %d %d\n", worm->oldcol, worm->oldrow,
		worm->newcol, worm->newrow, worm->newaltcol, worm->newaltrow);
#endif
	drawLine(mi, MI_GC(mi), worm->oldcol, worm->oldrow,
		worm->newaltcol, worm->newaltrow);
}

static void
patersonWorm(ModeInfo * mi, wormstruct *worm, int discreteDir)
{
	patersonstruct *pp = &patersons[MI_SCREEN(mi)];
	int fieldPosFrom, fieldPosTo;

	/* Can we change from nondirected? */
	fieldPosFrom = worm->oldcol + worm->oldrow * pp->ncols;
	pp->field[fieldPosFrom] |= (1 << discreteDir);
	fieldPosTo = worm->newcol + worm->newrow * pp->ncols;
	if (pp->neighbors == 3) {
		pp->field[fieldPosFrom] |= (1 << (discreteDir + 1) % 6);
		pp->field[fieldPosFrom] |= (1 << (discreteDir + 5) % 6);
		pp->field[fieldPosTo] |= (1 << ((discreteDir + 4) % 6));
	} else
		pp->field[fieldPosTo] |= (1 << ((discreteDir + pp->neighbors / 2) % pp->neighbors));
#ifdef DEBUG
	(void) printf("from %d fromField %d, to %d toField %d\n",
		fieldPosFrom, pp->field[fieldPosFrom],
		fieldPosTo, pp->field[fieldPosTo]);
#endif
	drawWorm(mi, worm);
}

static int
getPathsRemaining(patersonstruct *pp, int fieldPos, int discreteDir, int *pathMask)
{
	int i, count = 0, dirMask;
	*pathMask = 0;
#ifdef DEBUG
	(void) printf("path ");
#endif
	for (i = 0; i < pp->neighbors; i++) {
		dirMask = 1 << ((2 * pp->neighbors + discreteDir - i) % pp->neighbors);
		if ((pp->field[fieldPos] & dirMask) == 0) {
			*pathMask |= (1 << i);
#ifdef DEBUG
			(void) printf("%d, ", i);
#endif
			count++;
		}
	}
#ifdef DEBUG
	(void) printf("remaining: %d\n", count);
#endif
	return count;
}

/*
  Gardner's rule (in Gardner's book he has a simplified version of
  Beeler's rules (not given)).
  The advantage for this rule is that you can randomize it, though some
  rules are not interesting.
  The ability to randomize easily may not be as trivial if there is more
  than one worm.
  The other rule, called here as the Internet rule (not sure who came up
  with it), is more straight forward and documented on the internet.  This
  rule is created more as you encounter different configurations of empty
  lines (can be considered as where there was food for the worm).
*/

static Bool gardnerRule(char * stringRule)
{
	int i;

	for (i = 0; stringRule[i] != '\0'; i++) {
		if (stringRule[i] == 'a' || stringRule[i] == 'b') {
			return True;
		}
	}
	return False;
}

static void
setGardnerRule(char *stringRule, int *rule1Choice,
			int *rule2Choice, int *rule3Choice, int *rule4Choice)
{
	int i, pathsTaken = 1;
	int case2 = 0, case3 = 0, case4 = 0;

	*rule1Choice = -1;
	for (i = 0; i < RULE2_SIZE; i++)
		rule2Choice[i] = -1;
	for (i = 0; i < RULE3_SIZE; i++)
		rule3Choice[i] = -1;
	for (i = 0; i < RULE4_SIZE; i++)
		rule4Choice[i] = -1;
	if (!stringRule)
		return;
	for (i = 0; stringRule[i] != '\0'; i++) {
		if (stringRule[i] >= '1' && stringRule[i] <= '4')
			pathsTaken = stringRule[i] - '0';
		else {
			switch (pathsTaken) {
			case 1:
				if (stringRule[i] >= 'a' && stringRule[i] <= 'b') {
					*rule1Choice = stringRule[i] - 'a';
				}
				break;
			case 2:
				if (stringRule[i] >= 'a' && stringRule[i] <= 'd' && case2 < RULE2_SIZE) {
					rule2Choice[case2] = (stringRule[i] - 'a');
					case2++;
				}
				break;
			case 3:
				if (stringRule[i] >= 'a' && stringRule[i] <= 'c' && case3 < RULE3_SIZE) {
					rule3Choice[case3] = (stringRule[i] - 'a');
					case3++;
				}
				break;
			case 4:
				if (stringRule[i] >= 'a' && stringRule[i] <= 'b' && case4 < RULE4_SIZE) {
					rule4Choice[case4] = stringRule[i] - 'a';
					case4++;
				}
				break;
			}
		}
	}
}

static void
setInternetRule(char *stringRule, int *rule)
{
	int i = 0, n = 0;

	for (i = 0; i < RULE_SIZE; i++)
		rule[i] = -1;
	if (!stringRule)
		return;
	i = 0;
	while (stringRule[n] && i < RULE_SIZE) {
		if (stringRule[n] >= '0' && stringRule[n] <= '5') {
			rule[i] = stringRule[n] - '0';
			i++;
		}
		n++;
	}
}

static Bool createGardnerRule(patersonstruct *pp, char *newRule)
{
	int i, j;
	int size2, size3, size4;
	char init[4];

	if (pp->n == 1) { /* multiple worms seem to need more terms */
		size2 = 1;
		size3 = 4;
		init[2] = 'a'; /* init[2] may be used uninitialized? */
		size4 = 1;
	} else {
		size2 = RULE2_SIZE;
		size3 = RULE3_SIZE;
		size4 = RULE4_SIZE;
	}
	i = 0;
	newRule[i++] = '1';
	newRule[i] = NRAND(2) + 'a';
	init[0] = newRule[i];
	i++;
	newRule[i++] = '2';
	for (j = 0; j < size2; j++) {
		newRule[i] = NRAND(4) + 'a';
		if (j == 0)
			init[1] = newRule[i];
		i++;
	}
	newRule[i++] = '3';
	for (j = 0; j < size3; j++) {
		newRule[i] = NRAND(3) + 'a';
		if (j == 0)
			init[2] = newRule[i];
		i++;
	}
	newRule[i++] = '4';
	for (j = 0; j < size4; j++) {
		newRule[i] = NRAND(2) + 'a';
		if (j == 0)
			init[3] = newRule[i];
		i++;
	}
	newRule[i] = '\0';
	if (init[0] == 'b' && init[2] == 'b') {
		if (init[1] == 'b' || init[1] == 'd' || init[3] == 'b') {
#ifdef DEBUG
			(void) printf("zipper rule\n");
#endif
			return True;
		}
	}
	return False;
}

static int
reverseDir(patersonstruct *pp, int dir)
{
	return (2 * ANGLES - dir) % ANGLES;
}

static void
setGardnerRuleString(char *ruleString, int *pathMask2, int *pathMask3, int *pathMask4,
		int gardnerRule1, int *gardnerRule2, int *gardnerRule3, int *gardnerRule4)
{
	int n, i = 0;

	ruleString[i++] = '1';
	if (gardnerRule1 != -1)
		ruleString[i++] = (gardnerRule1 + 'a');
	ruleString[i++] = '2';
	for (n = 0; n < RULE2_SIZE; n++) {
		if (gardnerRule2[n] == -1 || (pathMask2 != NULL && pathMask2[n] == -1))
			break;
		else
			ruleString[i++] = (gardnerRule2[n] + 'a');
	}
	ruleString[i++] = '3';
	for (n = 0; n < RULE3_SIZE; n++) {
		if (gardnerRule3[n] == -1 || (pathMask3 != NULL && pathMask3[n] == -1))
			break;
		else
			ruleString[i++] = (gardnerRule3[n] + 'a');
	}
	ruleString[i++] = '4';
	for (n = 0; n < RULE4_SIZE; n++) {
		if (gardnerRule4[n] == -1 || (pathMask4 != NULL && pathMask4[4] == -1))
			break;
		else
			ruleString[i++] = (gardnerRule4[n] + 'a');
	}
	ruleString[i] = '\0';
}

static void
setInternetRuleString(char *ruleString, int *rule)
{
	int n, i = 0;

	ruleString[i++] = '{';
	ruleString[i++] = ' ';
	for (n = 0; n < RULE_SIZE; n++) {
		if (rule[n] < 0)
			break;
		if (n > 0)
			ruleString[i++] = ',';
		if (rule[n] < 6)
			ruleString[i++] = rule[n] + '0';
	}
	ruleString[i++] = ' ';
	ruleString[i++] = '}';
	ruleString[i] = '\0';
}

static void
free_paterson_screen(Display *display, patersonstruct *pp)
{
	if (pp == NULL) {
		return;
	}
	if (pp->field != NULL) {
		free(pp->field);
		pp->field = (unsigned char *) NULL;
	}
	if (pp->worms != NULL) {
		free(pp->worms);
		pp->worms = (wormstruct *) NULL;
	}
	pp = NULL;
}

ENTRYPOINT void
free_paterson(ModeInfo * mi)
{
	free_paterson_screen(MI_DISPLAY(mi), &patersons[MI_SCREEN(mi)]);
}

ENTRYPOINT void
init_paterson(ModeInfo * mi)
{
	Display    *display = MI_DISPLAY(mi);
	int	 size = MI_SIZE(mi);
	patersonstruct *pp;
	int	 col, row, dir;
	int	 i, j, k;

	MI_INIT(mi, patersons);
	pp = &patersons[MI_SCREEN(mi)];

	pp->generation = 0;
	pp->n = MI_COUNT(mi);
	if (pp->n < -MINWORMS) {
		/* if pp->n is random ... the size can change */
		if (pp->worms != NULL) {
			free(pp->worms);
			pp->worms = (wormstruct *) NULL;
		}
		/*pp->n = NRAND(-pp->n - MINWORMS + 1) + MINWORMS;*/
		/* skew so that 1 is more likely */
		pp->n = -pp->n - (int) (sqrt(NRAND(pp->n *pp->n - 1))) - 1 +
			MINWORMS;
	} else if (pp->n < MINWORMS)
		pp->n = MINWORMS;

	if (MI_IS_FULLRANDOM(mi)) {
		pp->vertical = (Bool) (LRAND() & 1);
	} else {
		pp->vertical = vertical;
	}
	pp->width = MI_WIDTH(mi);
	pp->height = MI_HEIGHT(mi);

	if (neighbors == 3 || neighbors == 4 || neighbors == 6)
		pp->neighbors = neighbors;
	else if (!NRAND(24))
		pp->neighbors = 4;
	else if (!NRAND(48))
		pp->neighbors = 3; /* kind of silly */
	else
		pp->neighbors = 6;
	if (pp->neighbors == 3 || pp->neighbors == 6) {
		int	 nccols, ncrows;

		pp->polygon = 6;
		if (!pp->vertical) {
			pp->height = MI_WIDTH(mi);
			pp->width = MI_HEIGHT(mi);
		}
		if (pp->width < 8)
			pp->width = 8;
		if (pp->height < 8)
			pp->height = 8;
		if (size < -MINSIZE) {
			pp->ys = NRAND(MIN(-size, MAX(MINSIZE, MIN(pp->width, pp->height) /
				      MINGRIDSIZE)) - MINSIZE + 1) + MINSIZE;
			if (pp->ys < MINRANDOMSIZE)
				pp->ys = MIN(MINRANDOMSIZE,
					     MAX(MINSIZE, MIN(pp->width, pp->height) / MINGRIDSIZE));
		} else if (size < MINSIZE) {
			if (!size)
				pp->ys = MAX(MINSIZE, MIN(pp->width, pp->height) / MINGRIDSIZE);
			else
				pp->ys = MINSIZE;
		} else
			pp->ys = MIN(size, MAX(MINSIZE, MIN(pp->width, pp->height) /
					       MINGRIDSIZE));
		pp->xs = pp->ys;
		nccols = MAX(pp->width / pp->xs - 2, 2);
		ncrows = MAX(pp->height / pp->ys - 1, 4);
		pp->ncols = nccols / 2;
		pp->nrows = 2 * (ncrows / 4);
		pp->xb = (pp->width - pp->xs * nccols) / 2 + pp->xs / 2;
		pp->yb = (pp->height - pp->ys * (ncrows / 2) * 2) / 2 + pp->ys - 2;
		for (i = 0; i < 6; i++) {
			if (pp->vertical) {
				pp->shape.hexagon[i].x =
					(pp->xs - 1) * hexagonUnit[i].x;
				pp->shape.hexagon[i].y =
					((pp->ys - 1) * hexagonUnit[i].y /
					2) * 4 / 3;
			} else {
				pp->shape.hexagon[i].y =
					(pp->xs - 1) * hexagonUnit[i].x;
				pp->shape.hexagon[i].x =
					((pp->ys - 1) * hexagonUnit[i].y /
					2) * 4 / 3;
			}
		}
		/* Avoid array bounds read of hexagonUnit */
		pp->shape.hexagon[6].x = 0;
		pp->shape.hexagon[6].y = 0;
	} else if (pp->neighbors == 4) {
		pp->polygon = 4;
		if (size < -MINSIZE) {
			pp->ys = NRAND(MIN(-size, MAX(MINSIZE, MIN(pp->width, pp->height) /
				      MINGRIDSIZE)) - MINSIZE + 1) + MINSIZE;
			if (pp->ys < MINRANDOMSIZE)
				pp->ys = MIN(MINRANDOMSIZE,
					     MAX(MINSIZE, MIN(pp->width, pp->height) / MINGRIDSIZE));
		} else if (size < MINSIZE) {
			if (!size)
				pp->ys = MAX(MINSIZE, MIN(pp->width, pp->height) / MINGRIDSIZE);
			else
				pp->ys = MINSIZE;
		} else
			pp->ys = MIN(size, MAX(MINSIZE, MIN(pp->width, pp->height) /
					       MINGRIDSIZE));
		pp->xs = pp->ys;
		pp->ncols = MAX(pp->width / pp->xs, 2);
		pp->nrows = MAX(pp->height / pp->ys, 2);
		pp->xb = (pp->width - pp->xs * pp->ncols) / 2;
		pp->yb = (pp->height - pp->ys * pp->nrows) / 2;
	}
	for (i = 0; i < RULE_SIZE; i++)
		internetRule[i] = -1;
	if (rule) {
		gardner = gardnerRule(rule);
		if (gardner) {
			setGardnerRule(rule, &gardnerRule1,
				&(gardnerRule2[0]), &(gardnerRule3[0]), &(gardnerRule4[0]));
			setGardnerRuleString(pp->gardnerRuleLabel, NULL, NULL, NULL,
				gardnerRule1, gardnerRule2, gardnerRule3, gardnerRule4);
			if (MI_IS_VERBOSE(mi))
				(void) printf("%s\n", pp->gardnerRuleLabel);
		} else {
			setInternetRule(rule, &(internetRule[0]));
			setInternetRuleString(pp->internetRuleLabel, internetRule);
			if (MI_IS_VERBOSE(mi))
				(void) printf("%s\n", pp->internetRuleLabel);
		}
	} else {
		/* no Gardner rules for 3 & 4 */
		if (pp->neighbors == 3) {
			internetRule[0] = 1;
			gardner = False;
			setInternetRuleString(pp->internetRuleLabel, internetRule);
			if (MI_IS_VERBOSE(mi))
				(void) printf("%s\n", pp->internetRuleLabel);
		} else if (pp->neighbors == 4) {
			internetRule[0] = 1;
			internetRule[1] = NRAND(2) * 3;
			gardner = False;
			setInternetRuleString(pp->internetRuleLabel, internetRule);
			if (MI_IS_VERBOSE(mi))
				(void) printf("%s\n", pp->internetRuleLabel);
		} else {
			/* if an annoying rule try one more time */
			if (createGardnerRule(pp, &newGardnerRule[0]))
				createGardnerRule(pp, &newGardnerRule[0]);
			gardner = True;
			setGardnerRule(newGardnerRule, &gardnerRule1,
				&(gardnerRule2[0]), &(gardnerRule3[0]), &(gardnerRule4[0]));
			if (pp->n == 1)
				setGardnerRuleString(pp->gardnerRuleLabel, NULL, NULL, NULL,
					gardnerRule1, gardnerRule2, gardnerRule3, gardnerRule4);
			setGardnerRuleString(pp->gardnerRuleString, NULL, NULL, NULL,
				gardnerRule1, gardnerRule2, gardnerRule3, gardnerRule4);
			if (MI_IS_VERBOSE(mi)) {
				(void) printf("%s\n", pp->gardnerRuleString);
			}
		}
	}

	XSetLineAttributes(display, MI_GC(mi), 1, LineSolid, CapNotLast, JoinMiter);
	MI_CLEARWINDOW(mi);
	pp->painted = False;

	pp->labelOffsetX = NRAND(8);
	pp->labelOffsetY = NRAND(8);
	/*parseRule(mi);*/
	if (MI_NPIXELS(mi) > 2) {
		int offset = NRAND(MI_NPIXELS(mi));

		for (i = 0; i < (int) pp->ncolors - 1; i++) {
			pp->colors[i] = (unsigned char) ((offset +
				(i * MI_NPIXELS(mi) /
				(int) (pp->ncolors - 1))) % MI_NPIXELS(mi));
		}
	}
	if (pp->worms == NULL) {
		if ((pp->worms = (wormstruct *) malloc(pp->n * sizeof (wormstruct))) ==
				NULL) {
			free_paterson(mi);
			return;
		}
	}
	if (pp->field != NULL)
		free(pp->field);
	if ((pp->field = (unsigned char *) calloc(pp->ncols * pp->nrows,
			sizeof (unsigned char))) == NULL) {
		free_paterson(mi);
		return;
	}

	row = pp->nrows / 2;
	col = pp->ncols / 2;
	if (pp->polygon == 5) {
		int mod = (col + 2 * (row - 1)) % 4;
		dir = 0;
		col -= mod + NRAND(4);
	}
#if 0
#define SIMPLIFY 1
#endif
	pp->left = NRAND(2);
#ifdef SIMPLIFY
	pp->left = False;
#endif
	/* Have them all start in the same spot, why not? */
	k = NRAND(2);
	for (i = 0; i < pp->n; i++) {
		wormstruct  *aworm;
		if (pp->neighbors == 3)
			dir = (NRAND(3) * 2 + k) * ANGLES / 6;
		else
			dir = NRAND(pp->neighbors) * ANGLES / pp->neighbors;
#ifdef SIMPLIFY
		dir = 0;
#endif
		aworm = &pp->worms[i];
		aworm->newcol = col/* + NRAND(3) - 1*/;
		aworm->newrow = row/* + NRAND(3) - 1*/;
		aworm->oldcol = 0;
		aworm->oldrow = 0;
		aworm->direction = dir;
		aworm->color = 0;
		aworm->alive = True;
		aworm->positions = 1;
		for (j = 0; j < RULE_SIZE; j++)
			aworm->internetRule[j] = -1;
		if (internetRule != NULL)
			for (j = 0; j < RULE_SIZE; j++)
				aworm->internetRule[j] = internetRule[j];
		for (j = 0; j < RULE2_SIZE; j++)
			aworm->pathMask2[j] = -1;
		for (j = 0; j < RULE3_SIZE; j++)
			aworm->pathMask3[j] = -1;
		for (j = 0; j < RULE4_SIZE; j++)
			aworm->pathMask4[j] = -1;
	}
}

ENTRYPOINT void
draw_paterson(ModeInfo * mi)
{
	wormstruct  *aworm;
	int	 i, j, fieldPos, clew = 0;
	patersonstruct *pp;

	if (patersons == NULL)
		return;
	pp = &patersons[MI_SCREEN(mi)];
	if (pp->worms == NULL)
		return;

#ifdef STANDALONE
	if (pp->eraser) {
		pp->eraser = erase_window (MI_DISPLAY(mi), MI_WINDOW(mi), pp->eraser);
		return;
	}
#endif

	MI_IS_DRAWN(mi) = True;
	pp->painted = True;
	for (i = 0; i < pp->n; i++) {
		aworm = &pp->worms[i];
		if (aworm->alive)
			clew++;
	}
	if (label) {
		int size = MAX(MIN(MI_WIDTH(mi), MI_HEIGHT(mi)) - 1, 1);

		if (size >= 10 * FONT_WIDTH) {
			/* hard code these to corners */
			XSetForeground(MI_DISPLAY(mi), MI_GC(mi),
				MI_WHITE_PIXEL(mi));
			XDrawString(MI_DISPLAY(mi), MI_WINDOW(mi), MI_GC(mi),
				16 + pp->labelOffsetX,
				16 + pp->labelOffsetY + FONT_HEIGHT,
				pp->internetRuleLabel, strlen(pp->internetRuleLabel));
			XDrawString(MI_DISPLAY(mi), MI_WINDOW(mi), MI_GC(mi),
				16 + pp->labelOffsetX, MI_HEIGHT(mi) - 16 -
				pp->labelOffsetY - FONT_HEIGHT / 2,
				pp->gardnerRuleLabel, strlen(pp->gardnerRuleLabel));
		}
	}
	if (clew == 0) {
		if (pp->sleepCount > SLEEP_COUNT) {
			init_paterson(mi);
			return;
		}
		pp->sleepCount++;
		return;
	}
	for (i = 0; i < pp->n; i++) {
		int col, row, altcol, altrow, ruleField;
		int ruleAngle, defaultAngle, discreteDir, paths;

		aworm = &pp->worms[i];
		if (!aworm->alive)
			continue;
		if (pp->neighbors == 3)
			defaultAngle = ANGLES / 6;
		else
			defaultAngle = ANGLES / pp->neighbors;
		col = aworm->newcol;
		row = aworm->newrow;
		fieldPos = col + row * pp->ncols;
		paths = getPathsRemaining(pp, fieldPos, aworm->direction / defaultAngle,
			&(aworm->pathMask));
		if (paths >= pp->neighbors - 1) { /* WHITE */
			int rule1;
			if (gardner)
				aworm->internetRule[0] = gardnerRule1 + 1;
			rule1 = aworm->internetRule[0];
		/*if ((aworm->pathMask & (1 << rule1)) != 0) */ /* WHITE */
#ifdef DEBUG
			(void) printf(" White: defAngle=%d, rl=%d, mask=%d true=%d ruleShift=%d\n",
				defaultAngle, rule1, aworm->pathMask,
				((aworm->pathMask & (1 << rule1)) != 0),
				(1 << rule1));
#endif
			ruleAngle = rule1 * defaultAngle;
		} else if (paths == 1) {
			ruleAngle = deshift(aworm->pathMask) * defaultAngle;
#ifdef DEBUG
			(void) printf(" ONLY 1 %d %d\n", aworm->pathMask, ruleAngle);
#endif
			if (pp->neighbors == 3)
				ruleAngle = 300;
		} else if (paths == 0) {
			pp->worms[i].alive = False;
			setGardnerRuleString(pp->gardnerRuleString,
				aworm->pathMask2, aworm->pathMask3, aworm->pathMask4,
				gardnerRule1, gardnerRule2, gardnerRule3, gardnerRule4);
			setInternetRuleString(aworm->internetRuleString, aworm->internetRule);
			if (pp->n == 1 && gardner)
				setInternetRuleString(pp->internetRuleLabel, aworm->internetRule);
			if (MI_IS_VERBOSE(mi)) {
				(void) printf("Death %s %s\n",
					pp->gardnerRuleString, aworm->internetRuleString);
			}
			return;
		} else if (gardner) {
			int choice = -1;
			ruleAngle = -1;
			if (paths == pp->neighbors - 2) {
				ruleField = 2;
				if (invMaskChoice[aworm->pathMask] != -1) {
					for (j = 0; j < RULE2_SIZE; j++) {
						if (gardnerRule2[j] == -1) {
							if (MI_IS_VERBOSE(mi))
								(void) printf(" Rule%d too short\n", ruleField);
							pp->worms[i].alive = False;
							return;
						}
						if (aworm->pathMask2[j] == -1) {
							choice = field2Choice[invMaskChoice[aworm->pathMask]][gardnerRule2[j]];
#ifdef DEBUG
							(void) printf(" Field%d,%d %d [%d, %c] %d\n", ruleField, j, aworm->pathMask,
								invMaskChoice[aworm->pathMask],
								(gardnerRule2[j] + 'a'), choice);
#endif
							ruleAngle = choice * defaultAngle;
							aworm->pathMask2[j] = aworm->pathMask;
							aworm->internetRule[aworm->positions] = choice;
							if (MI_IS_VERBOSE(mi)) {
								setInternetRuleString(aworm->internetRuleString, aworm->internetRule);
								if (pp->n == 1)
									(void) printf("field %d: mask %2d %s\n",
										ruleField, aworm->pathMask, aworm->internetRuleString);
								else
									(void) printf("worm %d, field %d: mask %2d %s\n",
										i, ruleField, aworm->pathMask, aworm->internetRuleString);
							}
							aworm->positions++;
							aworm->color = aworm->positions;
							break;
						} else if (aworm->pathMask2[j] == aworm->pathMask) {
							choice = field2Choice[invMaskChoice[aworm->pathMask]][gardnerRule2[j]];
							ruleAngle = choice * defaultAngle;
#ifdef DEBUG
							(void) printf(" field%d,%d %d [%d, %c] %d\n", ruleField, j, aworm->pathMask,
								invMaskChoice[aworm->pathMask],
								(gardnerRule2[j] + 'a'), choice);
#endif
							break;
						}
					}
					if (choice < 0) {
						if (choice == -1)
							(void) printf(" FIXME %d got here\n", aworm->pathMask);
						pp->worms[i].alive = False;
						return;
					}
				} else {
					if ((aworm->pathMask & 8) == 0) /* ignore initial collision */
						(void) printf(" FIXME %d %d\n", aworm->pathMask, ruleField);
					pp->worms[i].alive = False;
					return;
				}
			} else if (paths == pp->neighbors - 3) {
				ruleField = 3;
				if (invMaskChoice[aworm->pathMask] != -1) {
					for (j = 0; j < RULE3_SIZE; j++) {
						if (gardnerRule3[j] == -1) {
							if (MI_IS_VERBOSE(mi))
								(void) printf(" Rule%d too short\n", ruleField);
							pp->worms[i].alive = False;
							return;
						}
						if (aworm->pathMask3[j] == -1) {
							choice = field3Choice[invMaskChoice[aworm->pathMask]][gardnerRule3[j]];
#ifdef DEBUG
							(void) printf(" Field%d,%d %d [%d, %c] %d\n", ruleField, j, aworm->pathMask,
								invMaskChoice[aworm->pathMask],
								(gardnerRule3[j] + 'a'), choice);
#endif
							ruleAngle = choice * defaultAngle;
							aworm->pathMask3[j] = aworm->pathMask;
							aworm->internetRule[aworm->positions] = choice;
							if (MI_IS_VERBOSE(mi)) {
								setInternetRuleString(aworm->internetRuleString, aworm->internetRule);
								if (pp->n == 1)
									(void) printf("field %d: mask %2d %s\n",
										ruleField, aworm->pathMask, aworm->internetRuleString);
								else
									(void) printf("worm %d, field %d: mask %2d %s\n",
										i, ruleField, aworm->pathMask, aworm->internetRuleString);
							}
							aworm->positions++;
							aworm->color = aworm->positions;
							break;
						} else if (aworm->pathMask3[j] == aworm->pathMask) {
							choice = field3Choice[invMaskChoice[aworm->pathMask]][gardnerRule3[j]];
							ruleAngle = choice * defaultAngle;
#ifdef DEBUG
							(void) printf(" field%d,%d %d [%d, %c] %d\n", ruleField, j, aworm->pathMask,
								invMaskChoice[aworm->pathMask],
								(gardnerRule3[j] + 'a'), choice);
#endif
							break;
						}
					}
					if (choice < 0) {
						if (choice == -1)
							(void) printf(" FIXME %d got here\n", aworm->pathMask);
						pp->worms[i].alive = False;
						return;
					}
				} else {
					if ((aworm->pathMask & 8) == 0) /* ignore initial collision */
						(void) printf(" FIXME %d %d\n", aworm->pathMask, ruleField);
					pp->worms[i].alive = False;
					return;
				}
			} else if (paths == pp->neighbors - 4) {
				ruleField = 4;
				if (invMaskChoice[aworm->pathMask] != -1) {
					for (j = 0; j < RULE4_SIZE; j++) {
						if (gardnerRule4[j] == -1) {
							if (MI_IS_VERBOSE(mi))
								(void) printf(" Rule%d too short\n", ruleField);
							pp->worms[i].alive = False;
							return;
						}
						if (aworm->pathMask4[j] == -1) {
							choice = field4Choice[invMaskChoice[aworm->pathMask]][gardnerRule4[j]];
#ifdef DEBUG
							(void) printf(" Field%d,%d %d [%d, %c] %d\n", ruleField, j, aworm->pathMask,
								invMaskChoice[aworm->pathMask],
								(gardnerRule4[j] + 'a'), choice);
#endif
							ruleAngle = choice * defaultAngle;
							aworm->pathMask4[j] = aworm->pathMask;
							aworm->internetRule[aworm->positions] = choice;
							if (MI_IS_VERBOSE(mi)) {
								setInternetRuleString(aworm->internetRuleString, aworm->internetRule);
								if (pp->n == 1)
									(void) printf("field %d: mask %2d %s\n",
										ruleField, aworm->pathMask, aworm->internetRuleString);
								else
									(void) printf("worm %d, field %d: mask %2d %s\n",
										i, ruleField, aworm->pathMask, aworm->internetRuleString);
							}
							aworm->positions++;
							aworm->color = aworm->positions;
							break;
						} else if (aworm->pathMask4[j] == aworm->pathMask) {
							choice = field4Choice[invMaskChoice[aworm->pathMask]][gardnerRule4[j]];
							ruleAngle = choice * defaultAngle;
#ifdef DEBUG
							(void) printf(" field%d,%d %d [%d, %c] %d\n", ruleField, j, aworm->pathMask,
								invMaskChoice[aworm->pathMask],
								(gardnerRule4[j] + 'a'), choice);
#endif
							break;
						}
					}
					if (choice < 0) {
						if (choice == -1)
							(void) printf(" FIXME %d got here\n", aworm->pathMask);
						pp->worms[i].alive = False;
						return;
					}
				} else {
					if ((aworm->pathMask & 8) == 0) /* ignore initial collision */
						(void) printf(" FIXME %d %d\n", aworm->pathMask, ruleField);
					pp->worms[i].alive = False;
					return;
				}
			}
		} else {
			ruleAngle = -1;
			for (j = 2; j <= aworm->positions; j++) {
				if (aworm->state[j] == aworm->pathMask) {
					ruleAngle = aworm->internetRule[j - 1] * defaultAngle;
					break;
				}
			}
			if (ruleAngle == -1) {
				j = ++aworm->positions;
				aworm->state[j] = aworm->pathMask;
				ruleAngle = aworm->internetRule[j - 1] * defaultAngle;
#ifdef DEBUG
				(void) printf(" color %d: %d\n", j, aworm->pathMask);
#endif
				if (aworm->color < j - 1)
					aworm->color = j - 1;
			}
		}
		if (pp->neighbors == 3)
			aworm->direction = heading(6, aworm->direction, ruleAngle);
		else
			aworm->direction = heading(pp->neighbors, aworm->direction, ruleAngle);
		discreteDir = aworm->direction / defaultAngle;
		nextPosition(pp,
			((pp->left) ? reverseDir(pp, aworm->direction) : aworm->direction),
			&col, &row, &altcol, &altrow);
#ifdef DEBUG
		(void) printf("angle=%d, oldcol=%d, oldrow=%d, newcol=%d, newrow=%d, gen=%d\n",
			ruleAngle, aworm->oldcol, aworm->oldrow, aworm->newcol, aworm->newrow, pp->generation);
#endif
#ifdef DEBUG
		(void) printf("col=%d, row=%d, paths=%d, direction=%d, discreteDir=%d, field=%d\n",
			col, row, paths, aworm->direction, discreteDir,
			(pp->field[fieldPos] & (1 << discreteDir)));

#endif
		aworm->oldcol = aworm->newcol;
		aworm->oldrow = aworm->newrow;
		aworm->newcol = col;
		aworm->newrow = row;
		aworm->newaltcol = altcol;
		aworm->newaltrow = altrow;
		patersonWorm(mi, aworm, discreteDir);
		/* Translate relative direction to actual direction */
	}
	if (++pp->generation > MI_CYCLES(mi)) {
#ifdef STANDALONE
		pp->eraser = erase_window (MI_DISPLAY(mi), MI_WINDOW(mi), pp->eraser);
#endif
		init_paterson(mi);
	}
}

ENTRYPOINT void
release_paterson(ModeInfo * mi)
{
	if (patersons != NULL) {
		int	screen;

		for (screen = 0; screen < MI_NUM_SCREENS(mi); screen++)
			free_paterson_screen(MI_DISPLAY(mi), &patersons[screen]);
		free(patersons);
		patersons = (patersonstruct *) NULL;
	}
}

XSCREENSAVER_MODULE ("Paterson", paterson)

#endif /* MODE_paterson */
