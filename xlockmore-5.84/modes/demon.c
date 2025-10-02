/* -*- Mode: C; tab-width: 4 -*- */
/* demon --- David Griffeath's cellular automata */

#if 0
static const char sccsid[] = "@(#)demon.c	5.24 2007/01/18 xlockmore";

#endif

/*-
 * Copyright (c) 1995 by David Bagley.
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
 * 10-May-1997: Compatible with xscreensaver
 * 16-Apr-1997: -neighbors 3, 9 (not sound mathematically), 12, and 8 added
 * 30-May-1996: Ron Hitchens <ron@idiom.com>
 *            Fixed memory management that caused leaks
 * 14-Apr-1996: -neighbors 6 runtime-time option added
 * 21-Aug-1995: Coded from A.K. Dewdney's "Computer Recreations", Scientific
 *              American Magazine" Aug 1989 pp 102-105.  Also very similar
 *              to hodgepodge machine described in A.K. Dewdney's "Computer
 *              Recreations", Scientific American Magazine" Aug 1988
 *              pp 104-107.  Also used life.c as a guide.
 */

/*-
 * A cellular universe of 4 phases debris, droplets, defects, and demons.
 */

/*-
  Grid     Number of Neighbors
  ----     ------------------
  Square   4 or 8
  Hexagon  6
  Triangle 3, 9, or 12
  Pentagon 5 or 7
*/

#ifdef STANDALONE
#define MODE_demon
#define DEFAULTS "*delay: 50000 \n" \
	"*count: 0 \n" \
	"*cycles: 1000 \n" \
	"*size: -7 \n" \
	"*ncolors: 64 \n" \

# define free_demon 0
# define reshape_demon 0
# define demon_handle_event 0
#define UNIFORM_COLORS
#include "xlockmore.h"		/* in xscreensaver distribution */
#else /* STANDALONE */
#include "xlock.h"		/* in xlockmore distribution */
#define DO_STIPPLE
#endif /* STANDALONE */
#include "automata.h"

#ifdef MODE_demon

/*-
 * neighbors of 0 randomizes it between 3, 4, 5, 6, 7, 8, 9, and 12.
 */
#define DEF_NEIGHBORS  "0"      /* choose random value */
#define DEF_VERTICAL "False"
#define DEF_COLORS_ONLY "False"

static int  neighbors;
static Bool vertical;
static Bool colorsOnly;

