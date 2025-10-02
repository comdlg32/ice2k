/* -*- Mode: C; tab-width: 2 -*- */
/* rain --- show raindrops       */

#if 0
static const char sccsid[] = "@(#)rain.c  1.00 2007/05/22 xlockmore";
#endif

/*
 * Copyright (c) 1999-2007 by Frank Fesevur
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copywidth notice appear in all copies and that
 * both that copywidth notice and this permission notice appear in
 * supporting documentation.
 *
 * This file is provided AS IS with no warranties of any kind. The author
 * shall have no liability with respect to the infringement of copywidths,
 * trade secrets or any patents by this file or any part thereof. In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 *
 * Revision History:
 * 24-May-2007: port to xlockmore
 *    Jan-1999: Written to play with a MS-Windows screen saver
 */

#ifdef STANDALONE
#define MODE_rain
#define DEFAULTS "*delay: 35000 \n" \
	"*ncolors: 64 \n" \

# define free_rain 0
# define reshape_rain 0
# define rain_handle_event 0
#define SMOOTH_COLORS
#if 0
#define UNIFORM_COLORS    /* To get blue water uncomment, but ... */
#endif
#include "xlockmore.h"    /* in xscreensaver distribution */
#else /* STANDALONE */
#include "xlock.h"        /* in xlockmore distribution */
#endif /* STANDALONE */

#ifdef MODE_rain

ENTRYPOINT ModeSpecOpt rain_opts =
{0, (XrmOptionDescRec *) NULL, 0, (argtype *) NULL, (OptionStruct *) NULL};

#ifdef USE_MODULES
ModStruct rain_description =
{
  "rain", "init_rain", "draw_rain", "release_rain",
  "refresh_rain", "init_rain", (char *) NULL, &rain_opts,
  35000, 1, 1, 1, 64, 0.3, "",
  "It's raining", 0, NULL
};
#endif

#define MAX_RADIUS 25           /* Maximum radius of the drop               */

typedef struct _drop
{
  XRectangle      drop;         /* Rectangle of the falling rain-drop       */
  XPoint          offset;       /* Speed of the falling rain-drop           */
  XPoint          pool;         /* Centerpoint of the pool                  */
  unsigned long   color;        /* Color of the drop and pool               */
  int             radius;       /* Radius of the pool                       */
  int             radius_step;  /* Step-size to increase the pool           */
  int             max_radius;   /* Maximum width of the pool                */
  struct _drop   *next;
} dropstruct;

typedef struct
{
  int         direction;        /* Left to right (1) or Right to left (-1)  */
  int         colored_drops;    /* Colored drops? 1 or 0                    */
  int         base_color;       /* Colored drops, use base color?           */
  dropstruct *drops;            /* The drops                                */
} rainstruct;

static rainstruct *rain = (rainstruct *) NULL;

/*
 * Draw an ellipse at 'drop->drop' with a radius of 'drop->radius' and color 'color'
 * Not using the color of the 'drop' because we have to draw a black one as well.
 */

static void DrawEllipse(ModeInfo * mi, dropstruct *drop, unsigned long color)
{
  Display *display = MI_DISPLAY(mi);
  Window   window = MI_WINDOW(mi);
  GC       gc = MI_GC(mi);
  int      w = 2 * drop->radius;
  int      h = (drop->radius * 2) / 3;

  XSetForeground(display, gc, color);
  XDrawArc(display, window, gc,
    drop->drop.x - w, drop->drop.y - h, 2 * w + 1, 2 * h + 1, 0, 360 * 64);
}

/*
 * Draw a line from (rect.x, rect.height) to (rect.width, rect.y)
 * with color 'color'
 */

static void DrawLine(ModeInfo * mi, XRectangle rect, unsigned long color)
{
  Display *display = MI_DISPLAY(mi);
  Window   window = MI_WINDOW(mi);
  GC       gc = MI_GC(mi);

  XSetForeground(display, gc, color);
  XDrawLine(display, window, gc, rect.x, rect.height, rect.width, rect.y);
}

static void
free_rain_screen(rainstruct *rp)
{
  if (rp == NULL) {
    return;
  }
  if (rp->drops != NULL)
  {
    dropstruct *drop, *dropNext;
    /* Cleanup the drops */

    drop = rp->drops;
    while (drop != NULL) {
      dropNext = drop->next;
      free(drop);
      drop = dropNext;
    }
    rp->drops = NULL;
  }
  rp = NULL;
}

/*
 * Initialise a single drop
 */

static void InitDropColors(ModeInfo * mi, rainstruct *rp)
{
  /* Once in every 10 times show only white drops */
  rp->colored_drops = (NRAND(10) != 1);

  /* When using colored, use the all random colors or around the base color */
  if (rp->colored_drops)
  {
    if (NRAND(2) == 1)
      rp->base_color = MI_PIXEL(mi, NRAND(MI_NPIXELS(mi)));
    else
      rp->base_color = 0;
  }
}

/*
 * Initialise a single drop
 */

