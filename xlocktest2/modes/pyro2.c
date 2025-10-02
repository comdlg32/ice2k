/* -*- Mode: C; tab-width: 4 -*- */
/* pyro --- fireworks */

#if 0
static const char sccsid[] = "@(#)pyro2.c	5.26 2008/02/07 xlockmore";
#endif


/*-
 * 1991, Pezhman Givy.
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
 */

#ifdef STANDALONE
#define MODE_pyro2
#define DEFAULTS "*delay: 15000 \n" \
	"*count: 1 \n" \
	"*size: -3 \n" \
	"*ncolors: 200 \n" \
	"*use3d: False \n" \
	"*delta3d: 1.5 \n" \
	"*right3d: red \n" \
	"*left3d: blue \n" \
	"*both3d: magenta \n" \
	"*none3d: black \n" \

# define free_pyro2 0
# define reshape_pyro2 0
# define pyro2_handle_event 0
# define UNIFORM_COLORS
# include "xlockmore.h"		/* in xscreensaver distribution */
#else /* STANDALONE */
# include "xlock.h"		/* in xlockmore distribution */
# include "color.h"
# include "iostuff.h"
#endif /* STANDALONE */

#define PYRO_NHUES    7
#define PYRO_NSHADES  20
#define PYRO_NCOLORS (PYRO_NHUES*PYRO_NSHADES)
#define PYRO_NPOINTS  3  /* colorpath */


#ifdef MODE_pyro2

#define DEF_INVERT  "False"

static Bool invert;

typedef unsigned int uint32;

#ifdef __VMS
#define DEFAULT_MSG	"&0&1&2&3&4&5&6&7&8&9&a&b OpenVMS &a&b"
#else
#ifdef WIN32
#define DEFAULT_MSG	"&0&1&2&3&4&5&6&7&8&9&a&b Windows &a&b"
#else
#define DEFAULT_MSG	"&0&1&2&3&4&5&6&7&8&9&a&b Unix &a&b"
#endif
#endif
#define DEFAULT_FNT	"-*-helvetica-bold-r-*-240-*"

static int pyrocnt;
static char *modparam_msg, *modparam_fnt;
static XFontStruct *messagefont = None;

static XrmOptionDescRec opts[] = {
	{(char *) "-invert", (char *) ".pyro2.invert", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+invert", (char *) ".pyro2.invert", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-msg", (char *) ".pyro2.msg", XrmoptionSepArg, (caddr_t) NULL},
	{(char *) "-fnt", (char *) ".pyro2.fnt", XrmoptionSepArg, (caddr_t) NULL}
};

static argtype vars[] = {
	{(void *) &invert, (char *) "invert", (char *) "Invert", (char *) DEF_INVERT, t_Bool},
	{(void *) &modparam_msg, (char *) "msg", (char *) "Msg", (char *) DEFAULT_MSG, t_String},
	{(void *) &modparam_fnt, (char *) "fnt", (char *) "Fnt", (char *) DEFAULT_FNT, t_String}
};

static OptionStruct desc[] = {
        {(char *) "-/+invert", (char *) "turn on/off inverting of sparks for image"},
	{(char *) "-msg str", (char *) "Pyro command (default: " DEFAULT_MSG ")"},
	{(char *) "-fnt font", (char *) "Font to use (default " DEFAULT_FNT ")"}
};

ENTRYPOINT ModeSpecOpt pyro2_opts =
{	sizeof opts / sizeof opts[0], opts, sizeof vars / sizeof vars[0], vars, desc};

#ifdef USE_MODULES
ModStruct   pyro2_description =
{	"pyro2",		/* cmdline_arg: mode name */
	"init_pyro2",		/* init_name: name of init a mode */
	"draw_pyro2",		/* callback_name: name of run (tick) a mode */
	"release_pyro2",	/* release_name: name of shutdown of a mode */
	"refresh_pyro2",	/* refresh_name: name of mode to repaint */
	"init_pyro2",		/* change_name: name of mode to change */
	(char *) NULL,		/* unused_name: name for future expansion */
	&pyro2_opts,		/* msopt: this mode's def resources */
	30000,			/* def_delay: default delay for mode */
	400,			/* def_count: */
	1,			/* def_cycles: */
	-3,			/* def_size: */
	1024,			/* def_ncolors: */
	1.0,			/* def_saturation: */
	"",			/* def_bitmap: */
	"Shows other fireworks",/* desc: text description of mode */
	0,			/* flags: state flags for this mode */
	NULL			/* userdata: for use by the mode */
};

#endif

static char * ostext = NULL;

/*
 * 26051990 phg
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#ifdef USE_XVMSUTILS
#include <X11/unix_time.h>
#endif
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#endif

#if !defined(FALSE)
#define FALSE	0
#endif /* !defined(FALSE) */

#if !defined(TRUE)
#define TRUE	1
#endif /* !defined(TRUE) */

#define RAD(x)	((x) / 180.0 * M_PI)
#define DEG(x)	((x)/M_PI * 180.0)
#if !defined(ABS)
#define ABS(x)	((x) >=0? (x):(-(x)))
#endif /* !defined(ABS) */

#define SIN(x)	(sin(RAD(x)))
#define COS(x)	(cos(RAD(x)))
#define TAN(x)	(tan(RAD(x)))
#define NOP(x)	(1.0)

#define WAIT		0
#define PARABEL		1
#define EXPLOSION	2
#define READY		3

#ifndef WIN32
#ifndef STANDALONE
/* aliases for vars defined in the bitmap file */
#define LOGO_WIDTH   image_width
#define LOGO_HEIGHT    image_height
#define LOGO_BITS    image_bits

#include "pyro2.xbm"

#ifdef HAVE_XPM
#define LOGO_NAME  image_name
#include "pyro2.xpm"
#define DEFAULT_XPM 0
#endif
#endif

#if !defined( VMS ) || ( __VMS_VER >= 70000000 )
#include <sys/utsname.h>
#else
#ifdef USE_XVMSUTILS
#if 0
#include "../xvmsutils/utsname.h"
#else
#include <X11/utsname.h>
#endif
#endif /* USE_XVMSUTILS */
#endif
#endif

typedef struct para {
	int init;	/* == true : initialization needed */
	int color;
	double t;
	double th;
	double v0;
	double phi;
	int x1, y1;
	int x2, y2;
}PARA;

/*
for the moment there are 11 distinct internal explosion types.
Each type has its own ID. An ID is a character between
'0' and '9' or 'a' and 'z'. 'a' corresponds to 10 and
'z' to 35. The type ID 42 is reserved for glyph explosions.

you can address a particular internal explosion by typing '&' followed by
its ID. When the applied ID is not (yet) used the last used ID is assumed.

Each explosion consists of several generations.
a collection of sparks created at same time is denoted as a generation of sparks
how many generations are produced is controlled by the defined EXP_MAX_GENERATION
*/

