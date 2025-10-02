/* -*- Mode: C; tab-width: 4 -*- */
/* deluxe ---  */

#if 0
static const char sccsid[] = "@(#)deluxe.c	5.22 2006/03/07 xlockmore";
#endif

/* xscreensaver, Copyright (c) 1999, 2001, 2002 Jamie Zawinski <jwz AT jwz.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 * 
 * adapted for xlockmore : March 2006
 */

#ifdef STANDALONE
# define MODE_deluxe

# define DEFAULTS	"*delay: 5000 \n" \
			"*count: 5 \n" \
			"*ncolors: 64 \n" \
			"*fullrandom: True \n" \
			"*verbose: False \n" \

#if 0
# define DEFAULTS	"*delay: 10000 \n" \
			"*count: 5 \n", \
			"*ncolors: 20 \n", \
			"*nlayers: 0 \n", \
			"*doubleBuffer: True\n"
			".background: black\n", \
			"foreground: white\n", \

#ifdef HAVE_DOUBLE_BUFFER_EXTENSION
			, "*useDBE: True\n", \

#endif /* HAVE_DOUBLE_BUFFER_EXTENSION */
#endif
# define free_deluxe 0
# define reshape_deluxe 0
# define deluxe_handle_event 0
# include "xlockmore.h"		/* in xscreensaver distribution */
extern Bool allocate_alpha_colors (Screen *screen, Visual *visual,
                                   Colormap cmap,
                                   int *nplanesP, Bool additive_p,
                                   unsigned long **plane_masks,
                                   unsigned long *base_pixelP ,
                                   ModeInfo* mi );
#else /* STANDALONE */
# include "xlock.h"		/* in xlockmore distribution */
# include "color.h"
#endif /* STANDALONE */

#ifdef MODE_deluxe

#include <math.h>

#ifdef HAVE_DOUBLE_BUFFER_EXTENSION
# include "xdbe.h"
#endif /* HAVE_DOUBLE_BUFFER_EXTENSION */

#ifndef countof
#define countof(x) (sizeof(x)/sizeof(*(x)))
#endif

#define DEF_THICKNESS "50"
#define DEF_SPEED "15"
#define DEF_TRANSPARENT "False"
#ifdef WIN32
#define DEF_DB "False"
#else
#define DEF_DB "True"
#endif
#ifdef HAVE_DOUBLE_BUFFER_EXTENSION
#define DEF_USEDBECLEAR "True" 
#endif
#define DEF_MONO "False"
#define DEF_PLANES "0"
  
static int st_thickness;
static int st_speed;
static Bool st_transparent;
static Bool st_db;
#ifdef HAVE_DOUBLE_BUFFER_EXTENSION
static Bool st_dbeclear_p;
#endif
static Bool st_mono;
static int st_planes;

struct throbber {
  int x, y;
  int size;
  int max_size;
  int thickness;
  int speed;
  int fuse;
  GC gc;
  void (*draw) (Display *, Drawable, struct throbber *);
};

typedef struct {
  Bool transparent_p;
  int nplanes;
  unsigned long base_pixel, *plane_masks;
   int count;
   int ncolors;
   Bool dbuf;
#ifdef HAVE_DOUBLE_BUFFER_EXTENSION
   Bool dbeclear_p;
   XdbeBackBuffer backb;
#endif
   XColor *colors;
   GC erase_gc;
   struct throbber **throbbers;
   XWindowAttributes xgwa;
   Pixmap b, ba, bb;	/* double-buffer to reduce flicker */
} deluxestruct;

static deluxestruct *deluxes = (deluxestruct *) NULL;

static XrmOptionDescRec opts [] = {
  { "-thickness",	"deluxe.thickness",	XrmoptionSepArg,  (caddr_t) NULL },
  { "-speed",		"deluxe.speed",	XrmoptionSepArg,  (caddr_t) NULL },
  { "-planes",		"deluxe.planes",	XrmoptionSepArg,  (caddr_t) NULL },
  { "-transparent",	"deluxe.transparent",	 XrmoptionNoArg,  (caddr_t) "on" },
  { "+transparent",	"deluxe.transparent",	 XrmoptionNoArg,  (caddr_t) "off" },
#ifdef HAVE_DOUBLE_BUFFER_EXTENSION
  { "-useDBEClear",		"deluxe.useDBEClear", XrmoptionNoArg,  (caddr_t) "on" },
  { "+useDBEClear",		"deluxe.useDBEClear", XrmoptionNoArg,  (caddr_t) "off" },
#endif
  { "-mono",		"deluxe.mono", XrmoptionNoArg,  (caddr_t) "on" },
  { "+mono",		"deluxe.mono", XrmoptionNoArg,  (caddr_t) "off" },
  { "-db",		"deluxe.doubleBuffer", XrmoptionNoArg,  (caddr_t) "on" },
  { "+db",		"deluxe.doubleBuffer", XrmoptionNoArg,  (caddr_t) "off" }
};

