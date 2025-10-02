/* -*- Mode: C; tab-width: 4 -*- */
/* image --- image bouncer */

#if 0
static const char sccsid[] = "@(#)image.c	5.00 2000/11/01 xlockmore";

#endif

/*-
 * Copyright (c) 1991 by Patrick J. Naughton.
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
 * 01-Nov-2000: Allocation checks
 * 10-May-1997: Compatible with xscreensaver
 * 03-Nov-1995: Patched to add an arbitrary xpm file.
 * 21-Sep-1995: Patch if xpm fails to load <Markus.Zellner@anu.edu.au>.
 * 17-Jun-1995: Pixmap stuff of Skip_Burrell@sterling.com added.
 * 07-Dec-1994: Icons are now better centered if do not exactly fill an area.
 * 29-Jul-1990: Written.
 */

#ifdef STANDALONE
#define MODE_image
#define DEFAULTS "*delay: 2000000 \n" \
	"*count: -10 \n" \
	"*ncolors: 64 \n" \
	"*bitmap: \n" \

# define free_image 0
# define reshape_image 0
# define image_handle_event 0
# define UNIFORM_COLORS
#include "xlockmore.h"		/* in xscreensaver distribution */
static XImage blogo =
{
	0, 0,			/* width, height */
	0, XYBitmap, 0,		/* xoffset, format, data */
	LSBFirst, 8,		/* byte-order, bitmap-unit */
	LSBFirst, 8, 1		/* bitmap-bit-order, bitmap-pad, depth */
};
#else /* STANDALONE */
#include "xlock.h"		/* in xlockmore distribution */
#include "color.h"
#include "iostuff.h"
#endif /* STANDALONE */

#ifdef MODE_image
#define DEF_ICONONLY "FALSE"

static Bool icononly;

static XrmOptionDescRec opts[] =
{
	{(char *) "-icononly", (char *) ".image.icononly", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+icononly", (char *) ".image.icononly", XrmoptionNoArg, (caddr_t) "off"},
};
static argtype vars[] =
{
	{(void *) & icononly, (char *) "icononly", (char *) "IconOnly", (char *) DEF_ICONONLY, t_Bool},
};
static OptionStruct desc[] =
{
	{(char *) "-/+icononly", (char *) "turn on/off drawing only to password window"},
};

ENTRYPOINT ModeSpecOpt image_opts =
{sizeof opts / sizeof opts[0], opts, sizeof vars / sizeof vars[0], vars, desc};

#ifdef USE_MODULES
ModStruct image_description =
{"image", "init_image", "draw_image", "release_image",
 "refresh_image", "init_image", (char *) NULL, &image_opts,
 2000000, -10, 1, 1, 64, 1.0, "",
 "Shows randomly appearing logos", 0, NULL};

#endif

/* aliases for vars defined in the bitmap file */
#ifdef STANDALONE
#define IMAGE_WIDTH	xscreensaver_width
#define IMAGE_HEIGHT	xscreensaver_height
#define IMAGE_BITS	xscreensaver_bits
#include "bitmaps/xscreensaver.xbm"
#else
#define IMAGE_WIDTH	image_width
#define IMAGE_HEIGHT	image_height
#define IMAGE_BITS	image_bits
#include "image.xbm"
#endif

#ifdef HAVE_XPM
#ifdef STANDALONE
#include <X11/xpm.h>
#define IMAGE_NAME	xscreensaver
#include "pixmaps/xscreensaver.xpm"
#else
#define IMAGE_NAME	image_name
#include "image.xpm"
#endif
#define DEFAULT_XPM 1
#endif

#define MINICONS 1

#ifdef USE_MB
static XFontSet mode_font = None;
static int getFontHeight(XFontSet f)
{
	XRectangle ink, log;

	if (f == None) {
		return 8;
	} else {
		XmbTextExtents(mode_font, "My", strlen("My"), &ink, &log);
		return log.height;
	}
}
static int getTextWidth(char *string)
{
	XRectangle ink, logical;

	XmbTextExtents(mode_font, string, strlen(string), &ink, &logical);
	return logical.width;
}
#define DELTA 0.2
extern XFontSet getFontSet(Display * display);
#define DrawString(d, p, gc, start, ascent, string, len) (void) XmbDrawString(\
	d, p, mode_font, gc, start, ascent, string, len)
