/* -*- Mode: C; tab-width: 4 -*- */
/* marquee --- types a text-file or a text ribbon */

#if 0
static const char sccsid[] = "@(#)marquee.c	5.00 2000/11/01 xlockmore";

#endif

/*-
 * Copyright (c) 1995 by Tobias Gloth and David Bagley
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
 * 03-Nov-1995: Many changes (hopefully good ones) by David Bagley
 * 01-Oct-1995: Written by Tobias Gloth
 */

#ifdef STANDALONE
#define MODE_marquee
#define DEFAULTS "*delay: 100000 \n" \
	"*ncolors: 64 \n" \
	"*font: fixed\n" \
	"*text:\n" \
	"*filename: \n" \
	"*fortunefile: \n" \
	"*program: \n" \

# define free_marquee 0
# define reshape_marquee 0
# define marquee_handle_event 0
#define SMOOTH_COLORS
#include "xlockmore.h"		/* in xscreensaver distribution */
#else /* STANDALONE */
#include "xlock.h"		/* in xlockmore distribution */
#include "iostuff.h"
#endif /* STANDALONE */

#ifdef MODE_marquee

ENTRYPOINT ModeSpecOpt marquee_opts =
{0, (XrmOptionDescRec *) NULL, 0, (argtype *) NULL, (OptionStruct *) NULL};

#ifdef USE_MODULES
ModStruct   marquee_description =
{"marquee", "init_marquee", "draw_marquee", "release_marquee",
 "init_marquee", "init_marquee", (char *) NULL, &marquee_opts,
 100000, 1, 1, 1, 64, 1.0, "",
 "Shows messages", 0, NULL};

#endif

#ifdef USE_MB
static int font_height(XFontStruct *f)
{
	XRectangle mbRect;

	XmbTextExtents(fontset, "Aj", 1, NULL, &mbRect);
	return mbRect.height;
}
#else
#define font_height(f) ((f==None)?8:f->ascent + f->descent)
#endif

extern XFontStruct *getFont(Display * display);
extern char *getWords(int screen, int screens);
extern int  isRibbon(void);

typedef struct {
	int         ascent;
	int         height;
	int         win_height;
	int         win_width;
	int         x;
	int         y;
	int         t;
	int         startx;
	int         nonblanks;
	int         color;
	int         time;
	GC          gc;
	char       *words;
	char        modwords[256];
} marqueestruct;

static marqueestruct *marquees = (marqueestruct *) NULL;

static XFontStruct *mode_font = None;
static int  char_width[256];

static int
font_width(XFontStruct * font, char ch)
{
	int         dummy;
	XCharStruct xcs;

	(void) XTextExtents(font, &ch, 1, &dummy, &dummy, &dummy, &xcs);
	return xcs.width;
}

/* returns 1 if c is a printable char, the test should work for most 8 * bit
   charsets (e.g. latin1), if we would use isprint, we would * depend of
   locale settings that are probably incorrect. */

static int
is_valid_char(char c)
{
	return (unsigned char) c >= ' ';
}

static int
is_char_back_char(char *s)
{
	return is_valid_char(*s) && *(s + 1) == '\b' &&
		*(s + 2) && is_valid_char(*(s + 2));
}

static int
char_back_char_width(char *s)
{
	int         w1 = char_width[(int) (unsigned char) *s];
	int         w2 = char_width[(int) (unsigned char) *(s + 2)];

	return w2 < w1 ? w1 : w2;
}


/*-
 * fix strings of the form abc^H^H^H123 to a^H1b^H2c^H3, since we only
 * handle backspace for char, back, char correctly. We do this without
 * duplicating the string, but I'm not sure if there are conditions
 * when the string is actually const. (when it is def_message, no ^Hs
 * are present, I'm not sure about resource strings)
 */

