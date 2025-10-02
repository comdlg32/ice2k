/* -*- Mode: C; tab-width: 4 -*- */
/* anemone ---  */

#if 0
static const char sccsid[] = "@(#)anemone.c	5.22 2006/03/04 xlockmore";

#endif

/* anemon, Copyright (c) 2001 Gabriel Finch
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 */

/*------------------------------------------------------------------------
  |
  |  FILE            anemone.c
  |  MODULE OF       xscreensaver
  |
  |  DESCRIPTION     Anemone.
  |
  |  WRITTEN BY      Gabriel Finch
  |                  
  |
  |
  |  MODIFICATIONS   june 2001 started
  |                  March 2006 adaption for xlockmore
  |           
  +----------------------------------------------------------------------*/



#ifdef STANDALONE
# define MODE_anemone
# define DEFAULTS	"*delay: 100000 \n" \
			"*size: 4 \n" \
			"*ncolors: 8 \n" \
			"*fullrandom: True \n" \
			"*verbose: False \n" \
			"*delay: 40000 \n" \

#if 0
	".background: black \n" \

#endif
#if 0
#ifdef HAVE_DOUBLE_BUFFER_EXTENSION
	"*useDBE: True \n" \

#endif /* HAVE_DOUBLE_BUFFER_EXTENSION */
#endif
# define reshape_anemone 0
# define anemone_handle_event 0
# include "xlockmore.h"		/* in xscreensaver distribution */
#else /* STANDALONE */
# include "xlock.h"		/* in xlockmore distribution */
#endif /* STANDALONE */

#ifdef MODE_anemone

#undef HAVE_DOUBLE_BUFFER_EXTENSION
/* xdbe_get_backbuffer does not exist */
#ifdef HAVE_DOUBLE_BUFFER_EXTENSION
#include "xdbe.h"
#endif /* HAVE_DOUBLE_BUFFER_EXTENSION */


/*-----------------------------------------------------------------------+
  |  PRIVATE DATA                                                          |
  +-----------------------------------------------------------------------*/


#define TWO_PI     (2.0 * M_PI)
#define MAXPEND    2000
#define MAXPTS    200

#define DEF_ARMS "256"
#define DEF_FINPOINTS "64"
#define DEF_WIDTH "2"
#define DEF_WITHDRAW "2400"
#define DEF_TURNSPEED "5"

static int st_arms;
static int st_finpoints;
static int st_width;
static int st_withdraw;
static int st_turnspeed;

typedef struct {
  double x,y,z;
  int sx,sy,sz;
} vPend;

typedef struct {
  long col;
  int numpt;
  int growth;
  unsigned short rate;
} appDef;


typedef struct {
  Display *dpy;
  Window window;

  Pixmap b,ba,bb;

#ifdef HAVE_DOUBLE_BUFFER_EXTENSION
  XdbeBackBuffer backb;
#endif /* HAVE_DOUBLE_BUFFER_EXTENSION */

  int
    arms,                       /* number of arms */
    finpoints;                  /* final number of points in each array. */

  int         scrWidth, scrHeight;
  GC          gcDraw, gcClear;


  unsigned short dbuf;
  int width;

  vPend *vPendage;  /* 3D representation of appendages */
  appDef *appD;  /* defaults */
  vPend *vCurr, *vNext;
  appDef *aCurr;

  double turn, turndelta;

  int    mx, my;            /* max screen coordinates. */
  int withdraw;

  XGCValues         gcv;
  Colormap          cmap;
} anemonestruct;

static anemonestruct *anemones = (anemonestruct *) NULL;


/*-----------------------------------------------------------------------+
  |  PUBLIC DATA                                                           |
  +-----------------------------------------------------------------------*/

static XrmOptionDescRec opts[] =
{
   {(char *) "-arms", (char *) ".anemone.arms", XrmoptionSepArg, (caddr_t) NULL},
   {(char *) "-finpoints", (char *) ".anemone.finpoints", XrmoptionSepArg, (caddr_t) NULL},
   {(char *) "-width", (char *) ".anemone.width", XrmoptionSepArg, (caddr_t) NULL},
   {(char *) "-withdraw", (char *) ".anemone.withdraw", XrmoptionSepArg, (caddr_t) NULL},
   {(char *) "-turnspeed", (char *) ".anemone.turnspeed", XrmoptionSepArg, (caddr_t) NULL}
};

static argtype vars[] =
{
   {(void *) &st_arms, (char *) "arms",
   (char *) "Arms", (char *) DEF_ARMS , t_Int},
   {(void *) &st_finpoints, (char *) "finpoints",
   (char *) "Finpoints", (char *) DEF_FINPOINTS , t_Int},
   {(void *) &st_width, (char *) "width",
   (char *) "Width", (char *) DEF_WIDTH , t_Int},
   {(void *) &st_withdraw, (char *) "withdraw",
   (char *) "Withdt=raw", (char *) DEF_WITHDRAW , t_Int},
   {(void *) &st_turnspeed, (char *) "turnspeed",
   (char *) "Turnspeed", (char *) DEF_TURNSPEED , t_Int}
};