typedef struct	{
	int init;
	int x0, y0;
	double *time;	/* time since explosion for each spark :-) */
	double *v0;	/* initial speed of each spark */
	double *delta_t;/* value to be added to the time in each iteration for each spark */
	double *angle;	/* angle of the each spark as it leaves the explosion center */
	double *m;	/* */
	int *generation;/* the generation of each spark */
	int *x1, *y1;
	int *x2, *y2;
	int *ttl;	/* time to leave of each spark (max = EXP_MAX_TTL) */
	int current_generation;/* current generation of sparks */
	int sparkcnt;	/* current count of sparks */
	unsigned int type;	/* type of spark */
	int firsttime;
	int *color;
	union	{	/* custom data for each explosion type, if needed */
		int integer;
		double *dbl_ptr;
		int *int_ptr;
		char *chr_ptr;
		long *lng_ptr;
	} typedata;
}EXPL;

typedef struct	{
	int dotcnt;
	float *phi;
	float *v0;
	XImage *image;
} LOGO_BITMAP;

#define TEXT_LEN 256
typedef struct pyro {
	int stat;	/* status der rakette: parabell, explosion, fertig */
	int wait;	/* warten bevor start */
	int textlen;
	char text[TEXT_LEN];
	LOGO_BITMAP *bitmap;
	unsigned int etype;
	PARA para;
	EXPL expl;
	int  color1, color2; /* startcolor, index in pp->colors */
}PYRO;

typedef struct {
	GC		gc;
	Colormap	cmap;
	XColor		*colors;
	int		ncolors;
	unsigned long	blackpixel, whitepixel, fg, bg, love;

	int		nhues;
	int		nsteps;
} pyrostruct;

static void parabel(ModeInfo * mi, PYRO *p);
static void explosion(ModeInfo * mi, PYRO *p);
static int calc(ModeInfo * mi, PYRO *p);

static PYRO *pyro;

static pyrostruct *pyros = NULL;

static void
free_a_pyro(PYRO *p)
{

	if(p->expl.time)	{free(p->expl.time);	p->expl.time = NULL;}
	if(p->expl.v0)		{free(p->expl.v0);	p->expl.v0 = NULL;}
	if(p->expl.delta_t)	{free(p->expl.delta_t);	p->expl.delta_t = NULL;}
	if(p->expl.angle)	{free(p->expl.angle);	p->expl.angle = NULL;}
	if(p->expl.generation)	{free(p->expl.generation);	p->expl.generation = NULL;}
	if(p->expl.color)	{free(p->expl.color);	p->expl.color = NULL; }
	if(p->expl.ttl)		{free(p->expl.ttl);	p->expl.ttl = NULL;}

	return;
}

static void
free_pyro2_screen(ModeInfo *mi, Display *display, pyrostruct *pp)
{
	if (pp == NULL) {
		return;
	}
	if (MI_IS_INSTALL(mi) && MI_NPIXELS(mi) > 2) {
		if(pp->colors && pp->ncolors) { /* FIXME: if !no_colors */
			free_colors(
#ifdef STANDALONE
				MI_SCREENPTR(mi),
#else
				display,
#endif
				pp->cmap, pp->colors, pp->ncolors);
		}
		if (pp->colors != NULL) {
			free(pp->colors);
		}
		if (pp->cmap) {
			XFreeColormap(display, pp->cmap);
		}
		if (pp->gc != None) {
			MI_WHITE_PIXEL(mi) = pp->whitepixel;
			MI_BLACK_PIXEL(mi) = pp->blackpixel;
#ifndef STANDALONE
			MI_FG_PIXEL(mi) = pp->fg;
			MI_BG_PIXEL(mi) = pp->bg;
#endif
			XFreeGC(display, pp->gc);
		}
	}
	pp = NULL;
}

/*#define delay(x)	usleep(x)*/
#define delay(x)	(x)
static int calc(ModeInfo * mi, PYRO *pyro)
{
	int i, ret;

	ret = 0;

	for(i=0;i<pyrocnt;++i)
		switch(pyro[i].stat) {
			case WAIT:
				ret = 1;
				if(--pyro[i].wait <= 0) {
					pyro[i].stat = PARABEL;
				}
				/*else
					delay(1);*/
				break;
			case PARABEL:
				ret = 1;
				parabel(mi, &pyro[i]);
				/*delay(1);*/
				break;
			case EXPLOSION:
				ret = 1;
				explosion(mi, &pyro[i]);
				break;
			default:
				pyro[i].stat = READY;
				free_a_pyro(&pyro[i]);
				/*delay(1);*/
		}


	return ret;
}

/*
 * A bang body of the mass m leaves at the time t=0 the origin
 * of a xy-coordinate system with a start velocity v0 and angle phi.
 * Then the following aplies:
 *
 * x(t) = (v0*cos(phi))*t;
 * y(t) = (v0*sin(phi))*t - 1/2*g*t*t;
 *
 * The time at which the maximum ascend is arrived (th) will be
 * th = (v0*sin(phi))/g;
 *
 * "H. Heuser, Lehrbuch der Analysis, Part I, page 328 (Wurf)
 */


/*
 * ein knallkoerper der masse m verlasse zur zeit t=0 den nullpunkt
 * eines xy-koordinatensystems mit einer anfangsgeschwindigkeit vom
 * betrage v0 unter dem winkel phi (0<phi<=pi/2).
 * so gilt:
 *	x(t) = (v0*cos(phi))*t;
 *	y(t) = (v0*sin(phi))*t - 1/2*g*t*t;
 * die steigzeit th der maximalen steighoehe betraegt
 * th = (v0*sin(phi))/g;
 *
 * aus h. heuser, lehrbuch der analysis teil 1, seite 328(wurf)
 *
 * 30.10.1994, luftreibung wird nicht beruecksichtig
 */