static argtype vars[] =
{
	{(void *) & st_thickness, (char *) "thickness", (char *) "thickness",
	     (char *) DEF_THICKNESS, t_Int},
	{(void *) & st_speed, (char *) "speed", (char *) "speed",
	     (char *) DEF_SPEED, t_Int},
	{(void *) & st_planes, (char *) "planes", (char *) "Planes",
	     (char *) DEF_PLANES, t_Int},
	{(void *) & st_transparent, (char *) "transparent",
	     (char *) "Transparent", (char *) DEF_TRANSPARENT, t_Bool},
#ifdef HAVE_DOUBLE_BUFFER_EXTENSION
	{(void *) & st_dbeclear_p, (char *) "useDBEClear",
	     (char *) "useDBEClear",
	     (char *) DEF_USEDBECLEAR, t_Bool},
#endif
	{(void *) & st_mono, (char *) "mono", (char *) "mono",
	     (char *) DEF_MONO, t_Bool},
	{(void *) & st_db, (char *) "db", (char *) "db",
	     (char *) DEF_DB, t_Bool}

};
static OptionStruct desc[] =
{
	{(char *) "-thickness num", (char *) "Figure thickness"},
	{(char *) "-speed num", (char *) "Animation speed"},
	{(char *) "-planes num", (char *) "Number of planes"},
	{(char *) "-/+transparent", (char *) "turn on/off transparency"},
#ifdef HAVE_DOUBLE_BUFFER_EXTENSION
	{(char *) "-/+useDBEClear", (char *) "turn on/off useDBEClear"},
#endif
	{(char *) "-/+mono", (char *) "turn on/off monochromatic mode"},
	{(char *) "-/+db", (char *) "turn on/off double buffering"}
};

ENTRYPOINT ModeSpecOpt deluxe_opts =
{sizeof opts / sizeof opts[0], opts, sizeof vars / sizeof vars[0], vars, desc};

#ifdef USE_MODULES
ModStruct   deluxe_description =
{"deluxe", "init_deluxe", "draw_deluxe", "release_deluxe",
 (char *) NULL, "init_deluxe", (char *) NULL, &deluxe_opts,
 5000, 5, 1, 1, 64, 1.0, "",
 "Shows pulsing sequence of stars, circles, and lines.", 0, NULL};

#endif


static void
free_deluxe_screen(Display *dpy, deluxestruct *dlp)
{
   int i;

   if (dlp == NULL) {
     return;
   }   
   for (i = 0; i < dlp->count; i++)
     if ( dlp->throbbers[i] )
       {
          if (dlp->throbbers[i]->gc != None)
             XFreeGC (dpy, dlp->throbbers[i]->gc);
	  free( dlp->throbbers[i] );
       }
   if ( dlp->throbbers )
     {
	free( dlp->throbbers );
	dlp->throbbers = ( struct throbber** ) NULL;
     }
   if ( dlp->colors )
     {
	free( dlp->colors );
	dlp->colors = (XColor *) NULL;
     }
   if ( dlp->plane_masks )
     {
	free( dlp->plane_masks );
	dlp->plane_masks = (unsigned long*) NULL;
     }
   if (dlp->erase_gc != None)
     {
        XFreeGC (dpy, dlp->erase_gc);
   	dlp->erase_gc = None;
     }
   dlp = NULL;
}

/* http://mathworld.wolfram.com/Pentagram.html
 * R / p 
 * R=sqrt((25-11*sqrt(5))/10)
 * p=sqrt((5-sqrt(5))/10)
 * sqrt(14-6*sqrt(5))/2
 */