static char *
fixup_back(char *s)
{
	char       *p, *p1, *p2;
	char        tmp[1000];
	char       *t;
	char       *w;

	/* first of all, check if we have to do anything */

	if (!*s)
		return s;

	for (p = s + 1; *p; p++)
		if (*p == '\b' && *(p + 1) == '\b')
			break;
	if (!*p)
		return s;

	/* now search for runs of the form char*n, back*n, char. */

	for (p = s; *p; p++) {
		for (p1 = p; *p1 && is_valid_char(*p1); p1++);
		if (*p1 == '\b') {
			for (p2 = p1; *p2 && *p2 == '\b'; p2++);

			/* do we have `enough' chars for the backspaces? */

			if (p2 - p1 > 1 && p1 - p >= p2 - p1) {
				if (p1 - p > p2 - p1) {
					p = p1 - (p2 - p1);
				}
				/* the situation is as follows:
				   p points to the first char,
				   p1 to the first backspace (end first char run),
				   p2 to the first char in the 2nd run

				   Question: how to do that without tmp storage?
				 */
				(void) strncpy(tmp, p, p1 - p);
				t = tmp;
				w = p;
				while (t - tmp < p1 - p && *p2) {
					*w++ = *t++;
					*w++ = '\b';
					*w++ = *p2++;
				}
				p = p2;
			} else {
				p = p2;
			}
		} else {
			/* we hit some other control char, just continue at this
			   position */
			p = p1;
		}
	}
	return s;
}

static int
charBytes(unsigned char cb)
{
#ifdef USE_MB
	int mb = 1;

	/*if ((cb & 0x80) && !(cb & 0x40))
		an extended byte*/
	if ((cb & 0x80) && (cb & 0x40))
		mb = 2;
	else
		return mb;
	if (cb & 0x20)
		mb = 3;
	else
		return mb;
	if (cb & 0x10)
		mb = 4;
	else
		return mb;
	if (cb & 0x08)
		mb = 5;
	else
		return mb;
	if (cb & 0x04)
		return 6;
	else
		return mb;
#else
	return 1;
#endif
}

static int
charWidth(char * string)
{
#ifdef USE_MB
	XRectangle  rect;
	int mb = charBytes(*string);

	XmbTextExtents(fontset, string, mb, NULL, &rect);
	return rect.width;
#else
	return char_width[(int) (unsigned char) *string];
#endif
}

static int
text_font_width(char *string)
{
	int         n = 0, x = 0, t = 0;

	/* The following does not handle a tab or other weird junk */
	while (*string != '\0') {
		if (x > n)
			n = x;
		switch (*string) {
			case '\v':
			case '\f':
			case '\n':
				x = 0;
				t = 0;
				break;
			case '\b':
				/* we handle only char, ^H, char smartly, if
				 * we have something different, we use the
				 * (probably wrong) assumption that we have
				 * a monospaced font. */
				if (t) {
					t--;
					x -= char_width[(int) (' ')];
				}
				break;
			case '\t':
				x += char_width[(int) (' ')] * (8 - (t % 8));
				t = ((t + 8) / 8) * 8;
				break;
			case '\r':
				break;
			default:
				t++;
				/* handle char, ^H, char */
				if (is_char_back_char(string)) {
					x += char_back_char_width(string);
					string += 2;
				} else {
					x += charWidth(string);
				}
		}
		string++;
	}
	return n;
}

static int
text_height(char *string)
{
	int         n = 0;

	while (*string != '\0') {
		if ((*string == '\n') || (*string == '\f') || (*string == '\v'))
			n++;
		string++;
	}
	return n;
}

static int
add_blanks(marqueestruct * mp)
{
	int width;

	if (mp->t < 251) {
		mp->modwords[mp->t] = ' ';
		mp->t++;
		mp->modwords[mp->t] = ' ';
		mp->t++;
		mp->modwords[mp->t] = '\0';
		(void) strcat(mp->modwords, "  ");
	}
	mp->x -= 2 * char_width[(int) (' ')];
	width = charWidth(mp->modwords);
	if (mp->x <= -width) {
		mp->x += width;
		(void) memcpy(mp->modwords, &(mp->modwords[1]), mp->nonblanks);
		mp->nonblanks--;
	}
	return (mp->nonblanks < 0);
}