static OptionStruct desc[] =
{
   {(char *) "-arms num", (char *) "Number of arms"},
   {(char *) "-width num", (char *) "Width of arms"},
   {(char *) "-finpoints num", (char *) "final number of points in each array"},
   {(char *) "-withdraw num", (char *) "withdraw frequency"},
   {(char *) "-turnspeed num", (char *) "turning speed"}
};

ENTRYPOINT ModeSpecOpt anemone_opts =
{sizeof opts / sizeof opts[0], opts, sizeof vars / sizeof vars[0], vars, desc};

#ifdef USE_MODULES
ModStruct   anemone_description =
{"anemone", "init_anemone", "draw_anemone", "release_anemone",
 (char *) NULL, "init_anemone", "free_anemone", &anemone_opts,
 50000, 1, 1, 1, 64, 1.0, "",
 "Shows wiggling tentacles", 0, NULL};

#endif

/*-----------------------------------------------------------------------+
  |  PRIVATE FUNCTIONS                                                     |
  +-----------------------------------------------------------------------*/

static void
free_anemone_screen(Display *display, anemonestruct *sp)
{
   if (sp == NULL) {
        return;
   }
   if ( sp->vPendage )
     {
	free( sp->vPendage );
	sp->vPendage = (vPend*) NULL;
     }
   if ( sp->appD )
     {
	free( sp->appD );
	sp->appD = (appDef*) NULL;
     }
   if ( sp->gcDraw != None )
     {
	XFreeGC(display, sp->gcDraw );
	sp->gcDraw = None;
     }
   if ( sp->gcClear != None )
     {
	XFreeGC(display, sp->gcClear );
	sp->gcClear = None;
     }
   if (sp->ba != None)
     {
	XFreePixmap(display, sp->ba);
	sp->ba = None;
     }
   if (sp->bb != None)
     {
	XFreePixmap(display, sp->bb);
	sp->bb = None;
     }
   sp = NULL;
}

ENTRYPOINT void
free_anemone(ModeInfo * mi)
{
  free_anemone_screen(MI_DISPLAY(mi), &anemones[MI_SCREEN(mi)]);
}

static void *
xmalloc(size_t size)
{
  void *ret;

  if ((ret = malloc(size)) == NULL) {
    fprintf(stderr, "anemone: out of memory\n");
  }
  return ret;
}


static void
initAppendages(ModeInfo *mi, anemonestruct *sp)
{
  int    i;
  /*int    marginx, marginy; */
    
  /*double scalex, scaley;*/

  double x,y,z,dist;

  sp->mx = sp->scrWidth - 1;
  sp->my = sp->scrHeight - 1;

  /* each appendage will have: colour,
     number of points, and a grow or shrink indicator */

  /* added: growth rate 1-10 (smaller==faster growth) */
  /* each appendage needs virtual coords (x,y,z) with y and z combining to
     give the screen y */

  sp->vPendage = (vPend *) xmalloc((sp->finpoints + 1) * sizeof(vPend) * sp->arms);
  sp->appD = (appDef *) xmalloc(sizeof(appDef) * sp->arms);


  for (i = 0; i < sp->arms; i++) {
    sp->aCurr = sp->appD + i;
    sp->vCurr = sp->vPendage + (sp->finpoints + 1) * i;
    sp->vNext = sp->vCurr + 1;

#ifdef WIN32
    sp->aCurr->col = (long)NRAND(MI_NCOLORS(mi));
#else
    sp->aCurr->col = (long)NRAND(256)*NRAND(256)+32768;
#endif
    sp->aCurr->numpt = 1;
    sp->aCurr->growth=sp->finpoints/2+NRAND(sp->finpoints/2);
    sp->aCurr->rate=NRAND(11)*NRAND(11);

    dist=1.;

    do {
      x=(1-NRAND(1001)/500);
      y=(1-NRAND(1001)/500);
      z=(1-NRAND(1001)/500);
      dist=x*x+y*y+z*z;
    } while (dist>=1.);

    sp->vCurr->x=x*200;
    sp->vCurr->y=sp->my/2+y*200;
    sp->vCurr->z=0+z*200;

    /* start the arm going outwards */
    sp->vCurr->sx=(int) (sp->vCurr->x/5);
    sp->vCurr->sy=(int) ((sp->vCurr->y-sp->my/2)/5);
    sp->vCurr->sz=(int) (sp->vCurr->z/5);

    
    sp->vNext->x=sp->vCurr->x+sp->vCurr->sx;
    sp->vNext->y=sp->vCurr->y+sp->vCurr->sy;
    sp->vNext->z=sp->vCurr->z+sp->vCurr->sz;
  }
}