static void InitDrop(ModeInfo * mi, rainstruct *rp, dropstruct *drop)
{
  /* Where does the drop fall in the pool?
     At least 20% of height from top and not completely at the bottom */
  int yMin = MI_HEIGHT(mi) / 5;
  int yMax = MI_HEIGHT(mi) - ((MAX_RADIUS * 3) / 2);
  drop->pool.x = 0;
  drop->pool.y = yMin + NRAND(yMax - yMin);

  /* How fast does it fall? */
  drop->offset.x = 5 + NRAND(5);
  drop->offset.y = 20 + NRAND(20);

  /* Where does the drop start */
  drop->drop.x = NRAND(MI_WIDTH(mi));
  drop->drop.height = 0;
  drop->drop.width = drop->drop.x + drop->offset.x;
  drop->drop.y = drop->drop.height + drop->offset.y;

  /* How large is the pool and how fast does it grow? */
  drop->radius = 0;
  drop->radius_step = 1 + NRAND(2);
  drop->max_radius = (MAX_RADIUS / 2) + NRAND(MAX_RADIUS / 2);

  /* Colored drops? */
  if (rp->colored_drops)
  {
    if (rp->base_color == 0)
      drop->color = MI_PIXEL(mi, NRAND(MI_NPIXELS(mi)));
    else
      drop->color = rp->base_color + (NRAND(2) == 1 ? -1 : 1) * NRAND(12);
  }
  else
    drop->color = MI_WHITE_PIXEL(mi);
}

/*
 * Initialise the mode
 */

ENTRYPOINT void
init_rain(ModeInfo * mi)
{
  rainstruct *rp;
  int i, nr_drops, max_drops;
  dropstruct *drop;

  /* Allocate the memory */
  MI_INIT(mi, rain);
  rp = &rain[MI_SCREEN(mi)];
  MI_CLEARWINDOW(mi);

  /* "Left to Right"  or  "Right to Left" */
  rp->direction = (NRAND(2) == 1 ? -1 : 1);
  InitDropColors(mi, rp);
  /* Determine how many drops to show */
  max_drops = MI_WIDTH(mi) / 50;
  i = MAX(max_drops / 2, 2);
  nr_drops = NRAND(i) + i;
  nr_drops = MAX(1, nr_drops);
  nr_drops = MIN(nr_drops, max_drops);

  /* Initialise these drops */
  free_rain_screen(rp);
  for (i = 0; i < nr_drops; i++)
  {
    /* Allocate the memory for the drop */
    drop = (dropstruct *) malloc(sizeof(dropstruct));
    if (drop == NULL)
      return;

    /* Initialise the drop */
    InitDrop(mi, rp, drop);
    drop->next = NULL;

    /* Add the drop to the list */
    if (rp->drops != NULL)
      drop->next = rp->drops;
    rp->drops = drop;
  }
}

/*
 * Draw the drops
 */

ENTRYPOINT void
draw_rain(ModeInfo * mi)
{
  dropstruct *drop;
  rainstruct *rp;

  if (rain == NULL)
    return;
  rp = &rain[MI_SCREEN(mi)];
  /* Just in case */
  if (rp->drops == NULL)
    return;

  if (NRAND(MI_WIDTH(mi) / 4) == 1)
    InitDropColors(mi, rp);

  for (drop = rp->drops; drop != NULL; drop = drop->next)
  {
    /* Show the drop or show the pool */
    if (drop->drop.width < MI_WIDTH(mi) - drop->max_radius &&
        drop->drop.width > drop->max_radius &&
        drop->drop.y < drop->pool.y)
    {
      /* Erase the previous drop */
      DrawLine(mi, drop->drop, MI_BLACK_PIXEL(mi));

      /* Calculate the new position */
      drop->drop.x = drop->drop.width;
      drop->drop.height = drop->drop.y;
      drop->drop.width += rp->direction * drop->offset.x;
      drop->drop.y += drop->offset.y;

      /* Draw the new drop */
      DrawLine(mi, drop->drop, drop->color);
    }
    else
    {
      /* Erase the last drop */
      if (drop->radius == 0)
      {
        DrawLine(mi, drop->drop, MI_BLACK_PIXEL(mi));

        drop->pool.x = drop->drop.width;
        drop->pool.y = drop->drop.y;
      }

      /* Erase the previous pool ellipse */
      DrawEllipse(mi, drop, MI_BLACK_PIXEL(mi));

      /* Was last pool? Initialise again */
      if (drop->radius > drop->max_radius)
        InitDrop(mi, rp, drop);
      else
      {
        /* Enlarge the pool ellipse */
        drop->radius += drop->radius_step;

        /* Draw the new pool */
        DrawEllipse(mi, drop, drop->color);
      }
    }
  }
}

/*
 * Clean up the mess
 */

ENTRYPOINT void
release_rain(ModeInfo * mi)
{
  if (rain != NULL)
  {
    int screen;

    for (screen = 0; screen < MI_NUM_SCREENS(mi); screen++)
      free_rain_screen(&rain[screen]);
    free(rain);
    rain = (rainstruct *) NULL;
  }
}

#ifndef STANDALONE
/*
 *
 */

ENTRYPOINT void
refresh_rain(ModeInfo * mi)
{
  MI_CLEARWINDOW(mi);
}
#endif

XSCREENSAVER_MODULE ("Rain", rain)

#endif /* MODE_rain */
