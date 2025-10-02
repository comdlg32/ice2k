/* -*- Mode: C; tab-width: 4 -*- */
/* life3d --- Extension to Conway's game of Life, Carter Bays' B5/S45 3d life */

#if 0
static const char sccsid[] = "@(#)life3d.c	5.27 2008/07/28 xlockmore";

#endif

/*-
 * Copyright (c) 1994 by David Bagley.
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
 * 09-Nov-2010: cubic close packing (-neighbors 14)
 * 31-Jul-2008: 4544 3D life from paper by Carter Bays.
 * 18-Apr-2004: Added shooting gliders for B6/S567
 * 13-Mar-2004: Added references
 * 28-Apr-2003: Randomized "rotation" of life form
 * 25-Jan-2003: Spawned a life3d.h
 * 19-Jan-2003: added more gliders from http://www.cse.sc.edu/~bays
 * 08-Jan-2003: Double buffering
 * 01-Nov-2000: Allocation checks
 * 10-May-1997: Compatible with xscreensaver
 * 18-Apr-1997: Memory leak fixed by Tom Schmidt <tschmidt AT micron.com>
 * 12-Mar-1995: added LIFE_B6S567 compile-time option
 * 12-Feb-1995: shooting gliders added
 * 07-Dec-1994: used life.c and a DOS version of 3dlife
 * Copyright 1993 Anthony Wesley awesley@canb.auug.org.au found at
 * life.anu.edu.au /pub/complex_systems/alife/3DLIFE.ZIP
 *
 *
 * References:
 * Bays, Carter, "A Note About the Discovery of Many New Rules for the
 *   Game of 3D Life", Complex Systems 16 (2006) 381-386
 * Bays, Carter, "Candidates for the Game of Life in Three Dimensions",
 *   1987, Complex Systems 1 373-400
 * Bays, Carter, "Patterns for Simple Cellular Automata in a Universe of
 *   Dense-Packed Spheres", 1987, Complex Systems 1 853-875
 * Bays, Carter, "A Note on the Discovery of a New Game of Three-
 *   Dimensional Life", 1988, Complex Systems 2 255-258
 * Bays, Carter, "A New Candidate Rule for the Game of Three-Dimensional
 *   Life", 1992, Complex Systems 6 433-441
 * Bays, Carter, "Further Notes on the Game of Three-Dimensional Life",
 *   1994, Complex Systems 8 67-73
 * Bays, Carter, "A Note About the Discovery of Many New Rules for the
     Game of Three-Dimensional Life", 2006, Complex Systems 16 381-386
 * Dewdney, A.K., "The Armchair Universe, Computer Recreations from the
 *   Pages of Scientific American Magazine", W.H. Freedman and Company,
 *   New York, 1988 (February 1987 p 16)
 * Bays, Carter, "The Game of Three Dimensional Life", 86/11/20
 *   with (latest?) update from 87/2/1 http://www.cse.sc.edu/~bays/
 * Meeker, Lee Earl, "Four Dimensional Cellular Automata and the Game
 *   of Life" 1998 http://home.sc.rr.com/lmeeker/Lee/Home.html
 */

#ifdef STANDALONE
#define MODE_life3d
#define DEFAULTS "*delay: 1000000 \n" \
	"*count: 35 \n" \
	"*cycles: 85 \n" \
	"*ncolors: 200 \n" \
	"*wireframe: False \n" \
	"*fullrandom: False \n" \
	"*verbose: False \n" \

# define reshape_life3d 0
# define life3d_handle_event 0
#include "xlockmore.h"		/* in xscreensaver distribution */
#else /* STANDALONE */
#include "xlock.h"		/* in xlockmore distribution */
#include "iostuff.h"
#endif /* STANDALONE */
#define DO_STIPPLE
#include "automata.h"

#ifdef MODE_life3d
#define LIFE_NAMES 1
#include "life3d.h"

#ifdef LIFE_NAMES
#define DEF_LABEL "True"
#define FONT_HEIGHT 19
#define FONT_WIDTH 15
#endif
#define DEF_NEIGHBORS  "0"	/* choose best value (26) */
#define DEF_SERIAL "False"
#define DEF_REPEAT "0"		/* Frequently 2 */

#if 1
#define DEF_RULE  "G"		/* All rules with gliders */
#else
#define DEF_RULE  "P"		/* All rules with known patterns */
#define DEF_RULE  "B5/S45"
#define DEF_RULE  "B5/S56"
#define DEF_RULE  "B5/S678"
#define DEF_RULE  "B6/S567"
#define DEF_RULE  "B67/S67"
/* There is a glider for B67/S67 but its kind of big */
#endif

#define DEF_GLIDERSEARCH  "False"
#define DEF_PATTERNSEARCH  "False"
#define DEF_DRAW  "True"

#ifdef LIFE_NAMES
static Bool label;
#endif
static Bool draw;
static int  neighbors;
static int  repeat;
static char *rule;
static char *lifefile;
static Bool serial;
static Bool glidersearch;
static Bool patternsearch;