static void parabel(ModeInfo * mi, PYRO *p)
{
	Display    *display = MI_DISPLAY(mi);
	GC          gc = MI_GC(mi);
	Window      window = MI_WINDOW(mi);
	PARA *pa;
	int x, y;

	pa = &p->para;

	if(pa->init == FALSE) {
		pa->phi = 80.0 + (double)NRAND(20);
		/*pa->phi = 60.0 + (double)NRAND(60);*/
		pa->v0 = 23.0;
		/*pa->v0 = 10.0 + NRAND(10);*/
		pa->th = pa->v0 * SIN(pa->phi) / 9.81;
		pa->t = 0.0;
	}

	/*
	x = MI_WIDTH(mi) /2 + (int)(pa->v0*COS(pa->phi)*pa->t*30.0);
	y = MI_HEIGHT(mi) - (int)((pa->v0*SIN(pa->phi)*pa->t - 0.5*9.81*pa->t*pa->t)*30.0);
	*/

	x = MI_WIDTH(mi) /2 + (int)(pa->v0*COS(pa->phi)*pa->t* MI_WIDTH(mi)/35.0 );
	y = MI_HEIGHT(mi) - (int)((pa->v0*SIN(pa->phi)*pa->t - 0.5*9.81*pa->t*pa->t)* MI_HEIGHT(mi)/35.0 );

	if(x<0 || x>=MI_WIDTH(mi) || y<0 || y>=MI_HEIGHT(mi))	x = y = -1;

	if(pa->init == FALSE) {
		pa->x1 = x;
		pa->y1 = y;
		pa->x2 = -1;
		pa->y2 = -1;
	}
	else
		if(pa->x1>=0 && pa->y1>=0 && pa->x2>=0 && pa->y2>=0) {
			XSetForeground(display, gc, MI_BLACK_PIXEL(mi));
			XDrawLine(display, window, gc, pa->x1, pa->y1, pa->x2, pa->y2);
		}

	if(pa->t<pa->th-0.02 && pa->x2>=0 && pa->y2>=0 && x>=0 && y>=0) {
		XSetForeground(display, gc, MI_WHITE_PIXEL(mi));
		XDrawLine(display, window, gc, pa->x2, pa->y2, x, y);
	}

	pa->x1 = pa->x2;
	pa->y1 = pa->y2;
	pa->x2 = x;
	pa->y2 = y;
	if(pa->init == FALSE)	pa->init = TRUE;
	if((pa->t += 0.02) > pa->th) {
		p->stat = EXPLOSION;
		pa->init = FALSE;
	}

	return;
}

#define EXP_MAX_GENERATION	(30)
#define EXP_MAX_TTL		(200)
/*#define EXP_MAX_TTL		(250)*/
/*#define EXP_MIN_SPARKS		(10)*/
#define EXP_MIN_SPARKS		(20)
#define EXP_RND_SPARKS		(20)
#define EXP_MAX_SPARKS		(EXP_MIN_SPARKS + EXP_RND_SPARKS)
#define EXP_MAX_DOTS		(EXP_MAX_GENERATION * EXP_MAX_SPARKS * 20)

/*
static int rbits, gbits, bbits;
static int rpos, gpos, bpos;

static int get_red(double val)
{
	return ((int)(val*(double)((1<<rbits)-1))&((1<<rbits)-1)) << rpos;
}

static int get_green(double val)
{
	return ((int)(val*(double)((1<<gbits)-1))&((1<<gbits)-1)) << gpos;
}

static int get_blue(double val)
{
	return (int)(val*(double)((1<<bbits)-1))&((1<<bbits)-1) << bpos;
}

#define COLOR(r,g,b)	(get_red(r)|get_green(g)|get_blue(b))
*/

double heart[] = {
0.80999300,0.80999300,0.82499300,0.82599300,0.82699300,0.82799300,0.82899300,0.83099300,
0.83299300,0.84999300,0.83799300,0.83999300,0.84299300,0.84599300,0.84999300,0.85399300,
0.85799300,0.84699300,0.85199300,0.85699300,0.86199300,0.85199300,0.85799300,0.85299300,
0.85499300,0.85699300,0.85299300,0.85249300,0.85199300,0.84399300,0.83999300,0.83799300,
0.83599300,0.82799300,0.82899300,0.82999300,0.81299300,0.81499300,0.79999400,0.79999400,
0.78899400,0.78599400,0.77899400,0.76499400,0.76399400,0.75799400,0.74499400,0.73299500,
0.72099500,0.70999500,0.69999500,0.70799500,0.67999500,0.67099500,0.66299600,0.65399600,
0.64699600,0.63899600,0.63199600,0.60799600,0.60199700,0.57999700,0.57399700,0.56899700,
0.54799700,0.54399700,0.52299800,0.50399800,0.49999800,0.48099800,0.46299900,0.45999900,
0.44199900,0.42399900,0.40699900,0.40500000,0.38800000,0.35700000,0.34100100,0.34000100,
0.30900100,0.30800100,0.27800100,0.27800100,0.23300100,0.23300100,0.23300100,0.20300000,
0.20300000,0.20300000,0.20300000,0.20300000,0.20300000,0.20300000,0.20400000,0.23300100,
0.23300100,0.23400100,0.27800100,0.27900100,0.30900100,0.34000100,0.34100100,0.37200000,
0.38800000,0.39000000,0.40699900,0.42399900,0.45699900,0.45999900,0.46299900,0.48099800,
0.49999800,0.51899800,0.52299800,0.54399700,0.54799700,0.56899700,0.57399700,0.59599700,
0.60199700,0.60799600,0.63199600,0.63899600,0.64699600,0.65399600,0.66299600,0.67099500,
0.67999500,0.70799500,0.69999500,0.70999500,0.72099500,0.73299500,0.74499400,0.75799400,
0.76599400,0.76499400,0.77899400,0.78799400,0.78899400,0.80199300,0.79999400,0.81699300,
0.81299300,0.83199300,0.82899300,0.82999300,0.83799300,0.84399300,0.83999300,0.84499300,
0.85399300,0.84599300,0.85499300,0.85699300,0.85699300,0.85299300,0.85999300,0.85399300,
0.86399300,0.85899300,0.85399300,0.84899300,0.85999300,0.85499300,0.85199300,0.84799300,
0.84499300,0.84199300,0.83899300,0.83699300,0.83399300,0.83199300,0.83099300,0.82899300,
0.82799300,0.82699300,0.81199300,0.81199300,0.81199300,0.81199300,0.81199300,0.79799400,
0.79899400,0.78599400,0.78699400,0.77399400,0.76099400,0.76299400,0.75099400,0.75299400,
0.74099400,0.72899500,0.71799500,0.70599500,0.69399500,0.69299500,0.67099500,0.66699600,
0.66399600,0.64699600,0.64099600,0.62999600,0.61899600,0.61699600,0.59699700,0.60199700,
0.58599700,0.58099700,0.57899700,0.56199700,0.56499700,0.55799700,0.54399700,0.53099800,
0.53799700,0.52999800,0.51799800,0.52299800,0.51799800,0.50799800,0.50799800,0.50999800,
0.49999800,0.49299800,0.50199800,0.49499800,0.48699800,0.48699800,0.49199800,0.48499800,
0.47799800,0.48299800,0.48299800,0.47799800,0.47199900,0.47999800,0.47799800,0.47299800,
0.46799900,0.47199850,0.47599800,0.47999800,0.48399800,0.47999800,0.47599800,0.48399800,
0.48399800,0.48699800,0.49299800,0.48999800,0.48699800,0.49799800,0.49699800,0.50699800,
0.50799800,0.51799800,0.51799800,0.53099800,0.52999800,0.54299700,0.54099700,0.55499700,
0.56099700,0.56699700,0.59499700,0.59499700,0.62299600,0.65199600,0.65199600,0.65199600,
0.62299600,0.59499700,0.59499700,0.58199700,0.55299700,0.55499700,0.54099700,0.54299700,
0.52999800,0.52799800,0.51799800,0.51199800,0.50799800,0.50099800,0.49699800,0.49299800,
0.48699800,0.48999800,0.49299800,0.48299800,0.48399800,0.47999800,0.47599800,0.47999800,
0.48399800,0.47699800,0.47599800,0.47699800,0.46799900,0.47299800,0.47799800,0.47799800,
0.47199900,0.47799800,0.48299800,0.47999800,0.47799800,0.48499800,0.49199800,0.48499800,
0.48699800,0.49499800,0.49999800,0.49199800,0.49999800,0.50999800,0.50599800,0.50799800,
0.51799800,0.52099800,0.51799800,0.52999800,0.54199700,0.54699700,0.54399700,0.55799700,
0.56299700,0.56199700,0.57899700,0.57899700,0.58599700,0.60099700,0.59599700,0.61699600,
0.61799600,0.62999600,0.63999600,0.64699600,0.66199600,0.66699600,0.66899500,0.69299500,
0.69199500,0.70399500,0.71599500,0.71299500,0.73999400,0.75199400,0.74899400,0.74699400,
0.75999400,0.77299400,0.78599400,0.78399400,0.79799400,0.79699400,0.81099300,0.80999300
};