#else
static XFontStruct *mode_font = None;
#define getFontHeight(f) ((f == None) ? 8 : f->ascent + f->descent)
static int getTextWidth(char *string)
{
	return XTextWidth(mode_font, string, strlen(string));
}
#define DELTA 0
extern XFontStruct *getFont(Display * display);
#define DrawString(d, p, gc, start, ascent, string, len) (void) XDrawString(\
	d, p, gc, start, ascent, string, len)
#endif

#if 0
#define MAXWIDTH BUFSIZ
#else
#define MAXWIDTH 170
#endif
#define STRSIZE BUFSIZ
#define MAXLINES 40

typedef struct {
	int         x, y;
	int         color;
} imagetype;

typedef struct {
	int         width, height;
	int         nrows, ncols;
	XPoint      image_offset;
	XPoint      border;
	int         iconcount;
	imagetype  *icons;
	int         graphics_format;
	Pixmap      pixmap;
	GC          fgGC, bgGC;
	int         pixw, pixh;
	XImage     *logo;
	Colormap    cmap;
	unsigned long black;
	int         lines;
	char        strnew[MAXLINES][STRSIZE];
	short       textWidth[MAXLINES], textStart[MAXLINES];
	short       text_height, text_ascent, text_descent;
} imagestruct;

static imagestruct *ims = (imagestruct *) NULL;

extern char *message;

#ifdef STANDALONE
char * message = NULL;
#endif

static void
freeLogo(Display * display, imagestruct * ip)
{
	if (ip->cmap != None) {
		XFreeColormap(display, ip->cmap);
		if (ip->bgGC != None) {
			XFreeGC(display, ip->bgGC);
			ip->bgGC = None;
		}
		ip->cmap = None;
	} else
		ip->bgGC = None;
}

static void
free_image_screen(Display * display, imagestruct * ip)
{
	if (ip == NULL) {
		return;
	}
	if (ip->icons != NULL) {
		free(ip->icons);
		ip->icons = (imagetype *) NULL;
	}
	freeLogo(display, ip);
#ifndef STANDALONE
	if (ip->logo != None) {
		destroyImage(&ip->logo, &ip->graphics_format);
		ip->logo = None;
	}
#endif
	if (ip->fgGC != None) {
		XFreeGC(display, ip->fgGC);
		ip->fgGC = None;
	}
	if (ip->bgGC) {
		XFreeGC(display, ip->bgGC);
		ip->bgGC = None;
	}
	if (ip->pixmap) {
		XFreePixmap(display, ip->pixmap);
		ip->pixmap = None;
	}
	ip = NULL;
}