static XrmOptionDescRec opts[] =
{
#ifdef LIFE_NAMES
	{(char *) "-label", (char *) ".life3d.label", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+label", (char *) ".life3d.label", XrmoptionNoArg, (caddr_t) "off"},
#endif
	{(char *) "-draw", (char *) ".life3d.draw", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+draw", (char *) ".life3d.draw", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-neighbors", (char *) ".life3d.neighbors", XrmoptionSepArg, (caddr_t) NULL},
	{(char *) "-repeat", (char *) ".life3d.repeat", XrmoptionSepArg, (caddr_t) NULL},
	{(char *) "-rule", (char *) ".life3d.rule", XrmoptionSepArg, (caddr_t) NULL},
	{(char *) "-lifefile", (char *) ".life3d.lifefile", XrmoptionSepArg, (caddr_t) NULL},
	{(char *) "-serial", (char *) ".life3d.serial", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+serial", (char *) ".life3d.serial", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-glidersearch", (char *) ".life3d.glidersearch", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+glidersearch", (char *) ".life3d.glidersearch", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-patternsearch", (char *) ".life3d.patternsearch", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+patternsearch", (char *) ".life3d.patternsearch", XrmoptionNoArg, (caddr_t) "off"},
};
static argtype vars[] =
{
#ifdef LIFE_NAMES
	{(void *) & label, (char *) "label", (char *) "Label", (char *) DEF_LABEL, t_Bool},
#endif
	{(void *) & draw, (char *) "draw", (char *) "Draw", (char *) DEF_DRAW, t_Bool},
	{(void *) & neighbors, (char *) "neighbors", (char *) "Neighbors", (char *) DEF_NEIGHBORS, t_Int},
	{(void *) & repeat, (char *) "repeat", (char *) "Repeat", (char *) DEF_REPEAT, t_Int},
	{(void *) & rule, (char *) "rule", (char *) "Rule", (char *) DEF_RULE, t_String},
#ifdef STANDALONE
	{(void *) & lifefile, (char *) "lifefile", (char *) "LifeFile", (char *) "/dev/null", t_String},
#else
	{(void *) & lifefile, (char *) "lifefile", (char *) "LifeFile", (char *) "", t_String},
#endif
	{(void *) & serial, (char *) "serial", (char *) "Serial", (char *) DEF_SERIAL, t_Bool},
	{(void *) & glidersearch, (char *) "glidersearch", (char *) "GliderSearch", (char *) DEF_GLIDERSEARCH, t_Bool},
	{(void *) & patternsearch, (char *) "patternsearch", (char *) "PatternSearch", (char *) DEF_PATTERNSEARCH, t_Bool},
};
static OptionStruct desc[] =
{
#ifdef LIFE_NAMES
	{(char *) "-/+label", (char *) "turn on/off name labeling"},
#endif
	{(char *) "-/+draw", (char *) "turn on/off drawing to speed search"},
	{(char *) "-neighbors num", (char *) "cubes 6, 8, 18, 20, 26, rhombic dodecahedron 12, (-)18, truncated octahedron 14, or tetrahedron 22"},
	{(char *) "-repeat num", (char *) "repeat for period to exclude in search"}, /* this is not as good as Bays' signature idea */
	{(char *) "-rule string", (char *) "B<birth_neighborhood>/S<survival_neighborhood> parameters"},
	{(char *) "-lifefile file", (char *) "life file"},
	{(char *) "-/+serial", (char *) "turn on/off picking of sequential patterns"},
	{(char *) "-/+glidersearch", (char *) "search for gliders"},
	{(char *) "-/+patternsearch", (char *) "search for patterns"},
};

ENTRYPOINT ModeSpecOpt life3d_opts =
{sizeof opts / sizeof opts[0], opts, sizeof vars / sizeof vars[0], vars, desc};

#ifdef USE_MODULES
ModStruct life3d_description =
{"life3d", "init_life3d", "draw_life3d", "release_life3d",
 "refresh_life3d", "change_life3d", "free_life3d", &life3d_opts,
 1000000, 35, 85, 1, 64, 1.0, "",
 "Shows Bays' game of 3D Life", 0, NULL};

#endif

#define LIFE3DBITS(n,w,h)\
  if ((lp->pixmaps[lp->init_bits]=\
  XCreatePixmapFromBitmapData(display,window,(char *)n,w,h,1,0,1))==None){\
  free_life3d_screen(display,lp); return;} else {lp->init_bits++;}

#define ON 0x40
#define OFF 0

/* Don't change these numbers without changing the offset() macro below! */
#define MAXSTACKS 64
#define	MAXROWS 128
#define MAXCOLUMNS 128
#define BASESIZE ((MAXCOLUMNS*MAXROWS*MAXSTACKS)>>6)
#define SOUPPERCENT 30
/* #define SOUPSIZE(s)  MAX(s/4,10) */
/* 20B4S45 frequently has unlimited growth, which is not pretty */
#define SOUPSIZE(s)   (lp->neighbors == 20 && lp->patterned_rule == LIFE_20B4S45) ? 4 : ((lp->neighbors == 18 && lp->patterned_rule == LIFE_18B4S45) ? 6 : 10)

#define RT_ANGLE 90
#define HALFRT_ANGLE 45

/* Store state of cell in top bit. Reserve low bits for count of living nbrs */
#define Set3D(x,y,z) SetMem(lp,(unsigned int)x,(unsigned int)y,(unsigned int)z,ON)
#define Reset3D(x,y,z) SetMem(lp,(unsigned int)x,(unsigned int)y,(unsigned int)z,OFF)

#define SetList(x,y,z) if (!SetMem(lp,(unsigned int)x,(unsigned int)y,(unsigned int)z,ON)) return False; \
if (!addToList(lp,(unsigned int)x,(unsigned int)y,(unsigned int)z)) return False

#define CellState3D(c) ((c)&ON)
#define CellNbrs3D(c) ((c)&0x1f)	/* 26 <= 31 */

#define EyeToScreen 72.0	/* distance from eye to screen */
#define HalfScreenD 14.0	/* 1/2 the diameter of screen */
#define BUCKETSIZE 10
#define NBUCKETS ((MAXCOLUMNS+MAXROWS+MAXSTACKS)*BUCKETSIZE)
#define Distance(x1,y1,z1,x2,y2,z2) sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)+(z1-z2)*(z1-z2))

#define IP (M_PI / 180.0)

#define COLORBASE 3
#define COLORS (COLORBASE + 2)

#define BLACK 0
#define RED   1
#define GREEN 2
#define BLUE  3
#define UV    3 /* Place holder... make BLUE for now */
#define WHITE 4

typedef struct _CellList {
	unsigned char x, y, z;	/* Location in world coordinates */
	char        visible;	/* 1 if the cube is to be displayed */
	short       dist;	/* dist from cell to eye */
	struct _CellList *next;	/* pointer to next entry on linked list */
	struct _CellList *prev;
	struct _CellList *priority;
} CellList;

typedef struct {
	Bool        painted;
	int         neighbors;
	paramstruct param;
	int         pattern, patterned_rule;
	int         ox, oy, oz;	/* origin */
	double      vx, vy, vz;	/* viewpoint */
	int         generation;
	int         ncols, nrows, nstacks;
	int         memstart;
	char        visible;
	int         noChangeCount;
	int         ncells;
	unsigned char *base[BASESIZE];
	double      A, B, C, F;
	int         width, height;
	int         init_bits;
	unsigned long colors[COLORS];
	Pixmap      pixmaps[NUMSTIPPLES - 1];
	GC          stippledGC;
	double      azm;
	double      metaAlt, metaAzm, metaDist;
	CellList   *ptrhead, *ptrend, eraserhead, eraserend;
	CellList   *buckethead[NBUCKETS], *bucketend[NBUCKETS];
	Bool        wireframe;
	Pixmap      dbuf;
	int         allPatterns, allGliders;
	paramstruct input_param;
	int         labelOffsetX, labelOffsetY;
	Bool        foundBirth, foundSurvival;
	char        ruleString[80], nameString[80];
} life3dstruct;

static life3dstruct *life3ds = (life3dstruct *) NULL;

static char *filePattern = (char *) NULL;

#if 0
static int
invplot(int local_neighbors)
{
	switch (local_neighbors) {
	case 6: /* faces only */
		return 0;
	case 8: /* corners only */
		return 1;
	case 12: /* edges only, i.e. only odd #'d cells, hexagonal close packing */
		return 2;
	case 14: /* cubic close packing */
		return 3;
	case -14: /* faces and corners */
		return 4;
	case 16: /* tetrahedral */
		return 5;
	case 18: /* faces and edges */
		return 6;
	case -18: /* hexagonal close and corners */
		return 7;
	case 20: /* corners and edges */
		return 8;
	case 22: /* tetrahedral */
		return 9;
	case 26: /* all */
		return 10;
	default:
		(void) fprintf(stderr, "no neighborhood like %d known\n",
				local_neighbors);
			return 9;
	}
}
#endif

#define tetra(x, y, z)  (((x % 2) == 1) && ((y % 2) == 1) && ((z % 2) == 1))
#define rTetra(x, y, z) (tetra(x, y, z) && (((x + y + z) % 4) == 1))
#define lTetra(x, y, z) (tetra(x, y, z) && (((x + y + z) % 4) == 3))

static int
codeToPatternedRule(int local_neighbors, paramstruct param)
{
	unsigned int i;

	switch (local_neighbors) {
	case 12:
		for (i = 0; i < LIFE_12RULES; i++)
			if (param_12rules[i].birth == param.birth &&
					param_12rules[i].survival == param.survival)
				return i;
		return LIFE_12RULES;
	case 14:
		for (i = 0; i < LIFE_14RULES; i++)
			if (param_14rules[i].birth == param.birth &&
					param_14rules[i].survival == param.survival)
				return i;
		return LIFE_14RULES;
	case 18:
		for (i = 0; i < LIFE_18RULES; i++)
			if (param_18rules[i].birth == param.birth &&
					param_18rules[i].survival == param.survival)
				return i;
		return LIFE_18RULES;
	case -18:
		for (i = 0; i < LIFE__18RULES; i++)
			if (param__18rules[i].birth == param.birth &&
					param__18rules[i].survival == param.survival)
				return i;
		return LIFE__18RULES;
	case 20:
		for (i = 0; i < LIFE_20RULES; i++)
			if (param_20rules[i].birth == param.birth &&
					param_20rules[i].survival == param.survival)
				return i;
		return LIFE_20RULES;
	case 22:
		for (i = 0; i < LIFE_22RULES; i++)
			if (param_22rules[i].birth == param.birth &&
					param_22rules[i].survival == param.survival)
				return i;
		return LIFE_22RULES;
	case 26:
		for (i = 0; i < LIFE_26RULES; i++)
			if (param_26rules[i].birth == param.birth &&
					param_26rules[i].survival == param.survival)
				return i;
		return LIFE_26RULES;
	}
	return 0;
}

static void
copyFromPatternedRule(int local_neighbors, paramstruct * param,
		int patterned_rule)
{
	switch (local_neighbors) {
	case 12:
		param->birth = param_12rules[patterned_rule].birth;
		param->survival = param_12rules[patterned_rule].survival;
		break;
	case 14:
		param->birth = param_14rules[patterned_rule].birth;
		param->survival = param_14rules[patterned_rule].survival;
		break;
	case 18:
		param->birth = param_18rules[patterned_rule].birth;
		param->survival = param_18rules[patterned_rule].survival;
		break;
	case -18:
		param->birth = param__18rules[patterned_rule].birth;
		param->survival = param__18rules[patterned_rule].survival;
		break;
	case 20:
		param->birth = param_20rules[patterned_rule].birth;
		param->survival = param_20rules[patterned_rule].survival;
		break;
	case 22:
		param->birth = param_22rules[patterned_rule].birth;
		param->survival = param_22rules[patterned_rule].survival;
		break;
	case 26:
		param->birth = param_26rules[patterned_rule].birth;
		param->survival = param_26rules[patterned_rule].survival;
		break;
	}
}

static void
printRule(int local_neighbors, char * string, paramstruct param, Bool verbose)
{
	int i = 1, l;

	string[0] = 'B';

	if (verbose)
		(void) fprintf(stdout,
			"rule (Birth/Survival %d neighborhood): ",
			local_neighbors);
	for (l = 0; l <= ABS(local_neighbors) && l < 10; l++) {
		if (param.birth & (1 << l)) {
			(void) sprintf(&(string[i]), "%d", l);
			i++;
		}
	}
	(void) sprintf(&(string[i]), "/S");
	i += 2;
	for (l = 0; l <= ABS(local_neighbors) && l < 10; l++) {
		if (param.survival & (1 << l)) {
			(void) sprintf(&(string[i]), "%d", l);
			i++;
		}
	}
	string[i] = '\0';
	if (verbose)
		(void) fprintf(stdout,
			"%s\nbinary rule: Birth 0x%X, Survival 0x%X\n",
			string, param.birth, param.survival);
}

static void
positionOfNeighbor(life3dstruct * lp, int n, unsigned int *col,
	unsigned int *row, unsigned int *stack)
{
	/* 16 and 22 are tetrahedral, extra space was given to */
	/* allow for octahedron positions not present here */
	if (lp->neighbors == 16 || lp->neighbors == 22) {
		if (rTetra(*col, *row, *stack)) {
			if (lp->neighbors == 16) {
				if (n >= 16)
					return;
				n = n + 10;
			} else if (lp->neighbors == 22) {
				if (n >= 22)
					return;
				n = n + 4;
			}
			switch(n) {
			/* edges (tetra) */
			case 4:
				*col = (*col < 2) ? lp->ncols - 2 + *col : *col - 2;
				break;
			case 5:
				*col = (*col + 2 >= lp->ncols) ? *col + 2 - lp->ncols : *col + 2;
				break;
			case 6:
				*row = (*row < 2) ? lp->nrows - 2 + *row : *row - 2;
				break;
			case 7:
				*row = (*row + 2 >= lp->nrows) ? *row + 2 - lp->nrows : *row + 2;
				break;
			case 8:
				*stack = (*stack < 2) ? lp->nstacks - 2 + *stack : *stack - 2;
				break;
			case 9:
				*stack = (*stack + 2 >= lp->nstacks) ? *stack + 2 - lp->nstacks : *stack + 2;
				break;
			/* points (tetra) */
			case 10:
				*col = (*col < 2) ? lp->ncols - 2 + *col : *col - 2;
				*row = (*row < 2) ? lp->nrows - 2 + *row : *row - 2;
				break;
			case 11:
				*col = (*col + 2 >= lp->ncols) ? *col + 2 - lp->ncols : *col + 2;
				*row = (*row < 2) ? lp->nrows - 2 + *row : *row - 2;
				break;
			case 12:
				*col = (*col < 2) ? lp->ncols - 2 + *col : *col - 2;
				*row = (*row + 2 >= lp->nrows) ? *row + 2 - lp->nrows : *row + 2;
				break;
			case 13:
				*col = (*col + 2 >= lp->ncols) ? *col + 2 - lp->ncols : *col + 2;
				*row = (*row + 2 >= lp->nrows) ? *row + 2 - lp->nrows : *row + 2;
				break;
			case 14:
				*row = (*row < 2) ? lp->nrows - 2 + *row : *row - 2;
				*stack = (*stack < 2) ? lp->nstacks - 2 + *stack : *stack - 2;
				break;
			case 15:
				*row = (*row + 2 >= lp->nrows) ? *row + 2 - lp->nrows : *row + 2;
				*stack = (*stack < 2) ? lp->nstacks - 2 + *stack : *stack - 2;
				break;
			case 16:
				*row = (*row < 2) ? lp->nrows - 2 + *row : *row - 2;
				*stack = (*stack + 2 >= lp->nstacks) ? *stack + 2 - lp->nstacks : *stack + 2;
				break;
			case 17:
				*row = (*row + 2 >= lp->nrows) ? *row + 2 - lp->nrows : *row + 2;
				*stack = (*stack + 2 >= lp->nstacks) ? *stack + 2 - lp->nstacks : *stack + 2;
				break;
			case 18:
				*col = (*col < 2) ? lp->ncols - 2 + *col : *col - 2;
				*stack = (*stack < 2) ? lp->nstacks - 2 + *stack : *stack - 2;
				break;
			case 19:
				*col = (*col + 2 >= lp->ncols) ? *col + 2 - lp->ncols : *col + 2;
				*stack = (*stack < 2) ? lp->nstacks - 2 + *stack : *stack - 2;
				break;
			case 20:
				*col = (*col < 2) ? lp->ncols - 2 + *col : *col - 2;
				*stack = (*stack + 2 >= lp->nstacks) ? *stack + 2 - lp->nstacks : *stack + 2;
				break;
			case 21:
				*col = (*col + 2 >= lp->ncols) ? *col + 2 - lp->ncols : *col + 2;
				*stack = (*stack + 2 >= lp->nstacks) ? *stack + 2 - lp->nstacks : *stack + 2;
				break;
			/* far points (tetra) */
			case 22:
				*col = (*col < 2) ? lp->ncols - 2 + *col : *col - 2;
				*row = (*row < 2) ? lp->nrows - 2 + *row : *row - 2;
				*stack = (*stack < 2) ? lp->nstacks - 2 + *stack : *stack - 2;
				break;
			case 23:
				*col = (*col + 2 >= lp->ncols) ? *col + 2 - lp->ncols : *col + 2;
				*row = (*row + 2 >= lp->nrows) ? *row + 2 - lp->nrows : *row + 2;
				*stack = (*stack < 2) ? lp->nstacks - 2 + *stack : *stack - 2;
				break;
			case 24:
				*col = (*col + 2 >= lp->ncols) ? *col + 2 - lp->ncols : *col + 2;
				*row = (*row < 2) ? lp->nrows - 2 + *row : *row - 2;
				*stack = (*stack + 2 >= lp->nstacks) ? *stack + 2 - lp->nstacks : *stack + 2;
				break;
			case 25:
				*col = (*col < 2) ? lp->ncols - 2 + *col : *col - 2;
				*row = (*row + 2 >= lp->nrows) ? *row + 2 - lp->nrows : *row + 2;
				*stack = (*stack + 2 >= lp->nstacks) ? *stack + 2 - lp->nstacks : *stack + 2;
				break;
			default:
				break;
			}
		} else if (lTetra(*col, *row, *stack)) {
			if (lp->neighbors == 16) {
				if (n >= 16)
					return;
				n = n + 10;
			} else if (lp->neighbors == 22) {
				if (n >= 22)
					return;
				n = n + 4;
			}
			switch(n) {
			/* edges (tetra) */
			case 4:
				*col = (*col < 2) ? lp->ncols - 2 + *col : *col - 2;
				break;
			case 5:
				*col = (*col + 2 >= lp->ncols) ? *col + 2 - lp->ncols : *col + 2;
				break;
			case 6:
				*row = (*row < 2) ? lp->nrows - 2 + *row : *row - 2;
				break;
			case 7:
				*row = (*row + 2 >= lp->nrows) ? *row + 2 - lp->nrows : *row + 2;
				break;
			case 8:
				*stack = (*stack < 2) ? lp->nstacks - 2 + *stack : *stack - 2;
				break;
			case 9:
				*stack = (*stack + 2 >= lp->nstacks) ? *stack + 2 - lp->nstacks : *stack + 2;
				break;
			/* points (tetra) */
			case 10:
				*col = (*col < 2) ? lp->ncols - 2 + *col : *col - 2;
				*row = (*row < 2) ? lp->nrows - 2 + *row : *row - 2;
				break;
			case 11:
				*col = (*col + 2 >= lp->ncols) ? *col + 2 - lp->ncols : *col + 2;
				*row = (*row < 2) ? lp->nrows - 2 + *row : *row - 2;
				break;
			case 12:
				*col = (*col < 2) ? lp->ncols - 2 + *col : *col - 2;
				*row = (*row + 2 >= lp->nrows) ? *row + 2 - lp->nrows : *row + 2;
				break;
			case 13:
				*col = (*col + 2 >= lp->ncols) ? *col + 2 - lp->ncols : *col + 2;
				*row = (*row + 2 >= lp->nrows) ? *row + 2 - lp->nrows : *row + 2;
				break;
			case 14:
				*row = (*row < 2) ? lp->nrows - 2 + *row : *row - 2;
				*stack = (*stack < 2) ? lp->nstacks - 2 + *stack : *stack - 2;
				break;
			case 15:
				*row = (*row + 2 >= lp->nrows) ? *row + 2 - lp->nrows : *row + 2;
				*stack = (*stack < 2) ? lp->nstacks - 2 + *stack : *stack - 2;
				break;
			case 16:
				*row = (*row < 2) ? lp->nrows - 2 + *row : *row - 2;
				*stack = (*stack + 2 >= lp->nstacks) ? *stack + 2 - lp->nstacks : *stack + 2;
				break;
			case 17:
				*row = (*row + 2 >= lp->nrows) ? *row + 2 - lp->nrows : *row + 2;
				*stack = (*stack + 2 >= lp->nstacks) ? *stack + 2 - lp->nstacks : *stack + 2;
				break;
			case 18:
				*col = (*col < 2) ? lp->ncols - 2 + *col : *col - 2;
				*stack = (*stack < 2) ? lp->nstacks - 2 + *stack : *stack - 2;
				break;
			case 19:
				*col = (*col + 2 >= lp->ncols) ? *col + 2 - lp->ncols : *col + 2;
				*stack = (*stack < 2) ? lp->nstacks - 2 + *stack : *stack - 2;
				break;
			case 20:
				*col = (*col < 2) ? lp->ncols - 2 + *col : *col - 2;
				*stack = (*stack + 2 >= lp->nstacks) ? *stack + 2 - lp->nstacks : *stack + 2;
				break;
			case 21:
				*col = (*col + 2 >= lp->ncols) ? *col + 2 - lp->ncols : *col + 2;
				*stack = (*stack + 2 >= lp->nstacks) ? *stack + 2 - lp->nstacks : *stack + 2;
				break;
			/* far points (tetra) */
			case 22:
				*col = (*col + 2 >= lp->ncols) ? *col + 2 - lp->ncols : *col + 2;
				*row = (*row < 2) ? lp->nrows - 2 + *row : *row - 2;
				*stack = (*stack < 2) ? lp->nstacks - 2 + *stack : *stack - 2;
				break;
			case 23:
				*col = (*col < 2) ? lp->ncols - 2 + *col : *col - 2;
				*row = (*row + 2 >= lp->nrows) ? *row + 2 - lp->nrows : *row + 2;
				*stack = (*stack < 2) ? lp->nstacks - 2 + *stack : *stack - 2;
				break;
			case 24:
				*col = (*col < 2) ? lp->ncols - 2 + *col : *col - 2;
				*row = (*row < 2) ? lp->nrows - 2 + *row : *row - 2;
				*stack = (*stack + 2 >= lp->nstacks) ? *stack + 2 - lp->nstacks : *stack + 2;
				break;
			case 25:
				*col = (*col + 2 >= lp->ncols) ? *col + 2 - lp->ncols : *col + 2;
				*row = (*row + 2 >= lp->nrows) ? *row + 2 - lp->nrows : *row + 2;
				*stack = (*stack + 2 >= lp->nstacks) ? *stack + 2 - lp->nstacks : *stack + 2;
				break;
			default:
				break;
			}
		}
		return;
	}
	/* faces */
	if (lp->neighbors == 6 || lp->neighbors == 14 || lp->neighbors == -14 ||
				lp->neighbors == 18 || lp->neighbors == 26) {
		switch (n) {
		case 0:
			*col = (*col + 1 == lp->ncols) ? 0 : *col + 1;
			break;
		case 1:
			*row = (*row == 0) ? lp->nrows - 1 : *row - 1;
			break;
		case 2:
			*col = (*col == 0) ? lp->ncols - 1 : *col - 1;
			break;
		case 3:
			*row = (*row + 1 == lp->nrows) ? 0 : *row + 1;
			break;
		case 4:
			if (lp->neighbors == 14) {
				if (*stack + 1 == lp->nstacks)
					*stack = 1;
				else if (*stack + 2 == lp->nstacks)
					*stack = 0;
				else
					*stack = *stack + 2;
			} else {
				*stack = (*stack + 1 == lp->nstacks) ? 0 : *stack + 1;
			}
			break;
		case 5:
			if (lp->neighbors == 14) {
				if (*stack == 0)
					*stack = lp->nstacks - 2;
				else if ((*stack  - 1) == 0)
					*stack = lp->nstacks - 1;
				else
					*stack = *stack - 2;
			} else {
				*stack = (*stack == 0) ? lp->nstacks - 1 : *stack - 1;
			}
			break;
		}
	} else {
		n += 6;
	}
	/* edges */
	if (lp->neighbors == 12 || lp->neighbors == 18 || lp->neighbors == -18 ||
				lp->neighbors == 20 || lp->neighbors == 26) {
		switch (n) {
		case 6:
			*col = (*col == 0) ? lp->ncols - 1 : *col - 1;
			*row = (*row == 0) ? lp->nrows - 1 : *row - 1;
			break;
		case 7:
			*col = (*col + 1 == lp->ncols) ? 0 : *col + 1;
			*row = (*row == 0) ? lp->nrows - 1 : *row - 1;
			break;
		case 8:
			*col = (*col == 0) ? lp->ncols - 1 : *col - 1;
			*row = (*row + 1 == lp->nrows) ? 0 : *row + 1;
			break;
		case 9:
			*col = (*col + 1 == lp->ncols) ? 0 : *col + 1;
			*row = (*row + 1 == lp->nrows) ? 0 : *row + 1;
			break;
		case 10:
			*row = (*row == 0) ? lp->nrows - 1 : *row - 1;
			*stack = (*stack + 1 == lp->nstacks) ? 0 : *stack + 1;
			break;
		case 11:
			*row = (*row + 1 == lp->nrows) ? 0 : *row + 1;
			*stack = (*stack + 1 == lp->nstacks) ? 0 : *stack + 1;
			break;
		case 12:
			*row = (*row == 0) ? lp->nrows - 1 : *row - 1;
			*stack = (*stack == 0) ? lp->nstacks - 1 : *stack - 1;
			break;
		case 13:
			*row = (*row + 1 == lp->nrows) ? 0 : *row + 1;
			*stack = (*stack == 0) ? lp->nstacks - 1 : *stack - 1;
			break;
		case 14:
			*col = (*col == 0) ? lp->ncols - 1 : *col - 1;
			*stack = (*stack + 1 == lp->nstacks) ? 0 : *stack + 1;
			break;
		case 15:
			*col = (*col + 1 == lp->ncols) ? 0 : *col + 1;
			*stack = (*stack + 1 == lp->nstacks) ? 0 : *stack + 1;
			break;
		case 16:
			*col = (*col == 0) ? lp->ncols - 1 : *col - 1;
			*stack = (*stack == 0) ? lp->nstacks - 1 : *stack - 1;
			break;
		case 17:
			*col = (*col + 1 == lp->ncols) ? 0 : *col + 1;
			*stack = (*stack == 0) ? lp->nstacks - 1 : *stack - 1;
			break;
		}
	} else {
		n += 12;
	}
	/* corners */
	if (lp->neighbors == 8 || lp->neighbors == 14 || lp->neighbors == -14 ||
			lp->neighbors == 20 || lp->neighbors == 26) {
		switch (n) {
		case 18:
			*stack = (*stack + 1 == lp->nstacks) ? 0 : *stack + 1;
			if (lp->neighbors != 14 || (*stack & 1) == 0)
			*col = (*col + 1 == lp->ncols) ? 0 : *col + 1;
			if (lp->neighbors != 14 || (*stack & 1) == 1)
			*row = (*row == 0) ? lp->nrows - 1 : *row - 1;
			break;
		case 19:
			*stack = (*stack + 1 == lp->nstacks) ? 0 : *stack + 1;
			if (lp->neighbors != 14 || (*stack & 1) == 0)
			*col = (*col + 1 == lp->ncols) ? 0 : *col + 1;
			if (lp->neighbors != 14 || (*stack & 1) == 0)
			*row = (*row + 1 == lp->nrows) ? 0 : *row + 1;
			break;
		case 20:
			*stack = (*stack + 1 == lp->nstacks) ? 0 : *stack + 1;
			if (lp->neighbors != 14 || (*stack & 1) == 1)
			*col = (*col == 0) ? lp->ncols - 1 : *col - 1;
			if (lp->neighbors != 14 || (*stack & 1) == 1)
			*row = (*row == 0) ? lp->nrows - 1 : *row - 1;
			break;
		case 21:
			*stack = (*stack + 1 == lp->nstacks) ? 0 : *stack + 1;
			if (lp->neighbors != 14 || (*stack & 1) == 1)
			*col = (*col == 0) ? lp->ncols - 1 : *col - 1;
			if (lp->neighbors != 14 || (*stack & 1) == 0)
			*row = (*row + 1 == lp->nrows) ? 0 : *row + 1;
			break;
		case 22:
			*stack = (*stack == 0) ? lp->nstacks - 1 : *stack - 1;
			if (lp->neighbors != 14 || (*stack & 1) == 0)
			*col = (*col + 1 == lp->ncols) ? 0 : *col + 1;
			if (lp->neighbors != 14 || (*stack & 1) == 1)
			*row = (*row == 0) ? lp->nrows - 1 : *row - 1;
			break;
		case 23:
			*stack = (*stack == 0) ? lp->nstacks - 1 : *stack - 1;
			if (lp->neighbors != 14 || (*stack & 1) == 0)
			*col = (*col + 1 == lp->ncols) ? 0 : *col + 1;
			if (lp->neighbors != 14 || (*stack & 1) == 0)
			*row = (*row + 1 == lp->nrows) ? 0 : *row + 1;
			break;
		case 24:
			*stack = (*stack == 0) ? lp->nstacks - 1 : *stack - 1;
			if (lp->neighbors != 14 || (*stack & 1) == 1)
			*col = (*col == 0) ? lp->ncols - 1 : *col - 1;
			if (lp->neighbors != 14 || (*stack & 1) == 1)
			*row = (*row == 0) ? lp->nrows - 1 : *row - 1;
			break;
		case 25:
			*stack = (*stack == 0) ? lp->nstacks - 1 : *stack - 1;
			if (lp->neighbors != 14 || (*stack & 1) == 1)
			*col = (*col == 0) ? lp->ncols - 1 : *col - 1;
			if (lp->neighbors != 14 || (*stack & 1) == 0)
			*row = (*row + 1 == lp->nrows) ? 0 : *row + 1;
			break;
		}
	} else {
		n += 8;
	}
	/* far corners */
	if (neighbors == -18) {
		switch (n) {
		case 26:
			*col = (*col < 2) ? lp->ncols - 2 + *col : *col - 2;
			break;
		case 27:
			*col = (*col + 2 >= lp->ncols) ? *col + 2 - lp->ncols : *col + 2;
			break;
		case 28:
			*row = (*row < 2) ? lp->nrows - 2 + *row : *row - 2;
			break;
		case 29:
			*row = (*row + 2 >= lp->nrows) ? *row + 2 - lp->nrows : *row + 2;
			break;
		case 30:
			*stack = (*stack < 2) ? lp->nstacks - 2 + *stack : *stack - 2;
			break;
		case 31:
			*stack = (*stack + 2 >= lp->nstacks) ? *stack + 2 - lp->nstacks : *stack + 2;
			break;
		default:
			break;
		}
	}
}

/*-
 * This stuff is not good for rules above 9 cubes but it is unlikely that
 * these modes would be much good anyway....  death assumed.
 */
static void
parseRule(ModeInfo * mi, char * string)
{
	life3dstruct *lp = &life3ds[MI_SCREEN(mi)];
	int n, l = 0;
	char serving = 0;

	if (lp->foundBirth && lp->foundSurvival)
		return;
	lp->foundBirth = lp->foundSurvival = False;
	lp->input_param.birth = lp->input_param.survival = 0;
	if (rule) {
		n = 0;
		while (rule[n]) {
			if (rule[n] == 'P' || rule[n] == 'p') {
				lp->allPatterns = True;
				lp->foundBirth = lp->foundSurvival = True;
				if (MI_IS_VERBOSE(mi))
					(void) fprintf(stdout, "rule: All rules with known patterns\n");
				return;
			} else if (rule[n] == 'G' || rule[n] == 'g') {
				lp->allGliders = True;
				lp->foundBirth = lp->foundSurvival = True;
				if (MI_IS_VERBOSE(mi))
					(void) fprintf(stdout, "rule: All rules with known gliders\n");
				return;
			} else if (rule[n] == 'B' || rule[n] == 'F' ||
					rule[n] == 'b' || rule[n] == 'f') {
				serving = 'B';
			} else if (rule[n] == 'S' || rule[n] == 'E' || rule[n] == 'L' ||
					rule[n] == 's' || rule[n] == 'e' || rule[n] == 'l') {
				serving = 'S';
			} else {
				l = rule[n] - '0';
				if (l >= 0 && l <= 9 && l <= ABS(lp->neighbors)) {	/* no 10..26 */
					if (serving == 'B' || rule[n] == 'b') {
						lp->foundBirth = True;
						lp->input_param.birth |= (1 << l);
					} else if (serving == 'S' || rule[n] == 's') {
						lp->foundSurvival = True;
						lp->input_param.survival |= (1 << l);
					}
				}
			}
			n++;
		}
	}
	if (!lp->foundBirth || !lp->foundSurvival ||
			!(lp->input_param.birth || lp->input_param.survival)) {
		/* Default to Bays' rules if rule does not make sense */
		lp->allGliders = True;
		lp->foundBirth = lp->foundSurvival = !MI_IS_FULLRANDOM(mi);
		if (MI_IS_VERBOSE(mi))
			(void) fprintf(stdout,
				"rule: Defaulting to all rules with known gliders\n");
		return;
	}
	printRule(lp->neighbors, string, lp->input_param, MI_IS_VERBOSE(mi));
}

#ifndef STANDALONE
static void
parseFile(ModeInfo *mi)
{
	FILE *file;
	static Bool done = False;
	int firstx, firsty, x = 0, y = 0, z = 0, i = 0;
	int c, cprev = ' ', size;
	char line[256];

	if (done)
		return;
	done = True;
	if (MI_IS_FULLRANDOM(mi) || !lifefile || !*lifefile)
		return;
	if ((file = my_fopenSize(lifefile, "r", &size)) == NULL) {
		(void) fprintf(stderr, "could not read file \"%s\"\n", lifefile);
		return;
	}
	for (;;) {
		if (!fgets(line, 256, file)) {
			(void) fprintf(stderr,
				"could not read header of file \"%s\"\n",
				lifefile);
			(void) fclose(file);
			return;
		}
		if (strncmp(line, "#P", (size_t) 2) == 0 &&
				sscanf(line, "#P %d %d %d", &x, &y, &z) == 3)
			break;
	}
	c = getc(file);
	while (c != EOF && !(c == '0' || c == 'O' || c == '*' || c == '.')) {
		c = getc(file);
	}
	if (c == EOF || x <= -127 || y <= -127 || z <= -127 ||
			x >= 127 || y >= 127 || z >= 127) {
		(void) fprintf(stderr,
			"corrupt file \"%s\" or file to large\n", lifefile);
		(void) fclose(file);
		return;
	}
	firstx = x;
	firsty = y;
	if ((filePattern = (char *) malloc((3 * size) *
			 sizeof (char))) == NULL) {
		(void) fprintf(stderr, "not enough memory\n");
		(void) fclose(file);
	}

	while (c != EOF && x < 127 && y < 127 && z < 127 && i < 3 * size) {
		if (c == '0' || c == 'O' || c == '*') {
			filePattern[i++] = x++;
			filePattern[i++] = y;
			filePattern[i++] = z;
		} else if (c == '.') {
			x++;
		} else if (c == '\n') {
			if (cprev == '\n') {
				z++;
				y = firsty;
			} else {
				x = firstx;
				y++;
			}
		}
		cprev = c;
		c = getc(file);
	}
	(void) fclose(file);
	filePattern[i] = 127;
}
#endif

/*--- list ---*/
/* initialise the state of all cells to OFF */
static void
initList(life3dstruct * lp)
{
	lp->ptrhead = lp->ptrend = (CellList *) NULL;
	lp->eraserhead.next = &lp->eraserend;
	lp->eraserend.prev = &lp->eraserhead;
	lp->ncells = 0;
}

/*-
 * Function that adds the cell (assumed live) at (x,y,z) onto the search
 * list so that it is scanned in future generations
 */
static Bool
addToList(life3dstruct * lp, unsigned int x, unsigned int y, unsigned int z)
{
	CellList *tmp;

	if ((tmp = (CellList *) malloc(sizeof (CellList))) == NULL)
		return False;
	tmp->x = x;
	tmp->y = y;
	tmp->z = z;
	if (lp->ptrhead == NULL) {
		lp->ptrhead = lp->ptrend = tmp;
		tmp->prev = (struct _CellList *) NULL;
	} else {
		lp->ptrend->next = tmp;
		tmp->prev = lp->ptrend;
		lp->ptrend = tmp;
	}
	lp->ncells++;
	lp->ptrend->next = (struct _CellList *) NULL;
	return True;
}

static void
addToEraseList(life3dstruct * lp, CellList * cell)
{
	cell->next = &lp->eraserend;
	cell->prev = lp->eraserend.prev;
	lp->eraserend.prev->next = cell;
	lp->eraserend.prev = cell;
}

static void
delFromList(life3dstruct * lp, CellList * cell)
{
	if (cell != lp->ptrhead) {
		cell->prev->next = cell->next;
	} else {
		lp->ptrhead = cell->next;
		if (lp->ptrhead != NULL)
			lp->ptrhead->prev = (struct _CellList *) NULL;
	}

	if (cell != lp->ptrend) {
		cell->next->prev = cell->prev;
	} else {
		lp->ptrend = cell->prev;
		if (lp->ptrend != NULL)
			lp->ptrend->next = (struct _CellList *) NULL;
	}

	lp->ncells--;
	addToEraseList(lp, cell);
}

static void
delFromEraseList(CellList * cell)
{
	cell->next->prev = cell->prev;
	cell->prev->next = cell->next;
	free(cell);
}

#ifdef DEBUG
static void
printState(ModeInfo * mi, int state)
{
/*	life3dstruct *lp = &life3ds[MI_SCREEN(mi)];
	CellList *curr;
	int i = 0;

	curr = lp->first[state]->next;
	(void) printf("state %d\n", state);
	while (curr != lp->last[state]) {
		(void) printf("%d: position %ld,	age %d, state %d, toggle %d\n",
			i, curr->info.position, curr->info.age,
			curr->info.state, curr->info.toggle);
		curr = curr->next;
		i++;
	} */
}

#endif

static void
printList(life3dstruct * lp, int i)
{
	CellList *ptr, *ptrnextcell;
	/* did not make this hack multiscreen ready */
	static int tcol[2], trow[2], tstack[2], tcol2[2], trow2[2], tstack2[2];
	static int tcount[2];
	int hr = lp->nrows / 2;
	int hc = lp->ncols / 2;
	int hs = lp->nstacks / 2;

	tcol[i] = 0;
	trow[i] = 0;
	tstack[i] = 0;
	tcol2[i] = 0;
	trow2[i] = 0;
	tstack2[i] = 0;
	tcount[i] = lp->ncells;
	if ((glidersearch || patternsearch) && tcount[0] < 72) {
		ptr = lp->ptrend;
		while (ptr != NULL) {
			int col, row, stack;

			ptrnextcell = ptr->prev;
			col = ptr->x;
			row = ptr->y;
			stack = ptr->z;
			(void) printf("%d, %d, %d,\n", col - hc,
				row - hr, stack - hs);
			/* fudging a signature */
			tcol[i] += col;
			trow[i] += row;
			tstack[i] += stack;
			tcol2[i] += col * col;
			trow2[i] += row * row;
			tstack2[i] += stack * stack;
			ptr = ptrnextcell;
		}

		(void) printf("%d#%d, pos_sum %d %d %d %d %d %d\n",
			i, tcount[i], tcol[i], trow[i], tstack[i],
			tcol2[i], trow2[i], tstack2[i]);
		if (i == 1 && (tcount[0] <= NUMPTS || tcount[1] <= NUMPTS) &&
				(tcount[0] != tcount[1] ||
				tcol[0] != tcol[1] || trow[0] != trow[1] ||
				tstack[0] != tstack[1] ||
				tcol2[0] != tcol2[1] || trow2[0] != trow2[1] ||
				tstack2[0] != tstack2[1]))
			(void) printf("found\n");
	}
}

/*--- memory ---*/
/*-
 * Simulate a large array by dynamically allocating 4x4x4 size cells when
 * needed.
 */
static void
MemInit(life3dstruct * lp)
{
	int i;

	for (i = 0; i < BASESIZE; ++i) {
		if (lp->base[i] != NULL) {
			free(lp->base[i]);
			lp->base[i] = (unsigned char *) NULL;
		}
	}
	lp->memstart = 0;
}

#define BASE_OFFSET(x,y,z,b,o) \
b = ((x & 0x7c) << 7) + ((y & 0x7c) << 2) + ((z & 0x7c) >> 2); \
o = (x & 3) + ((y & 3) << 2) + ((z & 3) << 4); \
if (lp->base[b] == NULL) {\
if ((lp->base[b] = (unsigned char *) calloc(64, sizeof (unsigned char))) == NULL) {return False;}}


static Bool
GetMem(life3dstruct * lp, unsigned int x, unsigned int y, unsigned int z,
		int *m)
{
	int b, o;

	if (lp->memstart) {
		MemInit(lp);
	}
	BASE_OFFSET(x, y, z, b, o);
	*m = lp->base[b][o];
	return True;
}

static Bool
SetMem(life3dstruct * lp, unsigned int x, unsigned int y, unsigned int z,
		unsigned int val)
{
	int b, o;

	if (lp->memstart) {
		MemInit(lp);
	}
#define NBUCKETS ((MAXCOLUMNS+MAXROWS+MAXSTACKS)*BUCKETSIZE)
	BASE_OFFSET(x, y, z, b, o);
	lp->base[b][o] = val;
	return True;
}

static Bool
ChangeMem(life3dstruct * lp,
		unsigned int x, unsigned int y, unsigned int z,
		unsigned int val)
{
	int b, o;

	if (lp->memstart) {
		MemInit(lp);
	}
	BASE_OFFSET(x, y, z, b, o);
	lp->base[b][o] += val;
	return True;
}

static void
ClearMem(life3dstruct * lp)
{
	int i, j, count;

	for (i = 0; i < BASESIZE; ++i)
		if (lp->base[i] != NULL) {
			for (count = j = 0; j < 64 && count == 0; ++j)
				if (CellState3D(lp->base[i][j]))
					++count;
			if (!count) {
				free(lp->base[i]);
				lp->base[i] = (unsigned char *) NULL;
			}
		}
}

/* Checks to make sure there is a cell there */
static Bool cellCheck(int neighbors, int x, int y, int z)
{
	if (neighbors == 16 || neighbors == 22)
		return tetra(x, y, z);
	else
		return ((neighbors != 12 && neighbors != -18) ||
			((x + y + z) & 1) == 0);
}

/* Checks to make sure it wraps well */
/*
static Bool boundsCheck12(int neighbors, int x, int y, int z) {
	return ((neighbors != 12 && neighbors != -18) ||
		((x + y + z) & 1) == 0);
}

static Bool boundsCheck14(int neighbors, int z) {
	return (neighbors != 14 || (z & 1) == 0);
}

static Bool boundsCheck22(int neighbors, int x, int y, int z) {
	return (neighbors != 22 || (((x % 4) == 0) &&
		((y % 4) == 0) && ((z % 4) == 0)));
}
*/

/*-
 * This routine increments the values stored in the 27 cells centered on
 * (x,y,z) Note that the offset() macro implements wrapping - the world is a
 * 4d torus
 */
static Bool
IncrementNbrs3D(life3dstruct * lp, CellList * cell)
{
	unsigned int xc, yc, zc, x, y, z, i;

	for (i = 0; i < ABS(lp->neighbors); i++) {
		x = xc = cell->x;
		y = yc = cell->y;
		z = zc = cell->z;
		if (!cellCheck(lp->neighbors, x, y, z))
			continue;
	/*if (patternsearch &&
		(x == MAXCOLUMNS / 2 - 1 || y == MAXROWS / 2 - 1 || z == MAXSTACKS / 2 - 1))
		continue;*/
		positionOfNeighbor(lp, i, &x, &y, &z);
		if (x != xc || y != yc || z != zc)
			if (!ChangeMem(lp,
				  (unsigned int) x, (unsigned int) y, (unsigned int) z, 1))
				return False;
	}
	return True;
}

static void
endList(life3dstruct * lp)
{
	CellList *ptr;

	while (lp->ptrhead != NULL) {
		/* Reset3D(lp->ptrhead->x, lp->ptrhead->y, lp->ptrhead->z); */
		delFromList(lp, lp->ptrhead);
	}
	ptr = lp->eraserhead.next;
	while (ptr != &lp->eraserend) {
		delFromEraseList(ptr);
		ptr = lp->eraserhead.next;
	}
	MemInit(lp);
}

static Bool
RunLife3D(life3dstruct * lp)
{
	unsigned int i, x, y, z, xc = 0, yc = 0, zc = 0;
	int c;
	CellList *ptr, *ptrnextcell;
	Bool visible = False;

	/* Step 1 - Add 1 to all neighbors of living cells. */
	ptr = lp->ptrhead;
	while (ptr != NULL) {
		if (!IncrementNbrs3D(lp, ptr))
			return False;
		ptr = ptr->next;
	}

	/* Step 2 - Scan world and implement Survival rules. We have a list of live
	 * cells, so do the following:
	 * Start at the END of the list and work backwards (so we don't have to worry
	 * about scanning newly created cells since they are appended to the end) and
	 * for every entry, scan its neighbors for new live cells. If found, add them
	 * to the end of the list. If the centre cell is dead, unlink it.
	 * Make sure we do not append multiple copies of cells.
	 */
	ptr = lp->ptrend;
	while (ptr != NULL) {
		ptrnextcell = ptr->prev;
		for (i = 0; i < ABS(lp->neighbors); i++) {
			x = xc = ptr->x;
			y = yc = ptr->y;
			z = zc = ptr->z;
			if (!cellCheck(lp->neighbors, x, y, z))
				continue;
			positionOfNeighbor(lp, i, &x, &y, &z);
			if (x != xc || y != yc || z != zc) {
				if (!GetMem(lp, x, y, z, &c))
					return False;
				if (c) {
					if (CellState3D(c) == OFF) {
						if (lp->param.birth & (1 << CellNbrs3D(c))) {
							visible = True;
							SetList(x, y, z);
						} else {
							if (!Reset3D(x, y, z))
								return False;
						}
					}
				}
			}
		}
		if (!GetMem(lp, xc, yc, zc, &c))
			return False;
		if (lp->param.survival & (1 << CellNbrs3D(c))) {
			if (!Set3D(xc, yc, zc))
				return False;
		} else {
			if (!Reset3D(ptr->x, ptr->y, ptr->z))
				return False;
			delFromList(lp, ptr);
		}
		ptr = ptrnextcell;
	}
	ClearMem(lp);
	if (visible)
		lp->noChangeCount = 0;
	else
		lp->noChangeCount++;
	return True;
}

#if 0
static int
CountCells3D(life3dstruct * lp)
{
	CellList *ptr;
	int count = 0;

	ptr = lp->ptrhead;
	while (ptr != NULL) {
		++count;
		ptr = ptr->next;
	}
	return count;
}

void
DisplayList(life3dstruct * lp)
{
	CellList *ptr;
	int count = 0;

	ptr = lp->ptrhead;
	while (ptr != NULL) {
		(void) printf("(%x)=[%d,%d,%d] ", (int) ptr,
			ptr->x, ptr->y, ptr->z);
		ptr = ptr->next;
		++count;
	}
	(void) printf("Living cells = %d\n", count);
}

#endif

#define NOSYMRAND 0
#define ODDSYMRAND 1
#define EVENSYMRAND 2
#define ODDANTISYMRAND 3
#define EVENANTISYMRAND 4
#define DIAGSYMRAND 5

static Bool
RandomSoup(ModeInfo * mi, int n, int vx, int vy, int vz)
{
	life3dstruct *lp = &life3ds[MI_SCREEN(mi)];
	int row, col, stack;
	int xrand, yrand, zrand;
	int hr = lp->nrows / 2;
	int hc = lp->ncols / 2;
	int hs = lp->nstacks / 2;

	if (glidersearch) {
		xrand = NRAND(3);
		yrand = NRAND(3);
		zrand = NRAND(3);
	} else {
		xrand = NRAND(5);
		yrand = NRAND(5);
		zrand = NRAND(5);
	}
	if (glidersearch) {
		/* single sym or else glider pairs will crash into each other,
		hmmm, that may weed out some legitimate cases */
		if (xrand > NOSYMRAND && yrand > NOSYMRAND) {
			if (LRAND() & 1)
				xrand = NOSYMRAND;
			else
				yrand = NOSYMRAND;
		}
		if ((xrand > NOSYMRAND || yrand > NOSYMRAND) &&
				zrand > NOSYMRAND) {
			if (!NRAND(3))
				xrand =  yrand = NOSYMRAND;
			else
				zrand = NOSYMRAND;
		}
	}
	if (!NRAND(4)) {
		xrand = NOSYMRAND;
		yrand = DIAGSYMRAND;
		zrand = NRAND(3);
	}
	if (!NRAND(6)) {
		/* Full diagonal */
		xrand = NOSYMRAND;
		yrand = NOSYMRAND;
		zrand = DIAGSYMRAND;
	}
	if (zrand == DIAGSYMRAND) {
		if ((vx != vy) || (vy != vz))
			vx = vy = vz = MIN(vx, MIN(vy, vz));
	} else if (yrand == DIAGSYMRAND) {
		if (vx != vy)
			vx = vy = MIN(vx, vy);
	}
	vx /= 2;
	vy /= 2;
	vz /= 2;
	if (vx < 1)
		vx = 1;
	if (vy < 1)
		vy = 1;
	if (vz < 1)
		vz = 1;

	if (xrand == NOSYMRAND && yrand == NOSYMRAND && zrand == NOSYMRAND) {
	    for (stack = 0; stack < 2 * vz; ++stack)
		for (row = 0; row < 2 * vy; ++row)
		    for (col = 0; col < 2 * vx; ++col)
			if (NRAND(100) < n) {
				SetList(col + hc - vx, row + hr - vy, stack + hs - vz);
			}
	    (void) strcpy(lp->nameString, "random pattern");
	    if (MI_IS_VERBOSE(mi)) {
		(void) fprintf(stdout, "%s\n", lp->nameString);
	    }
	    return True;
	}
	if (zrand == DIAGSYMRAND) {
	    for (stack = 0; stack < 2 * vz; ++stack)
		for (row = stack; row < 2 * vy; ++row)
		    for (col = row; col < 2 * vx; ++col)
			if (NRAND(100) < n) {
				SetList(col + hc - vx, row + hr - vy, stack + hs - vz);
				SetList(row + hc - vx, stack + hr - vy, col + hs - vz);
				SetList(stack + hc - vx, col + hr - vy, row + hs - vz);
			}
	    (void) strcpy(lp->nameString, "full diag random pattern");
	    if (MI_IS_VERBOSE(mi)) {
		(void) fprintf(stdout, "%s\n", lp->nameString);
	    }
	    return True;
	}
#if 0
	if (yrand == DIAGSYMRAND) {
	    for (stack = 0; stack < 2 * vz; ++stack)
		for (row = 0; row < 2 * vy; ++row)
		    for (col = row; col < 2 * vx; ++col) {
			if (NRAND(100) < n) {
				SetList(col + hc - vx, row + hr - vy, stack + hs - vz);
				SetList(row + hc - vx, col + hr - vy, stack + hs - vz);
			}
			if (zrand == ODDSYMRAND) {
				SetList(col + hc - vx, row + hr - vy, vz - stack + hs);
				SetList(row + hc - vx, col + hr - vy, vz - stack + hs);
			} else if (zrand == EVENSYMRAND) {
				SetList(col + hc - vx, row + hr - vy, vz - stack + hs - 1);
				SetList(row + hc - vx, col + hr - vy, vz - stack + hs - 1);
			}
		    }
	    (void) strcpy(lp->nameString, "diag random pattern");
	    if (MI_IS_VERBOSE(mi)) {
		(void) fprintf(stdout, "%s\n", lp->nameString);
	    }
	    return True;
	}
#endif
	if (yrand != DIAGSYMRAND)
		zrand = NOSYMRAND; /* change me */
	if (yrand == NOSYMRAND) {
		if (xrand == NOSYMRAND) {
			(void) strcpy(lp->nameString, "full random pattern");
			for (stack = hs - vz; stack < hs + vz; ++stack)
			for (row = hr - vy; row < hr + vy; ++row)
			for (col = hc - vx; col < hc + vx; ++col) {

					if (NRAND(100) < n) {
						SetList(col, row, stack);
					}
				}
		} else if (xrand == ODDSYMRAND) {
				(void) sprintf(lp->nameString, "%s %srandom pattern", "x", "odd ");
			for (stack = hs - vz; stack < hs + vz; ++stack)
			for (row = hr - vy; row < hr + vy; ++row)
			for (col = hc; col < hc + vx; ++col) {

					if (NRAND(100) < n / 2) {
						SetList(col, row, stack);
						SetList(2 * hc - col, row, stack);
					}
				}
		} else if (xrand == EVENSYMRAND) {
			(void) sprintf(lp->nameString, "%s even random pattern", "x");
			for (stack = hs - vz; stack < hs + vz; ++stack)
			for (row = hr - vy; row < hr + vy; ++row)
			for (col = hc; col < hc + vx; ++col) {

					if (NRAND(100) < n / 2) {
						SetList(col, row, stack);
						SetList(2 * hc - col - 1, row, stack);
					}
				}
		} else if (xrand == ODDANTISYMRAND) {
			(void) sprintf(lp->nameString, "%s %srandom pattern", "x", "odd antisym ");
			for (stack = hs - vz; stack < hs + vz; ++stack)
			for (row = hr - vy; row < hr + vy; ++row)
			for (col = hc; col < hc + vx; ++col) {

					if (NRAND(100) < n / 2) {
						SetList(col, row, stack);
						SetList(2 * hc - col, 2 * hr - row - 1, stack);
					}
				}
		} else if (xrand == EVENANTISYMRAND) {
			(void) sprintf(lp->nameString, "%s even antisym pattern", "x");
			for (stack = hs - vz; stack < hs + vz; ++stack)
			for (row = hr - vy; row < hr + vy; ++row)
			for (col = hc; col < hc + vx; ++col) {

					if (NRAND(100) < n / 2) {
						SetList(col, row, stack);
						SetList(2 * hc - col - 1, 2 * hr - row - 1, stack);
					}
				}
		}
	} else if (yrand == ODDSYMRAND) {
		if (xrand == NOSYMRAND) {
			(void) sprintf(lp->nameString, "%s odd random pattern", "y");
			for (stack = hs - vz; stack < hs + vz; ++stack)
			for (row = hr; row < hr + vy; ++row)
			for (col = hc - vx; col < hc + vx; ++col) {

					if (NRAND(100) < n / 2) {
						SetList(col, row, stack);
						SetList(col, 2 * hr - row, stack);
					}
				}
		} else if (xrand == ODDSYMRAND) {
			(void) sprintf(lp->nameString, "x %sand y odd random pattern", "");
			for (stack = hs - vz; stack < hs + vz; ++stack)
			for (row = hr; row < hr + vy; ++row)
			for (col = hc; col < hc + vx; ++col) {

					if (NRAND(100) < n / 4) {
						SetList(col, row, stack);
						SetList(col, 2 * hr - row, stack);
						SetList(2 * hc - col, row, stack);
						SetList(2 * hc - col, 2 * hr - row, stack);
					}
				}
		} else if (xrand == EVENSYMRAND) {
			(void) sprintf(lp->nameString, "%s random pattern", "x even and y odd");
			for (stack = hs - vz; stack < hs + vz; ++stack)
			for (row = hr; row < hr + vy; ++row)
			for (col = hc; col < hc + vx; ++col) {
					if (NRAND(100) < n / 2) {
						SetList(col, row, stack);
						SetList(2 * hc - col - 1, row, stack);
						SetList(col, 2 * hr - row, stack);
						SetList(2 * hc - col - 1, 2 * hr - row, stack);
					}
				}
		}
	} else if (yrand == EVENSYMRAND) {
		if (xrand == NOSYMRAND) {
			(void) sprintf(lp->nameString, "%s even random pattern", "y");
			for (stack = hs - vz; stack < hs + vz; ++stack)
			for (row = hr; row < hr + vy; ++row)
			for (col = hc - vx; col < hc + vx; ++col) {
					if (NRAND(100) < n / 2) {
						SetList(col, row, stack);
						SetList(col, 2 * hr - row - 1, stack);
					}
				}
		} else if (xrand == ODDSYMRAND) {
			(void) sprintf(lp->nameString, "%s random pattern", "x odd and y even");
			for (stack = hs - vz; stack < hs + vz; ++stack)
			for (row = hr; row < hr + vy; ++row)
			for (col = hc; col < hc + vx; ++col) {
					if (NRAND(100) < n / 4) {
						SetList(col, row, stack);
						SetList(2 * hc - col, row, stack);
						SetList(col, 2 * hr - row - 1, stack);
						SetList(2 * hc - col, 2 * hr - row - 1, stack);
					}
				}
		} else if (xrand == EVENSYMRAND) {
			(void) sprintf(lp->nameString, "x and y even random pattern");
			for (stack = hs - vz; stack < hs + vz; ++stack)
			for (row = hr; row < hr + vy; ++row)
			for (col = hc; col < hc + vx; ++col) {
					if (NRAND(100) < n / 4) {
						SetList(col, row, stack);
						SetList(2 * hc - col - 1, row, stack);
						SetList(col, 2 * hr - row - 1, stack);
						SetList(2 * hc - col - 1, 2 * hr - row - 1, stack);
					}
				}
		}
	} else if (yrand == ODDANTISYMRAND) {
		if (xrand == NOSYMRAND) {
			(void) sprintf(lp->nameString, "%s odd antisym pattern", "y");
			for (stack = hs - vz; stack < hs + vz; ++stack)
			for (row = hr; row < hr + vy; ++row)
			for (col = hc - vx; col < hc + vx; ++col) {

					if (NRAND(100) < n / 2) {
						SetList(col, row, stack);
						SetList(2 * hc - col - 1, 2 * hr - row, stack);
					}
				}
		}
	} else if (yrand == EVENANTISYMRAND) {
		if (xrand == NOSYMRAND) {
			(void) sprintf(lp->nameString, "%s even antisym pattern", "y");
			for (stack = hs - vz; stack < hs + vz; ++stack)
			for (row = hr; row < hr + vy; ++row)
			for (col = hc - vx; col < hc + vx; ++col) {

					if (NRAND(100) < n / 2) {
						SetList(col, row, stack);
						SetList(2 * hc - col - 1, 2 * hr - row - 1, stack);
					}
				}
		}
	} else if (yrand == DIAGSYMRAND) {
		if (glidersearch) {
			/* single sym or else glider pairs will crash into each other */
			xrand = NRAND(2);
		} else {
			xrand = NRAND(4);
		}
		vx = MIN(vx, vy);
		if (xrand == 0) {
			(void) strcpy(lp->nameString, "\\ random pattern");
			for (stack = hs - vz; stack < hs + vz; ++stack)
			for (row = hr - vx; row < hr + vx; ++row) {
				for (col = row - hr + hc; col < hc + vx; ++col) {
					if (NRAND(100) < n / 2) {
						SetList(col, row, stack);
						SetList(row + hc - hr, col + hr - hc, stack);
					}
				}
			}
		} else if (xrand == 1) {
			(void) strcpy(lp->nameString, "/ random pattern");
			for (stack = hs - vz; stack < hs + vz; ++stack)
			for (row = hr - vx; row < hr + vx; ++row) {
				for (col = row - hr + hc; col < hc + vx; ++col) {
					if (NRAND(100) < n / 2) {
						SetList(2 * hc - col, row, stack);
						SetList(hc - row + hr, col + hr - hc, stack);
					}
				}
			}
		} else if (xrand == 2) {
			(void) strcpy(lp->nameString, "symmetric diagonal random pattern");
			for (stack = hs - vz; stack < hs + vz; ++stack)
			for (row = hr - vx; row < hr + vx; ++row) {
				for (col = row - hr + hc; col < hc + vx; ++col) {
					if (NRAND(100) < n / 4) {
						SetList(2 * hc - col, row, stack);
						SetList(hc - row + hr, col + hr - hc, stack);
						SetList(col, 2 * hr - row, stack);
						SetList(row + hc - hr, hr - col + hc, stack);
					}
				}
			}
		} else if (xrand == 3) {
			(void) strcpy(lp->nameString, "full symmetric random pattern");
			for (stack = hs - vz; stack < hs + vz; ++stack)
			for (row = hr; row < hr + vx; ++row) {
				for (col = row - hr + hc; col < hc + vx; ++col) {
					if (NRAND(100) < n / 8) {
						SetList(col, row, stack);
						SetList(row + hc - hr, col + hr - hc, stack);
						SetList(2 * hc - col, row, stack);
						SetList(hc - row + hr, col + hr - hc, stack);
						SetList(col, 2 * hr - row, stack);
						SetList(row + hc - hr, hr - col + hc, stack);
						SetList(2 * hc - col, 2 * hr - row, stack);
						SetList(hc - row + hr, hr - col + hc, stack);
					}
				}
			}
		}
	}


	if (MI_IS_VERBOSE(mi)) {
		(void) fprintf(stdout, "%s\n", lp->nameString);
	}
	return True;
}

static Bool
GetPattern(ModeInfo * mi, int pattern_rule, int pattern)
{
	life3dstruct *lp = &life3ds[MI_SCREEN(mi)];
	int x, y, z, orient, temp;
	char *patptr = (char *) NULL;
#ifdef LIFE_NAMES
	int pat = 2 * pattern + 1;
	char * patstrg = (char *) "";
#else
	int pat = pattern;
#endif

	if (filePattern) {
		patptr = &filePattern[0];
#ifdef LIFE_NAMES
		(void) strcpy(lp->nameString, patstrg);
#endif
	} else {
		switch (lp->neighbors) {
		case 12:
			switch (pattern_rule) {
			case LIFE_12B3S3:
				patptr = &patterns_12B3S3[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_12B3S3[2 * pattern][0];
#endif
				break;
			case LIFE_12B3S456:
				patptr = &patterns_12B3S456[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_12B3S456[2 * pattern][0];
#endif
				break;
			}
			break;
		case 14:
			switch (pattern_rule) {
			case LIFE_14B4S34:
				patptr = &patterns_14B4S34[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_14B4S34[2 * pattern][0];
#endif
				break;
			case LIFE_14B46S34:
				patptr = &patterns_14B46S34[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_14B46S34[2 * pattern][0];
#endif
				break;
			case LIFE_14B45S56:
				patptr = &patterns_14B45S56[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_14B45S56[2 * pattern][0];
#endif
				break;
			}
			break;
		case 18:
			switch (pattern_rule) {
			case LIFE_18B4S45:
				patptr = &patterns_18B4S45[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_18B4S45[2 * pattern][0];
#endif
				break;
			}
			break;
		case -18:
			switch (pattern_rule) {
			case LIFE__18B4S36:
				patptr = &patterns__18B4S36[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns__18B4S36[2 * pattern][0];
#endif
				break;
			case LIFE__18B4S45:
				patptr = &patterns__18B4S45[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns__18B4S45[2 * pattern][0];
#endif
				break;
			case LIFE__18B4S456:
				patptr = &patterns__18B4S456[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns__18B4S456[2 * pattern][0];
#endif
				break;
			case LIFE__18B4S46:
				patptr = &patterns__18B4S46[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns__18B4S46[2 * pattern][0];
#endif
				break;
			}
			break;
		case 20:
			switch (pattern_rule) {
			case LIFE_20B4S45:
				patptr = &patterns_20B4S45[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_20B4S45[2 * pattern][0];
#endif
				break;
			}
			break;
		case 22:
			switch (pattern_rule) {
			case LIFE_22B4S3:
				patptr = &patterns_22B4S3[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_22B4S3[2 * pattern][0];
#endif
				break;
			case LIFE_22B4S4:
				patptr = &patterns_22B4S4[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_22B4S4[2 * pattern][0];
#endif
				break;
			case LIFE_22B4S57:
				patptr = &patterns_22B4S57[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_22B4S57[2 * pattern][0];
#endif
				break;
			}
			break;
		case 26:
			switch (pattern_rule) {
			case LIFE_26B5S23:
				patptr = &patterns_26B5S23[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_26B5S23[2 * pattern][0];
#endif
				break;
			case LIFE_26B5S25:
				patptr = &patterns_26B5S25[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_26B5S25[2 * pattern][0];
#endif
				break;
			case LIFE_26B5S27:
				patptr = &patterns_26B5S27[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_26B5S27[2 * pattern][0];
#endif
				break;
			case LIFE_26B5S35:
				patptr = &patterns_26B5S35[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_26B5S35[2 * pattern][0];
#endif
				break;
			case LIFE_26B5S36:
				patptr = &patterns_26B5S36[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_26B5S36[2 * pattern][0];
#endif
				break;
			case LIFE_26B5S37:
				patptr = &patterns_26B5S37[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_26B5S37[2 * pattern][0];
#endif
				break;
			case LIFE_26B5S38:
				patptr = &patterns_26B5S38[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_26B5S38[2 * pattern][0];
#endif
				break;
			case LIFE_26B5S4:
				patptr = &patterns_26B5S4[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_26B5S4[2 * pattern][0];
#endif
				break;
			case LIFE_26B5S45:
				patptr = &patterns_26B5S45[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_26B5S45[2 * pattern][0];
#endif
				break;
			case LIFE_26B5S47:
				patptr = &patterns_26B5S47[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_26B5S47[2 * pattern][0];
#endif
				break;
			case LIFE_26B5S56:
				patptr = &patterns_26B5S56[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_26B5S56[2 * pattern][0];
#endif
				break;
			case LIFE_26B5S58:
				patptr = &patterns_26B5S58[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_26B5S58[2 * pattern][0];
#endif
				break;
			case LIFE_26B5S678:
				patptr = &patterns_26B5S678[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_26B5S678[2 * pattern][0];
#endif
				break;
			case LIFE_26B5S8:
				patptr = &patterns_26B5S8[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_26B5S8[2 * pattern][0];
#endif
				break;
			case LIFE_26B58S58:
				patptr = &patterns_26B58S58[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_26B58S58[2 * pattern][0];
#endif
				break;
			case LIFE_26B6S567:
				patptr = &patterns_26B6S567[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_26B6S567[2 * pattern][0];
#endif
				break;
			case LIFE_26B6S57:
				patptr = &patterns_26B6S57[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_26B6S57[2 * pattern][0];
#endif
				break;
			case LIFE_26B67S57:
				patptr = &patterns_26B67S57[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_26B67S57[2 * pattern][0];
#endif
				break;
			case LIFE_26B67S67:
				patptr = &patterns_26B67S67[pat][0];
#ifdef LIFE_NAMES
				patstrg = &patterns_26B67S67[2 * pattern][0];
#endif
				break;
			}
			break;
		}
#ifdef LIFE_NAMES
		(void) strcpy(lp->nameString, patstrg);
#endif
	}
#ifdef DEBUG
	orient = 0;
#else
	orient = NRAND(24);
#endif
	if (MI_IS_VERBOSE(mi) && !filePattern) {
#ifdef LIFE_NAMES
		(void) fprintf(stdout, "%s, ", patstrg);
#endif
		(void) fprintf(stdout, "table number %d\n", pattern);
	}
	while ((x = *patptr++) != 127) {
		y = *patptr++;
		z = *patptr++;
		if (orient >= 16) {
			temp = x;
			x = y;
			if (lp->neighbors == 14) {
				y = (z >= 0) ? z / 2 : (z - 1) / 2;
				z = (z & 1) + 2 * temp;
			} else {
				y = z;
				z = temp;
			}
		} else if (orient >= 8) {
			temp = x;
			if (lp->neighbors == 14) {
				x = (z >= 0) ? z / 2 : (z - 1) / 2;
				z = (z & 1) + 2 * y;
			} else {
				x = z;
				z = y;
			}
			y = temp;
		}
		if (orient % 8 >= 4) {
			if (lp->neighbors == 14 && (z & 1) == 1)
				x = -x - 1;
			else
				x = -x;
		}
		if (orient % 4 >= 2) {
			if (lp->neighbors == 14 && (z & 1) == 1)
				y = -y - 1;
			else
				y = -y;
		}
		if (orient % 2 == 1) {
			z = -z;
		}

		x += lp->ncols / 2;
		y += lp->nrows / 2;
		z += lp->nstacks / 2;
		if (x >= 0 && y >= 0 && z >= 0 &&
		    x < lp->ncols && y < lp->nrows && z < lp->nstacks) {
			SetList(x, y, z);
		}
	}
	return True;
}

static void
NewViewpoint(life3dstruct * lp, double x, double y, double z)
{
	double k, l, d1, d2;

	k = x * x + y * y;
	l = sqrt(k + z * z);
	k = sqrt(k);
	d1 = (EyeToScreen / HalfScreenD);
	d2 = EyeToScreen / (HalfScreenD * lp->height / lp->width);
	lp->A = d1 * l * (lp->width / 2) / k;
	lp->B = l * l;
	lp->C = d2 * (lp->height / 2) / k;
	lp->F = k * k;
}

static void
lissajous(life3dstruct * lp)
{
	double alt, azm, dist;

	alt = 30.0 * sin(lp->metaAlt * IP) + 45.0;
	lp->metaAlt += 1.123;
	if (lp->metaAlt >= 360.0)
		lp->metaAlt -= 360.0;
	if (lp->metaAlt < 0.0)
		lp->metaAlt += 360.0;
	azm = 30.0 * sin(lp->metaAzm * IP) + 45.0;
	lp->metaAzm += 0.987;
	if (lp->metaAzm >= 360.0)
		lp->metaAzm -= 360.0;
	if (lp->metaAzm < 0.0)
		lp->metaAzm += 360.0;
	dist = 10.0 * sin(lp->metaDist * IP) + 50.0;
	lp->metaDist += 1.0;
	if (lp->metaDist >= 360.0)
		lp->metaDist -= 360.0;
	if (lp->metaDist < 0.0)
		lp->metaDist += 360.0;
#if 0
	if (alt >= 90.0)
		alt = 90.0;
	else if (alt < -90.0)
		alt = -90.0;
#endif
	lp->azm = azm;
#ifdef DEBUG
	(void) printf("dist %g, alt %g, azm %g\n", dist, alt, azm);
#endif
	lp->vx = (sin(azm * IP) * cos(alt * IP) * dist);
	lp->vy = (cos(azm * IP) * cos(alt * IP) * dist);
	lp->vz = (sin(alt * IP) * dist);
	NewViewpoint(lp, lp->vx, lp->vy, lp->vz);
}

static void
NewPoint(life3dstruct * lp, double x, double y, double z,
		register XPoint * pts)
{
	double p1, E;

	p1 = x * lp->vx + y * lp->vy;
	E = lp->B - p1 - z * lp->vz;
	pts->x = (int) (lp->width / 2 -
		lp->A * (lp->vx * y - lp->vy * x) / E);
	pts->y = (int) (lp->height / 2 -
		lp->C * (z * lp->F - lp->vz * p1) / E);
}


/* Chain together all cells that are at the same distance.
 * These cannot mutually overlap. */
static void
SortList(life3dstruct * lp)
{
	short dist;
	double d, x, y, z, rsize;
	int i, r;
	XPoint point;
	CellList *ptr;

	for (i = 0; i < NBUCKETS; ++i)
		lp->buckethead[i] = lp->bucketend[i] = (CellList *) NULL;

	/* Calculate distances and re-arrange pointers to chain off buckets */
	ptr = lp->ptrhead;
	while (ptr != NULL) {

		x = (double) ptr->x - lp->ox;
		y = (double) ptr->y - lp->oy;
		z = (double) ptr->z - lp->oz;
		d = Distance(lp->vx, lp->vy, lp->vz, x, y, z);
		if (lp->vx * (lp->vx - x) + lp->vy * (lp->vy - y) +
		    lp->vz * (lp->vz - z) > 0 && d > 1.5)
			ptr->visible = 1;
		else
			ptr->visible = 0;

		ptr->dist = (short) d;
		dist = (short) (d * BUCKETSIZE);
		if (dist > NBUCKETS - 1)
			dist = NBUCKETS - 1;

		if (lp->buckethead[dist] == NULL) {
			lp->buckethead[dist] = lp->bucketend[dist] = ptr;
			ptr->priority = (struct _CellList *) NULL;
		} else {
			lp->bucketend[dist]->priority = ptr;
			lp->bucketend[dist] = ptr;
			lp->bucketend[dist]->priority =
				(struct _CellList *) NULL;
		}
		ptr = ptr->next;
	}

	/* Check for invisibility */
	rsize = 0.47 * lp->width / ((double) HalfScreenD * 2);
	i = (int) lp->azm;
	if (i < 0)
		i = -i;
	i = i % RT_ANGLE;
	if (i > HALFRT_ANGLE)
		i = RT_ANGLE - i;
	rsize /= cos(i * IP);

	lp->visible = 0;
	for (i = 0; i < NBUCKETS; ++i)
		if (lp->buckethead[i] != NULL) {
			ptr = lp->buckethead[i];
			while (ptr != NULL) {
				if (ptr->visible) {
					x = (double) ptr->x - lp->ox;
					y = (double) ptr->y - lp->oy;
					z = (double) ptr->z - lp->oz;
					NewPoint(lp, x, y, z, &point);
					r = (int) (rsize * (double) EyeToScreen / (double) ptr->dist);
					if (point.x + r >= 0 && point.y + r >= 0 &&
					    point.x - r < lp->width && point.y - r < lp->height)
						lp->visible = 1;
				}
				ptr = ptr->priority;
			}
		}
}

static void
drawTri(ModeInfo * mi, int color, XPoint * pts,
		int p1, int p2, int p3)
{
	Display *display = MI_DISPLAY(mi);
	GC gc = MI_GC(mi);
	life3dstruct *lp = &life3ds[MI_SCREEN(mi)];
	XPoint facepts[4];

	facepts[0] = pts[p1];
	facepts[1] = pts[p2];
	facepts[2] = pts[p3];
	facepts[3] = pts[p1];

	if (color != BLACK && MI_NPIXELS(mi) <= 2)
		XSetForeground(display, gc, MI_WHITE_PIXEL(mi));
	else
		XSetForeground(display, gc, lp->colors[color]);
	if (!lp->wireframe) {
		if (color != BLACK && MI_NPIXELS(mi) <= 2) {
			XGCValues gcv;

			gcv.stipple = lp->pixmaps[lp->colors[color]];
			gcv.foreground = MI_WHITE_PIXEL(mi);
			gcv.background = MI_BLACK_PIXEL(mi);
			XChangeGC(display, lp->stippledGC,
				GCStipple | GCForeground | GCBackground, &gcv);
			gc = lp->stippledGC;
		}
		XFillPolygon(display, (Drawable) lp->dbuf, gc, facepts, 3,
			Convex, CoordModeOrigin);
		gc = MI_GC(mi);
		if (color == BLACK || MI_NPIXELS(mi) <= 2) {
			XSetForeground(display, gc, MI_BLACK_PIXEL(mi));
		} else {
			XSetForeground(display, gc, MI_WHITE_PIXEL(mi));
		}
	}
	XDrawLines(display, (Drawable) lp->dbuf, gc, facepts, 4,
		CoordModeOrigin);
}
static void
drawQuad(ModeInfo * mi, int color, XPoint * pts,
		int p1, int p2, int p3, int p4)
{
	Display *display = MI_DISPLAY(mi);
	GC gc = MI_GC(mi);
	life3dstruct *lp = &life3ds[MI_SCREEN(mi)];
	XPoint facepts[5];

	facepts[0] = pts[p1];
	facepts[1] = pts[p2];
	facepts[2] = pts[p3];
	facepts[3] = pts[p4];
	facepts[4] = pts[p1];

	if (color != BLACK && MI_NPIXELS(mi) <= 2)
		XSetForeground(display, gc, MI_WHITE_PIXEL(mi));
	else
		XSetForeground(display, gc, lp->colors[color]);
	if (!lp->wireframe) {
		if (color != BLACK && MI_NPIXELS(mi) <= 2) {
			XGCValues gcv;

			gcv.stipple = lp->pixmaps[lp->colors[color]];
			gcv.foreground = MI_WHITE_PIXEL(mi);
			gcv.background = MI_BLACK_PIXEL(mi);
			XChangeGC(display, lp->stippledGC,
				GCStipple | GCForeground | GCBackground, &gcv);
			gc = lp->stippledGC;
		}
		XFillPolygon(display, (Drawable) lp->dbuf, gc, facepts, 4,
			Convex, CoordModeOrigin);
		gc = MI_GC(mi);
		if (color == BLACK || MI_NPIXELS(mi) <= 2) {
			XSetForeground(display, gc, MI_BLACK_PIXEL(mi));
		} else {
			XSetForeground(display, gc, MI_WHITE_PIXEL(mi));
		}
	}
	XDrawLines(display, (Drawable) lp->dbuf, gc, facepts, 5,
		CoordModeOrigin);
}

static void
drawHex(ModeInfo * mi, int color, XPoint * pts,
		int p1, int p2, int p3, int p4, int p5, int p6)
{
	Display *display = MI_DISPLAY(mi);
	GC gc = MI_GC(mi);
	life3dstruct *lp = &life3ds[MI_SCREEN(mi)];
	XPoint facepts[7];

	facepts[0] = pts[p1];
	facepts[1] = pts[p2];
	facepts[2] = pts[p3];
	facepts[3] = pts[p4];
	facepts[4] = pts[p5];
	facepts[5] = pts[p6];
	facepts[6] = pts[p1];

	if (color != BLACK && MI_NPIXELS(mi) <= 2)
		XSetForeground(display, gc, MI_WHITE_PIXEL(mi));
	else
		XSetForeground(display, gc, lp->colors[color]);
	if (!lp->wireframe) {
		if (color != BLACK && MI_NPIXELS(mi) <= 2) {
			XGCValues gcv;

			gcv.stipple = lp->pixmaps[lp->colors[color]];
			gcv.foreground = MI_WHITE_PIXEL(mi);
			gcv.background = MI_BLACK_PIXEL(mi);
			XChangeGC(display, lp->stippledGC,
				GCStipple | GCForeground | GCBackground, &gcv);
			gc = lp->stippledGC;
		}
		XFillPolygon(display, (Drawable) lp->dbuf, gc, facepts, 6,
			Convex, CoordModeOrigin);
		gc = MI_GC(mi);
		if (color == BLACK || MI_NPIXELS(mi) <= 2) {
			XSetForeground(display, gc, MI_BLACK_PIXEL(mi));
		} else {
			XSetForeground(display, gc, MI_WHITE_PIXEL(mi));
		}
	}
	XDrawLines(display, (Drawable) lp->dbuf, gc, facepts, 7,
		CoordModeOrigin);
}

#define LEN_2 0.225
#define LEN 0.45
#define LEN2 0.9

static int
DrawCube(ModeInfo * mi, CellList * cell)
{
	life3dstruct *lp = &life3ds[MI_SCREEN(mi)];
	XPoint pts[8];	/* screen coords for point */
	int i = 0, out = 0;
	unsigned int mask;
	double x, y, z;
	double dx, dy, dz;

	x = (double) cell->x - lp->ox;
	y = (double) cell->y - lp->oy;
	z = (double) cell->z - lp->oz;
	for (dz = z - LEN; dz <= z + LEN2; dz += LEN2)
		for (dy = y - LEN; dy <= y + LEN2; dy += LEN2)
			for (dx = x - LEN; dx <= x + LEN2; dx += LEN2) {
				NewPoint(lp, dx, dy, dz, &pts[i]);
				if (pts[i].x < 0 ||
				    pts[i].x >= lp->width ||
				    pts[i].y < 0 ||
				    pts[i].y >= lp->height)
					++out;
				++i;
			}
	if (out == 8)
		return (0);

	if (cell->visible)
		mask = 0xFFFF;
	else
		mask = 0x0;
	/* Only draw those faces that are visible */
	dx = lp->vx - x;
	dy = lp->vy - y;
	dz = lp->vz - z;
	/*
		6----7
	       /    /|
	      /    / |
	     2----3  |
	    |  4 |  5
	    |    | /
	    0____1/
	*/
	if (lp->wireframe) {
		if (dz <= LEN)
			drawQuad(mi, (int) (BLUE & mask), pts, 4, 5, 7, 6);
		else if (dz >= -LEN)
			drawQuad(mi, (int) (BLUE & mask), pts, 0, 1, 3, 2);
		if (dx <= LEN)
			drawQuad(mi, (int) (GREEN & mask), pts, 1, 3, 7, 5);
		else if (dx >= -LEN)
			drawQuad(mi, (int) (GREEN & mask), pts, 0, 2, 6, 4);
		if (dy <= LEN)
			drawQuad(mi, (int) (RED & mask), pts, 2, 3, 7, 6);
		else if (dy >= -LEN)
			drawQuad(mi, (int) (RED & mask), pts, 0, 1, 5, 4);
	}
	if (dz > LEN)
		drawQuad(mi, (int) (BLUE & mask), pts, 4, 5, 7, 6);
	else if (dz < -LEN)
		drawQuad(mi, (int) (BLUE & mask), pts, 0, 1, 3, 2);
	if (dx > LEN)
		drawQuad(mi, (int) (GREEN & mask), pts, 1, 3, 7, 5);
	else if (dx < -LEN)
		drawQuad(mi, (int) (GREEN & mask), pts, 0, 2, 6, 4);
	if (dy > LEN)
		drawQuad(mi, (int) (RED & mask), pts, 2, 3, 7, 6);
	else if (dy < -LEN)
		drawQuad(mi, (int) (RED & mask), pts, 0, 1, 5, 4);
	return (1);
}

static int
DrawTetrahedronTrBl(ModeInfo * mi, CellList * cell)
{
	life3dstruct *lp = &life3ds[MI_SCREEN(mi)];
	XPoint pts[4];	/* screen coords for point */
	int i = 0, out = 0;
	unsigned int mask;
	double x, y, z;
	double dx, dy, dz;

	x = (double) cell->x - lp->ox;
	y = (double) cell->y - lp->oy;
	z = (double) cell->z - lp->oz;
	dz = (z - LEN2) / 2;
	dy = (y - LEN2) / 2;
	dx = (x + LEN2) / 2;
	NewPoint(lp, dx, dy, dz, &pts[i]);
	if (pts[i].x < 0 || pts[i].x >= lp->width ||
			pts[i].y < 0 || pts[i].y >= lp->height)
		++out;
	++i;
	dz = (z - LEN2) / 2;
	dy = (y + LEN2) / 2;
	dx = (x - LEN2) / 2;
	NewPoint(lp, dx, dy, dz, &pts[i]);
	if (pts[i].x < 0 || pts[i].x >= lp->width ||
			pts[i].y < 0 || pts[i].y >= lp->height)
		++out;
	++i;
	dz = (z + LEN2) / 2;
	dy = (y - LEN2) / 2;
	dx = (x - LEN2) / 2;
	NewPoint(lp, dx, dy, dz, &pts[i]);
	if (pts[i].x < 0 || pts[i].x >= lp->width ||
			pts[i].y < 0 || pts[i].y >= lp->height)
		++out;
	++i;
	dz = (z + LEN2) / 2;
	dy = (y + LEN2) / 2;
	dx = (x + LEN2) / 2;
	NewPoint(lp, dx, dy, dz, &pts[i]);
	if (pts[i].x < 0 || pts[i].x >= lp->width ||
			pts[i].y < 0 || pts[i].y >= lp->height)
		++out;
	++i;
	if (out == 4)
		return (0);
	if (cell->visible)
		mask = 0xFFFF;
	else
		mask = 0x0;
	/* Only draw those faces that are visible */
	dx = lp->vx - x;
	dy = lp->vy - y;
	dz = lp->vz - z;
	/* back points looking through front
	       3

	   2
	  ----------
	   front points
	   1

	       0
	*/
	if (lp->wireframe) {
		if (dx + dy + dz > 0) {
			drawTri(mi, (int) (RED & mask), pts, 1, 0, 2);
		}
		if (dx + dy - dz < 0) {
			drawTri(mi, (int) (GREEN & mask), pts, 1, 3, 0);
		}
		if (dx - dy + dz < 0) {
			drawTri(mi, (int) (GREEN & mask), pts, 0, 2, 3);
		}
		if (-dx + dy + dz < 0) {
			drawTri(mi, (int) (BLUE & mask), pts, 1, 3, 2);
		}
	}
	if (dx + dy + dz <= 0) {
		drawTri(mi, (int) (RED & mask), pts, 1, 0, 2);
	}
	if (dx + dy - dz >= 0) {
		drawTri(mi, (int) (GREEN & mask), pts, 1, 3, 0);
	}
	if (dx - dy + dz >= 0) {
		drawTri(mi, (int) (GREEN & mask), pts, 0, 2, 3);
	}
	if (-dx + dy + dz >= 0) {
		drawTri(mi, (int) (BLUE & mask), pts, 1, 3, 2);
	}
	return (1);
}

static int
DrawTetrahedronTlBr(ModeInfo * mi, CellList * cell)
{
	life3dstruct *lp = &life3ds[MI_SCREEN(mi)];
	XPoint pts[4];	/* screen coords for point */
	int i = 0, out = 0;
	unsigned int mask;
	double x, y, z;
	double dx, dy, dz;

	x = (double) cell->x - lp->ox;
	y = (double) cell->y - lp->oy;
	z = (double) cell->z - lp->oz;
	dz = (z - LEN2) / 2;
	dy = (y - LEN2) / 2;
	dx = (x - LEN2) / 2;
	NewPoint(lp, dx, dy, dz, &pts[i]);
	if (pts[i].x < 0 || pts[i].x >= lp->width ||
			pts[i].y < 0 || pts[i].y >= lp->height)
		++out;
	++i;
	dz = (z - LEN2) / 2;
	dy = (y + LEN2) / 2;
	dx = (x + LEN2) / 2;
	NewPoint(lp, dx, dy, dz, &pts[i]);
	if (pts[i].x < 0 || pts[i].x >= lp->width ||
			pts[i].y < 0 || pts[i].y >= lp->height)
		++out;
	++i;
	dz = (z + LEN2) / 2;
	dy = (y - LEN2) / 2;
	dx = (x + LEN2) / 2;
	NewPoint(lp, dx, dy, dz, &pts[i]);
	if (pts[i].x < 0 || pts[i].x >= lp->width ||
			pts[i].y < 0 || pts[i].y >= lp->height)
		++out;
	++i;
	dz = (z + LEN2) / 2;
	dy = (y + LEN2) / 2;
	dx = (x - LEN2) / 2;
	NewPoint(lp, dx, dy, dz, &pts[i]);
	if (pts[i].x < 0 || pts[i].x >= lp->width ||
			pts[i].y < 0 || pts[i].y >= lp->height)
		++out;
	++i;
	if (out == 4)
		return (0);
	if (cell->visible)
		mask = 0xFFFF;
	else
		mask = 0x0;
	/* Only draw those faces that are visible */
	dx = lp->vx - x;
	dy = lp->vy - y;
	dz = lp->vz - z;
	/* back points looking through front
	   1

	       0
	  ----------
	   front points
	       3

	   2
	*/
	if (lp->wireframe) {
		if (dx + dy + dz > 0) {
			drawTri(mi, (int) (BLUE & mask), pts, 1, 0, 2);
		}
		if (dx + dy - dz < 0) {
			drawTri(mi, (int) (GREEN & mask), pts, 1, 3, 0);
		}
		if (dx - dy + dz < 0) {
			drawTri(mi, (int) (GREEN & mask), pts, 0, 2, 3);
		}
		if (-dx + dy + dz < 0) {
			drawTri(mi, (int) (RED & mask), pts, 1, 3, 2);
		}
	}
	if (dx + dy + dz <= 0) {
		drawTri(mi, (int) (BLUE & mask), pts, 1, 0, 2);
	}
	if (dx + dy - dz >= 0) {
		drawTri(mi, (int) (GREEN & mask), pts, 1, 3, 0);
	}
	if (dx - dy + dz >= 0) {
		drawTri(mi, (int) (GREEN & mask), pts, 0, 2, 3);
	}
	if (-dx + dy + dz >= 0) {
		drawTri(mi, (int) (RED & mask), pts, 1, 3, 2);
	}
	/* if both 1st and 2nd... something wrong, kludge */
	if (dx + dy - dz >= 0 && dx - dy + dz >= 0) {
		drawTri(mi, (int) (BLUE & mask), pts, 1, 0, 2);
		drawTri(mi, (int) (RED & mask), pts, 1, 3, 2);
	}
	return (1);
}

static int
DrawTetrahedron(ModeInfo * mi, CellList * cell) {
	if (rTetra(cell->x, cell->y, cell->z)) {
		return DrawTetrahedronTrBl(mi, cell);
	} else if (lTetra(cell->x, cell->y, cell->z)) {
		return DrawTetrahedronTlBr(mi, cell);
	/*} else {
		(void) fprintf(stderr, "bad cell %d %d %d\n",
			cell->x, cell->y, cell->z); */
	}
	return (0);
}

static int
DrawRhombicDodecahedron(ModeInfo * mi, CellList * cell)
{
	life3dstruct *lp = &life3ds[MI_SCREEN(mi)];
	XPoint pts[14];	/* screen coords for point */
	int i, out = 0, da, db, sortx, sorty, sortz;
	unsigned int mask;
	double x, y, z;
	double dx, dy, dz;

	x = (double) cell->x - lp->ox;
	y = (double) cell->y - lp->oy;
	z = (double) cell->z - lp->oz;
	i = 3;
	for (dx = x + LEN; dx >= x - LEN2; dx -= LEN2)
		for (dy = y + LEN; dy >= y - LEN2; dy -= LEN2)
			for (dz = z + LEN; dz >= z - LEN2; dz -= LEN2) {
				NewPoint(lp, dx, dy, dz, &pts[i]);
				if (pts[i].x < 0 ||
				    pts[i].x >= lp->width ||
				    pts[i].y < 0 ||
				    pts[i].y >= lp->height)
					++out;
				++i;
			}
	for (da = 0; da < 2; da++) {
		if (da == 1) {
			i = 0;
		} else
			i = 11; /* 3 + 8 */
		for (db = 0; db < 3; db++) {
			if (db == 0) {
				dx = ((da == 0) ? -LEN2 : LEN2) + x;
				dy = y;
				dz = z;
			} else if (db == 1) {
				dy = ((da == 0) ? -LEN2 : LEN2) + y;
				dz = z;
				dx = x;
			} else {
				dz = ((da == 0) ? -LEN2 : LEN2) + z;
				dx = x;
				dy = y;
			}
			NewPoint(lp, dx, dy, dz, &pts[i]);
			if (pts[i].x < 0 ||
			    pts[i].x >= lp->width ||
			    pts[i].y < 0 ||
			    pts[i].y >= lp->height)
				++out;
			++i;
		}
	}
	if (out == 14)
		return (0);

	if (cell->visible)
		mask = 0xFFFF;
	else
		mask = 0x0;

	/* Only draw those faces that are visible */
	dx = lp->vx - x;
	dy = lp->vy - y;
	dz = lp->vz - z;
	/* back points looking through front
	   8   1  4
	     7  3
	  11   2  0
	     9  5
	  10  12  6
	  ---------
	   front points
	  7    1  3
	     8  4
	  11  13  0
	    10  6
	  9   12  5
	*/
	if (lp->wireframe) {
		if (dx <= LEN) {
		drawQuad(mi, (int) (GREEN & mask), pts, 0, 3, 2, 5);
		drawQuad(mi, (int) (GREEN & mask), pts, 0, 6, 13, 4);
		drawQuad(mi, (int) (RED & mask), pts, 0, 4, 1, 3);
		drawQuad(mi, (int) (RED & mask), pts, 0, 5, 12, 6);
		} else if (dx >= -LEN) {
		drawQuad(mi, (int) (GREEN & mask), pts, 2, 7, 11, 9);
		drawQuad(mi, (int) (GREEN & mask), pts, 1, 8, 11, 7);
		drawQuad(mi, (int) (RED & mask), pts, 8, 13, 10, 11);
		drawQuad(mi, (int) (RED & mask), pts, 9, 11, 10, 12);
		}
		if (dy <= LEN) {
		drawQuad(mi, (int) (RED & mask), pts, 0, 4, 1, 3);
		drawQuad(mi, (int) (RED & mask), pts, 1, 8, 11, 7);
		drawQuad(mi, (int) (BLUE & mask), pts, 1, 7, 2, 3);
		drawQuad(mi, (int) (BLUE & mask), pts, 1, 4, 13, 8);
		} else if (dy >= -LEN) {
		drawQuad(mi, (int) (RED & mask), pts, 0, 5, 12, 6);
		drawQuad(mi, (int) (RED & mask), pts, 9, 11, 10, 12);
		drawQuad(mi, (int) (BLUE & mask), pts, 2, 9, 12, 5);
		drawQuad(mi, (int) (BLUE & mask), pts, 6, 12, 10, 13);
		}
		if (dz <= LEN) {
		drawQuad(mi, (int) (BLUE & mask), pts, 1, 7, 2, 3);
		drawQuad(mi, (int) (BLUE & mask), pts, 2, 9, 12, 5);
		drawQuad(mi, (int) (GREEN & mask), pts, 2, 7, 11, 9);
		drawQuad(mi, (int) (GREEN & mask), pts, 0, 3, 2, 5);
		} else if (dz >= -LEN) {
		drawQuad(mi, (int) (BLUE & mask), pts, 1, 4, 13, 8);
		drawQuad(mi, (int) (BLUE & mask), pts, 6, 12, 10, 13);
		drawQuad(mi, (int) (GREEN & mask), pts, 0, 6, 13, 4);
		drawQuad(mi, (int) (GREEN & mask), pts, 8, 13, 10, 11);
		}
	}
	sortx = sorty = sortz = 0;
	if (ABS(dz - LEN) > ABS(dy - LEN))
		sortz++;
	if (ABS(dz - LEN) > ABS(dx - LEN))
		sortz++;
	if (ABS(dy - LEN) > ABS(dx - LEN))
		sorty++;
	if (ABS(dy - LEN) > ABS(dz - LEN))
		sorty++;
	if (ABS(dx - LEN) > ABS(dz - LEN))
		sortx++;
	if (ABS(dx - LEN) > ABS(dy - LEN))
		sortx++;
	for (i = 0; i < 3; i++) {
	  if (sortx == i) {
	    if (dx > LEN) {
	      if (sortz > sorty) {
		if (dz < 0)
		  drawQuad(mi, (int) (GREEN & mask), pts, 0, 3, 2, 5);
		else
		  drawQuad(mi, (int) (GREEN & mask), pts, 0, 6, 13, 4);
		drawQuad(mi, (int) (RED & mask), pts, 0, 5, 12, 6);
		drawQuad(mi, (int) (RED & mask), pts, 0, 4, 1, 3);
		if (dz < 0)
		  drawQuad(mi, (int) (GREEN & mask), pts, 0, 6, 13, 4);
		else
		  drawQuad(mi, (int) (GREEN & mask), pts, 0, 3, 2, 5);
	      } else {
		if (dy < 0)
		  drawQuad(mi, (int) (RED & mask), pts, 0, 4, 1, 3);
		else
		  drawQuad(mi, (int) (RED & mask), pts, 0, 5, 12, 6);
		drawQuad(mi, (int) (GREEN & mask), pts, 0, 6, 13, 4);
		drawQuad(mi, (int) (GREEN & mask), pts, 0, 3, 2, 5);
		if (dy < 0)
		  drawQuad(mi, (int) (RED & mask), pts, 0, 5, 12, 6);
		else
		  drawQuad(mi, (int) (RED & mask), pts, 0, 4, 1, 3);
	      }
	    } else if (dx < -LEN) {
	      if (sortz > sorty) {
		if (dz < 0)
		  drawQuad(mi, (int) (GREEN & mask), pts, 2, 7, 11, 9);
		else
		  drawQuad(mi, (int) (GREEN & mask), pts, 8, 13, 10, 11);
		drawQuad(mi, (int) (RED & mask), pts, 9, 11, 10, 12);
		drawQuad(mi, (int) (RED & mask), pts, 1, 8, 11, 7);
		if (dz < 0)
		  drawQuad(mi, (int) (GREEN & mask), pts, 8, 13, 10, 11);
		else
		  drawQuad(mi, (int) (GREEN & mask), pts, 2, 7, 11, 9);
	      } else {
		if (dy < 0)
		  drawQuad(mi, (int) (RED & mask), pts, 1, 8, 11, 7);
		else
	  	  drawQuad(mi, (int) (RED & mask), pts, 9, 11, 10, 12);
		drawQuad(mi, (int) (GREEN & mask), pts, 8, 13, 10, 11);
		drawQuad(mi, (int) (GREEN & mask), pts, 2, 7, 11, 9);
		if (dy < 0)
	  	  drawQuad(mi, (int) (RED & mask), pts, 9, 11, 10, 12);
		else
		  drawQuad(mi, (int) (RED & mask), pts, 1, 8, 11, 7);
	      }
	    }
	  }
	  if (sorty == i) {
	    if (dy > LEN) {
	      if (sortx > sortz) {
		if (dx < 0)
		  drawQuad(mi, (int) (RED & mask), pts, 0, 4, 1, 3);
		else
		  drawQuad(mi, (int) (RED & mask), pts, 1, 8, 11, 7);
		drawQuad(mi, (int) (BLUE & mask), pts, 1, 4, 13, 8);
		drawQuad(mi, (int) (BLUE & mask), pts, 1, 7, 2, 3);
		if (dx < 0)
		  drawQuad(mi, (int) (RED & mask), pts, 1, 8, 11, 7);
		else
		  drawQuad(mi, (int) (RED & mask), pts, 0, 4, 1, 3);
	      } else {
		if (dz < 0)
		  drawQuad(mi, (int) (BLUE & mask), pts, 1, 7, 2, 3);
		else
		  drawQuad(mi, (int) (BLUE & mask), pts, 1, 4, 13, 8);
		drawQuad(mi, (int) (RED & mask), pts, 1, 8, 11, 7);
		drawQuad(mi, (int) (RED & mask), pts, 0, 4, 1, 3);
		if (dz < 0)
		  drawQuad(mi, (int) (BLUE & mask), pts, 1, 4, 13, 8);
		else
		  drawQuad(mi, (int) (BLUE & mask), pts, 1, 7, 2, 3);
	      }
	    } else if (dy < -LEN) {
	      if (sortx > sortz) {
		if (dx < 0)
		  drawQuad(mi, (int) (RED & mask), pts, 0, 5, 12, 6);
		else
		  drawQuad(mi, (int) (RED & mask), pts, 9, 11, 10, 12);
		drawQuad(mi, (int) (BLUE & mask), pts, 6, 12, 10, 13);
		drawQuad(mi, (int) (BLUE & mask), pts, 2, 9, 12, 5);
		if (dx < 0)
		  drawQuad(mi, (int) (RED & mask), pts, 9, 11, 10, 12);
		else
		  drawQuad(mi, (int) (RED & mask), pts, 0, 5, 12, 6);
	      } else {
		if (dz < 0)
		  drawQuad(mi, (int) (BLUE & mask), pts, 2, 9, 12, 5);
		else
		  drawQuad(mi, (int) (BLUE & mask), pts, 6, 12, 10, 13);
		drawQuad(mi, (int) (RED & mask), pts, 9, 11, 10, 12);
		drawQuad(mi, (int) (RED & mask), pts, 0, 5, 12, 6);
		if (dz < 0)
		  drawQuad(mi, (int) (BLUE & mask), pts, 6, 12, 10, 13);
		else
		  drawQuad(mi, (int) (BLUE & mask), pts, 2, 9, 12, 5);
	      }
	    }
	  }
	  if (sortz == i) {
	    if (dz > LEN) {
	      if (sorty > sortx) {
		if (dy < 0)
		  drawQuad(mi, (int) (BLUE & mask), pts, 1, 7, 2, 3);
		else
		  drawQuad(mi, (int) (BLUE & mask), pts, 2, 9, 12, 5);
		drawQuad(mi, (int) (GREEN & mask), pts, 0, 3, 2, 5);
		drawQuad(mi, (int) (GREEN & mask), pts, 2, 7, 11, 9);
		if (dy < 0)
		  drawQuad(mi, (int) (BLUE & mask), pts, 2, 9, 12, 5);
		else
		  drawQuad(mi, (int) (BLUE & mask), pts, 1, 7, 2, 3);
	      } else {
		if (dx < 0)
		  drawQuad(mi, (int) (GREEN & mask), pts, 0, 3, 2, 5);
		else
		  drawQuad(mi, (int) (GREEN & mask), pts, 2, 7, 11, 9);
		drawQuad(mi, (int) (BLUE & mask), pts, 2, 9, 12, 5);
		drawQuad(mi, (int) (BLUE & mask), pts, 1, 7, 2, 3);
		if (dx < 0)
		  drawQuad(mi, (int) (GREEN & mask), pts, 2, 7, 11, 9);
		else
		  drawQuad(mi, (int) (GREEN & mask), pts, 0, 3, 2, 5);
	      }
	    } else if (dz < -LEN) {
	      if (sorty > sortx) {
		if (dy < 0)
		  drawQuad(mi, (int) (BLUE & mask), pts, 1, 4, 13, 8);
		else
		  drawQuad(mi, (int) (BLUE & mask), pts, 6, 12, 10, 13);
		drawQuad(mi, (int) (GREEN & mask), pts, 1, 8, 13, 10);
		drawQuad(mi, (int) (GREEN & mask), pts, 0, 6, 13, 4);
		if (dy < 0)
		  drawQuad(mi, (int) (BLUE & mask), pts, 6, 12, 10, 13);
		else
		  drawQuad(mi, (int) (BLUE & mask), pts, 1, 4, 13, 8);
	      } else {
		if (dx < 0)
		  drawQuad(mi, (int) (GREEN & mask), pts, 0, 6, 13, 4);
		else
		  drawQuad(mi, (int) (GREEN & mask), pts, 1, 8, 13, 10);
		drawQuad(mi, (int) (BLUE & mask), pts, 6, 12, 10, 13);
		drawQuad(mi, (int) (BLUE & mask), pts, 1, 4, 13, 8);
		if (dx < 0)
		  drawQuad(mi, (int) (GREEN & mask), pts, 1, 8, 13, 10);
		else
		  drawQuad(mi, (int) (GREEN & mask), pts, 0, 6, 13, 4);
	      }
	    }
	  }
	}
	return (1);
}

static int
DrawTruncatedOctahedron(ModeInfo * mi, CellList * cell)
{
	life3dstruct *lp = &life3ds[MI_SCREEN(mi)];
	XPoint pts[24];	/* screen coords for point */
	int i, out = 0;
	unsigned int mask;
	double x, y, z;
	double dx, dy, dz;

	x = (double) cell->x - lp->ox;
	y = (double) cell->y - lp->oy;
	z = ((double) cell->z - lp->oz) / 2.0;
	if ((cell->z & 1) == 1) {
		x += 0.5;
		y += 0.5;
	}
	NewPoint(lp, x + LEN_2, y, z + LEN, &pts[0]);
	NewPoint(lp, x, y + LEN_2, z + LEN, &pts[1]);
	NewPoint(lp, x - LEN_2, y, z + LEN, &pts[2]);
	NewPoint(lp, x, y - LEN_2, z + LEN, &pts[3]);

	NewPoint(lp, x + LEN, y, z + LEN_2, &pts[4]);
	NewPoint(lp, x + LEN, y - LEN_2, z, &pts[15]);
	NewPoint(lp, x + LEN, y, z - LEN_2, &pts[16]);
	NewPoint(lp, x + LEN, y + LEN_2, z, &pts[8]);

	NewPoint(lp, x, y + LEN, z + LEN_2, &pts[5]);
	NewPoint(lp, x + LEN_2, y + LEN, z, &pts[9]);
	NewPoint(lp, x, y + LEN, z - LEN_2, &pts[17]);
	NewPoint(lp, x - LEN_2, y + LEN, z, &pts[10]);

	NewPoint(lp, x - LEN, y, z + LEN_2, &pts[6]);
	NewPoint(lp, x - LEN, y + LEN_2, z, &pts[11]);
	NewPoint(lp, x - LEN, y, z - LEN_2, &pts[18]);
	NewPoint(lp, x - LEN, y - LEN_2, z, &pts[12]);

	NewPoint(lp, x, y - LEN, z + LEN_2, &pts[7]);
	NewPoint(lp, x - LEN_2, y - LEN, z, &pts[13]);
	NewPoint(lp, x, y - LEN, z - LEN_2, &pts[19]);
	NewPoint(lp, x + LEN_2, y - LEN, z, &pts[14]);

	NewPoint(lp, x + LEN_2, y, z - LEN, &pts[20]);
	NewPoint(lp, x, y + LEN_2, z - LEN, &pts[21]);
	NewPoint(lp, x - LEN_2, y, z - LEN, &pts[22]);
	NewPoint(lp, x, y - LEN_2, z - LEN, &pts[23]);

	for (i = 0; i < 24; i++) {
		if (pts[i].x < 0 ||
		    pts[i].x >= lp->width ||
		    pts[i].y < 0 ||
		    pts[i].y >= lp->height)
			++out;
	}
	if (out == 24)
		return (0);

	if (cell->visible)
		mask = 0xFFFF;
	else
		mask = 0x0;

	/* Only draw those faces that are visible */
	dx = lp->vx - x;
	dy = lp->vy - y;
	dz = lp->vz - z;
	/* back points looking through front
	   10  5  9
	 11    1    8
	  6  2   0  4
	 12    3   15
	   13  7 14
	  ----------
	   front points
	   10 17  9
	 11   21    8
	 18 22  20 16
	 12   23   15
	   13 19 14
	*/
	if (lp->wireframe) {
		if (dz <= LEN)
			drawQuad(mi, (int) (BLUE & mask), pts, 0, 1, 2, 3);
		else if (dz >= -LEN)
			drawQuad(mi, (int) (BLUE & mask), pts, 23, 22, 21, 20);
		if (dy <= LEN)
			drawQuad(mi, (int) (GREEN & mask), pts, 5, 9, 17, 10);
		else if (dy >= -LEN)
			drawQuad(mi, (int) (GREEN & mask), pts, 7, 14, 19, 13);
		if (dx <= LEN)
			drawQuad(mi, (int) (RED & mask), pts, 4, 8, 16, 15);
		else if (dx >= -LEN)
			drawQuad(mi, (int) (RED & mask), pts, 6, 11, 18, 12);
		if (dx + dy + dz < 0) {
			drawHex(mi, (int) (UV & mask), pts, 1, 0, 4, 8, 9, 5);
		} else {
			drawHex(mi, (int) (UV & mask), pts, 22, 23, 19, 13, 12, 18);
		}
		if (dx + dy - dz < 0) {
			drawHex(mi, (int) (GREEN & mask), pts, 20, 21, 17, 9, 8, 16);
		} else {
			drawHex(mi, (int) (GREEN & mask), pts, 3, 2, 6, 12, 13, 7);
		}
		if (dx - dy + dz < 0) {
			drawHex(mi, (int) (RED & mask), pts, 0, 3, 7, 14, 15, 4);
		} else {
			drawHex(mi, (int) (RED & mask), pts, 21, 22, 18, 11, 10, 17);
		}
		if (-dx + dy + dz < 0) {
			drawHex(mi, (int) (BLUE & mask), pts, 2, 1, 5, 10, 11, 6);
		} else {
			drawHex(mi, (int) (BLUE & mask), pts, 23, 20, 16, 15, 14, 19);
		}
	}
	if (dx + dy + dz >= 0) {
		drawHex(mi, (int) (UV & mask), pts, 1, 0, 4, 8, 9, 5);
	} else {
		drawHex(mi, (int) (UV & mask), pts, 22, 23, 19, 13, 12, 18);
	}
	if (dx + dy - dz >= 0) {
		drawHex(mi, (int) (GREEN & mask), pts, 20, 21, 17, 9, 8, 16);
	} else {
		drawHex(mi, (int) (GREEN & mask), pts, 3, 2, 6, 12, 13, 7);
	}
	if (dx - dy + dz >= 0) {
		drawHex(mi, (int) (RED & mask), pts, 0, 3, 7, 14, 15, 4);
	} else {
		drawHex(mi, (int) (RED & mask), pts, 21, 22, 18, 11, 10, 17);
	}
	if (-dx + dy + dz >= 0) {
		drawHex(mi, (int) (BLUE & mask), pts, 2, 1, 5, 10, 11, 6);
	} else {
		drawHex(mi, (int) (BLUE & mask), pts, 23, 20, 16, 15, 14, 19);
	}
	if (dz > LEN)
		drawQuad(mi, (int) (BLUE & mask), pts, 0, 1, 2, 3);
	else if (dz < -LEN)
		drawQuad(mi, (int) (BLUE & mask), pts, 23, 22, 21, 20);
	if (dy > LEN)
		drawQuad(mi, (int) (GREEN & mask), pts, 5, 9, 17, 10);
	else if (dy < -LEN)
		drawQuad(mi, (int) (GREEN & mask), pts, 7, 14, 19, 13);
	if (dx > LEN)
		drawQuad(mi, (int) (RED & mask), pts, 4, 8, 16, 15);
	else if (dx < -LEN)
		drawQuad(mi, (int) (RED & mask), pts, 6, 11, 18, 12);
	return (1);
}

static void
DrawScreen(ModeInfo * mi)
{
	life3dstruct *lp = &life3ds[MI_SCREEN(mi)];
	Display *display = MI_DISPLAY(mi);
	GC gc = MI_GC(mi);
	CellList *ptr;
	CellList *eraserptr;
	int i;

	SortList(lp);

	if (draw) {
		XSetForeground(display, gc, MI_BLACK_PIXEL(mi));
		XFillRectangle(display, (Drawable) lp->dbuf, gc, 0, 0,
			lp->width, lp->height);
	}
	/* Erase dead cubes */
	eraserptr = lp->eraserhead.next;
	while (eraserptr != &lp->eraserend) {
		eraserptr->visible = 0;
		if (draw) {
			if (lp->neighbors == 12 || lp->neighbors == -18)
				(void) DrawRhombicDodecahedron(mi, eraserptr);
			else if (lp->neighbors == 14)
				(void) DrawTruncatedOctahedron(mi, eraserptr);
			else if (lp->neighbors == 22)
				(void) DrawTetrahedron(mi, eraserptr);
			else
				(void) DrawCube(mi, eraserptr);
		}
		delFromEraseList(eraserptr);
		eraserptr = lp->eraserhead.next;
	}
	if (!draw)
		return;

	/* draw furthest cubes first */
	for (i = NBUCKETS - 1; i >= 0; --i) {
		ptr = lp->buckethead[i];
		while (ptr != NULL) {
			/*if (ptr->visible) */
			if (lp->neighbors == 12 || lp->neighbors == -18)
				(void) DrawRhombicDodecahedron(mi, ptr);
			else if (lp->neighbors == 14)
				(void) DrawTruncatedOctahedron(mi, ptr);
			else if (lp->neighbors == 22)
				(void) DrawTetrahedron(mi, ptr);
			else
			/* v += */ (void) DrawCube(mi, ptr);
			ptr = ptr->priority;
			/* ++count; */
		}
	}
	if (label) {
		int size = MAX(MIN(MI_WIDTH(mi), MI_HEIGHT(mi)) - 1, 1);

		if (size >= 10 * FONT_WIDTH) {
			char ruleString[120];

			/* hard code these to corners */
			XSetForeground(display, gc, MI_WHITE_PIXEL(mi));
			(void) sprintf(ruleString, "N%d:%s",
				lp->neighbors, lp->ruleString);
			XDrawString(display, (Drawable) lp->dbuf, gc,
				16 + lp->labelOffsetX,
				16 + lp->labelOffsetY + FONT_HEIGHT,
				ruleString, strlen(ruleString));
			XDrawString(display, (Drawable) lp->dbuf, gc,
				16 + lp->labelOffsetX, MI_HEIGHT(mi) - 16 -
				lp->labelOffsetY - FONT_HEIGHT / 2,
				lp->nameString, strlen(lp->nameString));
		}
	}
	XFlush(display);
	XCopyArea(display, (Drawable) lp->dbuf, MI_WINDOW(mi), gc,
		0, 0, lp->width, lp->height, 0, 0);

#if 0
	{
		int count = 0, v = 0;


		(void) printf("Pop=%-4d  Viewpoint (%3d,%3d,%3d)  Origin (%3d,%3d,%3d)  Mode %dx%d\
(%d,%d) %d\n",
		count, (int) (lp->vx + lp->ox), (int) (lp->vy + lp->oy),
			(int) (lp->vz + lp->oz), (int) lp->ox, (int) lp->oy, (int) lp->oz,
			lp->width, lp->height, lp->alt, lp->azm, v);
	}
#endif
}

static Bool
orthogonal(life3dstruct * lp) /* or full diagonal */
{
	if (lp->neighbors == 12)
		return (lp->patterned_rule == LIFE_12B3S456);
	else if (lp->neighbors == 18)
		return ((lp->patterned_rule == LIFE_18B4S45) &&
			(LRAND() & 1));
	else if (lp->neighbors == -18)
		return ((lp->patterned_rule == LIFE__18B4S36) ||
			(lp->patterned_rule == LIFE__18B4S46));
	else if (lp->neighbors == 20)
		return (lp->patterned_rule == LIFE_20B4S45);
	else if (lp->neighbors == 22)
		return (((lp->patterned_rule == LIFE_22B4S3) &&
			(NRAND(3) == 0)) ||
			(lp->patterned_rule == LIFE_22B4S4));
	else if (lp->neighbors == 26)
		return ((lp->patterned_rule == LIFE_26B5S23) ||
			(lp->patterned_rule == LIFE_26B5S25) ||
			(lp->patterned_rule == LIFE_26B5S27) ||
			(lp->patterned_rule == LIFE_26B5S35) ||
			(lp->patterned_rule == LIFE_26B5S36) ||
			(lp->patterned_rule == LIFE_26B5S37) ||
			(lp->patterned_rule == LIFE_26B5S4) ||
			(lp->patterned_rule == LIFE_26B5S47) ||
			((lp->patterned_rule == LIFE_26B5S56) &&
			(LRAND() & 1)) ||
			((lp->patterned_rule == LIFE_26B6S57) &&
			(NRAND(4) == 0)) ||
			((lp->patterned_rule == LIFE_26B5S58) &&
			(NRAND(4) == 0)) ||
			((lp->patterned_rule == LIFE_26B5S678) &&
			(LRAND() & 1)) ||
			((lp->patterned_rule == LIFE_26B58S58) &&
			(NRAND(4) != 0)));
	return False;
}

static Bool
populateGlider(life3dstruct * lp, char *glider, int offsetSwitch,
		int hsp, int vsp, int asp, int hoff, int voff, int aoff)
{
	int x, y, z, temp;

	while ((x = *glider++) != 127) {
		int offx = 0, offy = 0, offz;
		y = *glider++;
		z = *glider++;
		if (offsetSwitch == 1) {
			temp = x;
			x = y;
			if (lp->neighbors == 14) {
				y = (z >= 0) ? z / 2 : (z - 1) / 2;
				z = (z & 1) + 2 * temp;
			} else {
				y = z;
				z = temp;
			}
		} else if (offsetSwitch == 2) {
			temp = z;
			if (lp->neighbors == 14) {
				z = (z & 1) + 2 * y;
				y = x;
				x = (temp >= 0) ? temp / 2 : (temp - 1) / 2;
			} else {
				z = y;
				y = x;
				x = temp;
			}
		}
		offz = asp + z * aoff;
		if (lp->neighbors == 14 && (offz & 1) == 1) {
			if (hoff == -1)
				offx = -1;
			if (voff == -1)
				offy = -1;
		}
		SetList(hsp + x * hoff + offx, vsp + y * voff + offy, offz);
	}
	return True;
}

/* My rule of thumb is if there is an 18 cell (or less) glider add it here */
static void
populateGliders(life3dstruct * lp, int dim, int orth,
		int hsp, int vsp, int asp, int hoff, int voff, int aoff)
{
	if (lp->neighbors == 12 && lp->patterned_rule == LIFE_12B3S3) {
			(void) populateGlider(lp, glider_12B3S3[2], dim,
				hsp, vsp, asp, hoff, voff, aoff);
		if (NRAND(3)) {
			(void) populateGlider(lp, glider_12B3S3[0], dim,
				hsp, vsp, asp, hoff, voff, aoff);
		} else if (NRAND(3)) {
			(void) populateGlider(lp, glider_12B3S3[1], dim,
				hsp, vsp, asp, hoff, voff, aoff);
		} else if (NRAND(2)) { /* lower probability of big gliders */
			(void) populateGlider(lp, glider_12B3S3[2], dim,
				hsp, vsp, asp, hoff, voff, aoff);
		} else {
			(void) populateGlider(lp, glider_12B3S3[3], dim,
				hsp, vsp, asp, hoff, voff, aoff);
		}
	} else if (lp->neighbors == 12 && lp->patterned_rule == LIFE_12B3S456) {
		(void) populateGlider(lp, glider_12B3S456[0], dim,
			hsp, vsp, asp, hoff, voff, aoff);
	} else if (lp->neighbors == 14 && lp->patterned_rule == LIFE_14B4S34) {
		(void) populateGlider(lp, glider_14B4S34[0], dim,
			hsp, vsp, asp, hoff, voff, aoff);
	} else if (lp->neighbors == 14 && lp->patterned_rule == LIFE_14B46S34) {
		(void) populateGlider(lp, glider_14B46S34[0], dim,
			hsp, vsp, asp, hoff, voff, aoff);
	} else if (lp->neighbors == 18 && lp->patterned_rule == LIFE_18B4S45) {
		if (orth) {
			if (LRAND() & 1) {
				(void) populateGlider(lp, glider_18B4S45[0], dim,
					hsp, vsp, asp, hoff, voff, aoff);
			} else {
				(void) populateGlider(lp, glider_18B4S45[1], dim,
					hsp, vsp, asp, hoff, voff, aoff);
			}
		} else {
			(void) populateGlider(lp, glider_18B4S45[2], dim,
				hsp, vsp, asp, hoff, voff, aoff);
		}
	} else if (lp->neighbors == -18 && lp->patterned_rule == LIFE__18B4S36) {
		(void) populateGlider(lp, glider__18B4S36[0], dim,
			hsp, vsp, asp, hoff, voff, aoff);
	} else if (lp->neighbors == -18 && lp->patterned_rule == LIFE__18B4S46) {
		(void) populateGlider(lp, glider__18B4S46[0], dim,
			hsp, vsp, asp, hoff, voff, aoff);
	} else if (lp->neighbors == 20 && lp->patterned_rule == LIFE_20B4S45) {
		if (LRAND() & 1) {
			(void) populateGlider(lp, glider_20B4S45[0], dim,
				hsp, vsp, asp, hoff, voff, aoff);
		} else {
			(void) populateGlider(lp, glider_20B4S45[1], dim,
				hsp, vsp, asp, hoff, voff, aoff);
		}
	} else if (lp->neighbors == 22 && lp->patterned_rule == LIFE_22B4S3) {
		if (orth) {
			(void) populateGlider(lp, glider_22B4S3[0], dim,
				hsp, vsp, asp, hoff, voff, aoff);
		} else {
			if (NRAND(3)) {
				(void) populateGlider(lp, glider_22B4S3[2], dim,
					hsp, vsp, asp, hoff, voff, aoff);
			} else {
				(void) populateGlider(lp, glider_22B4S3[1], dim,
					hsp, vsp, asp, hoff, voff, aoff);
			}
		}
	} else if (lp->neighbors == 22 && lp->patterned_rule == LIFE_22B4S4) {
		if (NRAND(3)) {
			(void) populateGlider(lp, glider_22B4S4[1], dim,
				hsp, vsp, asp, hoff, voff, aoff);
		} else {
			(void) populateGlider(lp, glider_22B4S4[0], dim,
				hsp, vsp, asp, hoff, voff, aoff);
		}
	} else if (lp->neighbors == 26 && lp->patterned_rule == LIFE_26B5S23) {
		(void) populateGlider(lp, glider_26B5S23[0], dim,
			hsp, vsp, asp, hoff, voff, aoff);
	} else if (lp->neighbors == 26 && lp->patterned_rule == LIFE_26B5S25) {
		(void) populateGlider(lp, glider_26B5S25[0], dim,
			hsp, vsp, asp, hoff, voff, aoff);
	} else if (lp->neighbors == 26 && lp->patterned_rule == LIFE_26B5S27) {
		(void) populateGlider(lp, glider_26B5S27[0], dim,
			hsp, vsp, asp, hoff, voff, aoff);
	} else if (lp->neighbors == 26 && lp->patterned_rule == LIFE_26B5S35) {
		(void) populateGlider(lp, glider_26B5S35[0], dim,
			hsp, vsp, asp, hoff, voff, aoff);
	} else if (lp->neighbors == 26 && lp->patterned_rule == LIFE_26B5S36) {
		(void) populateGlider(lp, glider_26B5S36[0], dim,
			hsp, vsp, asp, hoff, voff, aoff);
	} else if (lp->neighbors == 26 && lp->patterned_rule == LIFE_26B5S37) {
		(void) populateGlider(lp, glider_26B5S37[0], dim,
			hsp, vsp, asp, hoff, voff, aoff);
	} else if (lp->neighbors == 26 && lp->patterned_rule == LIFE_26B5S38) {
		if (LRAND() & 1) {
			(void) populateGlider(lp, glider_26B5S38[0], dim,
				hsp, vsp, asp, hoff, voff, aoff);
		} else {
			(void) populateGlider(lp, glider_26B5S38[1], dim,
				hsp, vsp, asp, hoff, voff, aoff);
		}
	} else if (lp->neighbors == 26 && lp->patterned_rule == LIFE_26B5S4) {
		(void) populateGlider(lp, glider_26B5S4[0], dim,
			hsp, vsp, asp, hoff, voff, aoff);
	} else if (lp->neighbors == 26 && lp->patterned_rule == LIFE_26B5S45) {
		(void) populateGlider(lp, glider_26B5S45[0], dim,
			hsp, vsp, asp, hoff, voff, aoff);
	} else if (lp->neighbors == 26 && lp->patterned_rule == LIFE_26B5S47) {
		(void) populateGlider(lp, glider_26B5S47[0], dim,
			hsp, vsp, asp, hoff, voff, aoff);
	} else if (lp->neighbors == 26 && lp->patterned_rule == LIFE_26B5S56) {
		if (orth) {
			if (LRAND() & 1) {
				(void) populateGlider(lp, glider_26B5S56[0], dim,
					hsp, vsp, asp, hoff, voff, aoff);
			} else {
				(void) populateGlider(lp, glider_26B5S56[1], dim,
					hsp, vsp, asp, hoff, voff, aoff);
			}
		} else {
			(void) populateGlider(lp, glider_26B5S56[2], dim,
				hsp, vsp, asp, hoff, voff, aoff);
		}
	} else if (lp->neighbors == 26 && lp->patterned_rule == LIFE_26B5S58) {
		if (orth)
			(void) populateGlider(lp, glider_26B5S58[4], dim,
				hsp, vsp, asp, hoff, voff, aoff);
		else if (NRAND(3)) {
			if (LRAND() & 1)
				(void) populateGlider(lp, glider_26B5S58[0], dim,
					hsp, vsp, asp, hoff, voff, aoff);
			else if (LRAND() & 1)
				(void) populateGlider(lp, glider_26B5S58[1], dim,
					hsp, vsp, asp, hoff, voff, aoff);
			else
				(void) populateGlider(lp, glider_26B5S58[2], dim,
					hsp, vsp, asp, hoff, voff, aoff);
		} else
			(void) populateGlider(lp, glider_26B5S58[3], dim,
				hsp, vsp, asp, hoff, voff, aoff);
	} else if (lp->neighbors == 26 && lp->patterned_rule == LIFE_26B5S678) {
		if (orth) {
			(void) populateGlider(lp, glider_26B5S678[1], dim,
				hsp, vsp, asp, hoff, voff, aoff);
		} else {
			(void) populateGlider(lp, glider_26B5S678[0], dim,
				hsp, vsp, asp, hoff, voff, aoff);
		}
	} else if (lp->neighbors == 26 && lp->patterned_rule == LIFE_26B5S8) {
		if (LRAND() & 1) {
			(void) populateGlider(lp, glider_26B5S8[0], dim,
				hsp, vsp, asp, hoff, voff, aoff);
		} else {
			if (LRAND() & 1) {
				(void) populateGlider(lp, glider_26B5S8[1], dim,
					hsp, vsp, asp, hoff, voff, aoff);
			} else {
				(void) populateGlider(lp, glider_26B5S8[2], dim,
					hsp, vsp, asp, hoff, voff, aoff);
			}
		}
	} else if (lp->neighbors == 26 && lp->patterned_rule == LIFE_26B58S58) {
		if (orth) {
			if (LRAND() & 1) {
				if (LRAND() & 1)
					(void) populateGlider(lp, glider_26B58S58[1], dim,
						hsp, vsp, asp, hoff, voff, aoff);
				else
					(void) populateGlider(lp, glider_26B58S58[2], dim,
						hsp, vsp, asp, hoff, voff, aoff);
			} else {
				if (LRAND() & 1)
					(void) populateGlider(lp, glider_26B58S58[3], dim,
						hsp, vsp, asp, hoff, voff, aoff);
				else if (NRAND(3))
					(void) populateGlider(lp, glider_26B58S58[4], dim,
						hsp, vsp, asp, hoff, voff, aoff);
				else if (LRAND() & 1)
					(void) populateGlider(lp, glider_26B58S58[5], dim,
						hsp, vsp, asp, hoff, voff, aoff);
				else
					(void) populateGlider(lp, glider_26B58S58[6], dim,
						hsp, vsp, asp, hoff, voff, aoff);
			}
		} else {
			(void) populateGlider(lp, glider_26B58S58[0], dim,
				hsp, vsp, asp, hoff, voff, aoff);
		}
	} else if (lp->neighbors == 26 && lp->patterned_rule == LIFE_26B6S567) {
		(void) populateGlider(lp, glider_26B6S567[0], dim,
			hsp, vsp, asp, hoff, voff, aoff);
	} else if (lp->neighbors == 26 && lp->patterned_rule == LIFE_26B6S57) {
		if (orth) {
			if (LRAND() & 1) {
				(void) populateGlider(lp, glider_26B6S57[1], dim,
					hsp, vsp, asp, hoff, voff, aoff);
			} else {
				(void) populateGlider(lp, glider_26B6S57[2], dim,
					hsp, vsp, asp, hoff, voff, aoff);
			}
		} else {
			(void) populateGlider(lp, glider_26B6S57[0], dim,
				hsp, vsp, asp, hoff, voff, aoff);
		}
	} else if (lp->neighbors == 26 && lp->patterned_rule == LIFE_26B67S57) {
		(void) populateGlider(lp, glider_26B67S57[0], dim,
			hsp, vsp, asp, hoff, voff, aoff);
	}
}

static Bool
shooter(life3dstruct * lp)
{
	int hsp, vsp, asp, hoff = 1, voff = 1, aoff = 1, dim, c2, r2, s2, orth;

	/* Generate the glider at the edge of the screen */
	int V = (lp->neighbors == 22) ? 20 : 10;
	int V2 = V / 2;
	c2 = lp->ncols / 2;
	r2 = lp->nrows / 2;
	s2 = lp->nstacks / 2;
	dim = NRAND(3);
	orth = orthogonal(lp);
	if (!dim) {
		if (orth) {
			hsp = (LRAND() & 1) ? c2 - V : c2 + V;
			vsp = NRAND(V2) + r2 - V2 / 2;
			asp = NRAND(V2) + s2 - V2 / 2;
		} else {
			hsp = NRAND(V2) + c2 - V2 / 2;
			vsp = (LRAND() & 1) ? r2 - V : r2 + V;
			asp = (LRAND() & 1) ? s2 - V : s2 + V;
		}
		if (asp > s2)
			aoff = -1;
		if (vsp > r2)
			voff = -1;
		if (hsp > c2)
			hoff = -1;
		if (lp->neighbors == 12 || lp->neighbors == -18) {
			/* Must ensure we have only odd cells (x+y+z=odd)
			   for glider, so this must be even */
		    if ((asp + vsp + hsp) & 1)
			hsp--;
		    populateGliders(lp, dim, orth,
			hsp, vsp, asp, hoff, voff, aoff);
		} else if (lp->neighbors == 14) {
		    /* Must ensure we have only z odd cells (z=odd)
		       for glider, so this must be even */
		    if (asp & 1)
			asp--;
		    populateGliders(lp, dim, orth,
			hsp, vsp, asp, hoff, voff, aoff);
		} else if (lp->neighbors == 18 || lp->neighbors == 20 || lp->neighbors == 26) {
		    populateGliders(lp, dim, orth,
			hsp, vsp, asp, hoff, voff, aoff);
		} else if (lp->neighbors == 22) {
		    if (hsp & 1)
			hsp--;
		    if (vsp & 1)
			vsp--;
		    if (asp & 1)
			asp--;
		    if ((hsp + vsp + asp) % 4)
			hsp++, vsp++;
		    populateGliders(lp, dim, orth,
			hsp, vsp, asp, hoff, voff, aoff);
		}
	} else if (dim == 1) {
		if (orth) {
			hsp = NRAND(V2) + c2 - V2 / 2;
			vsp = NRAND(V2) + r2 - V2 / 2;
			asp = (LRAND() & 1) ? s2 - V : s2 + V;
		} else {
			hsp = (LRAND() & 1) ? c2 - V : c2 + V;
			vsp = (LRAND() & 1) ? r2 - V : r2 + V;
			asp = NRAND(V2) + s2 - V2 / 2;
		}
		if (asp > s2)
			aoff = -1;
		if (vsp > r2)
			voff = -1;
		if (hsp > c2)
			hoff = -1;
		if (lp->neighbors == 12 || lp->neighbors == -18) {
			/* Must ensure we have only odd cells (x+y+z=odd)
			   for glider, so this must be even */
		    if ((asp + vsp + hsp) & 1)
			asp--;
		    populateGliders(lp, dim, orth,
			hsp, vsp, asp, hoff, voff, aoff);
		} else if (lp->neighbors == 14) {
		    /* Must ensure we have only z odd cells (z=odd)
		       for glider, so this must be even */
		    if (asp & 1)
			asp--;
		    populateGliders(lp, dim, orth,
			hsp, vsp, asp, hoff, voff, aoff);
		} else if (lp->neighbors == 18 || lp->neighbors == 20 || lp->neighbors == 26) {
		    populateGliders(lp, dim, orth,
			hsp, vsp, asp, hoff, voff, aoff);
		} else if (lp->neighbors == 22) {
		    if (hsp & 1)
			hsp--;
		    if (vsp & 1)
			vsp--;
		    if (asp & 1)
			asp--;
		    if ((hsp + vsp + asp) % 4)
			hsp++, vsp++;
		    populateGliders(lp, dim, orth,
			hsp, vsp, asp, hoff, voff, aoff);
		}
	} else {
		if (orth) {
			hsp = NRAND(V2) + c2 - V2 / 2;
			vsp = (LRAND() & 1) ? r2 - V : r2 + V;
			asp = NRAND(V2) + s2 - V2 / 2;
		} else {
			hsp = (LRAND() & 1) ? c2 - V : c2 + V;
			vsp = NRAND(V2) + r2 - V2 / 2;
			asp = (LRAND() & 1) ? s2 - V : s2 + V;
		}
		if (asp > s2)
			aoff = -1;
		if (vsp > r2)
			voff = -1;
		if (hsp > c2)
			hoff = -1;
		if (lp->neighbors == 12 || lp->neighbors == -18) {
			/* Must ensure we have only odd cells (x+y+z=odd)
			   for glider, so this must be even */
		    if ((asp + vsp + hsp) & 1)
			vsp--;
		    populateGliders(lp, dim, orth,
			hsp, vsp, asp, hoff, voff, aoff);
		} else if (lp->neighbors == 14) {
		    /* Must ensure we have only z odd cells (z=odd)
		       for glider, so this must be even */
		    if (asp & 1)
			asp--;
		    populateGliders(lp, dim, orth,
			hsp, vsp, asp, hoff, voff, aoff);
		} else if (lp->neighbors == 18 || lp->neighbors == 20 || lp->neighbors == 26) {
		    populateGliders(lp, dim, orth,
			hsp, vsp, asp, hoff, voff, aoff);
		} else if (lp->neighbors == 22) {
		    if (hsp & 1)
			hsp--;
		    if (vsp & 1)
			vsp--;
		    if (asp & 1)
			asp--;
		    if ((hsp + vsp + asp) % 4)
			hsp++, vsp++;
		    populateGliders(lp, dim, orth,
			hsp, vsp, asp, hoff, voff, aoff);
		}
	}
	return True;
}

static void
free_life3d_screen(Display *display, life3dstruct *lp)
{
	int shade;

	if (lp == NULL) {
		return;
	}
	if (lp->eraserhead.next != NULL)
		endList(lp);
	if (lp->dbuf != None) {
		XFreePixmap(display, lp->dbuf);
		lp->dbuf = None;
	}
	if (lp->stippledGC != None) {
		XFreeGC(display, lp->stippledGC);
		lp->stippledGC = None;
	}
	for (shade = 0; shade < lp->init_bits; shade++) {
		XFreePixmap(display, lp->pixmaps[shade]);
	}
	lp->init_bits = 0;
	lp = NULL;
}

ENTRYPOINT void
free_life3d(ModeInfo * mi)
{
	free_life3d_screen(MI_DISPLAY(mi), &life3ds[MI_SCREEN(mi)]);
}

ENTRYPOINT void
init_life3d(ModeInfo * mi)
{
	Display *display = MI_DISPLAY(mi);
	Window window = MI_WINDOW(mi);
	life3dstruct *lp;
	int i, npats;

	MI_INIT(mi, life3ds);
	lp = &life3ds[MI_SCREEN(mi)];

	if (MI_NPIXELS(mi) <= 2) {
		if (lp->stippledGC == None) {
			XGCValues gcv;
			gcv.fill_style = FillOpaqueStippled;
			if ((lp->stippledGC = XCreateGC(display, window,
					GCFillStyle, &gcv)) == None) {
				free_life3d_screen(display, lp);
				return;
			}
		}
		if (lp->init_bits == 0) {
			for (i = 1; i < NUMSTIPPLES; i++) {
				LIFE3DBITS(stipples[i], STIPPLESIZE, STIPPLESIZE);
			}
		}
	}
	lp->generation = 0;
	lp->foundBirth = lp->foundSurvival = 0;
	if (MI_IS_FULLRANDOM(mi)) {
		int r12n1 = patterns_12rules[0];
		int r12n2 = r12n1 + patterns_12rules[1];
		int r14n1 = patterns_14rules[0];
		int r14n2 = r14n1 + patterns_14rules[1];
		int r18n1 = patterns_18rules[0];
		int r_18n1 = patterns__18rules[0];
		int r_18n2 = r_18n1 + patterns__18rules[1];
		int r20n1 = patterns_20rules[0];
		int r22n1 = patterns_22rules[0];
		int r22n2 = r22n1 + patterns_22rules[1];
		int r26n1 = patterns_26rules[0];
		int r26n2 = r26n1 + patterns_26rules[1];
		int r26n3 = r26n2 + patterns_26rules[2];
		int r26n4 = r26n3 + patterns_26rules[3];
#if 1
		lp->neighbors =
			(NRAND(r12n2 + r14n2 + r18n1 + r_18n2 + r20n1 + r22n2 + r26n4) < r12n2) ? 12 :
			(NRAND(r14n2 + r18n1 + r_18n2 + r20n1 + r22n2 + r26n4) < r14n2) ? 14 :
			(NRAND(r18n1 + r_18n2 + r20n1 + r22n2 + r26n4) < r18n1) ? 18 :
			(NRAND(r_18n2 + r20n1 + r22n2 + r26n4) < r18n1) ? -18 :
			(NRAND(r20n1 + r22n2 + r26n4) < r20n1) ? 20 :
			(NRAND(r22n2 + r26n4) < r22n2) ? 22 : 26;
		lp->allGliders = True;
#else
		lp->neighbors = 26;
#endif
	}
	if (!lp->neighbors) {
		for (i = 0; i < NEIGHBORKINDS; i++) {
			if (neighbors == plots[i]) {
				lp->neighbors = neighbors;
				break;
			}
			if (i == NEIGHBORKINDS - 1) {
#if 0
				lp->neighbors = plots[NRAND(NEIGHBORKINDS)];
				lp->neighbors = (LRAND() & 1) ? 12 : 26;
#else
				lp->neighbors = 26;
#endif
				break;
			}
		}
	}

	lp->labelOffsetX = NRAND(8);
	lp->labelOffsetY = NRAND(8);
	parseRule(mi, lp->ruleString);
#ifndef STANDALONE
	parseFile(mi);
#endif
	if (lp->allPatterns) {
		switch (lp->neighbors) {
		case 12:
			lp->patterned_rule = NRAND(LIFE_12RULES);
			break;
		case 14:
			lp->patterned_rule = NRAND(LIFE_14RULES);
			break;
		case 18:
			lp->patterned_rule = 0; /* only one rule */
			break;
		case -18:
			lp->patterned_rule = NRAND(LIFE__18RULES);
			break;
		case 20:
			lp->patterned_rule = 0; /* only one rule */
			break;
		case 22:
			lp->patterned_rule = NRAND(LIFE_22RULES);
			break;
		case 26:
			lp->patterned_rule = NRAND(LIFE_26RULES);
			break;
		}
		copyFromPatternedRule(lp->neighbors, &lp->param,
			lp->patterned_rule);
		printRule(lp->neighbors, lp->ruleString, lp->param,
			MI_IS_VERBOSE(mi));
	} else if (lp->allGliders) {
		switch (lp->neighbors) {
		case 12:
			lp->patterned_rule = NRAND(LIFE_12GLIDERS);
			break;
		case 14:
			lp->patterned_rule = NRAND(LIFE_14GLIDERS);
			break;
		case 18:
			lp->patterned_rule = 0; /* only one rule */
			break;
		case -18:
			lp->patterned_rule = NRAND(LIFE__18GLIDERS);
			break;
		case 20:
			lp->patterned_rule = 0; /* only one rule */
			break;
		case 22:
			lp->patterned_rule = NRAND(LIFE_22GLIDERS);
			break;
		case 26:
			lp->patterned_rule = NRAND(LIFE_26GLIDERS);
			break;
		}
		copyFromPatternedRule(lp->neighbors, &lp->param,
			lp->patterned_rule);
		printRule(lp->neighbors, lp->ruleString, lp->param,
			MI_IS_VERBOSE(mi));
	} else {
		lp->param.birth = lp->input_param.birth;
		lp->param.survival = lp->input_param.survival;
		lp->patterned_rule = codeToPatternedRule(lp->neighbors, lp->param);
		printRule(lp->neighbors, lp->ruleString, lp->param,
			MI_IS_VERBOSE(mi));
	}
	if (!lp->eraserhead.next) {
		lp->metaDist = (double) NRAND(360);
		lp->metaAlt = (double) NRAND(360);
		lp->metaAzm = (double) NRAND(360);
		lp->ncols = MAXCOLUMNS;
		lp->nrows = MAXROWS;
		lp->nstacks = MAXSTACKS;
		lp->ox = lp->ncols / 2;
		lp->oy = lp->nrows / 2;
		lp->oz = lp->nstacks / 2;

		initList(lp);
	} else {
		endList(lp);
	}
	lp->colors[0] = MI_BLACK_PIXEL(mi);
	if (MI_NPIXELS(mi) > 2) {
#ifdef DISTRIB_COLOR
		i = NRAND(3);

		lp->colors[i + 1] = MI_PIXEL(mi,
			NRAND(MI_NPIXELS(mi) / COLORBASE));
		lp->colors[(i + 1) % 3 + 1] = MI_PIXEL(mi,
			NRAND(MI_NPIXELS(mi) / COLORBASE) +
			MI_NPIXELS(mi) / COLORBASE);
		lp->colors[(i + 2) % 3 + 1] = MI_PIXEL(mi,
			NRAND(MI_NPIXELS(mi) / COLORBASE) +
			2 * MI_NPIXELS(mi) / COLORBASE);
#else
		int j = NRAND(MI_NPIXELS(mi));
		i = NRAND(3);
		lp->colors[i + 1] =  MI_PIXEL(mi, j);
		lp->colors[(i + 1) % 3 + 1] =
			MI_PIXEL(mi, (j + 2) % MI_NPIXELS(mi));
		lp->colors[(i + 2) % 3 + 1] =
			MI_PIXEL(mi, (j + 4) % MI_NPIXELS(mi));
#endif
	} else {
		/*lp->colors[1] = lp->colors[2] = lp->colors[3] =
			MI_WHITE_PIXEL(mi);*/
		i = NRAND(3);

		lp->colors[1] = NRAND(NUMSTIPPLES - 1);
		lp->colors[2] = NRAND(NUMSTIPPLES - 1);
		while (lp->colors[2] == lp->colors[1])
			lp->colors[2] = NRAND(NUMSTIPPLES - 1);
		lp->colors[3] = NRAND(NUMSTIPPLES - 1);
		while (lp->colors[3] == lp->colors[1] ||
			lp->colors[3] == lp->colors[2])
			lp->colors[3] = NRAND(NUMSTIPPLES - 1);
	}
	lp->colors[4] = MI_WHITE_PIXEL(mi);
	lp->width = MI_WIDTH(mi);
	lp->height = MI_HEIGHT(mi);
	lp->memstart = 1;
	lp->noChangeCount = False;
	/*lp->tablesMade = 0; */

	if (MI_IS_FULLRANDOM(mi)) {
		lp->wireframe = (NRAND(16) == 0);
	} else {
		lp->wireframe = MI_IS_WIREFRAME(mi);
	}

	MI_CLEARWINDOW(mi);
	lp->painted = False;

	lissajous(lp);

	lp->patterned_rule = codeToPatternedRule(lp->neighbors, lp->param);
	npats = 0;
	switch (lp->neighbors) {
	case 12:
		if ((unsigned) lp->patterned_rule < LIFE_12RULES)
			npats = patterns_12rules[lp->patterned_rule];
		break;
	case 14:
		if ((unsigned) lp->patterned_rule < LIFE_14RULES)
			npats = patterns_14rules[lp->patterned_rule];
		break;
	case 18:
		if ((unsigned) lp->patterned_rule < LIFE_18RULES)
			npats = patterns_18rules[lp->patterned_rule];
		break;
	case -18:
		if ((unsigned) lp->patterned_rule < LIFE__18RULES)
			npats = patterns__18rules[lp->patterned_rule];
		break;
	case 20:
		if ((unsigned) lp->patterned_rule < LIFE_20RULES)
			npats = patterns_20rules[lp->patterned_rule];
		break;
	case 22:
		if ((unsigned) lp->patterned_rule < LIFE_22RULES)
			npats = patterns_22rules[lp->patterned_rule];
		break;
	case 26:
		if ((unsigned) lp->patterned_rule < LIFE_26RULES)
			npats = patterns_26rules[lp->patterned_rule];
		break;
	}
	if (glidersearch || patternsearch) {
		/* trying to find new patterns, formerly 10x10x10 */
		RandomSoup(mi, SOUPPERCENT, SOUPSIZE(lp->ncols),
			SOUPSIZE(lp->nrows), SOUPSIZE(lp->nstacks));
	} else {
		if (!filePattern)
			lp->pattern = NRAND(npats + 2);
		if (lp->pattern >= npats && !filePattern) {
			if (!RandomSoup(mi, SOUPPERCENT, SOUPSIZE(lp->ncols),
			    SOUPSIZE(lp->nrows), SOUPSIZE(lp->nstacks))) {
				if (lp->eraserhead.next != NULL)
					endList(lp);
				return;
			}
		} else {
			if (!GetPattern(mi, lp->patterned_rule, lp->pattern)) {
				if (lp->eraserhead.next != NULL)
					endList(lp);
				return;
			}
		}
	}

	if (lp->dbuf != None) {
		XFreePixmap(display, lp->dbuf);
		lp->dbuf = None;
	}
	if ((lp->dbuf = XCreatePixmap(display, window,
		lp->width, lp->height, MI_DEPTH(mi))) == None) {
		free_life3d_screen(display, lp);
		return;
	}
	DrawScreen(mi);
}

ENTRYPOINT void
draw_life3d(ModeInfo * mi)
{
	life3dstruct *lp;

	if (life3ds == NULL)
		return;
	lp = &life3ds[MI_SCREEN(mi)];
	if (lp->eraserhead.next == NULL)
		return;

	if (!RunLife3D(lp)) {
		if (lp->eraserhead.next != NULL)
			endList(lp);
		return;
	}
	lissajous(lp);
	if (lp->visible) /* kill static life also */
		DrawScreen(mi);

	MI_IS_DRAWN(mi) = True;

	if (!lp->visible || lp->noChangeCount >= 8) {
		/*CountCells3D(lp) == 0) */
		init_life3d(mi);
	} else if (++lp->generation > MI_CYCLES(mi)) {
		if (patternsearch || glidersearch)
			printList(lp, 1);
		init_life3d(mi);
	} else {
		if (MI_IS_VERBOSE(mi))
			(void) fprintf(stdout, "%s (%d cells)\n",
				lp->nameString, lp->ncells);
		if (patternsearch || glidersearch) {
			if (lp->generation == MI_CYCLES(mi) - repeat + 1)
				printList(lp, 0);
		}
		lp->painted = True;
	}

	/*
	 * generate a randomized shooter aimed roughly toward the center of the
	 * screen after batchcount.
	 */

	if (MI_COUNT(mi)) {
		if (lp->generation && lp->generation %
				((MI_COUNT(mi) < 0) ? 1 : MI_COUNT(mi)) == 0)
			if (!shooter(lp)) {
				if (lp->eraserhead.next != NULL)
					endList(lp);
			}
	}
}

ENTRYPOINT void
release_life3d(ModeInfo * mi)
{
	if (life3ds != NULL) {
		int screen;

		for (screen = 0; screen < MI_NUM_SCREENS(mi); screen++)
			free_life3d_screen(MI_DISPLAY(mi), &life3ds[screen]);
		free(life3ds);
		life3ds = (life3dstruct *) NULL;
	}
}

#ifndef STANDALONE
ENTRYPOINT void
refresh_life3d(ModeInfo * mi)
{
	life3dstruct *lp;

	if (life3ds == NULL)
		return;
	lp = &life3ds[MI_SCREEN(mi)];

	if (lp->painted) {
		MI_CLEARWINDOW(mi);
	}
}

ENTRYPOINT void
change_life3d(ModeInfo * mi)
{
	int npats;
	life3dstruct *lp;

	if (life3ds == NULL)
		return;
	lp = &life3ds[MI_SCREEN(mi)];

	lp->generation = 0;

	if (lp->eraserhead.next != NULL)
		endList(lp);
	/*lp->tablesMade = 0; */

	MI_CLEARWINDOW(mi);

	if (!filePattern)
		lp->pattern++;
	lp->patterned_rule = codeToPatternedRule(lp->neighbors, lp->param);
	npats = 0;
	switch (lp->neighbors) {
	case 12:
		if ((unsigned) lp->patterned_rule < LIFE_12RULES)
			npats = patterns_12rules[lp->patterned_rule];
		break;
	case 14:
		if ((unsigned) lp->patterned_rule < LIFE_14RULES)
			npats = patterns_14rules[lp->patterned_rule];
		break;
	case 18:
		if ((unsigned) lp->patterned_rule < LIFE_18RULES)
			npats = patterns_18rules[lp->patterned_rule];
		break;
	case -18:
		if ((unsigned) lp->patterned_rule < LIFE__18RULES)
			npats = patterns__18rules[lp->patterned_rule];
		break;
	case 20:
		if ((unsigned) lp->patterned_rule < LIFE_20RULES)
			npats = patterns_20rules[lp->patterned_rule];
		break;
	case 22:
		if ((unsigned) lp->patterned_rule < LIFE_22RULES)
			npats = patterns_22rules[lp->patterned_rule];
		break;
	case 26:
		if ((unsigned) lp->patterned_rule < LIFE_26RULES)
			npats = patterns_26rules[lp->patterned_rule];
		break;
	}
	if (lp->pattern >= npats + 2) {
		lp->pattern = 0;
		if (lp->allPatterns) {
			lp->patterned_rule++;
			switch (lp->neighbors) {
			case 12:
				if ((unsigned) lp->patterned_rule >= LIFE_12RULES)
					lp->patterned_rule = 0;
				break;
			case 14:
				if ((unsigned) lp->patterned_rule >= LIFE_14RULES)
					lp->patterned_rule = 0;
				break;
			case 18:
				if ((unsigned) lp->patterned_rule >= LIFE_18RULES)
					lp->patterned_rule = 0;
				break;
			case -18:
				if ((unsigned) lp->patterned_rule >= LIFE__18RULES)
					lp->patterned_rule = 0;
				break;
			case 20:
				if ((unsigned) lp->patterned_rule >= LIFE_20RULES)
					lp->patterned_rule = 0;
				break;
			case 22:
				if ((unsigned) lp->patterned_rule >= LIFE_22RULES)
					lp->patterned_rule = 0;
				break;
			case 26:
				if ((unsigned) lp->patterned_rule >= LIFE_26RULES)
					lp->patterned_rule = 0;
				break;
			}
			copyFromPatternedRule(lp->neighbors, &lp->param,
				lp->patterned_rule);
			printRule(lp->neighbors, lp->ruleString, lp->param,
				MI_IS_VERBOSE(mi));
		} else if (lp->allGliders) {
			lp->patterned_rule++;
			switch (lp->neighbors) {
			case 12:
				if ((unsigned) lp->patterned_rule >= LIFE_12GLIDERS)
					lp->patterned_rule = 0;
				break;
			case 14:
				if ((unsigned) lp->patterned_rule >= LIFE_14GLIDERS)
					lp->patterned_rule = 0;
				break;
			case 18:
				if ((unsigned) lp->patterned_rule >= LIFE_18GLIDERS)
					lp->patterned_rule = 0;
				break;
			case -18:
				if ((unsigned) lp->patterned_rule >= LIFE__18GLIDERS)
					lp->patterned_rule = 0;
				break;
			case 20:
				if ((unsigned) lp->patterned_rule >= LIFE_20GLIDERS)
					lp->patterned_rule = 0;
				break;
			case 22:
				if ((unsigned) lp->patterned_rule >= LIFE_22GLIDERS)
					lp->patterned_rule = 0;
				break;
			case 26:
				if ((unsigned) lp->patterned_rule >= LIFE_26GLIDERS)
					lp->patterned_rule = 0;
				break;
			}
			copyFromPatternedRule(lp->neighbors, &lp->param,
				lp->patterned_rule);
			printRule(lp->neighbors, lp->ruleString, lp->param,
				MI_IS_VERBOSE(mi));
		}
	}
	if (!serial && !filePattern)
		lp->pattern = NRAND(npats + 2);
	if (lp->pattern >= npats) {
		if (!RandomSoup(mi, SOUPPERCENT, SOUPSIZE(lp->ncols),
				SOUPSIZE(lp->nrows), SOUPSIZE(lp->nstacks))) {
			if (lp->eraserhead.next != NULL)
				endList(lp);
			return;
		}
	} else {
		if (!GetPattern(mi, lp->patterned_rule, lp->pattern)) {
			if (lp->eraserhead.next != NULL)
				endList(lp);
			return;
		}
	}
	DrawScreen(mi);
}
#endif

XSCREENSAVER_MODULE ("Life3d", life3d)

#endif /* MODE_life3d */