static void explosion(ModeInfo * mi, PYRO *p)
{
	Display    *display = MI_DISPLAY(mi);
	Window      window = MI_WINDOW(mi);
	int xk1, yk1, xk2, yk2;
	EXPL *e = &p->expl;
	double x1, y1, x2, y2;
	double phi, m, step;
	int i, sparks;
	int sumttl;
	static int num = 0;

	double v0, dt, t;

	pyrostruct *pp = &pyros[MI_SCREEN(mi)];

	GC          gc = pp->gc;

	++num;
	if(e->init == FALSE) {
		/* every spark has its own angle and initial speed (v0) */
		e->angle = (double *) calloc(EXP_MAX_DOTS, sizeof(double));
		e->time = (double *) calloc(EXP_MAX_DOTS, sizeof(double));
		e->delta_t = (double *) calloc(EXP_MAX_DOTS, sizeof(double));
		e->v0 = (double *) calloc(EXP_MAX_DOTS, sizeof(double));
		e->ttl = (int *) calloc(EXP_MAX_DOTS, sizeof(int));
		e->generation = (int *) calloc(EXP_MAX_DOTS, sizeof(int));
		e->x1 = (int *) calloc(EXP_MAX_DOTS, sizeof(int));
		e->y1 = (int *) calloc(EXP_MAX_DOTS, sizeof(int));
		e->x2 = (int *) calloc(EXP_MAX_DOTS, sizeof(int));
		e->y2 = (int *) calloc(EXP_MAX_DOTS, sizeof(int));
		e->color = (int *) calloc(EXP_MAX_DOTS, sizeof(int));
		e->m = (double *) calloc(EXP_MAX_DOTS, sizeof(double));
		e->current_generation = 0;
		e->firsttime = 0;
		e->x0 = p->para.x2;
		e->y0 = p->para.y2;
		e->sparkcnt = 0;
		e->init = TRUE;

		e->type = p->etype;

		switch(e->type)	{
			case 4:
				e->typedata.integer = 1;
				break;
			default:
				e->typedata.integer = 0;
				break;
		}
	}
	else
		++e->current_generation;

/* rho = 1.293 g/l = 0.001293 kg/l */
#define rho (5.0)
#define factor (2)
/*#define rho (3.0)*/
/*#define factor (2.0)*/

	sumttl = 0;
	for(i=0;i<e->sparkcnt;++i) sumttl += e->ttl[i]>0? e->ttl[i]:0;

	for(i=0;i<e->sparkcnt;++i)	{
		--e->ttl[i];
		phi = e->angle[i];
		v0 = e->v0[i];
		t = e->time[i];
		dt = e->delta_t[i];
		m = e->m[i];

		x1 = m * v0 * COS(phi) / rho * (1-exp(-rho/m*t));
		y1 = m/rho * (v0 * SIN(phi) + m*9.81/rho)*(1-exp(-rho/m*t)) - m*9.81/rho * t;
		xk1 = e->x0 + x1;
		yk1 = e->y0 - y1;

		if(e->firsttime == 1 && e->ttl[i] >= 0){
			XSetForeground(display, gc, MI_BLACK_PIXEL(mi));
			XDrawLine(display, window, gc, e->x1[i], e->y1[i], e->x2[i], e->y2[i]);
		}

		if(sumttl < 1)	{
			p->stat = READY;
			e->init = FALSE;
			continue;
		}

		if(e->ttl[i] < 1)	continue;

		t = e->time[i] + dt;

		x2 = m * v0 * COS(phi) / rho * (1-exp(-rho/m*t));
		y2 = m/rho * (v0 * SIN(phi) + m*9.81/rho)*(1-exp(-rho/m*t)) - m*9.81/rho * t;
		xk2 = e->x0 + x2;
		yk2 = e->y0 - y2;

		e->x1[i] = xk1;
		e->y1[i] = yk1;
		e->x2[i] = xk2;
		e->y2[i] = yk2;

		/*
		XSetForeground(display, gc,
			pp->colors[ (int)
			(((double)e->ttl[i]/(double)EXP_MAX_TTL)*pp->ncolors)].pixel);
		*/




		if(MI_IS_INSTALL(mi) && MI_NPIXELS(mi) > 2) {
		XSetForeground(display, gc, 
			pp->colors[ e->color[i] + PYRO_NSHADES-1 - (int)
			(((double)e->ttl[i]/(double)EXP_MAX_TTL)*PYRO_NSHADES)].pixel);
		} else {
			if (MI_NPIXELS(mi) > 2)
				XSetForeground(display, gc, (MI_PIXEL(mi, e->color[i] + PYRO_NSHADES-1 - (int) (((double)e->ttl[i]/(double)EXP_MAX_TTL)*PYRO_NSHADES) % MI_NCOLORS(mi)))); 
			else
				XSetForeground(display, gc, MI_WHITE_PIXEL(mi));
		}


		XDrawLine(display, window, gc, xk1, yk1, xk2, yk2);

		e->time[i] = t;
		e->delta_t[i] += 0.001;
		/*e->delta_t[i] += MI_DELAY(mi) / 1000000.0;*/
	}

	if(e->current_generation >= EXP_MAX_GENERATION) {
		return;
	}
	e->firsttime = 1;

	sparks = (p->bitmap != NULL && (e->type == 42 || e->type == 23))? p->bitmap->dotcnt:EXP_MIN_SPARKS+NRAND(EXP_RND_SPARKS);

	if(e->type == 42 && e->current_generation > 0) {
		return;
	}

	step = 360.0 / sparks;
	phi = 1.0+(double)NRAND(10);

	for(i=0;i<sparks;++i)	{
		e->angle[e->sparkcnt] = phi;
		e->color[e->sparkcnt] = ( (e->sparkcnt & 1) && p->color2!=-1) ? p->color2 : p->color1;

		e->ttl[e->sparkcnt] = EXP_MAX_TTL/2+NRAND(EXP_MAX_TTL/1);
		e->generation[e->sparkcnt] = e->current_generation;
		e->time[e->sparkcnt] = 0.0;
		e->delta_t[e->sparkcnt] = 0; /*0.001;*/
		/*e->m[e->sparkcnt] = 0.5 + NRAND(10)/10.0;*/ /*NRAND(2)? 1.0:1.3;*/
		e->m[e->sparkcnt] = 2 + NRAND(2)/2.0; /*NRAND(2)? 1.0:1.3;*/

		switch(e->type)	{
			case 0:	{/* ? */
				double tmp = (double)e->current_generation/(double)EXP_MAX_GENERATION;

				e->ttl[e->sparkcnt] = EXP_MAX_TTL/10+NRAND(EXP_MAX_TTL/2);
				e->v0[e->sparkcnt] = NRAND(30)+SIN(tmp *90.0) * MI_WIDTH(mi)/10 * factor;
			}
			break;
			case 1:	{
				double tmp = (double)e->current_generation/(double)EXP_MAX_GENERATION;
				e->v0[e->sparkcnt] = NRAND(30)+SIN(tmp *360.0) * MI_WIDTH(mi)/10 * factor;
			}
			break;
			case 2:	{
					double tmp = (double)e->current_generation/(double)EXP_MAX_GENERATION;
					e->v0[e->sparkcnt] = NRAND(30)+COS(tmp *180.0) * MI_WIDTH(mi)/5 * factor;
					/*e->v0[e->sparkcnt] = NRAND(30)+COS(tmp *180.0) * 200;*/
				}
			break;
			case 3:	{
					double tmp = (double)e->current_generation/(double)EXP_MAX_GENERATION;
					e->v0[e->sparkcnt] = NRAND(30)+SIN(tmp*360.0) * COS(tmp*360.0) * MI_WIDTH(mi)/5 * factor;
				}
			break;
			case 4: {
					if(i == 0)	e->typedata.integer = -e->typedata.integer;
					e->v0[e->sparkcnt] = NRAND(30)+(i+2)*e->typedata.integer*MI_WIDTH(mi)/200 * factor;
				}
			break;
			case 5:	{
				/*double tmp = (double)e->current_generation/(double)EXP_MAX_GENERATION;*/
				e->v0[e->sparkcnt] = NRAND(30)+COS((int)((double)e->current_generation/(double)EXP_MAX_GENERATION *360.0) % 90) * COS((double)e->current_generation/(double)EXP_MAX_GENERATION *360.0) * MI_WIDTH(mi)/5 * factor;
			}
			break;
			case 6:	{
				double tmp = (double)e->current_generation/(double)EXP_MAX_GENERATION;
				e->v0[e->sparkcnt] = NRAND(30)+((i-sparks/2)*SIN(phi*10)*SIN(tmp*360.0))*MI_WIDTH(mi)/100 * factor;
			}
			break;
			case 7:	{
				/*double tmp = (double)e->current_generation/(double)EXP_MAX_GENERATION;*/
				e->v0[e->sparkcnt] = NRAND(30)+((i-sparks/2)*SIN(phi*10)*SIN((int)((double)e->current_generation/(EXP_MAX_GENERATION)*360.0)%90))*MI_WIDTH(mi)/50 * factor;
			}
			break;
			case 8:
				e->v0[e->sparkcnt] = NRAND(30)+(SIN((i+1.0)/(EXP_MIN_SPARKS+EXP_RND_SPARKS)*360.0)*(1.0*i))*MI_WIDTH(mi)/100 * factor;
				break;
			case 9:	{
				double tmp = (double)e->current_generation/(double)EXP_MAX_GENERATION;
				e->v0[e->sparkcnt] = NRAND(30)+SIN(tmp *360.0*4.0) * MI_WIDTH(mi)/10 * factor;
			}
			break;
			case 10:
				e->ttl[e->sparkcnt] = EXP_MAX_TTL/2+NRAND(EXP_MAX_TTL/1)+5;
				e->v0[e->sparkcnt] = heart[(int)((double)i/(double)sparks *360.0)] * (e->current_generation)*MI_WIDTH(mi)/150 * factor;
				if (MI_IS_INSTALL(mi))
					e->color[e->sparkcnt] = PYRO_NSHADES; /* pp->love; MI_RED_MASK(mi); */
				else
					e->color[e->sparkcnt] = MI_NCOLORS(mi) - 2; /* pp->love; MI_RED_MASK(mi); */
				break;
			case 11:	{
				double tmp = (double)i/(double)sparks;
				e->v0[e->sparkcnt] = NRAND(30) + SIN(tmp*90)*COS(tmp*180)*i*(i&1? 1:-1)*MI_WIDTH(mi)/100 * factor;
				break;
			}
			case 13:	{
				/*double tmp = (double)e->current_generation/(double)EXP_MAX_GENERATION;*/
				e->v0[e->sparkcnt] = NRAND(2)? i * 20:(sparks-i)*20;
				break;

			}
			case 14:	{
				/*double tmp = (double)e->current_generation/(double)EXP_MAX_GENERATION;*/
				e->v0[e->sparkcnt] = 5+ (NRAND(20) == 3? i:(sparks-i))*20;
				break;

			}
			case 23:
			case 42:	/* glyphs */
				if (p->bitmap != NULL) {
				/*e->angle[e->sparkcnt] = (double)p->glyph->phi[i]/1000000.0;*/
				/*e->v0[e->sparkcnt] = (double)p->glyph->dist[i]/1000000.0 * 600;*/
				/*e->color[e->sparkcnt] = MI_RED_MASK(mi)|MI_GREEN_MASK(mi);*/

				e->angle[e->sparkcnt] = (double)p->bitmap->phi[i];
				e->v0[e->sparkcnt] = (double)p->bitmap->v0[i] * MI_WIDTH(mi)/4 * factor;
				/*e->v0[e->sparkcnt] = (double)p->bitmap->v0[i] * 300.0;*/
				e->m[e->sparkcnt] = 1.0 + NRAND(50)/800.0;

				e->current_generation = EXP_MAX_GENERATION;
				break;
				}
			default:
			case 12:	{
				double tmp = (double)e->current_generation/(double)EXP_MAX_GENERATION;
				/*e->v0[e->sparkcnt] = NRAND(30) + ABS(SIN(tmp * 90.0 * 3) * tmp ) * 200;*/
				/*e->v0[e->sparkcnt] = NRAND(30) + (tmp <= 0.3 ? SIN(tmp * 90.0):tmp <= 0.6? SIN((tmp-0.3) * 90.0):SIN((tmp-0.6)*90.0)) * (tmp + 0.5)* 200+10;*/
				e->v0[e->sparkcnt] = NRAND(10)+15 + (tmp <= 0.3 ? SIN(tmp * 90.0):tmp <= 0.6? SIN((tmp-0.3) * 90.0):SIN((tmp-0.6)*90.0)) * (tmp + 0.5)* MI_WIDTH(mi)/5 * factor;
				/*e->v0[e->sparkcnt] = NRAND(10)+15 + (tmp <= 0.3 ? SIN(tmp * 90.0):tmp <= 0.6? SIN((tmp-0.3) * 90.0):SIN((tmp-0.6)*90.0)) * (tmp + 0.5)* 200;*/
				break;
			}
		}

		phi += step;
		++e->sparkcnt;
	}

	return;
}