static void
deluxe_draw_star (Display *dpy, Drawable w, struct throbber *t)
{
  XPoint points[11];
  int x = t->x;
  int y = t->y;
  int s = (int) (t->size / 0.381966);
  int s2 = t->size;
  double c = M_PI * 2.0;
  double o = -M_PI / 2.0;

  points[0].x = (short) (x + s  * cos(o + 0.0*c));
  points[0].y = (short) (y + s  * sin(o + 0.0*c));
  points[1].x = (short) (x + s2 * cos(o + 0.1*c));
  points[1].y = (short) (y + s2 * sin(o + 0.1*c));
  points[2].x = (short) (x + s  * cos(o + 0.2*c));
  points[2].y = (short) (y + s  * sin(o + 0.2*c));
  points[3].x = (short) (x + s2 * cos(o + 0.3*c));
  points[3].y = (short) (y + s2 * sin(o + 0.3*c));
  points[4].x = (short) (x + s  * cos(o + 0.4*c));
  points[4].y = (short) (y + s  * sin(o + 0.4*c));
  points[5].x = (short) (x + s2 * cos(o + 0.5*c));
  points[5].y = (short) (y + s2 * sin(o + 0.5*c));
  points[6].x = (short) (x + s  * cos(o + 0.6*c));
  points[6].y = (short) (y + s  * sin(o + 0.6*c));
  points[7].x = (short) (x + s2 * cos(o + 0.7*c));
  points[7].y = (short) (y + s2 * sin(o + 0.7*c));
  points[8].x = (short) (x + s  * cos(o + 0.8*c));
  points[8].y = (short) (y + s  * sin(o + 0.8*c));
  points[9].x = (short) (x + s2 * cos(o + 0.9*c));
  points[9].y = (short) (y + s2 * sin(o + 0.9*c));
  points[10] = points[0];

  XDrawLines (dpy, w, t->gc, points, countof(points), CoordModeOrigin);
}

static void
draw_circle (Display *dpy, Drawable w, struct throbber *t)
{
  XDrawArc (dpy, w, t->gc,
            t->x - t->size / 2,
            t->y - t->size / 2,
            t->size, t->size,
            0, 360*64);
}

static void
draw_hlines (Display *dpy, Drawable w, struct throbber *t)
{
  XDrawLine (dpy, w, t->gc, 0,
             t->y - t->size, t->max_size,
             t->y - t->size);
  XDrawLine (dpy, w, t->gc, 0,
             t->y + t->size, t->max_size,
             t->y + t->size);
}

static void
draw_vlines (Display *dpy, Drawable w, struct throbber *t)
{
  XDrawLine (dpy, w, t->gc,
             t->x - t->size, 0,
             t->x - t->size, t->max_size);
  XDrawLine (dpy, w, t->gc,
             t->x + t->size, 0,
             t->x + t->size, t->max_size);
}

static void
draw_corners (Display *dpy, Drawable w, struct throbber *t)
{
  int s = (t->size + t->thickness) / 2;
  XPoint points[3];

  points[0].x = 0;        points[0].y = t->y - s;
  points[1].x = t->x - s; points[1].y = t->y - s;
  points[2].x = t->x - s; points[2].y = 0;
  XDrawLines (dpy, w, t->gc, points, countof(points), CoordModeOrigin);

  points[0].x = 0;        points[0].y = t->y + s;
  points[1].x = t->x - s; points[1].y = t->y + s;
  points[2].x = t->x - s; points[2].y = t->max_size;
  XDrawLines (dpy, w, t->gc, points, countof(points), CoordModeOrigin);

  points[0].x = t->x + s;    points[0].y = 0;
  points[1].x = t->x + s;    points[1].y = t->y - s;
  points[2].x = t->max_size; points[2].y = t->y - s;
  XDrawLines (dpy, w, t->gc, points, countof(points), CoordModeOrigin);

  points[0].x = t->x + s;    points[0].y = t->max_size;
  points[1].x = t->x + s;    points[1].y = t->y + s;
  points[2].x = t->max_size; points[2].y = t->y + s;
  XDrawLines (dpy, w, t->gc, points, countof(points), CoordModeOrigin);
}