static void
add_letter(marqueestruct * mp, char * letter)
{
	int width;
	int b = charBytes(letter[0]);
	int i;

	if (mp->t < 253 - b) {
		for (i = 0; i < b; i++) {
			mp->modwords[mp->t] = letter[i];
			mp->t++;
		}
		mp->modwords[mp->t] = '\0';
		(void) strcat(mp->modwords, "  ");
	}
	mp->x -= charWidth(letter);
	width = charWidth(mp->modwords);
	if (mp->x <= -width) {
		mp->x += width;
		(void) memcpy(mp->modwords, &(mp->modwords[1]), mp->t);
		for (i = 0; i < b; i++) {
			mp->modwords[mp->t] = ' ';
			mp->t--;
		}
	} else
		mp->nonblanks = mp->t;
}

static void
free_marquee_screen(Display * display, marqueestruct * mp)
{
	if (mp == NULL) {
		return;
	}
	if (mp->gc != None)
		XFreeGC(display, mp->gc);
	mp = NULL;
}

ENTRYPOINT void
init_marquee(ModeInfo * mi)
{
	Display    *display = MI_DISPLAY(mi);
	marqueestruct *mp;
	XGCValues   gcv;
	int         i;

	MI_INIT(mi, marquees);
	mp = &marquees[MI_SCREEN(mi)];

	mp->win_width = MI_WIDTH(mi);
	mp->win_height = MI_HEIGHT(mi);
	if (MI_NPIXELS(mi) > 2)
		mp->color = NRAND(MI_NPIXELS(mi));
	mp->time = 0;
	mp->t = 0;
	mp->nonblanks = 0;
	mp->x = 0;

	MI_CLEARWINDOW(mi);

	if (mode_font == None)
		mode_font = getFont(display);
	if (mp->gc == NULL && mode_font != None) {
		gcv.font = mode_font->fid;
		gcv.graphics_exposures = False;
		gcv.foreground = MI_WHITE_PIXEL(mi);
		gcv.background = MI_BLACK_PIXEL(mi);
		if ((mp->gc = XCreateGC(display, MI_WINDOW(mi),
				GCForeground | GCBackground | GCGraphicsExposures | GCFont,
				&gcv)) == None) {
			return;
		}
		mp->ascent = mode_font->ascent;
		mp->height = font_height(mode_font);
		for (i = 0; i < 256; i++)
			if ((i >= (int) mode_font->min_char_or_byte2) &&
			    (i <= (int) mode_font->max_char_or_byte2))
				char_width[i] = font_width(mode_font, (char) i);
			else
				char_width[i] = font_width(mode_font, (char) mode_font->default_char);
	} else if (mode_font == None) {
		for (i = 0; i < 256; i++)
			char_width[i] = 8;
	}
	mp->words = fixup_back(getWords(MI_SCREEN(mi), MI_NUM_SCREENS(mi)));
	mp->y = 0;

	if (isRibbon()) {
		mp->x = mp->win_width;
		if (mp->win_height > font_height(mode_font))
			mp->y += NRAND(mp->win_height - font_height(mode_font));
		else if (mp->win_height < font_height(mode_font))
			mp->y -= NRAND(font_height(mode_font) - mp->win_height);
	} else {
		int         text_ht = text_height(mp->words);
		int         text_font_wid = text_font_width(mp->words);

		if (mp->win_height > text_ht * font_height(mode_font))
			mp->y = NRAND(mp->win_height - text_ht * font_height(mode_font));
		if (mp->y < 0)
			mp->y = 0;
		mp->x = 0;
		if (mp->win_width > text_font_wid)
			mp->x += NRAND(mp->win_width - text_font_wid);
		/* else if (mp->win_width < text_font_wid)
		   mp->x -= NRAND(text_font_wid - mp->win_width); */
		mp->startx = mp->x;
	}
}