#ifndef WIN32
static Bool text2image(ModeInfo * mi, char *msg, XImage ** image)
{
	Display    *display = MI_DISPLAY(mi);
	char       *text1, *text2;
	char       *line, *token;
	int         width, height;
	int         lines;
	int         margin = 2;
	XCharStruct overall;
	XGCValues   gcv;
	GC          gc;
	Pixmap      text_pixmap;

	text1 = strdup(msg);

	while (*text1 && (text1[strlen(text1) - 1] == '\r' ||
			text1[strlen(text1) - 1] == '\n'))
		text1[strlen(text1) - 1] = 0;

	if ((text2 = (char *) strdup(text1)) == NULL) {
		free(text1);
		return False;
	}

	(void) memset(&overall, 0, sizeof (overall));
	token = text1;
	lines = 0;
	while ((line = strtok(token, "\r\n"))) {
		XCharStruct o2;
		int         ascent, descent, direction;

		token = 0;
		(void) XTextExtents(messagefont, line, strlen(line),
			&direction, &ascent, &descent, &o2);
		overall.lbearing = MAX(overall.lbearing, o2.lbearing);
		overall.rbearing = MAX(overall.rbearing, o2.rbearing);
		lines++;
	}

	width = overall.lbearing + overall.rbearing + margin + margin + 1;
	height = ((messagefont->ascent + messagefont->descent) * lines) +
		margin + margin;

	if ((text_pixmap = XCreatePixmap(display, MI_WINDOW(mi),
			width, height, 1)) == None) {
		free(text1);
		free(text2);
		return False;
	}

	gcv.font = messagefont->fid;
	gcv.foreground = 0;
	gcv.background = 0;
	if ((gc = XCreateGC(display, text_pixmap,
			GCFont | GCForeground | GCBackground, &gcv)) == None) {
		XFreePixmap(display, text_pixmap);
		free(text1);
		free(text2);
		return False;
	}
	XFillRectangle(display, text_pixmap, gc, 0, 0, width, height);
	XSetForeground(display, gc, 1);

	token = text2;
	lines = 0;
	while ((line = strtok(token, "\r\n"))) {
		XCharStruct o2;
		int         ascent, descent, direction, xoff;

		token = 0;

		(void) XTextExtents(messagefont, line, strlen(line),
			&direction, &ascent, &descent, &o2);
		xoff = ((overall.lbearing + overall.rbearing) -
			(o2.lbearing + o2.rbearing)) / 2;

		(void) XDrawString(display, text_pixmap, gc,
			overall.lbearing + margin + xoff,
			((messagefont->ascent * (lines + 1)) +
			(messagefont->descent * lines) + margin),
			line, strlen(line));
		lines++;
	}
	free(text1);
	free(text2);
	/*XUnloadFont(display, messagefont->fid); */
	XFreeGC(display, gc);

	if ((*image = XGetImage(display, text_pixmap, 0, 0, width, height,
			1L, XYPixmap)) == NULL) {
		XFreePixmap(display, text_pixmap);
		return False;
	}
	XFreePixmap(display, text_pixmap);

	return True;
}