static void
initAnemone( ModeInfo * mi , anemonestruct *sp )
{
  XWindowAttributes wa;

  sp->turn = 0.;
   if ( st_width < 0 )
     sp->width = NRAND( -st_width ) + 1;
   else
     sp->width = st_width;
   if ( st_arms < 0 )
     sp->arms = NRAND( -st_arms ) + 1;
   else
     sp->arms = st_arms;
   if ( st_finpoints < 0 )
     sp->finpoints = NRAND( -st_finpoints ) + 1;
   else
     sp->finpoints = st_finpoints;
   if ( st_withdraw < 0 )
     sp->withdraw = NRAND( -st_withdraw );
   else
     sp->withdraw = st_withdraw;
   if ( st_turnspeed < 0 )
     sp->turndelta = NRAND( -st_turnspeed ) / 10000.;
   else
     sp->turndelta =st_turnspeed / 10000.0;

#ifdef WIN32
  sp->dbuf=False;
#else
  sp->dbuf=True;
#endif

  sp->b=sp->ba=sp->bb=0;	/* double-buffer to reduce flicker */
#ifdef HAVE_DOUBLE_BUFFER_EXTENSION
  sp->b = sp->backb = xdbe_get_backbuffer (sp->dpy, sp->window, XdbeUndefined);
#endif /* HAVE_DOUBLE_BUFFER_EXTENSION */


  XGetWindowAttributes(sp->dpy, sp->window, &wa);
  sp->scrWidth = wa.width;
  sp->scrHeight = wa.height;
  sp->cmap = wa.colormap;
  sp->gcDraw = XCreateGC(sp->dpy, sp->window, GCForeground, &sp->gcv);
  sp->gcv.foreground = MI_BLACK_PIXEL(mi);
  sp->gcClear = XCreateGC(sp->dpy, sp->window, GCForeground, &sp->gcv);

  if (sp->dbuf) {
    if (!sp->b)
      {
	sp->ba = XCreatePixmap (sp->dpy, sp->window, sp->scrWidth, sp->scrHeight, wa.depth);
	sp->bb = XCreatePixmap (sp->dpy, sp->window, sp->scrWidth, sp->scrHeight, wa.depth);
	sp->b = sp->ba;
      }
  }
  else
    {	
     sp->b= sp->window;
    }

  if (sp->ba) XFillRectangle (sp->dpy, sp->ba, sp->gcClear, 0, 0, sp->scrWidth, sp->scrHeight);
  if (sp->bb) XFillRectangle (sp->dpy, sp->bb, sp->gcClear, 0, 0, sp->scrWidth, sp->scrHeight);

  XClearWindow(sp->dpy, sp->window);
  XSetLineAttributes(sp->dpy, sp->gcDraw,  sp->width, LineSolid, CapRound, JoinBevel);

  initAppendages(mi, sp);
}


static void
createPoints(anemonestruct *sp)
{
  int i;
  int withdrawall=NRAND(sp->withdraw);

  for (i = 0; i< sp->arms; i++) {
    sp->aCurr = sp->appD + i;
    if (!withdrawall) {
      sp->aCurr->growth=-sp->finpoints;
      sp->turndelta=-sp->turndelta;
    }

    else if (withdrawall<11) sp->aCurr->growth=-sp->aCurr->numpt;

    else if (NRAND(100)<sp->aCurr->rate) {
      if (sp->aCurr->growth>0) {
	if (!(--sp->aCurr->growth)) sp->aCurr->growth=-NRAND(sp->finpoints)-1;
	sp->vCurr = sp->vPendage + (sp->finpoints + 1) * i + sp->aCurr->numpt-1;
	if (sp->aCurr->numpt<sp->finpoints - 1) {
	  /* add a piece */	
	  sp->vNext=sp->vCurr + 1;
	  sp->aCurr->numpt++;
	  sp->vNext->sx=sp->vCurr->sx+NRAND(3)-1;
	  sp->vNext->sy=sp->vCurr->sy+NRAND(3)-1;
	  sp->vNext->sz=sp->vCurr->sz+NRAND(3)-1;
	  sp->vCurr=sp->vNext+1;
	  sp->vCurr->x=sp->vNext->x+sp->vNext->sx;
	  sp->vCurr->y=sp->vNext->y+sp->vNext->sy;
	  sp->vCurr->z=sp->vNext->z+sp->vNext->sz;
	}
      }
    }
  }
}