static XrmOptionDescRec opts[] =
{
	{(char *) "-neighbors", (char *) ".demon.neighbors", XrmoptionSepArg, (caddr_t) NULL},
	{(char *) "-vertical", (char *) ".demon.vertical", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+vertical", (char *) ".demon.vertical", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-colorsonly", (char *) ".demon.colorsonly", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+colorsonly", (char *) ".demon.colorsonly", XrmoptionNoArg, (caddr_t) "off"}

};
static argtype vars[] =
{
	{(void *) & neighbors, (char *) "neighbors", (char *) "Neighbors", (char *) DEF_NEIGHBORS, t_Int},
	{(void *) & vertical, (char *) "vertical", (char *) "Vertical", (char *) DEF_VERTICAL, t_Bool},
	{(void *) & colorsOnly, (char *) "colorsonly", (char *) "ColorsOnly", (char *) DEF_COLORS_ONLY, t_Bool}
};
static OptionStruct desc[] =
{
	{(char *) "-neighbors num", (char *) "squares 4 or 8, hexagons 6, triangles 3, 9 or 12, pentagons 5 or 7"},
	{(char *) "-/+vertical", (char *) "change orientation for hexagons and triangles"},
	{(char *) "-/+colorsonly", (char *) "no black cells"}
};

ENTRYPOINT ModeSpecOpt demon_opts =
{sizeof opts / sizeof opts[0], opts, sizeof vars / sizeof vars[0], vars, desc};

#ifdef USE_MODULES
ModStruct   demon_description =
{"demon", "init_demon", "draw_demon", "release_demon",
 "refresh_demon", "init_demon", (char *) NULL, &demon_opts,
 50000, 0, 1000, -7, 64, 1.0, "",
 "Shows Griffeath's cellular automata", 0, NULL};

#endif

#define DEMONBITS(n,w,h)\
  if ((dp->pixmaps[dp->init_bits]=\
  XCreatePixmapFromBitmapData(display,window,(char *)n,w,h,1,0,1))==None){\
  free_demon_screen(display,dp); return;} else {dp->init_bits++;}

#define REDRAWSTEP 2000		/* How many cells to draw per cycle */
#define MINSTATES 2
#define MINGRIDSIZE 24
#define MINSIZE 4
#define NEIGHBORKINDS 8

/* Singly linked list */
typedef struct _CellList {
	XPoint      pt;
	struct _CellList *next;
} CellList;

typedef struct {
	Bool        vertical;
	Bool        colorsOnly;
	int         generation;
	int         xs, ys;
	int         xb, yb;
	int         nrows, ncols;
	int         width, height;
	int         states;
	int         state;
	int         redrawing, redrawpos;
	int        *ncells;
	CellList  **cellList;
	unsigned char *oldcell, *newcell;
	int         neighbors, polygon;
	int         init_bits;
	GC          stippledGC;
	Pixmap      pixmaps[NUMSTIPPLES - 1];
	union {
		XPoint      hexagon[7];
		XPoint      triangle[2][4];
		XPoint      pentagon[4][6];
	} shape;
} demonstruct;

static char plots[2][NEIGHBORKINDS] =
{
	{3, 4, 5, 6, 7, 8, 9, 12},	/* Neighborhoods */
	{12, 16, 17, 18, 19, 20, 22, 24}	/* Number of states */
};

static demonstruct *demons = (demonstruct *) NULL;

static void
drawCell(ModeInfo * mi, int col, int row, unsigned char state)
{
	demonstruct *dp = &demons[MI_SCREEN(mi)];
	Display *display = MI_DISPLAY(mi);
	Window window = MI_WINDOW(mi);
	GC          gc;

	if (!state && !dp->colorsOnly) {
		XSetForeground(display, MI_GC(mi), MI_BLACK_PIXEL(mi));
		gc = MI_GC(mi);
	} else if (MI_NPIXELS(mi) >= NUMSTIPPLES) {
		int offset = (dp->colorsOnly) ? 0 : 1;

		XSetForeground(display, MI_GC(mi),
			MI_PIXEL(mi, (((int) state - offset) * MI_NPIXELS(mi) /
			(dp->states - offset)) % MI_NPIXELS(mi)));
		gc = MI_GC(mi);
	} else if (!state) {
		XSetForeground(display, MI_GC(mi), MI_BLACK_PIXEL(mi));
		gc = MI_GC(mi);
	} else {
		XGCValues   gcv;

		gcv.stipple = dp->pixmaps[(state - 1) % (NUMSTIPPLES - 1)];
		gcv.foreground = MI_WHITE_PIXEL(mi);
		gcv.background = MI_BLACK_PIXEL(mi);
		XChangeGC(display, dp->stippledGC,
			  GCStipple | GCForeground | GCBackground, &gcv);
		gc = dp->stippledGC;
	}
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
		else
			XFillPolygon(display, window, gc,
				dp->shape.hexagon, 6,
				Convex, CoordModePrevious);
	} else if (dp->neighbors == 4 || dp->neighbors == 8) {
		XFillRectangle(display, window, gc,
			dp->xb + dp->xs * col, dp->yb + dp->ys * row,
			dp->xs - (dp->xs > 3), dp->ys - (dp->ys > 3));
	} else if (dp->neighbors == 5 || dp->neighbors == 7) {
		int map[4] = {2, 0, 1, 3};
		int orient = ((row & 1) * 2 + col) % 4;
		int offsetX = 0, offsetY = 0;
		switch (orient) {
		case 0: /* up */
			offsetX = dp->xs;
			break;
		case 1: /* down */
			offsetY = 3 * dp->ys / 2 - 1;
			break;
		case 2: /* left */
			offsetX = -dp->xs / 2;
			offsetY = 3 * dp->ys / 4;
			break;
		case 3: /* right */
			offsetX = 3 * dp->xs / 2 - 1;
			offsetY = 3 * dp->ys / 4;
			break;
		default:
			(void) printf("wrong orient %d\n", orient);
		}
		orient = map[orient];
		dp->shape.pentagon[orient][0].x =  dp->xb +
			col * dp->xs + offsetX;
		dp->shape.pentagon[orient][0].y = dp->yb +
			row * dp->ys + offsetY;
		if (dp->xs <= 2 || dp->ys <= 2)
			XDrawPoint(display, window, gc,
				dp->shape.pentagon[orient][0].x,
				dp->shape.pentagon[orient][0].y);
		else
			XFillPolygon(display, window, gc,
				dp->shape.pentagon[orient], 5, Convex, CoordModePrevious);
	} else {		/* TRI */
		int orient = (col + row) % 2;	/* O left 1 right */
		Bool small = (dp->xs <= 3 || dp->ys <= 3);

		if (dp->vertical) {
			dp->shape.triangle[orient][0].x = dp->xb + col * dp->xs;
			dp->shape.triangle[orient][0].y = dp->yb + row * dp->ys;
			if (small)
				dp->shape.triangle[orient][0].x +=
					((orient) ? -1 : 1);
			else
				dp->shape.triangle[orient][0].x +=
					(dp->xs / 2  - 1) * ((orient) ? 1 : -1);
		} else {
			dp->shape.triangle[orient][0].y = dp->xb + col * dp->xs;
			dp->shape.triangle[orient][0].x = dp->yb + row * dp->ys;
			if (small)
				dp->shape.triangle[orient][0].y +=
					((orient) ? -1 : 1);
			else
				dp->shape.triangle[orient][0].y +=
					(dp->xs / 2  - 1) * ((orient) ? 1 : -1);
		}
		if (small)
			XDrawPoint(display, window, gc,
				dp->shape.triangle[orient][0].x,
				dp->shape.triangle[orient][0].y);
		else {
			XFillPolygon(display, window, gc,
				dp->shape.triangle[orient], 3,
				Convex, CoordModePrevious);
			XSetForeground(display, gc, MI_BLACK_PIXEL(mi));
			XDrawLines(display, window, gc,
				dp->shape.triangle[orient], 4,
				CoordModePrevious);
		}
	}
}