static Bool XImage2bitmap(XImage *image, LOGO_BITMAP *bitmap, int white)
{
	int x, y, offset;
	char mask;
	int xbeg, xend, ybeg, yend;
	int xmid, ymid, dotcnt = 0;
	double maxdist;
	int xstep, ystep;

	xbeg = image->width;
	xend = 0;
	ybeg = image->height;
	yend = 0;
	dotcnt = 0;
	xstep = 1;
	ystep = 1;

	for(y=0;y<image->height;y+=ystep)	{
		for(x=0;x<image->width;x+=xstep)	{
			offset = image->bytes_per_line * y + x / 8;
			mask = 1 << (x%8);

			if (((image->data[offset] & mask) == (white << (x%8)) && invert) ||
			((image->data[offset] & mask) != (white << (x%8)) && !invert)) {
				/* fprintf(stderr, "X");*/
				if(x < xbeg)	xbeg = x;
				if(x > xend)	xend = x;
				if(y < ybeg)	ybeg = y;
				if(y > yend)	yend = y;
				++dotcnt;
			}
			/* else
				fprintf(stderr, "_"); */
		}
		/* fprintf(stderr, "\n");*/

	}

	bitmap->dotcnt = dotcnt;
	bitmap->phi = (float *) calloc(dotcnt, sizeof(float));
	bitmap->v0 = (float *) calloc(dotcnt, sizeof(float));

	xmid = xbeg + (xend - xbeg) / 2;
	ymid = ybeg + (yend - ybeg) / 2;

	maxdist = sqrt( (xend - xbeg)*(xend - xbeg) + (yend - ybeg)*(yend - ybeg) ) / 2;

	dotcnt = 0;
	for(y=0;y<image->height;y+=ystep)	{
		for(x=0;x<image->width;x+=xstep)	{
			offset = y * image->bytes_per_line  + x / 8;
			mask = 1 << (x%8);

			if (((image->data[offset] & mask) == (white << (x%8)) && invert) ||
			((image->data[offset] & mask) != (white << (x%8)) && !invert)) {
				double a = x - xmid;
				double b = y - ymid;
				double dist = sqrt(a * a + b * b) / maxdist;
				double phi = -DEG(fabs(a) < 1 && fabs(b) < 1? 0 : atan2(b, a));

				bitmap->phi[dotcnt] = phi;
				bitmap->v0[dotcnt] = dist;

				++dotcnt;
			}
		}
	}

	return TRUE;
}