static struct throbber *
make_throbber (Display *dpy, Drawable d, int w, int h, unsigned long pixel ,
	       deluxestruct *dlp, ModeInfo * mi)
{
  XGCValues gcv;
  unsigned long flags;
  struct throbber *t = (struct throbber *) malloc (sizeof (*t));
  t->x = w / 2;
  t->y = h / 2;
  t->max_size = w;
  t->speed = st_speed;
  t->fuse = 1 + (NRAND(4));
  t->thickness = st_thickness;

  if (t->speed < 0) t->speed = -t->speed;
  t->speed += (((NRAND(t->speed)) / 2) - (t->speed / 2));
  if (t->speed > 0) t->speed = -t->speed;

  if (NRAND(4))
    t->size = t->max_size;
  else
    t->size = t->thickness, t->speed = -t->speed;

  flags = GCForeground;
  if (dlp->transparent_p)
    {
      gcv.foreground = (unsigned long) ~0L;
      gcv.plane_mask = dlp->base_pixel |
	      dlp->plane_masks[NRAND(dlp->nplanes)];
      flags |= GCPlaneMask;
    }
  else
    {
#ifdef WIN32
      gcv.foreground = MI_PIXEL(mi, NRAND(MI_NPIXELS(mi)));
#else
      gcv.foreground = pixel;
#endif
    }

  gcv.line_width = t->thickness;
  gcv.line_style = LineSolid;
  gcv.cap_style = CapProjecting;
  gcv.join_style = JoinMiter;

  flags |= (GCLineWidth | GCLineStyle | GCCapStyle | GCJoinStyle);
  t->gc = XCreateGC (dpy, d, flags, &gcv);
  switch (NRAND(11)) {
  case 0: case 1: case 2: case 3: t->draw = deluxe_draw_star; break;
  case 4: case 5: case 6: case 7: t->draw = draw_circle; break;
  case 8: t->draw = draw_hlines; break;
  case 9: t->draw = draw_vlines; break;
  case 10: t->draw = draw_corners; break;
  default: abort(); break;
  }

  return t;
}

static int
throb (Display *dpy, Drawable window, struct throbber *t)
{
  t->size += t->speed;
  if (t->size <= (t->thickness / 2))
    {
      t->speed = -t->speed;
      t->size += (t->speed * 2);
    }
  else if (t->size > t->max_size)
    {
      t->speed = -t->speed;
      t->size += (t->speed * 2);
      t->fuse--;
    }

  if (t->fuse <= 0)
    {
      XFreeGC (dpy, t->gc);
      memset (t, 0, sizeof(*t));
      free (t);
      return -1;
    }
  else
    {
      t->draw (dpy, window, t);
      return 0;
    }
}

ENTRYPOINT void
init_deluxe(ModeInfo * mi)
{
  deluxestruct *dlp;
  Display *dpy = MI_DISPLAY(mi);
  Window window = MI_WINDOW(mi);
  XGCValues gcv;
  Bool writeable = (MI_IS_INSTALL(mi) && MI_NPIXELS(mi) > 2);
  int i;

   /* initialize */
   MI_INIT(mi, deluxes);
   dlp = &deluxes[MI_SCREEN(mi)];

  dlp->count = MI_COUNT(mi);
  dlp->ncolors =  MI_NCOLORS(mi);
  dlp->dbuf = st_db;
#ifdef HAVE_DOUBLE_BUFFER_EXTENSION
  dlp->dbeclear_p = st_dbeclear_p;
#endif
  dlp->colors = 0;
  dlp->erase_gc = 0;
  dlp->b=0;
  dlp->ba=0;
  dlp->bb=0;	/* double-buffer to reduce flicker */
#ifdef HAVE_DOUBLE_BUFFER_EXTENSION
  dlp->backb =0;
#endif /* HAVE_DOUBLE_BUFFER_EXTENSION */

  XGetWindowAttributes (dpy, window, &dlp->xgwa);

  dlp->transparent_p = st_transparent;

  dlp->colors = (XColor *) calloc (sizeof(XColor), dlp->ncolors);

  if ( st_mono )
    {
    MONO:
      dlp->ncolors = 1;
      dlp->colors[0].pixel = MI_WHITE_PIXEL(mi);
    }
  else if (dlp->transparent_p)
    {
      dlp->nplanes = st_planes;
      if (dlp->nplanes <= 0)
        dlp->nplanes = NRAND(dlp->xgwa.depth-2) + 2;

      allocate_alpha_colors (dlp->xgwa.screen, dlp->xgwa.visual,
		dlp->xgwa.colormap,
                &dlp->nplanes, True, &dlp->plane_masks,
		&dlp->base_pixel , mi );
      if (dlp->nplanes <= 1)
	{
	  fprintf (stderr,
         "deluxe: couldn't allocate any color planes; turning transparency off.\n"
		  );
          dlp->transparent_p = False;
	  goto COLOR;
	}
    }
  else
    {
    COLOR:
      make_random_colormap (
#ifdef STANDALONE
		dlp->xgwa.screen, dlp->xgwa.visual,
		dlp->xgwa.colormap, dlp->colors, &dlp->ncolors,
		True, True, &writeable, True
#else
		mi,
		dlp->xgwa.colormap, dlp->colors, &dlp->ncolors,
		True, True, &writeable
#endif
		);
      if (dlp->ncolors < 2)
        goto MONO;
    }

  if (dlp->dbuf)
    {
#ifdef HAVE_DOUBLE_BUFFER_EXTENSION
      if (dlp->dbeclear_p)
        dlp->b = xdbe_get_backbuffer (dpy, window, XdbeBackground);
      else
        dlp->b = xdbe_get_backbuffer (dpy, window, XdbeUndefined);
      dlp->backb = dlp->b;
#endif /* HAVE_DOUBLE_BUFFER_EXTENSION */

      if (!dlp->b)
        {
          dlp->ba = XCreatePixmap (dpy, window, dlp->xgwa.width,
				   dlp->xgwa.height, dlp->xgwa.depth);
          dlp->bb = XCreatePixmap (dpy, window, dlp->xgwa.width,
				   dlp->xgwa.height, dlp->xgwa.depth);
          dlp->b = dlp->ba;
        }
    }
  else
    {
      dlp->b = window;
    }

  dlp->throbbers = (struct throbber **) calloc ( dlp->count, sizeof(struct throbber *));
  for (i = 0; i < dlp->count; i++)
    dlp->throbbers[i] = make_throbber (dpy, dlp->b, dlp->xgwa.width,
				       dlp->xgwa.height,
                                  dlp->colors[ NRAND( dlp->ncolors )].pixel ,
				       dlp, mi);

  gcv.foreground = MI_BLACK_PIXEL(mi);
  dlp->erase_gc = XCreateGC (dpy, dlp->b, GCForeground, &gcv);

  if ( dlp->ba) XFillRectangle (dpy, dlp->ba, dlp->erase_gc, 0, 0,
				dlp->xgwa.width, dlp->xgwa.height);
  if ( dlp->bb) XFillRectangle (dpy, dlp->bb, dlp->erase_gc, 0, 0,
				dlp->xgwa.width, dlp->xgwa.height);
}