static void
drawImage(Drawable curr_window, double sint, double cost , anemonestruct *sp, ModeInfo *mi)
{
  int q,numpt,mx2=sp->mx/2;
  double cx,cy,cz,nx=0,ny=0,nz=0;

  if ((numpt=sp->aCurr->numpt)==1) return;
#ifdef WIN32
  XSetForeground(sp->dpy, sp->gcDraw, MI_PIXEL(mi, sp->aCurr->col));
#else
  XSetForeground(sp->dpy, sp->gcDraw, sp->aCurr->col);
#endif
    
  sp->vNext=sp->vCurr+1;

  cx=sp->vCurr->x;
  cy=sp->vCurr->y;
  cz=sp->vCurr->z;


  for (q = 0; q < numpt-1; q++) {
    nx=sp->vNext->x+2-NRAND(5);
    ny=sp->vNext->y+2-NRAND(5);
    nz=sp->vNext->z+2-NRAND(5);

    XDrawLine(sp->dpy, curr_window, sp->gcDraw,
	(int) (mx2+cx*cost-cz*sint), (int) cy,
	(int) (mx2+nx*cost-nz*sint), (int) ny);
    sp->vCurr++;
    sp->vNext++;

    cx=nx;
    cy=ny;
    cz=nz;
  }
  XSetLineAttributes(sp->dpy, sp->gcDraw, sp->width*3, LineSolid, CapRound, JoinBevel);
  XDrawLine(sp->dpy, curr_window, sp->gcDraw,
	(int) (sp->mx/2+cx*cost-cz*sint), (int) cy,
	(int) (sp->mx/2+nx*cost-nz*sint), (int) ny);
  XSetLineAttributes(sp->dpy, sp->gcDraw, sp->width, LineSolid, CapRound, JoinBevel);

}

static void
animateAnemone(Drawable curr_window , anemonestruct *sp, ModeInfo *mi)
{
  int i;
  double sint=sin(sp->turn),cost=cos(sp->turn);

  sp->aCurr = sp->appD;
  for (i = 0; i< sp->arms; i++) {
    sp->vCurr=sp->vPendage + (sp->finpoints + 1) * i;
    if (NRAND(25)<sp->aCurr->rate) {
      if (sp->aCurr->growth<0) {
	sp->aCurr->numpt-=sp->aCurr->numpt>1;
	if (!(++sp->aCurr->growth)) sp->aCurr->growth=NRAND(sp->finpoints-sp->aCurr->numpt)+1;
      }
    }
    drawImage(curr_window, sint, cost , sp, mi);
    sp->turn+=sp->turndelta;
    sp->aCurr++;
  }
  createPoints(sp);

  if (sp->turn>=TWO_PI) sp->turn-=TWO_PI;
}

/*-----------------------------------------------------------------------+
  |  PUBLIC FUNCTIONS                                                      |
  +-----------------------------------------------------------------------*/

ENTRYPOINT void
init_anemone(ModeInfo * mi)
{
   anemonestruct *sp;

   MI_INIT(mi, anemones);
   sp = &anemones[MI_SCREEN(mi)];

   sp->dpy= MI_DISPLAY(mi);
   sp->window=MI_WINDOW(mi);

   initAnemone( mi , sp );

   XFillRectangle (sp->dpy, sp->b, sp->gcClear, 0, 0, sp->scrWidth, sp->scrHeight);

   animateAnemone( sp->b , sp, mi);
}
   
ENTRYPOINT void
draw_anemone(ModeInfo * mi)
{
   anemonestruct *sp = &anemones[MI_SCREEN(mi)];
   
   if (anemones == NULL)
	return;
   MI_IS_DRAWN(mi) = True;

#ifdef HAVE_DOUBLE_BUFFER_EXTENSION
    if (sp->backb)
      {
	XdbeSwapInfo info[1];
	info[0].swap_window = sp->window;
	info[0].swap_action = XdbeUndefined;
	XdbeSwapBuffers (sp->dpy, info, 1);
      }
    else
#endif /* HAVE_DOUBLE_BUFFER_EXTENSION */
      if (sp->dbuf)
	{
	  XCopyArea (sp->dpy, sp->b, sp->window, sp->gcClear, 0, 0,
		     sp->scrWidth, sp->scrHeight, 0, 0);
	 sp->b = (sp->b == sp->ba ? sp->bb : sp->ba);
	}

   XFillRectangle (sp->dpy, sp->b, sp->gcClear, 0, 0, sp->scrWidth, sp->scrHeight);

    animateAnemone( sp->b , sp, mi);
}

ENTRYPOINT void
release_anemone(ModeInfo * mi)
{
   if (anemones != NULL) {
      int screen;

      for (screen = 0; screen < MI_NUM_SCREENS(mi); screen++)
	free_anemone_screen(MI_DISPLAY(mi), &anemones[screen]);
      free(anemones);
      anemones = (anemonestruct *) NULL;
   }
}

/*
void
refresh_anemone(ModeInfo * mi)
{
  if (anemones == NULL)
    return;
}
*/

XSCREENSAVER_MODULE ("Anemone", anemone)

#endif /* MODE_anemone */