static Bool
addtolist(ModeInfo * mi, int col, int row, unsigned char state)
{
	demonstruct *dp = &demons[MI_SCREEN(mi)];
	CellList   *current;

	current = dp->cellList[state];
	if ((dp->cellList[state] = (CellList *)
		malloc(sizeof (CellList))) == NULL) {
		return False;
	}
	dp->cellList[state]->pt.x = col;
	dp->cellList[state]->pt.y = row;
	dp->cellList[state]->next = current;
	dp->ncells[state]++;
	return True;
}

#ifdef DEBUG
static void
print_state(ModeInfo * mi, int state)
{
	demonstruct *dp = &demons[MI_SCREEN(mi)];
	CellList   *locallist;
	int         i = 0;

	locallist = dp->cellList[state];
	(void) printf("state %d\n", state);
	while (locallist) {
		(void) printf("%d	x %d, y %d\n", i,
			      locallist->pt.x, locallist->pt.y);
		locallist = locallist->next;
		i++;
	}
}

#endif

static void
free_state(demonstruct * dp, int state)
{
	CellList   *current;

	while (dp->cellList[state]) {
		current = dp->cellList[state];
		dp->cellList[state] = dp->cellList[state]->next;
		free(current);
	}
	dp->cellList[state] = (CellList *) NULL;
	if (dp->ncells != NULL)
		dp->ncells[state] = 0;
}


static void
free_list(demonstruct * dp)
{
	int         state;

	for (state = 0; state < dp->states; state++)
		free_state(dp, state);
	free(dp->cellList);
	dp->cellList = (CellList **) NULL;
}

static void
free_struct(demonstruct * dp)
{
	if (dp->cellList != NULL) {
		free_list(dp);
	}
	if (dp->ncells != NULL) {
		free(dp->ncells);
		dp->ncells = (int *) NULL;
	}
	if (dp->oldcell != NULL) {
		free(dp->oldcell);
		dp->oldcell = (unsigned char *) NULL;
	}
	if (dp->newcell != NULL) {
		free(dp->newcell);
		dp->newcell = (unsigned char *) NULL;
	}
}

static void
free_demon_screen(Display *display, demonstruct *dp)
{
	int         shade;

	if (dp == NULL) {
		return;
	}
	if (dp->stippledGC != None) {
		XFreeGC(display, dp->stippledGC);
		dp->stippledGC = None;
	}
	for (shade = 0; shade < dp->init_bits; shade++) {
		XFreePixmap(display, dp->pixmaps[shade]);
	}
	dp->init_bits = 0;
	free_struct(dp);
	dp = NULL;
}

