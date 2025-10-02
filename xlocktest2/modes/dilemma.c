/* -*- Mode: C; tab-width: 4 -*- */
/* dilemma --- Lloyd's Prisoner's Dilemma Simulation */

#if 0
static const char sccsid[] = "@(#)dilemma.c	5.24 2007/01/18 xlockmore";

#endif

/*-
 * Copyright (c) 1997 by David Bagley.
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
 * 18-Jan-2007: Added vertical option.
 * 01-Nov-2000: Allocation checks
 * 20-Oct-1997: Computing Bouts of the Prisoner's Dilemma by Alun L. Lloyd
 *              Scientific American Magazine June 1995
 *              Used voters.c as a guide.
 */

/*-
 *                      Opponent's Strategy
 *
 *                      Cooperate     Defect
 *                     -----------------------
 *                    |           |           |
 *          Cooperate |     1     |     0     |
 * Player's           |           |           |
 * Strategy           |-----------+-----------|
 *                    |           |           |
 *          Defect    |     b     |     0     |
 *                    |           |           |
 *                     -----------------------
 *
 *                        The Payoff Matrix
 *
 * An interesting value of "b" for a 8 neighbor grid is 1.85
 * What does b stand for?  "bonus"?
 * Cells get 1 if they and their opponent cooperates.
 * Cells get b if they cheat and their opponent cooperates.
 * Cells get 0 in the 2 other cases.
 * If b is greater then a cell should always cheat except
 * they have to live with their neighbor... so on the next go around
 * their neighbor might not see any benefit in cooperating.
 * Cells add up the previous results of their neighbors
 * and decide if its their best strategy is to cheat or not to cheat.
 *
 * I have noticed round off errors  I have not as yet tracked them down.
 * Try
 * -bonus 1.99 -neighbors 12  -size 10
 * -bonus 241 -neighbors 6  -size 8
 * -bonus 1.71 -neighbors 4  -size 4
 */

#ifdef STANDALONE
#define MODE_dilemma
#define DEFAULTS "*delay: 200000 \n" \
	"*batchcount: -2 \n" \
	"*cycles: 1000 \n" \
	"*size: 0 \n" \
	"*ncolors: 6 \n" \

# define free_dilemma 0
# define reshape_dilemma 0
# define dilemma_handle_event 0
#include "xlockmore.h"		/* in xscreensaver distribution */
#define UNIFORM_COLORS
#define BRIGHT_COLORS
#define SMOOTH_COLORS
#else /* STANDALONE */
#include "xlock.h"		/* in xlockmore distribution */
#endif /* STANDALONE */
#include "automata.h"

#ifdef MODE_dilemma

/*-
 * neighbors of 0 randomizes it between 3, 4, 6, 8, 9, and 12.
 */
#define DEF_NEIGHBORS  "0"      /* choose random value */
#define DEF_BONUS  "1.85"
#define DEF_CONSCIOUS  "True"
#define DEF_VERTICAL "False"

static int  neighbors;
static float bonus;
static Bool conscious;
static Bool vertical;