static Bool
initLogo(ModeInfo * mi)
{
	imagestruct *ip = &ims[MI_SCREEN(mi)];
#ifdef STANDALONE
#ifdef HAVE_XPM
	XpmAttributes attrib;
	attrib.visual = MI_VISUAL(mi);
	attrib.colormap = MI_COLORMAP(mi);
	attrib.depth = MI_DEPTH(mi);
	attrib.valuemask = XpmVisual | XpmColormap | XpmDepth;
	if (MI_NPIXELS(mi) > 2 && 
		(XpmSuccess == XpmCreateImageFromData(MI_DISPLAY(mi),
			IMAGE_NAME, &(ip->logo), (XImage **) NULL, &attrib))) {
		ip->graphics_format = IS_XPM;
		ip->pixw = ip->logo->width;
		ip->pixh = ip->logo->height;
	} else
#endif
	{
		int default_width = IMAGE_WIDTH;
		int default_height = IMAGE_HEIGHT;
		unsigned char * default_bits = IMAGE_BITS;
		if (!blogo.data) {
			blogo.data = (char *) default_bits;
			blogo.width = default_width;
			blogo.height = default_height;
			blogo.bytes_per_line = (blogo.width + 7) / 8;
		}
		ip->logo = &blogo;
		ip->graphics_format = IS_XBM;
		ip->pixw = ip->logo->width;
		ip->pixh = ip->logo->height;
	}
#else
	Display *display = MI_DISPLAY(mi);

	if (ip->logo == None) {
		getImage(mi, &ip->logo, IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_BITS,
#ifdef HAVE_XPM
			DEFAULT_XPM, IMAGE_NAME,
#endif
			&ip->graphics_format, &ip->cmap, &ip->black);
		if (ip->logo == None) {
			free_image_screen(display, ip);
			return False;
		}
		ip->pixw = ip->logo->width;
		ip->pixh = ip->logo->height;
	}
	if (ip->cmap != None) {
		Window window = MI_WINDOW(mi);
		setColormap(display, window, ip->cmap, MI_IS_INWINDOW(mi));
		if (ip->bgGC == None) {
			XGCValues xgcv;

			xgcv.background = ip->black;
			if ((ip->bgGC = XCreateGC(display, window, GCBackground,
					&xgcv)) == None) {
				free_image_screen(display, ip);
				return False;
			}
		}
	} else
#endif /* STANDALONE */
	{
		ip->black = MI_BLACK_PIXEL(mi);
		ip->bgGC = MI_GC(mi);
	}
	return True;
}

static void
initStrings(ModeInfo * mi)
{
	imagestruct *ip = &ims[MI_SCREEN(mi)];
	char *p, *p2, *p3, buf[BUFSIZ];
	int height, w, i, j;

	p = strncpy(buf, message, BUFSIZ - 1);
	while (strlen(p) > 0) {
		p2 = p + strlen(p) - 1;
		if (*p2 == '\n')
			*p2 = 0;
		else
			break;
	}
	for (height = 0; p && height < MAXLINES; height++) {
		if (!(p2 = (char *) strchr(p, '\n')) ||
				!p2[1]) {
			p2 = p + strlen(p);
		}
		/* p2 now points to the first '\n' */
		*p2 = 0;
		for (w = 0; w < p2 - p; w++) {
			p3 = p + w;
			if (*p3 == '\t') /* this should be improved */
				*p3 = ' ';
		}
		(void) strncpy(ip->strnew[height], p,
			MAXWIDTH - 1);
		ip->strnew[height][MAXWIDTH - 1] = '\0';
		if (p == p2)
			break;
		p = p2 + 1;
	}
	ip->lines = height;
	j = 0;
	for (i = 0; i < ip->lines; i++) {
		if (ip->strnew[i] && *(ip->strnew[i])) {
			ip->textWidth[i] = getTextWidth(ip->strnew[i]) +
				getTextWidth((char *) "I");
			/* add a little more to give some border room */
			if (ip->textWidth[i] > ip->textWidth[j]) {
				j = i;
			}
		}
		ip->pixw = ip->textWidth[j];
	}
	for (i = 0; i < ip->lines; i++) {
		ip->textStart[i] = (ip->pixw - ip->textWidth[i]) / 2;
	}
	ip->text_height = getFontHeight(mode_font);
	ip->pixh = ip->lines * ip->text_height;
}