static Bool
draw_state(ModeInfo * mi, int state)
{
	demonstruct *dp = &demons[MI_SCREEN(mi)];
	Display *display = MI_DISPLAY(mi);
	Window window = MI_WINDOW(mi);
	GC          gc;
	XRectangle *rects;
	CellList   *current;

	if (!state && !dp->colorsOnly) {
		XSetForeground(display, MI_GC(mi), MI_BLACK_PIXEL(mi));
		gc = MI_GC(mi);
	} else if (MI_NPIXELS(mi) >= NUMSTIPPLES) {
		int offset = (dp->colorsOnly) ? 0 : 1;

		XSetForeground(display, MI_GC(mi),
			   MI_PIXEL(mi, (((int) state - offset) * MI_NPIXELS(mi) /
					 (dp->states - offset)) % MI_NPIXELS(mi)));
		gc = MI_GC(mi);
	} else if (!state) {
		XSetForeground(display, MI_GC(mi), MI_BLACK_PIXEL(mi));
		gc = MI_GC(mi);
	} else {
		XGCValues   gcv;

		gcv.stipple = dp->pixmaps[(state - 1) % (NUMSTIPPLES - 1)];
		gcv.foreground = MI_WHITE_PIXEL(mi);
		gcv.background = MI_BLACK_PIXEL(mi);
		XChangeGC(display, dp->stippledGC,
			  GCStipple | GCForeground | GCBackground, &gcv);
		gc = dp->stippledGC;
	}
	if (dp->neighbors == 6) {	/* Draw right away, slow */
		current = dp->cellList[state];
		while (current) {
			int col = current->pt.x;
			int row = current->pt.y;
			int ccol = 2 * col + !(row & 1);
			int crow = 2 * row;

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
			else
				XFillPolygon(display, window, gc,
					dp->shape.hexagon, 6,
					Convex, CoordModePrevious);
			current = current->next;
		}
	} else if (dp->neighbors == 4 || dp->neighbors == 8) {
		/* Take advantage of XDrawRectangles */
		int         ncells = 0;

		/* Create Rectangle list from part of the cellList */
		if ((rects = (XRectangle *) malloc(dp->ncells[state] *
			 sizeof (XRectangle))) == NULL) {
			return False;
		}
		current = dp->cellList[state];
		while (current) {
			rects[ncells].x = dp->xb + current->pt.x * dp->xs;
			rects[ncells].y = dp->yb + current->pt.y * dp->ys;
			rects[ncells].width = dp->xs - (dp->xs > 3);
			rects[ncells].height = dp->ys - (dp->ys > 3);
			current = current->next;
			ncells++;
		}
		/* Finally get to draw */
		XFillRectangles(display, window, gc, rects, ncells);
		/* Free up rects list and the appropriate part of the cellList */
		free(rects);
	} else if (dp->neighbors == 5 || dp->neighbors == 7) {
		int map[4] = {2, 0, 1, 3};
		current = dp->cellList[state];
		while (current) {
			int col = current->pt.x;
			int row = current->pt.y;
			int orient = ((row & 1) * 2 + col) % 4;
			int offsetX = 0, offsetY = 0;

			switch (orient) {
			case 0: /* up */
				offsetX = dp->xs;
				break;
			case 1: /* down */
				offsetY = 3 * dp->ys / 2 - 1;
				break;
			case 2: /* left */
				offsetX = -dp->xs / 2;
				offsetY = 3 * dp->ys / 4;
				break;
			case 3: /* right */
				offsetX = 3 * dp->xs / 2 - 1;
				offsetY = 3 * dp->ys / 4;
				break;
			default:
				(void) printf("wrong orient %d\n", orient);
			}
			orient = map[orient];
			dp->shape.pentagon[orient][0].x =  dp->xb +
				col * dp->xs + offsetX;
			dp->shape.pentagon[orient][0].y = dp->yb +
				row * dp->ys + offsetY;
			if (dp->xs <= 2 || dp->ys <= 2)
				XDrawPoint(display, window, gc,
					dp->shape.pentagon[orient][0].x,
					dp->shape.pentagon[orient][0].y);
			else
				XFillPolygon(display, window, gc,
					dp->shape.pentagon[orient], 5,
					Convex, CoordModePrevious);
			current = current->next;
		}
	} else {		/* TRI */
		XGCValues values;
		GC borderGC = XCreateGC(display, window, 0, &values);
		Bool small = (dp->xs <= 3 || dp->ys <= 3);

		current = dp->cellList[state];
		while (current) {
			int         col, row, orient;

			col = current->pt.x;
			row = current->pt.y;
			orient = (col + row) % 2;	/* O left 1 right */
			if (dp->vertical) {
				dp->shape.triangle[orient][0].x = dp->xb + col * dp->xs;
				dp->shape.triangle[orient][0].y = dp->yb + row * dp->ys;
				if (small)
					dp->shape.triangle[orient][0].x +=
						((orient) ? -1 : 1);
				else
					dp->shape.triangle[orient][0].x +=
						(dp->xs / 2  - 1) * ((orient) ? 1 : -1);
			} else {
				dp->shape.triangle[orient][0].y = dp->xb + col * dp->xs;
				dp->shape.triangle[orient][0].x = dp->yb + row * dp->ys;
				if (small)
					dp->shape.triangle[orient][0].y +=
						((orient) ? -1 : 1);
				else
					dp->shape.triangle[orient][0].y +=
						(dp->xs / 2  - 1) * ((orient) ? 1 : -1);
			}
			if (small)
				XDrawPoint(display, window, gc,
					dp->shape.triangle[orient][0].x,
					dp->shape.triangle[orient][0].y);
			else {
				XFillPolygon(display, window, gc,
					dp->shape.triangle[orient], 3,
					Convex, CoordModePrevious);
				if (borderGC != None) {
					XSetForeground(display, borderGC, MI_BLACK_PIXEL(mi));
					XDrawLines(display, window, borderGC,
						dp->shape.triangle[orient], 4,
						CoordModePrevious);
				}
			}
			current = current->next;
		}
		if (borderGC != None)
			XFreeGC(display, borderGC);
	}
	free_state(dp, state);
	XFlush(display);
	return True;
}

