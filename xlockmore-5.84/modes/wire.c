/* -*- Mode: C; tab-width: 4 -*- */
/* wire --- logical circuits based on simple state-changes (wireworld) */

#if 0
static const char sccsid[] = "@(#)wire.c	5.24 2007/01/18 xlockmore";

#endif

/*-
 * Copyright (c) 1996 by David Bagley.
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
 * 05-Dec-1997: neighbors option added.
 * 10-May-1997: Compatible with xscreensaver
 * 14-Jun-1996: Coded from A.K. Dewdney's "Computer Recreations", Scientific
 *              American Magazine" Jan 1990 pp 146-148.  Used ant.c as an
 *              example.  do_gen() based on code by Kevin Dahlhausen
 *              <ap096@po.cwru.edu> and Stefan Strack
 *              <stst@vuse.vanderbilt.edu>.
 */

/*-
 *  # Rules file for Wireworld
 *
 *  # 0 is space, 1 is wire, 2 is tail, 3 is head
 *  states 4
 *
 *  passive 2
 *  0[0123][0123][0123][0123]0	# No way to make a space into signal or wire
 *
 *  # Signal propagation
 *  2[0123][0123][0123][0123]1	# tail -> wire
 *  3[0123][0123][0123][0123]2	# head -> tail
 *
 *  # 1 or 2 heads adjacent to a wire makes it a head
 *  1(3*1)3			# wire with 1 head adjacent -> head
 *  1(3*2)3			# wire with 2 heads adjacent -> head
 *
 *
 *  " " is space, X is wire, o is tail, O is head
 *
 *   ->XXXoOXXX-> Electron moving in a wire
 *
 * Diode -- permits an electron to pass right to left, but not the other way.
 *    XX
 *  XX XXX
 *    XX
 *
 * OR gate or fan-in where inputs are protected by diodes
 *           XX     XX
 * Input ->XXX XX XX XXX<- Input
 *           XX  X  XX
 *               X
 *               X
 *               |
 *               V
 *             Output
 *
 * Dewdney's synchronous-logic flipflop.
 * Bottom left input is write 1, top left is remember 0.
 * When gate is on, 1 electron goes to output at right every 13 cycles
 * memory element, about to forget 1 and remember 0
 *         Remember 0
 *             o
 *            X O XX XX   Memory Loop
 *            X  XX X XXXX
 *            X   XX XX   X oOX-> 1 Output
 * Inputs ->XX        X    X
 *                X   X XX o
 * Inputs ->XXXXXXX XX X XO Memory of 1
 *                XX    XX
 *         Remember 1
 */

#ifdef STANDALONE
#define MODE_wire
#define DEFAULTS "*delay: 500000 \n" \
	"*count: 1000 \n" \
	"*cycles: 150 \n" \
	"*size: -8 \n" \
	"*ncolors: 64 \n" \

# define free_wire 0
# define reshape_wire 0
# define wire_handle_event 0
#include "xlockmore.h"		/* in xscreensaver distribution */
#else /* STANDALONE */
#include "xlock.h"		/* in xlockmore distribution */
#define DO_STIPPLE
#endif /* STANDALONE */
#include "automata.h"

#ifdef MODE_wire

/*-
 * neighbors of 0 randomizes it between 3, 4, 6, 8, 9, and 12.
 */
#define DEF_NEIGHBORS  "0"      /* choose random value */
#define DEF_VERTICAL "False"

static int  neighbors;
static Bool vertical;