static void
drawImages(ModeInfo * mi)
{
	Display *display = MI_DISPLAY(mi);
	Window window = MI_WINDOW(mi);
	imagestruct *ip = &ims[MI_SCREEN(mi)];
	GC gc = (message && *message) ? MI_GC(mi) : ip->bgGC;
	int i;

	MI_CLEARWINDOWCOLORMAPFAST(mi, gc, ip->black);
	for (i = 0; i < ip->iconcount; i++) {
	  if (ip->icons[i].x >= 0) {
	    if (message && *message) {
		if (MI_NPIXELS(mi) > 2)
			XSetForeground(display, gc,
				MI_PIXEL(mi, ip->icons[i].color));
		else
			XSetForeground(display, gc, MI_WHITE_PIXEL(mi));
		XCopyPlane(display, ip->pixmap, window, gc,
	                0, 0,
			ip->pixw, ip->pixh,
			ip->pixw * ip->icons[i].x + ip->image_offset.x,
			ip->pixh * ip->icons[i].y + ip->image_offset.y,
			1L);
	    } else if  (ip->logo != NULL) {
		if (MI_NPIXELS(mi) <= 2)
			XSetForeground(display, gc, MI_WHITE_PIXEL(mi));
		else if (ip->graphics_format < IS_XPM)
			XSetForeground(display, gc,
				MI_PIXEL(mi, ip->icons[i].color));
		(void) XPutImage(display, window, gc, ip->logo, 0, 0,
			ip->pixw * ip->icons[i].x + ip->image_offset.x,
			ip->pixh * ip->icons[i].y + ip->image_offset.y,
			ip->pixw, ip->pixh);
	    }
	  }
	}
}

ENTRYPOINT void
draw_image(ModeInfo * mi)
{
	Display *display = MI_DISPLAY(mi);
	Window window = MI_WINDOW(mi);
	int i;
	GC gc;
	imagestruct *ip;

	if (ims == NULL)
		return;
	ip = &ims[MI_SCREEN(mi)];
	if (ip->icons == NULL)
		return;
	if (icononly && !MI_IS_ICONIC(mi))
		return;
	gc = (message && *message) ? MI_GC(mi) : ip->bgGC;

	MI_IS_DRAWN(mi) = True;
	XSetForeground(display, gc, ip->black);
	if (ip->border.x < 0)
		ip->image_offset.x = -NRAND(-ip->border.x);
	else if (ip->border.x > 0)
		ip->image_offset.x = NRAND(ip->border.x);
	else
		ip->image_offset.x = 0;
	if (ip->border.y < 0)
		ip->image_offset.y = -NRAND(-ip->border.y);
	else if (ip->border.y > 0)
		ip->image_offset.y = NRAND(ip->border.y);
	else
		ip->image_offset.y = 0;
	for (i = 0; i < ip->iconcount; i++) {
		ip->icons[i].x = NRAND(ip->ncols);
		ip->icons[i].y = NRAND(ip->nrows);
		ip->icons[i].color = NRAND(MI_NPIXELS(mi));
		if (ip->ncols * ip->nrows > ip->iconcount &&
				ip->icons[i].x >= 0)
			XFillRectangle(display, window, gc,
				ip->pixw * ip->icons[i].x + ip->image_offset.x,
				ip->pixh * ip->icons[i].y + ip->image_offset.y,
				ip->pixw, ip->pixh);
	}
	drawImages(mi);
}