static void
RandomSoup(ModeInfo * mi)
{
	demonstruct *dp = &demons[MI_SCREEN(mi)];
	int         row, col, mrow = 0;

	for (row = 0; row < dp->nrows; ++row) {
		for (col = 0; col < dp->ncols; ++col) {
			dp->oldcell[col + mrow] =
				(unsigned char) LRAND() % ((unsigned char) dp->states);
			if (!addtolist(mi, col, row, dp->oldcell[col + mrow]))
				return; /* sparse soup */
		}
		mrow += dp->ncols;
	}
}

static int
positionOfNeighbor(demonstruct * dp, int n, int col, int row)
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
	}
	return (row * dp->ncols + col);
}

ENTRYPOINT void
init_demon(ModeInfo * mi)
{
	Display    *display = MI_DISPLAY(mi);
	Window      window = MI_WINDOW(mi);
	int         size = MI_SIZE(mi), nk;
	demonstruct *dp;

	MI_INIT(mi, demons);
	dp = &demons[MI_SCREEN(mi)];

	dp->generation = 0;
	dp->redrawing = 0;
	if (MI_NPIXELS(mi) < NUMSTIPPLES) {
		if (dp->stippledGC == None) {
			XGCValues   gcv;

			gcv.fill_style = FillOpaqueStippled;
			if ((dp->stippledGC = XCreateGC(display, window,
				 GCFillStyle, &gcv)) == None) {
				free_demon_screen(display, dp);
				return;
			}
		}
		if (dp->init_bits == 0) {
			int         i;

			for (i = 1; i < NUMSTIPPLES; i++) {
				DEMONBITS(stipples[i], STIPPLESIZE, STIPPLESIZE);
			}
		}
	}
	free_struct(dp);

	for (nk = 0; nk < NEIGHBORKINDS; nk++) {
		if (neighbors == plots[0][nk]) {
			dp->neighbors = plots[0][nk];
			break;
		}
		if (nk == NEIGHBORKINDS - 1) {
			nk = NRAND(NEIGHBORKINDS);
			dp->neighbors = plots[0][nk];
			break;
		}
	}

	dp->states = MI_COUNT(mi);
	if (dp->states < -MINSTATES)
		dp->states = NRAND(-dp->states - MINSTATES + 1) + MINSTATES;
	else if (dp->states < MINSTATES)
		dp->states = plots[1][nk];
	if ((dp->cellList = (CellList **) calloc(dp->states,
		sizeof (CellList *))) == NULL) {
		free_demon_screen(display, dp);
		return;
	}
	if ((dp->ncells = (int *) calloc(dp->states, sizeof (int))) == NULL) {
		free_demon_screen(display, dp);
		return;
	}

	dp->state = 0;
	if (MI_IS_FULLRANDOM(mi)) {
		dp->vertical = (Bool) (LRAND() & 1);
		dp->colorsOnly = (Bool) (LRAND() & 1);
	} else {
		dp->vertical = vertical;
		dp->colorsOnly = colorsOnly;
	}
	dp->width = MI_WIDTH(mi);
	dp->height = MI_HEIGHT(mi);
	if (dp->neighbors == 6) {
		int nccols, ncrows, side;

		dp->polygon = 6;
		if (!dp->vertical) {
			dp->height = MI_WIDTH(mi);
			dp->width = MI_HEIGHT(mi);
		}
		if (dp->width < 8)
			dp->width = 8;
		if (dp->height < 8)
			dp->height = 8;
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
		nccols = MAX(dp->width / dp->xs - 2, 2);
		ncrows = MAX(dp->height / dp->ys - 1, 4);
		dp->ncols = nccols / 2;
		dp->nrows = 2 * (ncrows / 4);
		dp->xb = (dp->width - dp->xs * nccols) / 2 + dp->xs / 2;
		dp->yb = (dp->height - dp->ys * (ncrows / 2) * 2) / 2 +
			dp->ys / 3;
		for (side = 0; side < 6; side++) {
			if (dp->vertical) {
				dp->shape.hexagon[side].x =
					(dp->xs - 1) * hexagonUnit[side].x;
				dp->shape.hexagon[side].y =
					((dp->ys - 1) * hexagonUnit[side].y /
					2) * 4 / 3;
			} else {
				dp->shape.hexagon[side].y =
					(dp->xs - 1) * hexagonUnit[side].x;
				dp->shape.hexagon[side].x =
					((dp->ys - 1) * hexagonUnit[side].y /
					2) * 4 / 3;
			}
		}
	} else if (dp->neighbors == 4 || dp->neighbors == 8) {
		dp->polygon = 4;
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
		dp->ncols = MAX(dp->width / dp->xs, 2);
		dp->nrows = MAX(dp->height / dp->ys, 2);
		dp->xb = (dp->width - dp->xs * dp->ncols) / 2;
		dp->yb = (dp->height - dp->ys * dp->nrows) / 2;
	} else if (dp->neighbors == 5 || dp->neighbors == 7) {
				int orient, side;
		dp->polygon = 5;
		if (dp->width < 2)
			dp->width = 2;
		if (dp->height < 2)
			dp->height = 2;
		if (size < -MINSIZE)
			dp->xs = NRAND(MIN(-size, MAX(MINSIZE, MIN(dp->width, dp->height) /
				MINGRIDSIZE)) - MINSIZE + 1) + MINSIZE;
		else if (size < MINSIZE) {
			if (!size) {
				int min = MIN(dp->width, dp->height) / (4 * MINGRIDSIZE);
				int max = MIN(dp->width, dp->height) / (MINGRIDSIZE);

				dp->xs = MAX(MINSIZE, min + NRAND(max - min + 1));
			} else
				dp->xs = MINSIZE;
		} else
			dp->xs = MIN(size, MAX(MINSIZE, MIN(dp->width, dp->height) /
				MINGRIDSIZE));
		dp->ys = dp->xs * 2;
		dp->ncols = (MAX((dp->width - 4) / dp->xs, 8) / 4) * 4;
		dp->nrows = (MAX((dp->height - dp->ys / 2) / dp->ys, 8)) / 2 * 2;
		dp->xb = (dp->width - dp->xs * dp->ncols) / 2;
		dp->yb = (dp->height - dp->ys * dp->nrows) / 2 - 2;
		for (orient = 0; orient < 4; orient++) {
			for (side = 0; side < 5; side++) {
				dp->shape.pentagon[orient][side].x =
					2 * (dp->xs - 1) * pentagonUnit[orient][side].x / 4;
				dp->shape.pentagon[orient][side].y =
					(dp->ys - 1) * pentagonUnit[orient][side].y / 4;
			}
		}
	} else {		/* TRI */
		int orient, side;

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
		dp->ncols = (MAX(dp->width / dp->xs - 1, 2) / 2) * 2;
		dp->nrows = (MAX(dp->height / dp->ys - 1, 2) / 2) * 2;
		dp->xb = (dp->width - dp->xs * dp->ncols) / 2 + dp->xs / 2;
		dp->yb = (dp->height - dp->ys * dp->nrows) / 2 + dp->ys / 2;
		for (orient = 0; orient < 2; orient++) {
			for (side = 0; side < 3; side++) {
				if (dp->vertical) {
					dp->shape.triangle[orient][side].x =
						(dp->xs - 2) * triangleUnit[orient][side].x;
					dp->shape.triangle[orient][side].y =
						(dp->ys - 2) * triangleUnit[orient][side].y;
				} else {
					dp->shape.triangle[orient][side].y =
						(dp->xs - 2) * triangleUnit[orient][side].x;
					dp->shape.triangle[orient][side].x =
						(dp->ys - 2) * triangleUnit[orient][side].y;
				}
			}
		}
	}

	MI_CLEARWINDOW(mi);

	if ((dp->oldcell = (unsigned char *)
		malloc(dp->ncols * dp->nrows * sizeof (unsigned char))) == NULL) {
		free_demon_screen(display, dp);
		return;
	}

	if ((dp->newcell = (unsigned char *)
		malloc(dp->ncols * dp->nrows * sizeof (unsigned char))) == NULL) {
		free_demon_screen(display, dp);
		return;
	}

	RandomSoup(mi);
}