static XrmOptionDescRec opts[] =
{
	{(char *) "-neighbors", (char *) ".dilemma.neighbors", XrmoptionSepArg, (caddr_t) NULL},
	{(char *) "-bonus", (char *) ".dilemma.bonus", XrmoptionSepArg, (caddr_t) NULL},
	{(char *) "-conscious", (char *) ".dilemma.conscious", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+conscious", (char *) ".dilemma.conscious", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-vertical", (char *) ".dilemma.vertical", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+vertical", (char *) ".dilemma.vertical", XrmoptionNoArg, (caddr_t) "off"}
};
static argtype vars[] =
{
	{(void *) & neighbors, (char *) "neighbors", (char *) "Neighbors", (char *) DEF_NEIGHBORS, t_Int},
	{(void *) & bonus, (char *) "bonus", (char *) "Bonus", (char *) DEF_BONUS, t_Float},
	{(void *) & conscious, (char *) "conscious", (char *) "Conscious", (char *) DEF_CONSCIOUS, t_Bool},
	{(void *) & vertical, (char *) "vertical", (char *) "Vertical", (char *) DEF_VERTICAL, t_Bool}
};
static OptionStruct desc[] =
{
	{(char *) "-neighbors num", (char *) "squares 4 or 8, hexagons 6, triangles 3, 9 or 12"},
	{(char *) "-bonus value", (char *) "bonus for cheating... between 1.0 and 4.0"},
	{(char *) "-/+conscious", (char *) "turn on/off self-awareness"},
	{(char *) "-/+vertical", (char *) "change orientation for hexagons and triangles"}
};

ENTRYPOINT ModeSpecOpt dilemma_opts =
{sizeof opts / sizeof opts[0], opts, sizeof vars / sizeof vars[0], vars, desc};

#ifdef USE_MODULES
ModStruct   dilemma_description =
{"dilemma", "init_dilemma", "draw_dilemma", "release_dilemma",
 "refresh_dilemma", "init_dilemma", (char *) NULL, &dilemma_opts,
 200000, -2, 1000, 0, 64, 1.0, "",
 "Shows Lloyd's Prisoner's Dilemma simulation", 0, NULL};

#endif

/* Better bitmaps needed :) */
#include "bitmaps/cooperat.xbm"	/* age > 1 then blue, age = 1 then green */
#include "bitmaps/defect.xbm"	/* age > 1 then red, age = 1 then yellow */

#define DEFECTOR 0
#define COOPERATOR 1
#define COLORS 4
#define BLUE (45 * MI_NPIXELS(mi) / 64)		/* COOPERATING, was cooperating */
#define GREEN (23 * MI_NPIXELS(mi) / 64)	/* COOPERATING, was defecting */
#define YELLOW (MI_NPIXELS(mi) / 6)	/* DEFECTING, was cooperating */
#define RED	0		/* DEFECTING, was defecting */
#define MINDEFECT 1
#define BITMAPS 2
#define MINGRIDSIZE 16
#define MINSIZE 10
#define FACTOR 10
#define NEIGHBORKINDS 2
#define REDRAWSTEP 2000		/* How many cells to draw per cycle */
#define ROUND_FLOAT(x,a) ((float) ((int) ((x) / (a) + 0.5)) * (a))

static XImage logo[BITMAPS] =
{
    {0, 0, 0, XYBitmap, (char *) cooperat_bits, LSBFirst, 8, LSBFirst, 8, 1},
    {0, 0, 0, XYBitmap, (char *) defect_bits, LSBFirst, 8, LSBFirst, 8, 1}
};

/* Dilemma data */

/* Singly linked list */
typedef struct _CellList {
	XPoint      pt;
	struct _CellList *next;
} CellList;

typedef struct {
	Bool        vertical;
	int         defectors;	/* portion of defectors */
	float       pm[2][2];	/* payoff matrix */
	unsigned long colors[2][2];
	CellList   *cellList[COLORS];
	char       *s, *sn;	/* cell strategies */
	float      *payoff;
	int         initialized;
	int         xs, ys;	/* Size of cooperators and defectors */
	int         xb, yb;	/* Bitmap offset for cooperators and defectors */
	int         state;
	int         redrawing, redrawpos;
	int         pixelmode;
	int         generation;
	int         ncols, nrows;
	int         npositions;
	int         width, height;
	int         neighbors, polygon;
	union {
		XPoint      hexagon[7];
		XPoint      triangle[2][4];
	} shape;
} dilemmastruct;

static char plots[NEIGHBORKINDS] =
{
	4, 8	/* Neighborhoods: 3, 6, 9 & 12 do not really work */
};

static dilemmastruct *dilemmas = (dilemmastruct *) NULL;
static int  icon_width, icon_height;

static void
drawcell(ModeInfo * mi, int col, int row, unsigned long color, int bitmap,
		Bool firstChange)
{
	Display    *display = MI_DISPLAY(mi);
	Window      window = MI_WINDOW(mi);
	GC          gc = MI_GC(mi);
	dilemmastruct *dp = &dilemmas[MI_SCREEN(mi)];
	unsigned long colour = (MI_NPIXELS(mi) >= COLORS) ?
		color : MI_WHITE_PIXEL(mi);

	XSetForeground(display, gc, colour);
	if (dp->neighbors == 6) {
		int ccol = 2 * col + !(row & 1), crow = 2 * row;

		if (dp->vertical) {
			dp->shape.hexagon[0].x = dp->xb + ccol * dp->xs;
			dp->shape.hexagon[0].y = dp->yb + crow * dp->ys;
		} else {
			dp->shape.hexagon[0].y = dp->xb + ccol * dp->xs;
			dp->shape.hexagon[0].x = dp->yb + crow * dp->ys;
		}
		if (dp->xs == 1 && dp->ys == 1)
			XDrawPoint(display, window, gc,
				dp->shape.hexagon[0].x,
				dp->shape.hexagon[0].y);
		else if (bitmap == BITMAPS - 1)
			XFillPolygon(display, window, gc,
				dp->shape.hexagon, 6,
				Convex, CoordModePrevious);
		else {
			int ix = 0, iy = 0, sx, sy;

			if (firstChange) {
				XSetForeground(display, gc, MI_BLACK_PIXEL(mi));
				XFillPolygon(display, window, gc,
					dp->shape.hexagon, 6,
					Convex, CoordModePrevious);
				XSetForeground(display, gc, colour);
			}
			if (dp->vertical) {
				dp->shape.hexagon[0].x -= dp->xs;
				dp->shape.hexagon[0].y += dp->ys / 4;
				sx = 2 * dp->xs - 6;
				sy = 2 * dp->ys - 2;
				if (dp->xs <= 6 || dp->ys <= 2) {
					ix = 3;
					iy = 1;
				} else
					ix = 5;
			} else {
				dp->shape.hexagon[0].y -= dp->xs;
				dp->shape.hexagon[0].x += dp->ys / 4;
				sy = 2 * dp->xs - 6;
				sx = 2 * dp->ys - 2;
				if (dp->xs <= 6 || dp->ys <= 2) {
					iy = 3;
					ix = 1;
				} else
					iy = 5;
			}
			if (dp->xs <= 6 || dp->ys <= 2)
				XFillRectangle(display, window, gc,
					dp->shape.hexagon[0].x + ix,
					dp->shape.hexagon[0].y + iy,
					dp->xs, dp->ys);
			else
				XFillArc(display, window, gc,
					dp->shape.hexagon[0].x + ix,
					dp->shape.hexagon[0].y + iy,
					sx, sy,
					0, 23040);
		}
	} else if (dp->neighbors == 4 || dp->neighbors == 8) {
		if (dp->pixelmode) {
			if (bitmap == BITMAPS - 1 || (dp->xs <= 2 || dp->ys <= 2))
				XFillRectangle(display, window, gc,
					dp->xb + dp->xs * col,
					dp->yb + dp->ys * row,
					dp->xs - (dp->xs > 3),
					dp->ys - (dp->ys > 3));
			else {
				if (firstChange) {
					XSetForeground(display, gc,
						MI_BLACK_PIXEL(mi));
					XFillRectangle(display, window, gc,
						dp->xb + dp->xs * col,
						dp->yb + dp->ys * row,
						dp->xs, dp->ys);
					XSetForeground(display, gc, colour);
				}
				XFillArc(display, window, gc,
					dp->xb + dp->xs * col,
					dp->yb + dp->ys * row,
					dp->xs - 1, dp->ys - 1,
					0, 23040);
			}
		} else
			(void) XPutImage(display, window, gc,
				&logo[bitmap], 0, 0,
				dp->xb + dp->xs * col, dp->yb + dp->ys * row,
				icon_width, icon_height);
	} else {		/* TRI */
		int orient = (col + row) % 2;	/* O left 1 right */
		Bool small = (dp->xs <= 3 || dp->ys <= 3);
		int ix = 0, iy = 0;

		if (dp->vertical) {
			dp->shape.triangle[orient][0].x = dp->xb + col * dp->xs;
			dp->shape.triangle[orient][0].y = dp->yb + row * dp->ys;
			if (small)
				dp->shape.triangle[orient][0].x +=
					((orient) ? -1 : 1);
			else
				dp->shape.triangle[orient][0].x +=
					(dp->xs / 2  - 1) * ((orient) ? 1 : -1);
			ix = ((orient) ? -dp->xs / 2 : dp->xs / 2);
		} else {
			dp->shape.triangle[orient][0].y = dp->xb + col * dp->xs;
			dp->shape.triangle[orient][0].x = dp->yb + row * dp->ys;
			if (small)
				dp->shape.triangle[orient][0].y +=
					((orient) ? -1 : 1);
			else
				dp->shape.triangle[orient][0].y +=
					(dp->xs / 2  - 1) * ((orient) ? 1 : -1);
			iy = ((orient) ? -dp->xs / 2 : dp->xs / 2);
		}
		if (small)
			XDrawPoint(display, window, gc,
				dp->shape.triangle[orient][0].x,
				dp->shape.triangle[orient][0].y);
		else {
			if (bitmap == BITMAPS - 1) {
				XFillPolygon(display, window, gc,
					dp->shape.triangle[orient], 3,
					Convex, CoordModePrevious);
				XSetForeground(display, gc, MI_BLACK_PIXEL(mi));
				XDrawLines(display, window, gc,
					dp->shape.triangle[orient], 4,
					CoordModePrevious);
			} else {
				if (firstChange) {
					XSetForeground(display, gc,
						MI_BLACK_PIXEL(mi));
					XFillPolygon(display, window, gc,
						dp->shape.triangle[orient], 3,
						Convex, CoordModePrevious);
					XSetForeground(display, gc, colour);
				}
				if (dp->vertical) {
					dp->shape.triangle[orient][0].x += -4 * dp->xs / 5 +
						((orient) ? dp->xs / 3 : 3 * dp->xs / 5);
					dp->shape.triangle[orient][0].y += -dp->ys / 2 + 1;
					ix = ((orient) ? -dp->xs / 2 : dp->xs / 2);
				} else {
					dp->shape.triangle[orient][0].y += -4 * dp->xs / 5 +
						((orient) ? dp->xs / 3 : 3 * dp->xs / 5);
					dp->shape.triangle[orient][0].x += -dp->ys / 2 + 1;
					iy = ((orient) ? -dp->xs / 2 : dp->xs / 2);
				}
				XFillArc(display, window, gc,
					dp->shape.triangle[orient][0].x + ix,
					dp->shape.triangle[orient][0].y + iy,
					dp->ys - 3, dp->ys - 3,
					0, 23040);
			}
		}
	}
}

static void
addtolist(ModeInfo * mi, int col, int row, int state)
{
	dilemmastruct *dp = &dilemmas[MI_SCREEN(mi)];
	CellList   *current;

	current = dp->cellList[state];
	dp->cellList[state] = (CellList *) malloc(sizeof (CellList));
	dp->cellList[state]->pt.x = col;
	dp->cellList[state]->pt.y = row;
	dp->cellList[state]->next = current;
}

#ifdef DEBUG
static void
print_state(ModeInfo * mi, int state)
{
	dilemmastruct *dp = &dilemmas[MI_SCREEN(mi)];
	CellList   *locallist;
	int         i = 0;

	locallist = dp->cellList[state];
	(void) printf("state %d\n", state);
	while (locallist) {
		(void) printf("%d x %d, y %d\n", i,
			      locallist->pt.x, locallist->pt.y);
		locallist = locallist->next;
		i++;
	}
}

#endif

static void
free_state(dilemmastruct * dp, int state)
{
	CellList   *current;

	while (dp->cellList[state]) {
		current = dp->cellList[state];
		dp->cellList[state] = dp->cellList[state]->next;
		free(current);
	}
	dp->cellList[state] = (CellList *) NULL;
}


static void
free_list(dilemmastruct * dp)
{
	int         state;

	for (state = 0; state < COLORS; state++)
		free_state(dp, state);
}

static void
free_dilemma_screen(dilemmastruct *dp)
{
	if (dp == NULL) {
		return;
	}
	free_list(dp);
	if (dp->sn != NULL) {
		free(dp->sn);
		dp->sn = (char *) NULL;
	}
	if (dp->s != NULL) {
		free(dp->s);
		dp->s = (char *) NULL;
	}
	if (dp->payoff != NULL) {
		free(dp->payoff);
		dp->payoff = (float *) NULL;
	}
	dp = NULL;
}

static void
alloc_dilemma(dilemmastruct *dp)
{
	if ((dp->s = (char *) calloc(dp->npositions, sizeof (char))) == NULL) {
		free_dilemma_screen(dp);
		return;
	}
	if ((dp->sn = (char *) calloc(dp->npositions, sizeof (char))) == NULL) {
		free_dilemma_screen(dp);
		return;
	}
	if ((dp->payoff = (float *) calloc(dp->npositions,
			sizeof (float))) == NULL) {
		free_dilemma_screen(dp);
		return;
	}
}


static int
positionOfNeighbor(dilemmastruct * dp, int n, int col, int row)
{
	int dir = n * (360 / dp->neighbors);

	if (dp->polygon == 4 || dp->polygon == 6) {
		switch (dir) {
		case 0:
			col = (col + 1 == dp->ncols) ? 0 : col + 1;
			break;
		case 45:
			col = (col + 1 == dp->ncols) ? 0 : col + 1;
			row = (!row) ? dp->nrows - 1 : row - 1;
			break;
		case 60:
			if (!(row & 1))
				col = (col + 1 == dp->ncols) ? 0 : col + 1;
			row = (!row) ? dp->nrows - 1 : row - 1;
			break;
		case 90:
			row = (!row) ? dp->nrows - 1 : row - 1;
			break;
		case 120:
			if (row & 1)
				col = (!col) ? dp->ncols - 1 : col - 1;
			row = (!row) ? dp->nrows - 1 : row - 1;
			break;
		case 135:
			col = (!col) ? dp->ncols - 1 : col - 1;
			row = (!row) ? dp->nrows - 1 : row - 1;
			break;
		case 180:
			col = (!col) ? dp->ncols - 1 : col - 1;
			break;
		case 225:
			col = (!col) ? dp->ncols - 1 : col - 1;
			row = (row + 1 == dp->nrows) ? 0 : row + 1;
			break;
		case 240:
			if (row & 1)
				col = (!col) ? dp->ncols - 1 : col - 1;
			row = (row + 1 == dp->nrows) ? 0 : row + 1;
			break;
		case 270:
			row = (row + 1 == dp->nrows) ? 0 : row + 1;
			break;
		case 300:
			if (!(row & 1))
				col = (col + 1 == dp->ncols) ? 0 : col + 1;
			row = (row + 1 == dp->nrows) ? 0 : row + 1;
			break;
		case 315:
			col = (col + 1 == dp->ncols) ? 0 : col + 1;
			row = (row + 1 == dp->nrows) ? 0 : row + 1;
			break;
		default:
			(void) fprintf(stderr, "wrong direction %d\n", dir);
		}
	} else if (dp->polygon == 3) {
		if ((col + row) & 1) {	/* right */
			switch (dir) {
			case 0:
				col = (!col) ? dp->ncols - 1 : col - 1;
				break;
			case 30:
			case 40:
				col = (!col) ? dp->ncols - 1 : col - 1;
				row = (row + 1 == dp->nrows) ? 0 : row + 1;
				break;
			case 60:
				col = (!col) ? dp->ncols - 1 : col - 1;
				if (row + 1 == dp->nrows)
					row = 1;
				else if (row + 2 == dp->nrows)
					row = 0;
				else
					row = row + 2;
				break;
			case 80:
			case 90:
				if (row + 1 == dp->nrows)
					row = 1;
				else if (row + 2 == dp->nrows)
					row = 0;
				else
					row = row + 2;
				break;
			case 120:
				row = (row + 1 == dp->nrows) ? 0 : row + 1;
				break;
			case 150:
			case 160:
				col = (col + 1 == dp->ncols) ? 0 : col + 1;
				row = (row + 1 == dp->nrows) ? 0 : row + 1;
				break;
			case 180:
				col = (col + 1 == dp->ncols) ? 0 : col + 1;
				break;
			case 200:
			case 210:
				col = (col + 1 == dp->ncols) ? 0 : col + 1;
				row = (!row) ? dp->nrows - 1 : row - 1;
				break;
			case 240:
				row = (!row) ? dp->nrows - 1 : row - 1;
				break;
			case 270:
			case 280:
				if (!row)
					row = dp->nrows - 2;
				else if (!(row - 1))
					row = dp->nrows - 1;
				else
					row = row - 2;
				break;
			case 300:
				col = (!col) ? dp->ncols - 1 : col - 1;
				if (!row)
					row = dp->nrows - 2;
				else if (!(row - 1))
					row = dp->nrows - 1;
				else
					row = row - 2;
				break;
			case 320:
			case 330:
				col = (!col) ? dp->ncols - 1 : col - 1;
				row = (!row) ? dp->nrows - 1 : row - 1;
				break;
			default:
				(void) fprintf(stderr, "wrong direction %d\n",
					dir);
			}
		} else {	/* left */
			switch (dir) {
			case 0:
				col = (col + 1 == dp->ncols) ? 0 : col + 1;
				break;
			case 30:
			case 40:
				col = (col + 1 == dp->ncols) ? 0 : col + 1;
				row = (!row) ? dp->nrows - 1 : row - 1;
				break;
			case 60:
				col = (col + 1 == dp->ncols) ? 0 : col + 1;
				if (!row)
					row = dp->nrows - 2;
				else if (row == 1)
					row = dp->nrows - 1;
				else
					row = row - 2;
				break;
			case 80:
			case 90:
				if (!row)
					row = dp->nrows - 2;
				else if (row == 1)
					row = dp->nrows - 1;
				else
					row = row - 2;
				break;
			case 120:
				row = (!row) ? dp->nrows - 1 : row - 1;
				break;
			case 150:
			case 160:
				col = (!col) ? dp->ncols - 1 : col - 1;
				row = (!row) ? dp->nrows - 1 : row - 1;
				break;
			case 180:
				col = (!col) ? dp->ncols - 1 : col - 1;
				break;
			case 200:
			case 210:
				col = (!col) ? dp->ncols - 1 : col - 1;
				row = (row + 1 == dp->nrows) ? 0 : row + 1;
				break;
			case 240:
				row = (row + 1 == dp->nrows) ? 0 : row + 1;
				break;
			case 270:
			case 280:
				if (row + 1 == dp->nrows)
					row = 1;
				else if (row + 2 == dp->nrows)
					row = 0;
				else
					row = row + 2;
				break;
			case 300:
				col = (col + 1 == dp->ncols) ? 0 : col + 1;
				if (row + 1 == dp->nrows)
					row = 1;
				else if (row + 2 == dp->nrows)
					row = 0;
				else
					row = row + 2;
				break;
			case 320:
			case 330:
				col = (col + 1 == dp->ncols) ? 0 : col + 1;
				row = (row + 1 == dp->nrows) ? 0 : row + 1;
				break;
			default:
				(void) fprintf(stderr, "wrong direction %d\n",
					dir);
			}
		}
#if 0
	} else {
		int orient = ((row & 1) * 2 + col) % 4;
		switch (orient) { /* up, down, left, right */
		case 0:
			switch (dir) {
			case 0:
				col++;
				break;
			case 51: /* 7 */
			case 72: /* 5 */
				col = (col + 2 >= dp->ncols) ? 0 : col + 2;
				break;
			case 102: /* 7 corner */
				col = (col + 3 >= dp->ncols) ? 1 : col + 3;
				row = (row == 0) ? dp->nrows - 1 : row - 1;
				break;
			case 144: /* 5 */
			case 153: /* 7 */
				col++;
				row = (row == 0) ? dp->nrows - 1 : row - 1;
				break;
			case 204: /* 7 */
			case 216: /* 5 */
				row = (row == 0) ? dp->nrows - 1 : row - 1;
				break;
			case 255: /* 7 */
				col = (col == 0) ? dp->ncols - 1 : col - 1;
				row = (row == 0) ? dp->nrows - 1 : row - 1;
				break;
			case 288: /* 5 */
			case 306: /* 7 */
				col = (col == 0) ? dp->ncols - 1 : col - 1;
				break;
			default:
				(void) fprintf(stderr, "wrong direction %d\n",
					dir);
			}
			break;
		case 1:
			switch (dir) {
			case 0:
				col--;
				break;
			case 51: /* 7 */
			case 72: /* 5 */
				col = (col == 1) ? dp->ncols - 1 : col - 2;
				break;
			case 102: /* 7 */
				col = (col == 1) ? dp->ncols - 2 : col - 3;
				row = (row + 1 == dp->nrows) ? 0 : row + 1;
				break;
			case 144: /* 5 */
			case 153: /* 7 */
				col--;
				row = (row + 1 == dp->nrows) ? 0 : row + 1;
				break;
			case 204: /* 7 */
			case 216: /* 5 */
				row = (row + 1 == dp->nrows) ? 0 : row + 1;
				break;
			case 255: /* 7 */
				col = (col + 1 >= dp->ncols) ? 0 : col + 1;
				row = (row + 1 == dp->nrows) ? 0 : row + 1;
				break;
			case 288: /* 5 */
			case 306: /* 7 */
				col = (col + 1 >= dp->ncols) ? 0 : col + 1;
				break;
			default:
				(void) fprintf(stderr, "wrong direction %d\n",
					dir);
			}
			break;
		case 2:
			switch (dir) {
			case 0:
				col = (col + 1 >= dp->ncols) ? 0 : col + 1;
				break;
			case 51: /* 7 */
			case 72: /* 5 */
				row = (row == 0) ? dp->nrows - 1 : row - 1;
				col++;
				break;
			case 102: /* 7 */
				col = (col == 0) ? dp->ncols - 1 : col - 1;
				row = (row == 0) ? dp->nrows - 1 : row - 1;
				break;
			case 144: /* 5 */
			case 153: /* 7 */
				col = (col == 0) ? dp->ncols - 2 : col - 2;
				break;
			case 204: /* 7 */
			case 216: /* 5 */
				col = (col == 0) ? dp->ncols - 1 : col - 1;
				break;
			case 255: /* 7 */
				row = (row + 1 == dp->nrows) ? 0 : row + 1;
				col = (col == 0) ? dp->ncols - 1 : col - 1;
				break;
			case 288: /* 5 */
			case 306: /* 7 */
				row = (row + 1 == dp->nrows) ? 0 : row + 1;
				break;
			default:
				(void) fprintf(stderr, "wrong direction %d\n",
					dir);
			}
			break;
		case 3:
			switch (dir) {
			case 0:
				col--;
				break;
			case 51: /* 7 */
			case 72: /* 5 */
				col = (col == 0) ? dp->ncols - 1 : col - 1;
				row = (row + 1 == dp->nrows) ? 0 : row + 1;
				break;
			case 102: /* 7 */
				col = (col + 1 >= dp->ncols) ? 0 : col + 1;
				row = (row + 1 == dp->nrows) ? 0 : row + 1;
				break;
			case 144: /* 5 */
			case 153: /* 7 */
				col = (col + 2 >= dp->ncols) ? 1 : col + 2;
				break;
			case 204: /* 7 */
			case 216: /* 5 */
				col = (col + 1 >= dp->ncols) ? 0 : col + 1;
				break;
			case 255: /* 7 */
				col = (col + 1 >= dp->ncols) ? 0 : col + 1;
				row = (row == 0) ? dp->nrows - 1 : row - 1;
				break;
			case 288: /* 5 */
			case 306: /* 7 */
				row = (row == 0) ? dp->nrows - 1 : row - 1;
				break;
			default:
				(void) fprintf(stderr, "wrong direction %d\n",
					dir);
			}
			break;
		default:
			(void) fprintf(stderr, "wrong orient %d\n",
				orient);
		}
#endif
	}
	return (row * dp->ncols + col);
}
static void
draw_state(ModeInfo * mi, int state)
{
	dilemmastruct *dp = &dilemmas[MI_SCREEN(mi)];
	CellList   *current;


	current = dp->cellList[state];
	while (current) {
		int         col = current->pt.x;
		int         row = current->pt.y;
		int         colrow = col + row * dp->ncols;

		drawcell(mi, col, row,
		       dp->colors[(int) dp->sn[colrow]][(int) dp->s[colrow]],
			 dp->sn[colrow], False);
		if (dp->s[colrow] && !dp->sn[colrow])
			dp->defectors--;
		if (!dp->s[colrow] && dp->sn[colrow])
			dp->defectors++;
		dp->s[colrow] = dp->sn[colrow];
		current = current->next;
	}
	free_state(dp, state);
	XFlush(MI_DISPLAY(mi));
}

ENTRYPOINT void
init_dilemma(ModeInfo * mi)
{
	int         size = MI_SIZE(mi);
	int         i, col, row, colrow, mrow;
	dilemmastruct *dp;

	MI_INIT(mi, dilemmas);
	dp = &dilemmas[MI_SCREEN(mi)];

	dp->generation = 0;
	dp->redrawing = 0;
	dp->state = 0;
	free_dilemma_screen(dp);

	if (!dp->initialized) {	/* Genesis */
		icon_width = cooperat_width;
		icon_height = cooperat_height;
		dp->initialized = 1;
		for (i = 0; i < BITMAPS; i++) {
			logo[i].width = icon_width;
			logo[i].height = icon_height;
			logo[i].bytes_per_line = (icon_width + 7) / 8;
		}
	}
	if (MI_IS_FULLRANDOM(mi)) {
		dp->vertical = (Bool) (LRAND() & 1);
	} else {
		dp->vertical = vertical;
	}
	dp->width = MI_WIDTH(mi);
	dp->height = MI_HEIGHT(mi);

	for (i = 0; i < NEIGHBORKINDS; i++) {
		if (neighbors == plots[i]) {
			dp->neighbors = neighbors;
			break;
		}
		if (i == NEIGHBORKINDS - 1) {
#if 0
			dp->neighbors = plots[NRAND(NEIGHBORKINDS)];
			dp->neighbors = (LRAND() & 1) ? 4 : 8;
#else
			dp->neighbors = 8;
#endif
			break;
		}
	}

	if (dp->neighbors == 6) {
		int nccols, ncrows, sides;

		dp->polygon = 6;
		if (!dp->vertical) {
			dp->height = MI_WIDTH(mi);
			dp->width = MI_HEIGHT(mi);
		}
		if (dp->width < 2)
			dp->width = 2;
		if (dp->height < 4)
			dp->height = 4;
		if (size < -MINSIZE)
			dp->ys = NRAND(MIN(-size, MAX(MINSIZE, MIN(dp->width, dp->height) /
				      MINGRIDSIZE)) - MINSIZE + 1) + MINSIZE;
		else if (size < MINSIZE) {
			if (!size)
				dp->ys = MAX(MINSIZE, MIN(dp->width, dp->height) / MINGRIDSIZE);
			else
				dp->ys = MINSIZE;
		} else
			dp->ys = MIN(size, MAX(MINSIZE, MIN(dp->width, dp->height) /
					       MINGRIDSIZE));
		dp->xs = dp->ys;
		dp->pixelmode = True;
		nccols = MAX(dp->width / dp->xs - 2, 2);
		ncrows = MAX(dp->height / dp->ys - 1, 2);
		dp->ncols = nccols / 2;
		dp->nrows = 2 * (ncrows / 4);
		dp->xb = (dp->width - dp->xs * nccols) / 2 + dp->xs / 2;
		dp->yb = (dp->height - dp->ys * (ncrows / 2) * 2) / 2 +
			dp->ys - 2;
		for (sides = 0; sides < 6; sides++) {
			if (dp->vertical) {
				dp->shape.hexagon[sides].x =
					(dp->xs - 1) * hexagonUnit[sides].x;
				dp->shape.hexagon[sides].y =
					((dp->ys - 1) * hexagonUnit[sides].y /
					2) * 4 / 3;
			} else {
				dp->shape.hexagon[sides].y =
					(dp->xs - 1) * hexagonUnit[sides].x;
				dp->shape.hexagon[sides].x =
					((dp->ys - 1) * hexagonUnit[sides].y /
					2) * 4 / 3;
			}
		}
	} else if (dp->neighbors == 4 || dp->neighbors == 8) {
		dp->polygon = 4;
		if (dp->width < 2)
			dp->width = 2;
		if (dp->height < 2)
			dp->height = 2;
		if (size == 0 ||
		    MINGRIDSIZE * size > dp->width || MINGRIDSIZE * size > dp->height) {
			if (dp->width > MINGRIDSIZE * icon_width &&
			    dp->height > MINGRIDSIZE * icon_height) {
				dp->pixelmode = False;
				dp->xs = icon_width;
				dp->ys = icon_height;
			} else {
				dp->pixelmode = True;
				dp->xs = dp->ys = MAX(MINSIZE, MIN(dp->width, dp->height) /
						      MINGRIDSIZE);
			}
		} else {
			dp->pixelmode = True;
			if (size < -MINSIZE)
				dp->ys = NRAND(MIN(-size, MAX(MINSIZE, MIN(dp->width, dp->height) /
				      MINGRIDSIZE)) - MINSIZE + 1) + MINSIZE;
			else if (size < MINSIZE)
				dp->ys = MINSIZE;
			else
				dp->ys = MIN(size, MAX(MINSIZE, MIN(dp->width, dp->height) /
						       MINGRIDSIZE));
			dp->xs = dp->ys;
		}
		dp->ncols = MAX(dp->width / dp->xs, 2);
		dp->nrows = MAX(dp->height / dp->ys, 2);
		dp->xb = (dp->width - dp->xs * dp->ncols) / 2;
		dp->yb = (dp->height - dp->ys * dp->nrows) / 2;
	} else {		/* TRI */
		int orient;

		dp->polygon = 3;
		if (!dp->vertical) {
			dp->height = MI_WIDTH(mi);
			dp->width = MI_HEIGHT(mi);
		}
		if (dp->width < 2)
			dp->width = 2;
		if (dp->height < 2)
			dp->height = 2;
		if (size < -MINSIZE)
			dp->ys = NRAND(MIN(-size, MAX(MINSIZE, MIN(dp->width, dp->height) /
				      MINGRIDSIZE)) - MINSIZE + 1) + MINSIZE;
		else if (size < MINSIZE) {
			if (!size)
				dp->ys = MAX(MINSIZE, MIN(dp->width, dp->height) / MINGRIDSIZE);
			else
				dp->ys = MINSIZE;
		} else
			dp->ys = MIN(size, MAX(MINSIZE, MIN(dp->width, dp->height) /
					       MINGRIDSIZE));
		dp->xs = (int) (1.52 * dp->ys);
		dp->pixelmode = True;
		dp->ncols = (MAX(dp->width / dp->xs - 1, 2) / 2) * 2;
		dp->nrows = (MAX(dp->height / dp->ys - 1, 2) / 2) * 2;
		dp->xb = (dp->width - dp->xs * dp->ncols) / 2 + dp->xs / 2;
		dp->yb = (dp->height - dp->ys * dp->nrows) / 2 + dp->ys / 2;
		for (orient = 0; orient < 2; orient++) {
			for (i = 0; i < 3; i++) {
				if (dp->vertical) {
					dp->shape.triangle[orient][i].x =
						(dp->xs - 2) * triangleUnit[orient][i].x;
					dp->shape.triangle[orient][i].y =
						(dp->ys - 2) * triangleUnit[orient][i].y;
				} else {
					dp->shape.triangle[orient][i].y =
						(dp->xs - 2) * triangleUnit[orient][i].x;
					dp->shape.triangle[orient][i].x =
						(dp->ys - 2) * triangleUnit[orient][i].y;
				}
			}
		}
	}

	dp->npositions = dp->ncols * dp->nrows;

	dp->pm[0][0] = 1, dp->pm[0][1] = 0;
	if (bonus < 1.0 || bonus > 4.0)
		dp->pm[1][0] = 1.85;
	else
		dp->pm[1][0] = bonus;
	dp->pm[1][1] = 0;

	if (MI_NPIXELS(mi) >= COLORS) {

		dp->colors[0][0] = MI_PIXEL(mi, BLUE);	/* COOPERATING, was cooperating */
		dp->colors[0][1] = MI_PIXEL(mi, GREEN);		/* COOPERATING, was defecting */
		dp->colors[1][0] = MI_PIXEL(mi, YELLOW);	/* DEFECTING, was cooperating */
		dp->colors[1][1] = MI_PIXEL(mi, RED);	/* DEFECTING, was defecting */
	} else {
		dp->colors[0][0] = MI_WHITE_PIXEL(mi);
		dp->colors[0][1] = MI_WHITE_PIXEL(mi);
		dp->colors[1][0] = MI_WHITE_PIXEL(mi);
		dp->colors[1][1] = MI_WHITE_PIXEL(mi);
	}
	alloc_dilemma(dp);
	if (dp->s == NULL)
		return;
	MI_CLEARWINDOW(mi);

	dp->defectors = MI_COUNT(mi);
	if (dp->defectors < -MINDEFECT) {
		dp->defectors = NRAND(-dp->defectors - MINDEFECT + 1) + MINDEFECT;
	} else if (dp->defectors < MINDEFECT)
		dp->defectors = MINDEFECT;
	if (dp->defectors > dp->npositions)
		dp->defectors = dp->npositions;

	for (i = 0; i < dp->defectors; i++) {
		do {
			colrow = NRAND(dp->npositions);
		} while (dp->sn[colrow]);
		dp->sn[colrow] = 1;
	}
#if 0				/* if p was a float... */
	mrow = 0;
	for (row = 0; row < dp->nrows; row++) {
		for (col = 0; col < dp->ncols; col++) {
			dp->sn[col + mrow] = ((float) LRAND() / MAXRAND < dp->p);
		}
		mrow += dp->ncols;
	}
#endif

	dp->defectors = 0;

	/* Show initial state... real important for debugging */
	mrow = 0;
	for (row = 0; row < dp->nrows; ++row) {
		for (col = 0; col < dp->ncols; ++col) {
			addtolist(mi, col, row,
			   dp->sn[col + mrow] * BITMAPS + dp->s[col + mrow]);
		}
		mrow += dp->ncols;
	}
}

ENTRYPOINT void
draw_dilemma(ModeInfo * mi)
{
	int         col, row, mrow, colrow, n, i;
	dilemmastruct *dp;

	if (dilemmas == NULL)
		return;
	dp = &dilemmas[MI_SCREEN(mi)];
	if (dp->s == NULL)
		return;

	MI_IS_DRAWN(mi) = True;

	if (dp->state >= 2 * COLORS) {

		for (col = 0; col < dp->ncols; col++) {
			for (row = 0; row < dp->nrows; row++) {
				colrow = col + row * dp->ncols;
				if (conscious)
					dp->payoff[colrow] =
						dp->pm[(int) dp->s[colrow]][(int) dp->s[colrow]];
				else
					dp->payoff[colrow] = 0.0;
				for (n = 0; n < dp->neighbors; n++)
					dp->payoff[colrow] +=
						dp->pm[(int) dp->s[colrow]][(int)
						  dp->s[positionOfNeighbor(dp,
					n, col, row)]];

			}
		}
		for (row = 0; row < dp->nrows; row++) {
			for (col = 0; col < dp->ncols; col++) {
				float       hp;
				int         position;

				colrow = col + row * dp->ncols;
				hp = dp->payoff[colrow];
				dp->sn[colrow] = dp->s[colrow];
				for (n = 0; n < dp->neighbors; n++) {
					position = positionOfNeighbor(dp, n, col, row);
					if (ROUND_FLOAT(dp->payoff[position], 0.001) >
					    ROUND_FLOAT(hp, 0.001)) {
						hp = dp->payoff[position];
						dp->sn[colrow] = dp->s[position];
					}
				}
			}
		}
		mrow = 0;
		for (row = 0; row < dp->nrows; row++) {
			for (col = 0; col < dp->ncols; col++) {
				addtolist(mi, col, row,
					  dp->sn[col + mrow] * BITMAPS + dp->s[col + mrow]);
			}
			mrow += dp->ncols;
		}

		if (++dp->generation > MI_CYCLES(mi) ||
		    dp->defectors == dp->npositions || dp->defectors == 0)
			init_dilemma(mi);
		dp->state = 0;
	} else {
		if (dp->state < COLORS) {
			draw_state(mi, dp->state);
		}
		dp->state++;
	}
#if 1
	if (dp->redrawing) {
		for (i = 0; i < REDRAWSTEP; i++) {
			drawcell(mi, dp->redrawpos % dp->ncols, dp->redrawpos / dp->ncols,
				 dp->colors[(int) dp->sn[dp->redrawpos]][(int) dp->s[dp->redrawpos]],
				 dp->sn[dp->redrawpos], False);
			if (++(dp->redrawpos) >= dp->npositions) {
				dp->redrawing = 0;
				break;
			}
		}
	}
#endif
}

ENTRYPOINT void
release_dilemma(ModeInfo * mi)
{
	if (dilemmas != NULL) {
		int         screen;

		for (screen = 0; screen < MI_NUM_SCREENS(mi); screen++)
			free_dilemma_screen(&dilemmas[screen]);
		free(dilemmas);
		dilemmas = (dilemmastruct *) NULL;
	}
}

#ifndef STANDALONE
ENTRYPOINT void
refresh_dilemma(ModeInfo * mi)
{
	dilemmastruct *dp = &dilemmas[MI_SCREEN(mi)];

	dp->redrawing = 1;
	dp->redrawpos = 0;
}
#endif

XSCREENSAVER_MODULE ("Dilemma", dilemma)

#endif /* MODE_dilemma */