static XrmOptionDescRec opts[] =
{
	{(char *) "-neighbors", (char *) ".wire.neighbors", XrmoptionSepArg, (caddr_t) NULL},
	{(char *) "-vertical", (char *) ".wire.vertical", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+vertical", (char *) ".wire.vertical", XrmoptionNoArg, (caddr_t) "off"}
};

static argtype vars[] =
{
	{(void *) & neighbors, (char *) "neighbors", (char *) "Neighbors", (char *) DEF_NEIGHBORS, t_Int},
	{(void *) & vertical, (char *) "vertical", (char *) "Vertical", (char *) DEF_VERTICAL, t_Bool}
};
static OptionStruct desc[] =
{
	{(char *) "-neighbors num", (char *) "squares 4 or 8, hexagons 6, triangles 3, 9 or 12"},
	{(char *) "-/+vertical", (char *) "change orientation for hexagons and triangles"}
};

ENTRYPOINT ModeSpecOpt wire_opts =
{sizeof opts / sizeof opts[0], opts, sizeof vars / sizeof vars[0], vars, desc};

#ifdef USE_MODULES
ModStruct   wire_description =
{"wire", "init_wire", "draw_wire", "release_wire",
 "refresh_wire", "init_wire", (char *) NULL, &wire_opts,
 500000, 1000, 150, -8, 64, 1.0, "",
 "Shows a random circuit with 2 electrons", 0, NULL};

#endif

#define WIREBITS(n,w,h)\
  if ((wp->pixmaps[wp->init_bits]=\
  XCreatePixmapFromBitmapData(display,window,(char *)n,w,h,1,0,1))==None){\
  free_wire_screen(display,wp); return;} else {wp->init_bits++;}

#define COLORS 4
#define MINWIRES 32
#define MINGRIDSIZE 24
#define MINSIZE 3
#define ANGLES 360
#define NEIGHBORKINDS 6

#define SPACE 0
#define WIRE 1			/* Normal wire */
#define HEAD 2			/* electron head */
#define TAIL 3			/* electron tail */

#define REDRAWSTEP 2000		/* How much wire to draw per cycle */

/* Singly linked list */
typedef struct _CellList {
	XPoint      pt;
	struct _CellList *next;
} CellList;

typedef struct {
	Bool        vertical;
	int         init_bits;
	int         neighbors, polygon;
	int         generation;
	int         xs, ys;
	int         xb, yb;
	int         nrows, ncols;
	int         bnrows, bncols;
	int         mincol, minrow, maxcol, maxrow;
	int         width, height;
	int         redrawing, redrawpos;
	unsigned char *oldcells, *newcells;
	int         ncells[COLORS - 1];
	CellList   *cellList[COLORS - 1];
	unsigned char colors[COLORS - 1];
	GC          stippledGC;
	Pixmap      pixmaps[COLORS - 1];
	int         prob_array[12];
	union {
		XPoint      hexagon[6];
		XPoint      triangle[2][3];
	} shape;
} circuitstruct;

static char plots[NEIGHBORKINDS] =
{3, 4, 6, 8, 9, 12};		/* Neighborhoods */

static circuitstruct *circuits = (circuitstruct *) NULL;

static void
positionOfNeighbor(circuitstruct * wp, int dir, int *pcol, int *prow)
{
	int         col = *pcol, row = *prow;

	/* NO WRAPPING */

	if (wp->polygon == 4 || wp->polygon == 6) {
		switch (dir) {
			case 0:
				col = col + 1;
				break;
			case 45:
				col = col + 1;
				row = row - 1;
				break;
			case 60:
				if (!(row & 1))
					col = col + 1;
				row = row - 1;
				break;
			case 90:
				row = row - 1;
				break;
			case 120:
				if (row & 1)
					col = col - 1;
				row = row - 1;
				break;
			case 135:
				col = col - 1;
				row = row - 1;
				break;
			case 180:
				col = col - 1;
				break;
			case 225:
				col = col - 1;
				row = row + 1;
				break;
			case 240:
				if (row & 1)
					col = col - 1;
				row = row + 1;
				break;
			case 270:
				row = row + 1;
				break;
			case 300:
				if (!(row & 1))
					col = col + 1;
				row = row + 1;
				break;
			case 315:
				col = col + 1;
				row = row + 1;
				break;
			default:
				(void) fprintf(stderr, "wrong direction %d\n", dir);
		}
	} else if (wp->polygon == 3) {
		if ((col + row) & 1) {	/* right */
			switch (dir) {
				case 0:
					col = col - 1;
					break;
				case 30:
				case 40:
					col = col - 1;
					row = row - 1;
					break;
				case 60:
					col = col - 1;
					row = row - 2;
					break;
				case 80:
				case 90:
					row = row - 2;
					break;
				case 120:
					row = row - 1;
					break;
				case 150:
				case 160:
					col = col + 1;
					row = row - 1;
					break;
				case 180:
					col = col + 1;
					break;
				case 200:
				case 210:
					col = col + 1;
					row = row + 1;
					break;
				case 240:
					row = row + 1;
					break;
				case 270:
				case 280:
					row = row + 2;
					break;
				case 300:
					col = col - 1;
					row = row + 2;
					break;
				case 320:
				case 330:
					col = col - 1;
					row = row + 1;
					break;
				default:
					(void) fprintf(stderr, "wrong direction %d\n", dir);
			}
		} else {	/* left */
			switch (dir) {
				case 0:
					col = col + 1;
					break;
				case 30:
				case 40:
					col = col + 1;
					row = row + 1;
					break;
				case 60:
					col = col + 1;
					row = row + 2;
					break;
				case 80:
				case 90:
					row = row + 2;
					break;
				case 120:
					row = row + 1;
					break;
				case 150:
				case 160:
					col = col - 1;
					row = row + 1;
					break;
				case 180:
					col = col - 1;
					break;
				case 200:
				case 210:
					col = col - 1;
					row = row - 1;
					break;
				case 240:
					row = row - 1;
					break;
				case 270:
				case 280:
					row = row - 2;
					break;
				case 300:
					col = col + 1;
					row = row - 2;
					break;
				case 320:
				case 330:
					col = col + 1;
					row = row - 1;
					break;
				default:
					(void) fprintf(stderr, "wrong direction %d\n", dir);
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
				col = col + 2;
				break;
			case 102: /* 7 corner */
				col = col + 3;
				row = row - 1;
				break;
			case 144: /* 5 */
			case 153: /* 7 */
				col++;
				row = row - 1;
				break;
			case 204: /* 7 */
			case 216: /* 5 */
				row = row - 1;
				break;
			case 255: /* 7 */
				col = col - 1;
				row = row - 1;
				break;
			case 288: /* 5 */
			case 306: /* 7 */
				col = col - 1;
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
				col = col - 2;
				break;
			case 102: /* 7 */
				col = col - 3;
				row = row + 1;
				break;
			case 144: /* 5 */
			case 153: /* 7 */
				col--;
				row = row + 1;
				break;
			case 204: /* 7 */
			case 216: /* 5 */
				row = row + 1;
				break;
			case 255: /* 7 */
				col = col + 1;
				row = row + 1;
				break;
			case 288: /* 5 */
			case 306: /* 7 */
				col = col + 1;
				break;
			default:
				(void) fprintf(stderr, "wrong direction %d\n",
					dir);
			}
			break;
		case 2:
			switch (dir) {
			case 0:
				col = col + 1;
				break;
			case 51: /* 7 */
			case 72: /* 5 */
				row = row - 1;
				col++;
				break;
			case 102: /* 7 */
				col = col - 1;
				row = row - 1;
				break;
			case 144: /* 5 */
			case 153: /* 7 */
				col = col - 2;
				break;
			case 204: /* 7 */
			case 216: /* 5 */
				col = col - 1;
				break;
			case 255: /* 7 */
				row = row + 1;
				col = col - 1;
				break;
			case 288: /* 5 */
			case 306: /* 7 */
				row = row + 1;
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
				col = col - 1;
				row = row + 1;
				break;
			case 102: /* 7 */
				col = col + 1;
				row = row + 1;
				break;
			case 144: /* 5 */
			case 153: /* 7 */
				col = col + 2;
				break;
			case 204: /* 7 */
			case 216: /* 5 */
				col = col + 1;
				break;
			case 255: /* 7 */
				col = col + 1;
				row = row - 1;
				break;
			case 288: /* 5 */
			case 306: /* 7 */
				row = row - 1;
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
	*pcol = col;
	*prow = row;
}

static      Bool
withinBounds(circuitstruct * wp, int col, int row)
{
	return (row >= 2 && row < wp->bnrows - 2 &&
		col >= 2 && col < wp->bncols - 2 - (wp->polygon == 6 && !(row % 2)));
}

static void
fillcell(ModeInfo * mi, GC gc, int col, int row)
{
	circuitstruct *wp = &circuits[MI_SCREEN(mi)];

	if (wp->neighbors == 6) {
		int         ccol = 2 * col + !(row & 1), crow = 2 * row;

		if (wp->vertical) {
			wp->shape.hexagon[0].x = wp->xb + ccol * wp->xs;
			wp->shape.hexagon[0].y = wp->yb + crow * wp->ys;
		} else {
			wp->shape.hexagon[0].y = wp->xb + ccol * wp->xs;
			wp->shape.hexagon[0].x = wp->yb + crow * wp->ys;
		}
		if (wp->xs <= 3 || wp->ys <= 3)
			XDrawPoint(MI_DISPLAY(mi), MI_WINDOW(mi), gc,
				wp->shape.hexagon[0].x,
				wp->shape.hexagon[0].y);
		else
			XFillPolygon(MI_DISPLAY(mi), MI_WINDOW(mi), gc,
				wp->shape.hexagon, 6,
				Convex, CoordModePrevious);
	} else if (wp->neighbors == 4 || wp->neighbors == 8) {
		XFillRectangle(MI_DISPLAY(mi), MI_WINDOW(mi), gc,
			wp->xb + wp->xs * col, wp->yb + wp->ys * row,
			wp->xs - (wp->xs > 3), wp->ys - (wp->ys > 3));
	} else {		/* TRI */
		int orient = (col + row) % 2;	/* O left 1 right */
		Bool small = (wp->xs <= 3 || wp->ys <= 3);

		if (wp->vertical) {
			wp->shape.triangle[orient][0].x = wp->xb + col * wp->xs;
			wp->shape.triangle[orient][0].y = wp->yb + row * wp->ys;
			if (small)
				wp->shape.triangle[orient][0].x +=
					((orient) ? -1 : 1);
			else
				wp->shape.triangle[orient][0].x +=
					(wp->xs / 2  - 1) * ((orient) ? 1 : -1);
		} else {
			wp->shape.triangle[orient][0].y = wp->xb + col * wp->xs;
			wp->shape.triangle[orient][0].x = wp->yb + row * wp->ys;
			if (small)	
				wp->shape.triangle[orient][0].y +=
					((orient) ? -1 : 1);
			else
				wp->shape.triangle[orient][0].y +=
					(wp->xs / 2  - 1) * ((orient) ? 1 : -1);
		}
		if (small)
			XDrawPoint(MI_DISPLAY(mi), MI_WINDOW(mi), gc,
				wp->shape.triangle[orient][0].x,
				wp->shape.triangle[orient][0].y);
		else {
			XFillPolygon(MI_DISPLAY(mi), MI_WINDOW(mi), gc,
				wp->shape.triangle[orient], 3,
				Convex, CoordModePrevious);
		}
	}
}

static void
drawCell(ModeInfo * mi, int col, int row, unsigned char state)
{
	circuitstruct *wp = &circuits[MI_SCREEN(mi)];
	GC          gc;

	if (MI_NPIXELS(mi) > 2) {
		gc = MI_GC(mi);
		XSetForeground(MI_DISPLAY(mi), gc, MI_PIXEL(mi, wp->colors[state]));
	} else {
		XGCValues   gcv;

		gcv.stipple = wp->pixmaps[state];
		gcv.foreground = MI_WHITE_PIXEL(mi);
		gcv.background = MI_BLACK_PIXEL(mi);
		XChangeGC(MI_DISPLAY(mi), wp->stippledGC,
			  GCStipple | GCForeground | GCBackground, &gcv);
		gc = wp->stippledGC;
	}
	fillcell(mi, gc, col, row);
}

#if 0
static void
drawCell_notused(ModeInfo * mi, int col, int row, unsigned char state)
{
	circuitstruct *wp = &circuits[MI_SCREEN(mi)];
	XGCValues   gcv;
	GC          gc;

	if (MI_NPIXELS(mi) > 2) {
		gc = MI_GC(mi);
		XSetForeground(MI_DISPLAY(mi), gc, MI_PIXEL(mi, wp->colors[state]));
	} else {
		gcv.stipple = wp->pixmaps[state];
		gcv.foreground = MI_WHITE_PIXEL(mi);
		gcv.background = MI_BLACK_PIXEL(mi);
		XChangeGC(MI_DISPLAY(mi), wp->stippledGC,
			  GCStipple | GCForeground | GCBackground, &gcv);
		gc = wp->stippledGC;
	}
	XFillRectangle(MI_DISPLAY(mi), MI_WINDOW(mi), MI_GC(mi),
	       wp->xb + wp->xs * col, wp->yb + wp->ys * row,
		wp->xs - (wp->xs > 3), wp->ys - (wp->ys > 3));
}
#endif

static Bool
addtolist(ModeInfo * mi, int col, int row, unsigned char state)
{
	circuitstruct *wp = &circuits[MI_SCREEN(mi)];
	CellList   *current = wp->cellList[state];

	wp->cellList[state] = (CellList *) NULL;
	if ((wp->cellList[state] = (CellList *) malloc(sizeof (CellList))) ==
			NULL) {
		return False;
	}
	wp->cellList[state]->pt.x = col;
	wp->cellList[state]->pt.y = row;
	wp->cellList[state]->next = current;
	wp->ncells[state]++;
	return True;
}

#ifdef DEBUG
static void
print_state(ModeInfo * mi, int state)
{
	circuitstruct *wp = &circuits[MI_SCREEN(mi)];
	CellList   *locallist = wp->cellList[state];
	int         i = 0;

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
free_state(circuitstruct * wp, int state)
{
	CellList   *current;

	while (wp->cellList[state]) {
		current = wp->cellList[state];
		wp->cellList[state] = wp->cellList[state]->next;
		free(current);
	}
	wp->ncells[state] = 0;
}

static Bool
draw_state(ModeInfo * mi, int state)
{
	circuitstruct *wp = &circuits[MI_SCREEN(mi)];
	GC          gc;
	XGCValues   gcv;
	CellList   *current = wp->cellList[state];

	if (MI_NPIXELS(mi) > 2) {
		gc = MI_GC(mi);
		XSetForeground(MI_DISPLAY(mi), gc, MI_PIXEL(mi, wp->colors[state]));
	} else {
		gcv.stipple = wp->pixmaps[state];
		gcv.foreground = MI_WHITE_PIXEL(mi);
		gcv.background = MI_BLACK_PIXEL(mi);
		XChangeGC(MI_DISPLAY(mi), wp->stippledGC,
			  GCStipple | GCForeground | GCBackground, &gcv);
		gc = wp->stippledGC;
	}

	if (wp->neighbors == 6) {	/* Draw right away, slow */
		while (current) {
			int         col, row, ccol, crow;

			col = current->pt.x;
			row = current->pt.y;
			ccol = 2 * col + !(row & 1), crow = 2 * row;
			if (wp->vertical) {
				wp->shape.hexagon[0].x = wp->xb + ccol * wp->xs;
				wp->shape.hexagon[0].y = wp->yb + crow * wp->ys;
			} else {
				wp->shape.hexagon[0].y = wp->xb + ccol * wp->xs;
				wp->shape.hexagon[0].x = wp->yb + crow * wp->ys;
			}
			if (wp->xs <= 1 || wp->ys <= 1)
				XDrawPoint(MI_DISPLAY(mi), MI_WINDOW(mi), gc,
					wp->shape.hexagon[0].x,
					wp->shape.hexagon[0].y);
			else
				XFillPolygon(MI_DISPLAY(mi), MI_WINDOW(mi), gc,
					wp->shape.hexagon, 6,
					Convex, CoordModePrevious);
			current = current->next;
		}
	} else if (wp->neighbors == 4 || wp->neighbors == 8) {
		XRectangle *rects;
		/* Take advantage of XFillRectangles */
		int         nrects = 0;

		/* Create Rectangle list from part of the cellList */
		if ((rects = (XRectangle *) malloc(wp->ncells[state] *
				sizeof (XRectangle))) == NULL) {
			return False;
    	}

		while (current) {
			rects[nrects].x = wp->xb + current->pt.x * wp->xs;
			rects[nrects].y = wp->yb + current->pt.y * wp->ys;
			rects[nrects].width = wp->xs - (wp->xs > 3);
			rects[nrects].height = wp->ys - (wp->ys > 3);
			current = current->next;
			nrects++;
		}
		/* Finally get to draw */
		XFillRectangles(MI_DISPLAY(mi), MI_WINDOW(mi), gc, rects, nrects);
		/* Free up rects list and the appropriate part of the cellList */
		free(rects);
	} else {		/* TRI */
		while (current) {
			int col, row, orient;
			Bool small = (wp->xs <= 3 || wp->ys <= 3);

			col = current->pt.x;
			row = current->pt.y;
			orient = (col + row) % 2;	/* O left 1 right */
			if (wp->vertical) {
				wp->shape.triangle[orient][0].x = wp->xb + col * wp->xs;
				wp->shape.triangle[orient][0].y = wp->yb + row * wp->ys;
				if (small)
					wp->shape.triangle[orient][0].x +=
						((orient) ? -1 : 1);
				else
					wp->shape.triangle[orient][0].x +=
						(wp->xs / 2  - 1) * ((orient) ? 1 : -1);
			} else {
				wp->shape.triangle[orient][0].y = wp->xb + col * wp->xs;
				wp->shape.triangle[orient][0].x = wp->yb + row * wp->ys;
				if (small)	
					wp->shape.triangle[orient][0].y +=
						((orient) ? -1 : 1);
				else
					wp->shape.triangle[orient][0].y +=
						(wp->xs / 2  - 1) * ((orient) ? 1 : -1);
			}
			if (small)
				XDrawPoint(MI_DISPLAY(mi), MI_WINDOW(mi), gc,
					wp->shape.triangle[orient][0].x,
					wp->shape.triangle[orient][0].y);
			else {
				XFillPolygon(MI_DISPLAY(mi), MI_WINDOW(mi), gc,
					wp->shape.triangle[orient], 3,	
					Convex, CoordModePrevious);
			}
			current = current->next;
		}
	}
	free_state(wp, state);
	XFlush(MI_DISPLAY(mi));
	return True;
}

#if 0
static void
RandomSoup(circuitstruct * wp)
{
	int         i, j;

	for (j = 2; j < wp->bnrows - 2; j++)
		for (i = 2; i < wp->bncols - 2; i++) {
			*(wp->newcells + i + j * wp->bncols) =
				(NRAND(100) > wp->n) ? SPACE : (NRAND(4)) ? WIRE : (NRAND(2)) ?
				HEAD : TAIL;
		}
}

#endif

static void
create_path(circuitstruct * wp, int n)
{
	int         col, row;
	int         count = 0;
	int         dir, prob;
	int         nextcol, nextrow, i;

#ifdef RANDOMSTART
	/* Path usually "mushed" in a corner */
	col = NRAND(wp->ncols) + 1;
	row = NRAND(wp->nrows) + 1;
#else
	/* Start from center */
	col = wp->ncols / 2;
	row = wp->nrows / 2;
#endif
	wp->mincol = col - 1, wp->minrow = row - 2;
	wp->maxcol = col + 1, wp->maxrow = row + 2;
	dir = NRAND(wp->neighbors) * ANGLES / wp->neighbors;
	*(wp->newcells + col + row * wp->bncols) = HEAD;
	while (++count < n) {
		prob = NRAND(wp->prob_array[wp->neighbors - 1]);
		i = 0;
		while (prob > wp->prob_array[i])
			i++;
		dir = ((dir * wp->neighbors / ANGLES + i) %
		       wp->neighbors) * ANGLES / wp->neighbors;
		nextcol = col;
		nextrow = row;
		positionOfNeighbor(wp, dir, &nextcol, &nextrow);
		if (withinBounds(wp, nextcol, nextrow)) {
			col = nextcol;
			row = nextrow;
			if (col == wp->mincol && col > 2)
				wp->mincol--;
			if (row == wp->minrow && row > 2)
				wp->minrow--;
			else if (row == wp->minrow - 1 && row > 3)
				wp->minrow -= 2;
			if (col == wp->maxcol && col < wp->bncols - 3)
				wp->maxcol++;
			if (row == wp->maxrow && row < wp->bnrows - 3)
				wp->maxrow++;
			else if (row == wp->maxrow + 1 && row < wp->bnrows - 4)
				wp->maxrow += 2;

			if (!*(wp->newcells + col + row * wp->bncols))
				*(wp->newcells + col + row * wp->bncols) = WIRE;
		} else {
			if (wp->neighbors == 3)
				break;	/* There is no reverse step */
			dir = ((dir * wp->neighbors / ANGLES + wp->neighbors / 2) %
			       wp->neighbors) * ANGLES / wp->neighbors;
		}
	}
	*(wp->newcells + col + row * wp->bncols) = HEAD;
}

static void
do_gen(circuitstruct * wp)
{
	int         i, j, k;
	unsigned char *z;
	int         count;

#define LOC(X, Y) (*(wp->oldcells + (X) + ((Y) * wp->bncols)))
#define ADD(X, Y) if (LOC((X), (Y)) == HEAD) count++

	for (j = wp->minrow; j <= wp->maxrow; j++) {
		for (i = wp->mincol; i <= wp->maxcol; i++) {
			z = wp->newcells + i + j * wp->bncols;
			switch (LOC(i, j)) {
				case SPACE:
					*z = SPACE;
					break;
				case TAIL:
					*z = WIRE;
					break;
				case HEAD:
					*z = TAIL;
					break;
				case WIRE:
					count = 0;
					for (k = 0; k < wp->neighbors; k++) {
						int         newi = i, newj = j;

						positionOfNeighbor(wp, k * ANGLES / wp->neighbors, &newi, &newj);
						ADD(newi, newj);
					}
					if (count == 1 || count == 2)
						*z = HEAD;
					else
						*z = WIRE;
					break;
				default:
					{
						(void) fprintf(stderr,
							       "bad internal character %d at %d,%d\n",
						      (int) LOC(i, j), i, j);
					}
			}
		}
	}
}

static void
free_list(circuitstruct * wp)
{
	int         state;

	for (state = 0; state < COLORS - 1; state++)
		free_state(wp, state);
}

static void
free_wire_screen(Display *display, circuitstruct *wp)
{
	int         shade;

	if (wp == NULL) {
		return;
	}
	for (shade = 0; shade < wp->init_bits; shade++)
		XFreePixmap(display, wp->pixmaps[shade]);
	wp->init_bits = 0;
	if (wp->stippledGC != None) {
		XFreeGC(display, wp->stippledGC);
		wp->stippledGC = None;
	}
	if (wp->oldcells != NULL) {
		free(wp->oldcells);
		wp->oldcells = (unsigned char *) NULL;
	}
	if (wp->newcells != NULL) {
		free(wp->newcells);
		wp->newcells = (unsigned char *) NULL;
	}
	free_list(wp);
	wp = NULL;
}

ENTRYPOINT void
init_wire(ModeInfo * mi)
{
	Display    *display = MI_DISPLAY(mi);
	Window      window = MI_WINDOW(mi);
	int         i, size = MI_SIZE(mi), n;
	circuitstruct *wp;
	XGCValues   gcv;

	MI_INIT(mi, circuits);
	wp = &circuits[MI_SCREEN(mi)];

	wp->redrawing = 0;

	if ((MI_NPIXELS(mi) <= 2) && (wp->init_bits == 0)) {
		if (wp->stippledGC == None) {
			gcv.fill_style = FillOpaqueStippled;
			if ((wp->stippledGC = XCreateGC(display, window, GCFillStyle,
					&gcv)) == None) {
				free_wire_screen(display, wp);
				return;
			}
		}
		WIREBITS(stipples[NUMSTIPPLES - 1], STIPPLESIZE, STIPPLESIZE);
		WIREBITS(stipples[NUMSTIPPLES - 3], STIPPLESIZE, STIPPLESIZE);
		WIREBITS(stipples[2], STIPPLESIZE, STIPPLESIZE);
	}
	if (MI_NPIXELS(mi) > 2) {
		wp->colors[0] = (NRAND(MI_NPIXELS(mi)));
		wp->colors[1] = (wp->colors[0] + MI_NPIXELS(mi) / 6 +
			     NRAND(MI_NPIXELS(mi) / 4 + 1)) % MI_NPIXELS(mi);
		wp->colors[2] = (wp->colors[1] + MI_NPIXELS(mi) / 6 +
			     NRAND(MI_NPIXELS(mi) / 4 + 1)) % MI_NPIXELS(mi);
	}
	free_list(wp);
	wp->generation = 0;
	if (MI_IS_FULLRANDOM(mi)) {
		wp->vertical = (Bool) (LRAND() & 1);
	} else {
		wp->vertical = vertical;
	}
	wp->width = MI_WIDTH(mi);
	wp->height = MI_HEIGHT(mi);

	for (i = 0; i < NEIGHBORKINDS; i++) {
		if (neighbors == plots[i]) {
			wp->neighbors = plots[i];
			break;
		}
		if (i == NEIGHBORKINDS - 1) {
			i = NRAND(NEIGHBORKINDS - 3) + 1;	/* Skip triangular ones */
			wp->neighbors = plots[i];
			break;
		}
	}

	wp->prob_array[wp->neighbors - 1] = 100;
	if (wp->neighbors == 3) {
		wp->prob_array[1] = 67;
		wp->prob_array[0] = 33;
	} else {
		int         incr = 24 / wp->neighbors;

		for (i = wp->neighbors - 2; i >= 0; i--) {
			wp->prob_array[i] = wp->prob_array[i + 1] - incr -
				incr * ((i + 1) != wp->neighbors / 2);
		}
	}

	if (wp->neighbors == 6) {
		int  nccols, ncrows;

		wp->polygon = 6;
		if (!wp->vertical) {
			wp->height = MI_WIDTH(mi);
			wp->width = MI_HEIGHT(mi);
		}
		if (wp->width < 8)
			wp->width = 8;
		if (wp->height < 8)
			wp->height = 8;
		if (size < -MINSIZE)
			wp->ys = NRAND(MIN(-size, MAX(MINSIZE, MIN(wp->width, wp->height) /
				      MINGRIDSIZE)) - MINSIZE + 1) + MINSIZE;
		else if (size < MINSIZE) {
			if (!size)
				wp->ys = MAX(MINSIZE, MIN(wp->width, wp->height) / MINGRIDSIZE);
			else
				wp->ys = MINSIZE;
		} else
			wp->ys = MIN(size, MAX(MINSIZE, MIN(wp->width, wp->height) /
					       MINGRIDSIZE));
		wp->xs = wp->ys;
		nccols = MAX(wp->width / wp->xs - 2, 16);
		ncrows = MAX(wp->height / wp->ys - 1, 16);
		wp->ncols = nccols / 2;
		wp->nrows = ncrows / 2;
		wp->nrows -= !(wp->nrows & 1);	/* Must be odd */
		wp->xb = (wp->width - wp->xs * nccols) / 2 + wp->xs;
		wp->yb = (wp->height - wp->ys * ncrows) / 2 + wp->ys;
		for (i = 0; i < 6; i++) {
			if (wp->vertical) {
				wp->shape.hexagon[i].x =
					(wp->xs - 1) * hexagonUnit[i].x;
				wp->shape.hexagon[i].y =
					((wp->ys - 1) * hexagonUnit[i].y /
					2) * 4 / 3;
			} else {
				wp->shape.hexagon[i].y =
					(wp->xs - 1) * hexagonUnit[i].x;
				wp->shape.hexagon[i].x =
					((wp->ys - 1) * hexagonUnit[i].y /
					2) * 4 / 3;
			}
		}
	} else if (wp->neighbors == 4 || wp->neighbors == 8) {
		wp->polygon = 4;
		if (size < -MINSIZE)
			wp->ys = NRAND(MIN(-size, MAX(MINSIZE, MIN(wp->width, wp->height) /
				      MINGRIDSIZE)) - MINSIZE + 1) + MINSIZE;
		else if (size < MINSIZE) {
			if (!size)
				wp->ys = MAX(MINSIZE, MIN(wp->width, wp->height) / MINGRIDSIZE);
			else
				wp->ys = MINSIZE;
		} else
			wp->ys = MIN(size, MAX(MINSIZE, MIN(wp->width, wp->height) /
					       MINGRIDSIZE));
		wp->xs = wp->ys;
		wp->ncols = MAX(wp->width / wp->xs, 8);
		wp->nrows = MAX(wp->height / wp->ys, 8);
		wp->xb = (wp->width - wp->xs * wp->ncols) / 2;
		wp->yb = (wp->height - wp->ys * wp->nrows) / 2;
	} else {		/* TRI */
		int orient;

		wp->polygon = 3;
		if (!wp->vertical) {
			wp->height = MI_WIDTH(mi);
			wp->width = MI_HEIGHT(mi);
		}
		if (wp->width < 4)
			wp->width = 4;
		if (wp->height < 2)
			wp->height = 2;
		if (size < -MINSIZE)
			wp->ys = NRAND(MIN(-size, MAX(MINSIZE, MIN(wp->width, wp->height) /
				      MINGRIDSIZE)) - MINSIZE + 1) + MINSIZE;
		else if (size < MINSIZE) {
			if (!size)
				wp->ys = MAX(MINSIZE, MIN(wp->width, wp->height) / MINGRIDSIZE);
			else
				wp->ys = MINSIZE;
		} else
			wp->ys = MIN(size, MAX(MINSIZE, MIN(wp->width, wp->height) /
					       MINGRIDSIZE));
		wp->xs = (int) (1.52 * wp->ys);
		wp->ncols = (MAX(wp->width / wp->xs - 1, 8) / 2) * 2;
		wp->nrows = (MAX(wp->height / wp->ys - 1, 8) / 2) * 2 - 1;
		wp->xb = (wp->width - wp->xs * wp->ncols) / 2 + wp->xs / 2;
		wp->yb = (wp->height - wp->ys * wp->nrows) / 2 + wp->ys;
		for (orient = 0; orient < 2; orient++) {
			for (i = 0; i < 3; i++) {
				if (wp->vertical) {
					wp->shape.triangle[orient][i].x =
						(wp->xs - 2) * triangleUnit[orient][i].x;
					wp->shape.triangle[orient][i].y =
						(wp->ys - 2) * triangleUnit[orient][i].y;
				} else {
					wp->shape.triangle[orient][i].y =
						(wp->xs - 2) * triangleUnit[orient][i].x;
					wp->shape.triangle[orient][i].x =
						(wp->ys - 2) * triangleUnit[orient][i].y;
				}
			}
		}
	}

	/*
	 * I am being a bit naughty here wasting a little bit of memory
	 * but it will give me a real headache to figure out the logic
	 * and to refigure the mappings to save a few bytes
	 * ncols should only need a border of 2 and nrows should only need
	 * a border of 4 when in the neighbors = 9 or 12
	 */
	wp->bncols = wp->ncols + 4;
	wp->bnrows = wp->nrows + 4;

	if (MI_IS_VERBOSE(mi))
		(void) fprintf(stdout,
			"neighbors %d, ncols %d, nrows %d\n",
			wp->neighbors, wp->ncols, wp->nrows);
	MI_CLEARWINDOW(mi);

	if (wp->oldcells != NULL) {
		free(wp->oldcells);
		wp->oldcells = (unsigned char *) NULL;
	}
	if ((wp->oldcells = (unsigned char *) calloc(wp->bncols * wp->bnrows,
			sizeof (unsigned char))) == NULL) {
		free_wire_screen(display, wp);
		return;
	}

	if (wp->newcells != NULL) {
		free(wp->newcells);
		wp->newcells = (unsigned char *) NULL;
	}
	if ((wp->newcells = (unsigned char *) calloc(wp->bncols * wp->bnrows,
			sizeof (unsigned char))) == NULL) {
		free_wire_screen(display, wp);
		return;
	}

	n = MI_COUNT(mi);
	i = (1 + (wp->neighbors == 6)) * wp->ncols * wp->nrows / 4;
	if (n < -MINWIRES && i > MINWIRES) {
		n = NRAND(MIN(-n, i) - MINWIRES + 1) + MINWIRES;
	} else if (n < MINWIRES) {
		n = MINWIRES;
	} else if (n > i) {
		n = MAX(MINWIRES, i);
	}
	create_path(wp, n);
}

ENTRYPOINT void
draw_wire(ModeInfo * mi)
{
	int         offset, i, j, found = 0;
	unsigned char *z, *znew;
	circuitstruct *wp;

	if (circuits == NULL)
		return;
	wp = &circuits[MI_SCREEN(mi)];
	if (wp->newcells == NULL)
		return;

	MI_IS_DRAWN(mi) = True;

	/* wires do not grow so min max stuff does not change */
	for (j = wp->minrow; j <= wp->maxrow; j++) {
		for (i = wp->mincol; i <= wp->maxcol; i++) {
			offset = j * wp->bncols + i;
			z = wp->oldcells + offset;
			znew = wp->newcells + offset;
			if (*z != *znew) {	/* Counting on once a space always a space */
				found = 1;
				*z = *znew;
				if (!addtolist(mi, i - 2, j - 2, *znew - 1)) {
					free_wire_screen(MI_DISPLAY(mi), wp);
					return;
				}
			}
		}
	}
	for (i = 0; i < COLORS - 1; i++)
		if (!draw_state(mi, i)) {
			free_wire_screen(MI_DISPLAY(mi), wp);
			return;
		}
	if (++wp->generation > MI_CYCLES(mi) || !found) {
		init_wire(mi);
		return;
	} else
		do_gen(wp);

	if (wp->redrawing) {
		for (i = 0; i < REDRAWSTEP; i++) {
			if ((*(wp->oldcells + wp->redrawpos))) {
				drawCell(mi, wp->redrawpos % wp->bncols - 2,
					 wp->redrawpos / wp->bncols - 2, *(wp->oldcells + wp->redrawpos) - 1);
			}
			if (++(wp->redrawpos) >= wp->bncols * (wp->bnrows - 2)) {
				wp->redrawing = 0;
				break;
			}
		}
	}
}

ENTRYPOINT void
release_wire(ModeInfo * mi)
{
	if (circuits != NULL) {
		int         screen;

		for (screen = 0; screen < MI_NUM_SCREENS(mi); screen++)
			free_wire_screen(MI_DISPLAY(mi), &circuits[screen]);
		free(circuits);
		circuits = (circuitstruct *) NULL;
	}
}

#ifndef STANDALONE
void
refresh_wire(ModeInfo * mi)
{
	circuitstruct *wp;

	if (circuits == NULL)
		return;
	wp = &circuits[MI_SCREEN(mi)];

	MI_CLEARWINDOW(mi);
	wp->redrawing = 1;
	wp->redrawpos = 2 * wp->ncols + 2;
}
#endif

XSCREENSAVER_MODULE ("Wire", wire)

#endif /* MODE_wire */