ENTRYPOINT void
draw_marquee(ModeInfo * mi)
{
	Display    *display = MI_DISPLAY(mi);
	Window      window = MI_WINDOW(mi);
	char       *space = (char *) "        ";
	unsigned char       *ch;
	marqueestruct *mp = &marquees[MI_SCREEN(mi)];

	if (marquees == NULL)
		return;
	mp = &marquees[MI_SCREEN(mi)];
	if (mp->gc == None && mode_font != None)
		return;

	MI_IS_DRAWN(mi) = True;
	ch = (unsigned char*) mp->words;
	if (isRibbon()) {
		ch = (unsigned char*) mp->words;
		switch (*ch) {
			case '\0':
				if (add_blanks(mp)) {
					init_marquee(mi);
					return;
				}
				break;
			case '\b':
			case '\r':
			case '\n':
			case '\t':
			case '\v':
			case '\f':
				add_letter(mp, (char *) " ");
				mp->words++;
				break;
			default:
				add_letter(mp, (char *) ch);
				mp->words++;
		}
		if (MI_NPIXELS(mi) > 2) {
			XSetForeground(display, mp->gc, MI_PIXEL(mi, mp->color));
			if (++mp->color == MI_NPIXELS(mi))
				mp->color = 0;
		} else
			XSetForeground(display, mp->gc, MI_WHITE_PIXEL(mi));
		(void) XDrawImageString(display, MI_WINDOW(mi), mp->gc,
			 mp->x, mp->y + mp->ascent, mp->modwords, mp->t + 2);
	} else {
		switch (*ch) {
			case '\0':
				if (++mp->time > 16)
					init_marquee(mi);
				return;
			case '\b':
				if (mp->t) {
					/* see note in text_font_width */
					mp->t--;
					mp->x -= char_width[(int) (' ')];
				}
				break;
			case '\v':
			case '\f':
			case '\n':
				mp->x = mp->startx;
				mp->t = 0;
				mp->y += mp->height;
				if (mp->y + mp->height > mp->win_height) {
					XCopyArea(display, window, window, mp->gc,
						  0, mp->height, mp->win_width, mp->y - mp->height, 0, 0);
					XSetForeground(display, mp->gc, MI_BLACK_PIXEL(mi));
					mp->y -= mp->height;
					XFillRectangle(display, window, mp->gc,
					0, mp->y, mp->win_width, mp->height);
				}
				break;
			case '\t':
				(void) XDrawString(display, window, mp->gc, mp->x, mp->y + mp->ascent,
						   space, 8 - (mp->t % 8));
				mp->x += char_width[(int) (' ')] * (8 - (mp->t % 8));
				mp->t = ((mp->t + 8) / 8) * 8;
				break;
			case '\r':
				break;
			default:
				if (MI_NPIXELS(mi) > 2) {
					XSetForeground(display, mp->gc, MI_PIXEL(mi, mp->color));
					if (++mp->color == MI_NPIXELS(mi))
						mp->color = 0;
				} else
					XSetForeground(display, mp->gc, MI_WHITE_PIXEL(mi));
				if (is_char_back_char((char *) ch)) {
					int         xmid = mp->x + (char_back_char_width((char *) ch) + 1) / 2;

					(void) XDrawString(display, window, mp->gc,
							   xmid - char_width[(int) (const char) *ch] / 2,
						mp->y + mp->ascent, (char *) ch, 1);
					(void) XDrawString(display, window, mp->gc,
							   xmid - char_width[(int) (const char) *(ch + 2)] / 2,
						mp->y + mp->ascent, (char *) ch + 2, 1);
					mp->x += char_back_char_width((char *) ch);
					mp->words += 2;
				} else {
					int mb = charBytes(*ch);

					(void) XDrawString(display, window, mp->gc,
						mp->x, mp->y + mp->ascent, (char *) ch, mb);
					mp->x += charWidth((char *) ch);
				}
				mp->t++;
		}
		mp->words += charBytes(*ch);
	}
}

ENTRYPOINT void
release_marquee(ModeInfo * mi)
{
	if (marquees != NULL) {
		int         screen;

		for (screen = 0; screen < MI_NUM_SCREENS(mi); screen++) {
			marqueestruct *mp = &marquees[screen];
			free_marquee_screen(MI_DISPLAY(mi), mp);
		}
		free(marquees);
		marquees = (marqueestruct *) NULL;
	}
	if (mode_font != None) {
		XFreeFont(MI_DISPLAY(mi), mode_font);
		mode_font = None;
	}
}

XSCREENSAVER_MODULE ("Marquee", marquee)

#endif /* MODE_marquee */