ENTRYPOINT void
draw_deluxe(ModeInfo * mi)
{
   deluxestruct *dlp = &deluxes[MI_SCREEN(mi)];
   Display *dpy = MI_DISPLAY(mi);
   Window window = MI_WINDOW(mi);
   int i;
   
   if (deluxes == NULL)
	return;
   MI_IS_DRAWN(mi) = True;


#ifdef HAVE_DOUBLE_BUFFER_EXTENSION
      if (!dlp->dbeclear_p || !dlp->backb )
#endif /* HAVE_DOUBLE_BUFFER_EXTENSION */
        XFillRectangle (dpy, dlp->b, dlp->erase_gc, 0, 0, dlp->xgwa.width,
			dlp->xgwa.height);

      for (i = 0; i < dlp->count; i++)
        if (throb (dpy, dlp->b, dlp->throbbers[i]) < 0)
          dlp->throbbers[i] = make_throbber (dpy, dlp->b, dlp->xgwa.width,
					     dlp->xgwa.height,
                                        dlp->colors[ NRAND( dlp->ncolors )].pixel ,
					     dlp, mi);

#ifdef HAVE_DOUBLE_BUFFER_EXTENSION
      if (dlp->backb)
        {
          XdbeSwapInfo info[1];
          info[0].swap_window = window;
          info[0].swap_action = (dlp->dbeclear_p ? XdbeBackground : XdbeUndefined);
          XdbeSwapBuffers (dpy, info, 1);
        }
      else
#endif /* HAVE_DOUBLE_BUFFER_EXTENSION */
      if (dlp->dbuf)
        {
          XCopyArea (dpy, dlp->b, window, dlp->erase_gc, 0, 0,
                     dlp->xgwa.width, dlp->xgwa.height, 0, 0);
          dlp->b = (dlp->b == dlp->ba ? dlp->bb : dlp->ba);
        }
      XSync (dpy, False);
}

ENTRYPOINT void
release_deluxe(ModeInfo * mi)
{
   if (deluxes != NULL) {
      int screen;

      for (screen = 0; screen < MI_NUM_SCREENS(mi); screen++)
	free_deluxe_screen(MI_DISPLAY(mi), &deluxes[screen]);
      free(deluxes);
      deluxes = (deluxestruct *) NULL;
   }
}

XSCREENSAVER_MODULE ("Deluxe", deluxe)

#endif /* MODE_deluxe */