ENTRYPOINT void
init_image(ModeInfo * mi)
{
	Display *display = MI_DISPLAY(mi);
	Window window = MI_WINDOW(mi);
	imagestruct *ip;
	int i;

	MI_INIT(mi, ims);
	ip = &ims[MI_SCREEN(mi)];
	if (message && *message) {
		XGCValues gcv;

#ifdef USE_MB
		mode_font = getFontSet(display);
		ip->text_descent = 0;
		ip->text_ascent = getFontHeight(mode_font);
#else
		mode_font = getFont(display);
		ip->text_descent = mode_font->descent;
		ip->text_ascent = mode_font->ascent;
#endif
		if (mode_font == None) {
			return;
		}
		initStrings(mi);
		ip->black = MI_BLACK_PIXEL(mi);

		free_image_screen(display, ip);
		ip->pixh = (ip->lines + DELTA) * ip->text_height;
		if ((ip->pixmap = XCreatePixmap(display, window,
				ip->pixw, ip->pixh, 1)) == None) {
			free_image_screen(display, ip);
			ip->pixw = 0;
			ip->pixh = 0;
			return;
		}
#ifndef USE_MB
		gcv.font = mode_font->fid;
#endif
		gcv.background = 0;
		gcv.foreground = 1;
		gcv.graphics_exposures = False;
		if ((ip->fgGC = XCreateGC(display, ip->pixmap,
				GCForeground | GCBackground | GCGraphicsExposures
#ifndef USE_MB
				| GCFont
#endif
				, &gcv)) == None) {
			free_image_screen(display, ip);
			ip->pixw = 0;
			ip->pixh = 0;
			return;
		}
		gcv.foreground = 0;
		if ((ip->bgGC = XCreateGC(display, ip->pixmap,
				GCForeground | GCBackground | GCGraphicsExposures
#ifndef USE_MB
				| GCFont
#endif
				, &gcv)) == None) {
			free_image_screen(display, ip);
			ip->pixw = 0;
			ip->pixh = 0;
			return;
		}
		XFillRectangle(display, ip->pixmap, ip->bgGC,
			0, 0, ip->pixw, ip->pixh);
		XSetForeground(display, MI_GC(mi), MI_WHITE_PIXEL(mi));
		for (i = 0; i < ip->lines; i++) {
			DrawString(display, ip->pixmap, ip->fgGC,
				ip->textStart[i],
				ip->text_ascent + i * ip->text_height,
				ip->strnew[i], strlen(ip->strnew[i]));
		}
		/* don't want any exposure events from XCopyPlane */
		XSetGraphicsExposures(display, MI_GC(mi), False);
	} else if (!initLogo(mi))
		return;

	ip->width = MI_WIDTH(mi);
	ip->height = MI_HEIGHT(mi);
	if (ip->width > ip->pixw && ip->pixw != 0)
		ip->ncols = ip->width / ip->pixw;
	else
		ip->ncols = 1;
	if (ip->height > ip->pixh && ip->pixh != 0)
		ip->nrows = ip->height / ip->pixh;
	else
		ip->nrows = 1;
	ip->border.x = ip->width - ip->ncols * ip->pixw;
	ip->border.y = ip->height - ip->nrows * ip->pixh;
	ip->iconcount = MI_COUNT(mi);
	if (ip->iconcount < -MINICONS)
		ip->iconcount = NRAND(-ip->iconcount - MINICONS + 1) + MINICONS;
	else if (ip->iconcount < MINICONS)
		ip->iconcount = MINICONS;
	if (ip->iconcount > ip->ncols * ip->nrows)
		ip->iconcount = ip->ncols * ip->nrows;
	if (ip->icons != NULL)
		free(ip->icons);
	if ((ip->icons = (imagetype *) malloc(ip->iconcount *
			sizeof (imagetype))) == NULL) {
		free_image_screen(display, ip);
		return;
	}
	for (i = 0; i < ip->iconcount; i++)
		ip->icons[i].x = -1;
	if (!(message && *message)) {
		MI_CLEARWINDOWCOLORMAP(mi, ip->bgGC, ip->black);
	}
	draw_image(mi);
}

ENTRYPOINT void
release_image(ModeInfo * mi)
{
	if (ims != NULL) {
		int screen;

		for (screen = 0; screen < MI_NUM_SCREENS(mi); screen++)
			free_image_screen(MI_DISPLAY(mi), &ims[screen]);
		free(ims);
		ims = (imagestruct *) NULL;
	}
}

#ifndef STANDALONE
ENTRYPOINT void
refresh_image(ModeInfo * mi)
{
#ifdef HAVE_XPM
	imagestruct *ip;

	if (ims == NULL)
		return;
	ip = &ims[MI_SCREEN(mi)];
	if (ip->icons == NULL)
		return;
	if (ip->graphics_format >= IS_XPM) {
		/* This is needed when another program changes the colormap. */
		free_image_screen(MI_DISPLAY(mi), ip);
		init_image(mi);
		return;
	}
#endif
	drawImages(mi);
}
#endif

XSCREENSAVER_MODULE ("Image", image)

#endif /* MODE_image */