ENTRYPOINT void
draw_demon(ModeInfo * mi)
{
	int         i, j, k, l, n, mj = 0;
	Bool        change = False;
	demonstruct *dp;

	if (demons == NULL)
		return;
	dp = &demons[MI_SCREEN(mi)];
	if (dp->cellList == NULL)
		return;

	MI_IS_DRAWN(mi) = True;
	if (dp->state >= dp->states) {
		(void) memcpy((char *) dp->newcell, (char *) dp->oldcell,
			      dp->ncols * dp->nrows * sizeof (unsigned char));

		for (j = 0; j < dp->nrows; j++) {
			mj =  dp->ncols * j;
			for (i = 0; i < dp->ncols; i++) {
				for (n = 0; n < dp->neighbors; n++) {
					l = positionOfNeighbor(dp, n, i, j);
					k = i + mj;
					if (dp->oldcell[l] ==
						    (int) (dp->oldcell[k] + 1) % dp->states)
						dp->newcell[k] = dp->oldcell[l];
				}
			}
		}
		mj = 0;
		for (j = 0; j < dp->nrows; j++) {
			for (i = 0; i < dp->ncols; i++)
				if (dp->oldcell[i + mj] != dp->newcell[i + mj]) {
					dp->oldcell[i + mj] = dp->newcell[i + mj];
					if (!addtolist(mi, i, j, dp->oldcell[i + mj])) {
						free_demon_screen(MI_DISPLAY(mi), dp);
						return;
					}
					change = True;
				}
			mj += dp->ncols;
		}
		if (++dp->generation > MI_CYCLES(mi) || !change)
			init_demon(mi);
		dp->state = 0;
	} else {
		if (dp->ncells[dp->state])
			if (!draw_state(mi, dp->state)) {
				free_demon_screen(MI_DISPLAY(mi), dp);
				return;
			}
		dp->state++;
	}
	if (dp->redrawing) {
		for (i = 0; i < REDRAWSTEP; i++) {
			if (dp->oldcell[dp->redrawpos]) {
				drawCell(mi, dp->redrawpos % dp->ncols, dp->redrawpos / dp->ncols,
					 dp->oldcell[dp->redrawpos]);
			}
			if (++(dp->redrawpos) >= dp->ncols * dp->nrows) {
				dp->redrawing = 0;
				break;
			}
		}
	}
}

ENTRYPOINT void
release_demon(ModeInfo * mi)
{
	if (demons != NULL) {
		int         screen;

		for (screen = 0; screen < MI_NUM_SCREENS(mi); screen++)
			free_demon_screen(MI_DISPLAY(mi), &demons[screen]);
		free(demons);
		demons = (demonstruct *) NULL;
	}
}

#ifndef STANDALONE
ENTRYPOINT void
refresh_demon(ModeInfo * mi)
{
	demonstruct *dp;

	if (demons == NULL)
		return;
	dp = &demons[MI_SCREEN(mi)];

	dp->redrawing = 1;
	dp->redrawpos = 0;
}
#endif

XSCREENSAVER_MODULE ("Demon", demon)

#endif /* MODE_demon */