static LOGO_BITMAP *text2bitmap(ModeInfo *mi, char *text)
{
	LOGO_BITMAP *bitmap;

	if(!(bitmap = (LOGO_BITMAP *) calloc(1, sizeof(LOGO_BITMAP))))	return NULL;

	if(!text2image(mi, text, &bitmap->image))	{
		free(bitmap);
		return NULL;
	}

	XImage2bitmap(bitmap->image, bitmap, 1);

	return bitmap;
}
#endif

#ifndef STANDALONE
	extern char *background;
	extern char *foreground;
#endif

ENTRYPOINT void
release_pyro2(ModeInfo * mi)
{
	if (pyros != NULL) {
		int         screen;
		Display *display = MI_DISPLAY(mi);

		for (screen = 0; screen < MI_NUM_SCREENS(mi); screen++)
			free_pyro2_screen(mi, display, &pyros[screen]);
		free(pyros);
		pyros = (pyrostruct *) NULL;
	}
	if (messagefont != None) {
		XFreeFont(MI_DISPLAY(mi), messagefont);
		messagefont = None;
	}
	if (ostext != NULL) {
		free(ostext);
		ostext = NULL;
	}
}

ENTRYPOINT void
init_pyro2(ModeInfo * mi)
{
	int i;
	/*time_t t = time(NULL);*/
	int text_in_buffer = FALSE;
	char *rov;
	/*static char defaultString[] =
#ifdef __VMS
	"&0&1&2&3&4&5&6&7&8&9&a&b OpenVMS &a&b";
#else
	"&0&1&2&3&4&5&6&7&8&9&a&b Unix &a&b";
#endif*/
	Display *display = MI_DISPLAY(mi);
	Window  window = MI_WINDOW(mi);
	pyrostruct *pp;

	MI_INIT(mi, pyros);
        if (messagefont == None) {
		if (modparam_fnt != NULL &&
				!(messagefont = XLoadQueryFont(display, modparam_fnt)))	{
			(void) fprintf(stderr, "can not find font '%s', using %s\n", modparam_fnt, DEFAULT_FNT);
		}
        	if (messagefont == None) {
			if ((messagefont = getFont(display)) == None) {
				release_pyro2(mi);
				return;
			}
		}
	}

	pp = &pyros[MI_SCREEN(mi)];
	if(!pp->gc) {
		if(MI_IS_INSTALL(mi) && MI_NPIXELS(mi) > 2) {
#ifdef STANDALONE
			Screen *screen = MI_SCREENPTR(mi);
			Visual *visual = MI_VISUAL(mi);
#endif
			XColor      color;
			int hue, ncol;

#ifndef STANDALONE
			pp->fg = MI_FG_PIXEL(mi);
			pp->bg = MI_BG_PIXEL(mi);
#endif
			pp->blackpixel = MI_BLACK_PIXEL(mi);
			pp->whitepixel = MI_WHITE_PIXEL(mi);
			pp->cmap = XCreateColormap(display, window,
					MI_VISUAL(mi), AllocNone);
			XSetWindowColormap(display, window, pp->cmap);
			(void) XParseColor(display, pp->cmap, "black", &color);
			(void) XAllocColor(display, pp->cmap, &color);
			MI_BLACK_PIXEL(mi) = color.pixel;
			(void) XParseColor(display, pp->cmap, "white", &color);
			(void) XAllocColor(display, pp->cmap, &color);
			MI_WHITE_PIXEL(mi) = color.pixel;
#ifndef STANDALONE
			(void) XParseColor(display, pp->cmap, background, &color);
			(void) XAllocColor(display, pp->cmap, &color);
			MI_BG_PIXEL(mi) = color.pixel;
			(void) XParseColor(display, pp->cmap, foreground, &color);
			(void) XAllocColor(display, pp->cmap, &color);
			MI_FG_PIXEL(mi) = color.pixel;

			(void) XParseColor(display, pp->cmap, "red", &color);
			(void) XAllocColor(display, pp->cmap, &color);
			pp->love = color.pixel;
#endif

			pp->ncolors = PYRO_NCOLORS;

			if((pp->colors = (XColor *) calloc(pp->ncolors,sizeof(XColor))) == NULL) {
				return;
			}
#undef PYRO_RAMP
#ifdef PYRO_RAMP
			/* glitter, huebsch */
			make_color_ramp(
#ifdef STANDALONE
							screen, visual,
#else
							display,
#endif
							pp->cmap,
							45,(double)0.0,(double)0.0,
							0,(double)1.0,(double)1.0,
							pp->colors,
							&pp->ncolors,
							False,
							True,
							False);
#endif

			pp->ncolors = 0;

			for(hue=0; hue < PYRO_NHUES ; ++hue ) {
				int h;

				h = hue ? (360/(PYRO_NHUES-1)) * (hue-1) : 0;

				ncol = PYRO_NSHADES/10;
				make_color_ramp(
#ifdef STANDALONE
								screen, visual,
#else
								display,
#endif
								pp->cmap,
								h,!hue?0.0:(double)atof(getenv("S1") ? getenv("S1") : "0.0"),(double)atof(getenv("B1") ? getenv("B1") : "1.0"),
								h,!hue?0.0:(double)atof(getenv("S2") ? getenv("S2") : "0.8"),(double)atof(getenv("B2") ? getenv("B1") : "1.0"),
								&pp->colors[hue*PYRO_NSHADES],
								&ncol,
								False,
								True,
								False);

#if defined(DEBUG)
				fprintf(stderr, "1Got %d colors\n", ncol);
#endif /* defined(DEBUG) */
				pp->ncolors += ncol;

				ncol = PYRO_NSHADES/10*9;
				make_color_ramp(
#ifdef STANDALONE
								screen, visual,
#else
								display,
#endif
								pp->cmap,
								h,!hue?0.0:(double)atof(getenv("S2") ? getenv("S2") : "0.8"),(double)atof(getenv("B2") ? getenv("B2") : "1.0"),
								h,!hue?0.0:(double)atof(getenv("S3") ? getenv("S3") : "1.0"),(double)atof(getenv("B3") ? getenv("B3") : "0.0"),
								&pp->colors[hue*PYRO_NSHADES+PYRO_NSHADES/10],
								&ncol,
								False,
								True,
								False);

#if defined(DEBUG)
				fprintf(stderr, "2Got %d colors\n", ncol);
#endif /* defined(DEBUG) */
				pp->ncolors += ncol;
			}


			/*
			make_color_path(display, pp->cmap,
						PYRO_NHUES * PYRO_NPOINTS,
						col_h, col_s, col_b,
						pp->colors,
						&pp->ncolors,
						True,
						False);

			fprintf(stderr, "Got %d colors\n", pp->ncolors);
			*/
		}

		if ((pp->gc = XCreateGC(display, MI_WINDOW(mi), (unsigned long) 0, (XGCValues *) NULL)) == None) return;
	}

	/* Clear Display */
	MI_CLEARWINDOW(mi);

	if(pyro) {
		for(i=0;i<pyrocnt;++i)
			if(pyro[i].bitmap)	{
				free(pyro[i].bitmap->phi);
				free(pyro[i].bitmap->v0);
				free(pyro[i].bitmap);
			}

		free(pyro);
	}

#if !defined( __VMS ) && !defined( WIN32 )
	if (!modparam_msg || !*modparam_msg) {
		struct utsname uts;

		if (uname(&uts) < 0) {
			ostext = (char *) strdup("uname() failed");
		} else {
			if ((ostext = (char *) malloc(strlen(uts.sysname) +
					31)) != NULL)
				(void) sprintf(ostext,
					"&0&1&2&3&4&5&6&7&8&9&a&b %s &a&b",
					uts.sysname);
		}
	}
#endif
	if (!modparam_msg || !*modparam_msg) {
		if (ostext == NULL)
			rov = (char *) DEFAULT_MSG;
		else
			rov = ostext;
	} else
		rov = modparam_msg;
	pyrocnt = 0;
	text_in_buffer = FALSE;

	while(*rov)	{
		switch(*rov)	{
			case '&':		/* internal explosion type */
				rov += 2;
				if(text_in_buffer == TRUE)	++pyrocnt;
				text_in_buffer = FALSE;
				++pyrocnt;
				break;
			case '#':
				++rov;
				if(text_in_buffer == TRUE)	++pyrocnt;
				text_in_buffer = FALSE;
				++pyrocnt;
				break;
			case ' ':
				++rov;
				break;
			default:

				if(*rov == ' ' && text_in_buffer == FALSE)
					/*++rov;*/
					break;

				if(*rov == '\\')
					switch(rov[1])	{
						case '&':
						case '\\':
						case '#':
						case ' ':
							++rov;
							break;
						default:
							break;
				}
				text_in_buffer = TRUE;
				++rov;
				break;
		}
	}
	if(text_in_buffer)	++pyrocnt;

	pyro = (PYRO *) calloc(pyrocnt, sizeof(PYRO));
	if (!modparam_msg || !*modparam_msg) {
		if (ostext == NULL)
			rov = (char*) DEFAULT_MSG;
		else
			rov = ostext;
	} else
		rov = modparam_msg;
	text_in_buffer = FALSE;
	i = 0;

	while(*rov)	{
		switch(*rov)	{
			case '&':	{
				static char buf[8];
				int type = 0;
				if(text_in_buffer == TRUE)	++i;
				text_in_buffer = FALSE;

				memset(buf, 0, sizeof(buf));
				++rov;
				*buf = *rov++;
				if(*buf >= '0' && *buf <= '9')	type = *buf - '0';
				if(*buf >= 'a' && *buf <= 'z')	type = *buf - 'a' + 10;
				if(*buf >= 'A' && *buf <= 'Z')	type = *buf - 'A' + 10;
				pyro[i].etype = type;
				++i;
			}
			break;
#if !defined(STANDALONE) && !defined(WIN32)
			case '#':	{	/* logo */
				XImage *image;
				int graphics_format;
				Colormap cmap;
				unsigned long black;
				LOGO_BITMAP *bitmap;

				if(text_in_buffer == TRUE)	++i;
				text_in_buffer = FALSE;

				getImage(mi, &image, LOGO_WIDTH, LOGO_HEIGHT, LOGO_BITS,
#ifdef HAVE_XPM
				DEFAULT_XPM, LOGO_NAME,
#endif
				&graphics_format, &cmap, &black);
				bitmap = (LOGO_BITMAP *) calloc(1, sizeof(LOGO_BITMAP));
				XImage2bitmap(image, bitmap, 0);
				pyro[i].bitmap = bitmap;
				pyro[i].etype = 23;
				++i;
				++rov;

				break;
			}
#endif
			default:	{
				uint32 charcode = 0;

				if(*rov == ' ' && text_in_buffer == FALSE)	{
					++rov;
					break;
				}

				if(*rov == '\\')
					switch(rov[1])	{
						case '&':
						case '\\':
						case ' ':
							++rov;
							break;
						case 'n':
							++rov;
							++rov;
							charcode = '\n';
							break;
						default:
							/*++rov;*/
							break;
					}

				if(!charcode)
					charcode = ((unsigned char)*rov++) & 0xFF;
				/*if (pyro[i].textlen >= TEXT_LEN - 1)
					pyro[i].textlen = 0;
				else*/
					pyro[i].textlen++;
				pyro[i].text[pyro[i].textlen] = charcode;
				pyro[i].etype = 42;
				text_in_buffer = TRUE;
			}
			break;
		}
	}

	for(i=0;i<pyrocnt;++i)	{
#ifndef WIN32
		if(pyro[i].textlen)
			pyro[i].bitmap = text2bitmap(mi, pyro[i].text);
#endif

		pyro[i].stat = WAIT;
		pyro[i].wait = i * 100;

		pyro[i].color1 = NRAND(PYRO_NHUES)*PYRO_NSHADES; /* offset into pp->colors */
		pyro[i].color2 = getenv("MULTICOL") ? NRAND(PYRO_NHUES)*PYRO_NSHADES : -1; /* offset into pp->colors */
	}
}

ENTRYPOINT void
draw_pyro2(ModeInfo * mi)
{
	Display    *display = MI_DISPLAY(mi);
	GC          gc = MI_GC(mi);
	Window      window = MI_WINDOW(mi);
	int i;

	pyrostruct *pp = &pyros[MI_SCREEN(mi)];

#if 1

	if(getenv("COLTEST")) {
#if defined(DEBUG)
		fprintf(stderr, "red   = %08lX\n", MI_RED_MASK(mi));
		fprintf(stderr, "green = %08lX\n", MI_GREEN_MASK(mi));
		fprintf(stderr, "blue  = %08lX\n", MI_BLUE_MASK(mi));
#endif /* defined(DEBUG) */

		for(i=0;i<MI_WIDTH(mi);++i)	{
			XSetForeground(display, gc, pp->colors[i / (MI_WIDTH(mi)/pp->ncolors + 1)].pixel);
			XDrawLine(display, window, gc, i, 0, i, MI_HEIGHT(mi));
		}

		return;
	}
#endif

	if(!calc(mi, pyro)) init_pyro2(mi);
}

#ifndef STANDALONE
ENTRYPOINT void
refresh_pyro2(ModeInfo * mi)
{
	if (MI_IS_INSTALL(mi) && MI_IS_USE3D(mi)) {
		MI_CLEARWINDOWCOLOR(mi, MI_NONE_COLOR(mi));
	} else {
		MI_CLEARWINDOW(mi);
	}
}
#endif

XSCREENSAVER_MODULE ("Pyro2", pyro2)

#endif /* MODE_pyro2 */
