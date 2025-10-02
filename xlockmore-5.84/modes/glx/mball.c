/* -*- Mode: C; tab-width: 4 -*- */
/* oct --- Shows an auto-solving Oct */

#if 0
static const char sccsid[] = "@(#)mball.c	5.78 2024/05/15 xlockmore";
#endif

#undef DEBUG_LISTS

/*-
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
 * This mode shows an auto-solving Masterball puzzle.
 *
 * Thanks goes also to Brian Paul for making it possible and inexpensive
 * to use OpenGL at home.
 *
 * Based on rubik.c by Marcelo F. Vianna
 *
 * Revision History:
 * 23-May-2024: Started writing, using xlock oct.c and MballGL.c from
		xpuzzles.  Available are sizes 2-4.
 */

#ifdef VMS
#include <types.h>
#endif

#ifdef STANDALONE
# define MODE_mball
# define DEFAULTS	"*delay: 100000 \n" \
			"*count: -30 \n" \
			"*cycles: 5 \n" \

# define free_mball 0
# include "xlockmore.h"		/* from the xscreensaver distribution */
# include "gltrackball.h"

#else /* !STANDALONE */
#include "xlock.h"		/* from the xlockmore distribution */
#include "visgl.h"
#endif /* !STANDALONE */

#ifdef MODE_mball
#define glTranslate(x,y,z) glTranslatef((float) x, (float) y, (float) z)

#include <GL/gl.h>
#define DEF_HIDESHUFFLING     "False"
#define DEF_WEDGES     "8"

static Bool hideshuffling;
static int wedges;

static XrmOptionDescRec opts[] =
{
	{(char *) "-hideshuffling", (char *) ".mball.hideshuffling", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+hideshuffling", (char *) ".mball.hideshuffling", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-wedges", (char *) ".mball.wedges", XrmoptionSepArg, (caddr_t) 0}
};

static argtype vars[] =
{
	{(void *) & hideshuffling, (char *) "hideshuffling", (char *) "Hideshuffling", (char *) DEF_HIDESHUFFLING, t_Bool},
	{(void *) & wedges, (char *) "wedges", (char *) "Wedges", (char *) DEF_WEDGES, t_Int}
};

static OptionStruct desc[] =
{
	{(char *) "-/+hideshuffling", (char *) "turn on/off hidden shuffle phase"},
	{(char *) "-wedges", (char *) "one of (2, 4, 6, 8, 10, 12)"}
};

ENTRYPOINT ModeSpecOpt mball_opts =
{sizeof opts / sizeof opts[0], opts, sizeof vars / sizeof vars[0], vars, desc};

#ifdef USE_MODULES
ModStruct   mball_description =
{"mball", "init_mball", "draw_mball", "release_mball",
 "draw_mball", "change_mball", "(char *) NULL", &mball_opts,
 100000, -30, 5, -6, 64, 1.0, "",
 "Shows an auto-solving Masterball puzzle", 0, NULL};

#endif

#define ACTION_SOLVE    1
#define ACTION_SHUFFLE  0

#define DELAY_AFTER_SHUFFLING  5
#define DELAY_AFTER_SOLVING   20

/*************************************************************************/

#define MINSIZE 2

#define Scale4Window	       (0.5)
#define Scale4Iconic	       (1.0)

/* Directions relative to the wedge of a piece */
#define TOP 0
#define TTR 1
#define TR 2
#define RIGHT 3
#define BR 4
#define BBR 5
#define BOTTOM 6
#define BBL 7
#define BL 8
#define LEFT 9
#define TL 10
#define TTL 11
#define COORD 12
#define CW 13
#define CCW 23
#define CUTS (COORD / 2)
#define SAME 0
#define OPPOSITE 1
#define DOWN 0
#define UP 1
#define MIN_WEDGES 2
#define MAX_WEDGES 12
#define MAX_VIEWS 2
#define MAX_SIDES (MAX_WEDGES/MAX_VIEWS)

#define IGNORE_DIR (-1)

#define NO_WEDGE (MAX_WEDGES+1)
#define NO_BAND (-1)
#define NO_ROTATION (2*MAX_ORIENT)

#define ObjCubit	0
#define MaxObj	  1

typedef struct _MballLoc {
	int	 wedge;
	int	 direction;  /* Not used yet */
} MballLoc;

typedef struct _MballLocPos {
	int	 wedge, band, direction;
} MballLocPos;

typedef struct _MballMove {
	int	 wedge, direction;
	int	 band;
	int	 control; /* Not used yet */
} MballMove;

typedef struct {
#ifdef WIN32
	HGLRC       glx_context;
#else
	GLXContext *glx_context;
#endif
	GLint       WindH, WindW;
	GLfloat     step;
	MballMove  *moves;
	int	 storedmoves;
	int      degreeTurn;
	int	 shufflingmoves;
	int      wedges, bands;
	int      slicesPerWedge, stacksPerBand;
	int	 action;
	int	 done;
	int      mono;
	GLfloat     angleStep;
	MballLoc *mballLoc[MAX_WEDGES];
	MballMove   movement;
	GLfloat     rotateStep;
	GLfloat     PX, PY, VX, VY;
	Bool	AreObjectsDefined[2];
#ifdef STANDALONE
	Bool button_down_p;
	trackball_state *trackball;
#endif
} mballstruct;

static float front_shininess[] =
{60.0};
static float front_specular[] =
{0.7, 0.7, 0.7, 1.0};
static float ambient[] =
{0.0, 0.0, 0.0, 1.0};
static float diffuse[] =
{1.0, 1.0, 1.0, 1.0};
static float position0[] =
{1.0, 0.0, 0.0, 0.0};
static float position1[] =
{0.0, 0.0, 1.0, 0.0};
static float position2[] =
{-1.0, 0.0, 0.0, 0.0};
static float position3[] =
{0.0, 0.0, -1.0, 0.0};
static float position4[] =
{0.0, 1.0, 0.0, 0.0};
static float position5[] =
{0.0, -1.0, 0.0, 0.0};
static float lmodel_ambient[] =
{0.5, 0.5, 0.5, 1.0};
static float lmodel_twoside[] =
{GL_TRUE};

static float MaterialColor[MAX_WEDGES][4] = {
	{0.7, 0.7, 0.0, 1.0}, /* Yellow */
	{0.0, 0.5, 0.0, 1.0}, /* Green */
	{0.1, 0.4, 0.3, 1.0}, /* SeaGreen */
	{0.0, 0.0, 0.5, 1.0}, /* Blue */
	{0.0, 0.7, 0.7, 1.0}, /* Cyan */
	{0.7, 0.0, 0.7, 1.0}, /* Magenta */
	{0.5, 0.0, 0.0, 1.0}, /* Red */
	{0.9, 0.45, 0.0, 1.0}, /* Orange */
	{0.9, 0.5, 0.5, 1.0}, /* Pink */
	{0.0, 0.4, 0.0, 1.0}, /* tan */
	{0.4, 0.5, 0.6, 1.0}, /* LightSteelBlue */
	{0.4, 0.2, 0.2, 1.0} /* IndianRed */
};
static float MaterialGray[MAX_WEDGES][4] = {
	{0.7, 0.7, 0.7, 1.0}, /* 70 */
	{0.5, 0.5, 0.5, 1.0}, /* 50 */
	{0.3, 0.3, 0.3, 1.0}, /* 30 */
	{0.65, 0.65, 0.65, 1.0}, /* 65 */
	{0.45, 0.45, 0.45, 1.0}, /* 45 */
	{0.25, 0.25, 0.25, 1.0}, /* 25 */
	{0.6, 0.6, 0.6, 1.0}, /* 60 */
	{0.4, 0.4, 0.4, 1.0}, /* 40 */
	{0.2, 0.2, 0.2, 1.0}, /* 20 */
	{0.75, 0.75, 0.75, 1.0}, /* 75 */
	{0.55, 0.55, 0.55, 1.0}, /* 55 */
	{0.35, 0.35, 0.35, 1.0} /* 35 */
};
static float MaterialBorder[] =
{0.1, 0.1, 0.1, 1.0};

static mballstruct *mball = (mballstruct *) NULL;

static void
pickColor(int c, int mono)
{
	if (mono)
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray[c]);
	else
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialColor[c]);
}

typedef struct { GLfloat x, y, z; } XYZ;

/* TODO, draw a wire frame of wedge where it is composed of
   more than one stack and slice, as cannot tell where one wedge
   ends and another begins if they are the same color */

static int
drawWedge(mballstruct * op, Bool wire, int wedge, int band)
{
	int stacks = op->stacksPerBand * op->bands;
	int slices = op->slicesPerWedge * op->wedges;
	int polys = 0;
	int i, j;
	double theta1, theta2, theta3;
	XYZ e, p;
	XYZ la = {0, 0, 0}, lb = {0, 0, 0};
	XYZ c = {0, 0, 0};	/* center */
	double r = 1.0;		/* radius */
	int stacks2 = stacks * 2;
	int firstStack, firstSlice;
	int lastStack, lastSlice;

	if (r < 0)
		r = -r;
	if (slices < 0)
		slices = -slices;
	if (slices < 4 || stacks < 2 || r <= 0) {
		glBegin(GL_POINTS);
		glVertex3f(c.x, c.y, c.z);
		glEnd();
		return 1;
	}
	glFrontFace(GL_CW);
	firstStack = band * op->stacksPerBand;
	lastStack = firstStack + op->stacksPerBand - 1;
	if (wire)
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialBorder);
	for (j = firstStack; j <= lastStack; j++) {
		theta1 = 2 * j * M_PI / stacks2 - M_PI_2;
		theta2 = 2 * (j + 1) * M_PI / stacks2 - M_PI_2;
		glBegin((wire) ? GL_LINE_LOOP : GL_TRIANGLE_STRIP);
		firstSlice = wedge * op->slicesPerWedge;
		lastSlice = firstSlice + op->slicesPerWedge;
		for (i = firstSlice; i <= lastSlice; i++) {
#ifdef WEDGE_EDGE
			if (i == firstSlice || i == lastSlice || j == firstStack || j == lastStack)
				glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialBorder);
			else
#else
			if (!wire)
#endif
				pickColor(op->mballLoc[wedge][op->bands - 1 - band].wedge, op->mono);
			theta3 = i * 2 * M_PI / slices;
			if (wire && i != firstSlice) {
				glVertex3f(lb.x, lb.y, lb.z);
				glVertex3f(la.x, la.y, la.z);
			}
			e.x = (float) (cos(theta2) * cos(theta3));
			e.y = (float) sin(theta2);
			e.z = (float) (cos(theta2) * sin(theta3));
			p.x = (float) (c.x + r * e.x);
			p.y = (float) (c.y + r * e.y);
			p.z = (float) (c.z + r * e.z);
			glNormal3f(e.x, e.y, e.z);
			glTexCoord2f((float) (i / (double) slices),
				(float) (2 * (j + 1) / (double) stacks2));
			glVertex3f(p.x, p.y, p.z);
			if (wire)
				la = p;
			e.x = (float) (cos(theta1) * cos(theta3));
			e.y = (float) sin(theta1);
			e.z = (float) (cos(theta1) * sin(theta3));
			p.x = (float) (c.x + r * e.x);
			p.y = (float) (c.y + r * e.y);
			p.z = (float) (c.z + r * e.z);
			glNormal3f(e.x, e.y, e.z);
			glTexCoord2f((float) (i / (double) slices),
				(float) (2 * j / (double) stacks2));
			glVertex3f(p.x, p.y, p.z);
			if (wire)
				lb = p;
			polys++;
		}
		glEnd();
	}
	return polys;
}

static void
drawBand(mballstruct * op, int band) {
	int wedge;
	for (wedge = 0; wedge < op->wedges; wedge++)
		drawWedge(op, False, wedge, op->bands - 1 - band);
}

static void
drawNotBand(mballstruct * op, int notBand) {
	int wedge, band;
	for (wedge = 0; wedge < op->wedges; wedge++)
		for (band = 0; band < op->bands; band++)
			if (notBand != band)
				drawWedge(op, False, wedge, op->bands - 1 - band);
}

static void
drawHalf(mballstruct * op, int offset) {
	int wedge, band;
	for (wedge = 0; wedge < op->wedges / 2; wedge++)
		for (band = 0; band < op->bands; band++)
			drawWedge(op, False, (wedge + offset) % op->wedges,
				 op->bands - 1 - band);
}

static void
drawWedges(mballstruct * op, int wedge, int band, int direction,
		Bool use, Bool all)
{
	int half = op->wedges / 2;
	int whole = op->wedges;

	switch (direction) {
	case TOP:
	case BOTTOM:
		if (all || use) {
			glPushMatrix();
			drawHalf(op, (wedge / half) * half);
			glPopMatrix();
		} else if (!all)
			drawHalf(op, ((wedge / half) * half + half) % whole);
		break;
	case TTR:
	case BBL:
		if (op->wedges % 12 == 0) {
			if (all || use) {
				glPushMatrix();
				drawHalf(op, ((((wedge + 11 * half / 6) % whole) / half) * half + half / 6) % whole);
				glPopMatrix();
			} else if (!all)
				drawHalf(op, ((((wedge + 11 * half / 6) % whole) / half) * half + 7 * half / 6) % whole);
		} else if (op->wedges % 10 == 0) {
			if (all || use) {
				glPushMatrix();
				drawHalf(op, ((((wedge + 9 * half / 5) % whole) / half) * half + half / 5) % whole);
				glPopMatrix();
			} else if (!all)
				drawHalf(op, ((((wedge + 9 * half / 5) % whole) / half) * half + 6 * half / 5) % whole);
		}
		break;
	case TR:
	case BL:
		if (op->wedges % 10 == 0) {
			if (all || use) {
				glPushMatrix();
				drawHalf(op, ((((wedge + 8 * half / 5) % whole) / half) * half + 2 * half / 5) % whole);
				glPopMatrix();
			} else if (!all)
				drawHalf(op, ((((wedge + 8 * half / 5) % whole) / half) * half + 7 * half / 5) % whole);
		} else if (op->wedges % 8 == 0) {
			if (all || use) {
				glPushMatrix();
				drawHalf(op, ((((wedge + 7 * half / 4) % whole) / half) * half + half / 4) % whole);
				glPopMatrix();
			} else if (!all)
				drawHalf(op, ((((wedge + 7 * half / 4) % whole) / half) * half + 5 * half / 4) % whole);
		} else if (op->wedges % 6 == 0) {
			if (all || use) {
				glPushMatrix();
				drawHalf(op, ((((wedge + 5 * half / 3) % whole) / half) * half + half / 3) % whole);
				glPopMatrix();
			} else if (!all)
				drawHalf(op, ((((wedge + 5 * half / 3) % whole) / half) * half + 2 * whole / 3) % whole);
		}
		break;
	case RIGHT:
	case LEFT:
		if (op->wedges % 4 == 0) {
			if (all || use) {
				glPushMatrix();
				drawHalf(op, ((((wedge + 3 * half / 2) % whole) / half) * half + half / 2) % whole);
				glPopMatrix();
			} else if (!all)
				drawHalf(op, ((((wedge + 3 * half / 2) % whole) / half) * half + 3 * half / 2) % whole);
		}
		break;
	case BR:
	case TL:
		if (op->wedges % 10 == 0) {
			if (all || use) {
				glPushMatrix();
				drawHalf(op, ((((wedge + 7 * half / 5) % whole) / half) * half + 3 * half / 5) % whole);
				glPopMatrix();
			} else if (!all)
				drawHalf(op, ((((wedge + 7 * half / 5) % whole) / half) * half + 8 * half / 5) % whole);
		} else if (op->wedges % 8 == 0) {
			if (all || use) {
				glPushMatrix();
				drawHalf(op, ((((wedge + 5 * half / 4) % whole) / half) * half + 3 * half / 4) % whole);
				glPopMatrix();
			} else if (!all)
				drawHalf(op, ((((wedge + 5 * half / 4) % whole) / half) * half + 7 * half / 4) % whole);
		} else if (op->wedges % 6 == 0) {
			if (all || use) {
				glPushMatrix();
				drawHalf(op, ((((wedge + 4 * half / 3) % whole) / half) * half + whole / 3) % whole);
				glPopMatrix();
			} else if (!all)
				drawHalf(op, ((((wedge + 4 * half / 3) % whole) / half) * half + 5 * half / 3) % whole);
		}
		break;
	case BBR:
	case TTL:
		if (op->wedges % 12 == 0) {
			if (all || use) {
				glPushMatrix();
				drawHalf(op, ((((wedge + 13 * half / 6) % whole) / half) * half + 11 * half / 6) % whole);
				glPopMatrix();
			} else if (!all)
				drawHalf(op, ((((wedge + 13 * half / 6) % whole) / half) * half + 5 * half / 6) % whole);
		} else if (op->wedges % 10 == 0) {
			if (all || use) {
				glPushMatrix();
				drawHalf(op, ((((wedge + 11 * half / 5) % whole) / half) * half + 9 * half / 5) % whole);
				glPopMatrix();
			} else if (!all)
				drawHalf(op, ((((wedge + 11 * half / 5) % whole) / half) * half + 4 * half / 5) % whole);
		}
		break;
	case CW:
	case CCW:
		if (all || use) {
			glPushMatrix();
			drawBand(op, band);
			glPopMatrix();
		} else if (!all)
			drawNotBand(op, band);
		break;
	/*default:
		 (void) fprintf(stderr,
			"drawWedges direction %d unknown\n", direction);*/
	}
}

static void
rotateWedgeSlice(mballstruct * op, GLfloat rotateStep,
		GLfloat x, GLfloat y, GLfloat z,
		int wedge, int band, int direction)
{
	int b;
	glPushMatrix();
	glRotatef(rotateStep, x, y, z);
	if (op->movement.control) {
		if (direction == CW || direction == CCW)
			for (b = 0; b < op->bands; b++)
				drawWedges(op, wedge, b, direction, False, True);
		else
			for (b = 0; b < 2; b++)
				drawWedges(op, b * op->wedges / 2, band, direction, False, True);
	} else
		drawWedges(op, wedge, band, direction, True, False);
	glPopMatrix();
	if (!op->movement.control)
		drawWedges(op, wedge, band, direction, False, False);
}

static void
drawSphere(mballstruct * op)
{
	GLfloat rotateStep = 0.0;
	int wedge = op->movement.wedge;
	int band = op->movement.band;
	int direction = op->movement.direction;

	if (wedge == NO_WEDGE || direction == IGNORE_DIR) {
		drawNotBand(op, -1);
	} else {
		float sqrt3 = (float) 1.7320508075688;
		float tanpidot3 = (float) 1.3763819204711; /* tan(3*pi/10) */
		float tanpidot = (float) 0.3249196962329; /* tan(pi/10) */
		rotateStep = (float) op->rotateStep;
		switch(direction) {
		case TOP:
			rotateWedgeSlice(op, rotateStep, 0, 0, -1,
				wedge, band, direction);
			break;
		case TTR:
			if (op->wedges % 12 == 0)
				rotateWedgeSlice(op, rotateStep, 1, 0, -sqrt3,
					wedge, band, direction);
			else if (op->wedges % 10 == 0)
				rotateWedgeSlice(op, rotateStep, 1, 0, -tanpidot3,
					wedge, band, direction);
			break;
		case TR:
			if (op->wedges % 10 == 0)
				rotateWedgeSlice(op, rotateStep, 1, 0, -tanpidot,
					wedge, band, direction);
			else if (op->wedges % 8 == 0)
				rotateWedgeSlice(op, rotateStep, 1, 0, -1,
					wedge, band, direction);
			else if (op->wedges % 6 == 0)
				rotateWedgeSlice(op, rotateStep, sqrt3, 0, -1,
					wedge, band, direction);
			break;
		case RIGHT:
			if (op->wedges % 4 == 0)
				rotateWedgeSlice(op, rotateStep, 1, 0, 0,
					wedge, band, direction);
			break;
		case BR:
			if (op->wedges % 10 == 0)
				rotateWedgeSlice(op, rotateStep, 1, 0, tanpidot,
					wedge, band, direction);
			else if (op->wedges % 8 == 0)
				rotateWedgeSlice(op, rotateStep, 1, 0, 1,
					wedge, band, direction);
			else if (op->wedges % 6 == 0)
				rotateWedgeSlice(op, rotateStep, sqrt3, 0, 1,
					wedge, band, direction);
			break;
		case BBR:
			if (op->wedges % 12 == 0)
				rotateWedgeSlice(op, rotateStep, 1, 0, sqrt3,
					wedge, band, direction);
			else if (op->wedges % 10 == 0)
				rotateWedgeSlice(op, rotateStep, 1, 0, tanpidot3,
					wedge, band, direction);
			break;
		case BOTTOM:
			rotateWedgeSlice(op, rotateStep, 0, 0, 1,
				wedge, band, direction);
			break;
		case BBL:
			if (op->wedges % 12 == 0)
				rotateWedgeSlice(op, rotateStep, -1, 0, sqrt3,
					wedge, band, direction);
			else if (op->wedges % 10 == 0)
				rotateWedgeSlice(op, rotateStep, -1, 0, tanpidot3,
					wedge, band, direction);
			break;
		case BL:
			if (op->wedges % 10 == 0)
				rotateWedgeSlice(op, rotateStep, -1, 0, tanpidot,
					wedge, band, direction);
			else if (op->wedges % 8 == 0)
				rotateWedgeSlice(op, rotateStep, -1, 0, 1,
					wedge, band, direction);
			else if (op->wedges % 6 == 0)
				rotateWedgeSlice(op, rotateStep, -sqrt3, 0, 1,
					wedge, band, direction);
			break;
		case LEFT:
			if (op->wedges % 4 == 0)
				rotateWedgeSlice(op, rotateStep, -1, 0, 0,
					wedge, band, direction);
			break;
		case TL:
			if (op->wedges % 10 == 0)
				rotateWedgeSlice(op, rotateStep, -1, 0, -tanpidot,
					wedge, band, direction);
			else if (op->wedges % 8 == 0)
				rotateWedgeSlice(op, rotateStep, -1, 0, -1,
					wedge, band, direction);
			else if (op->wedges % 6 == 0)
				rotateWedgeSlice(op, rotateStep, -sqrt3, 0, -1,
					wedge, band, direction);
			break;
		case TTL:
			if (op->wedges % 12 == 0)
				rotateWedgeSlice(op, rotateStep, -1, 0, -sqrt3,
					wedge, band, direction);
			else if (op->wedges % 10 == 0)
				rotateWedgeSlice(op, rotateStep, -1, 0, -tanpidot3,
					wedge, band, direction);
			break;
		case CW:
			rotateWedgeSlice(op, rotateStep / (op->wedges / 2), 0, -1, 0,
				wedge, band, direction);
			break;
		case CCW:
			rotateWedgeSlice(op, rotateStep / (op->wedges / 2), 0, 1, 0,
				wedge, band, direction);
			break;
		/*default:
			(void) fprintf(stderr,
				"draw direction %d unknown\n", direction);*/
		}
	}
}

/* From xmball used by permission ;) */

static int mapDirToWedge[(MAX_WEDGES - MIN_WEDGES) / 2 + 1][COORD] =
{
	{
		0, 6, 6, 6, 6, 6, 0, 6, 6, 6, 6, 6
	},
	{
		0, 6, 6, 1, 6, 6, 0, 6, 6, 1, 6, 6
	},
	{
		0, 6, 1, 6, 2, 6, 0, 6, 1, 6, 2, 6
	},
	{
		0, 6, 1, 2, 3, 6, 0, 6, 1, 2, 3, 6
	},
	{
		0, 1, 2, 6, 3, 4, 0, 1, 2, 6, 3, 4
	},
	{
		0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5
	}
};

int mapWedgeToDir[(MAX_WEDGES - MIN_WEDGES) / 2 + 1][MAX_WEDGES] =
{
	{
		0, 6, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12
	},
	{
		0, 3, 6, 9, 12, 12, 12, 12, 12, 12, 12, 12
	},
	{
		0, 2, 4, 6, 8, 10, 12, 12, 12, 12, 12, 12
	},
	{
		0, 2, 3, 4, 6, 8, 9, 10, 12, 12, 12, 12
	},
	{
		0, 1, 2, 4, 5, 6, 7, 8, 10, 11, 12, 12
	},
	{
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
	}
};

static void
swapPieces(mballstruct *op, int wedge1, int wedge2)
{
	MballLoc temp;
	int band;

	if (wedge1 == wedge2) {
		for (band = 0; band < op->bands / 2; band++) {
			temp = op->mballLoc[wedge1][band];
			op->mballLoc[wedge1][band] =
				op->mballLoc[wedge1][op->bands - 1 - band];
			op->mballLoc[wedge1][op->bands - 1 - band] = temp;
		}
		for (band = 0; band < op->bands; band++)
			op->mballLoc[wedge1][band].direction =
				!op->mballLoc[wedge1][band].direction;
		/*drawWedge(op, wedge1);*/
	} else {
		for (band = 0; band < op->bands; band++) {
			temp = op->mballLoc[wedge1][band];
			op->mballLoc[wedge1][band] =
				op->mballLoc[wedge2][op->bands - 1 - band];
			op->mballLoc[wedge2][op->bands - 1 - band] = temp;
			op->mballLoc[wedge1][band].direction =
				!op->mballLoc[wedge1][band].direction;
			op->mballLoc[wedge2][op->bands - 1 - band].direction =
				!op->mballLoc[wedge2][op->bands - 1 - band].direction;
		}
		/*drawWedge(op, wedge1);*/
		/*drawWedge(op, wedge2);*/
	}
}

static void
movePieces(mballstruct *op, int wedge, int band, int direction,
		Bool control)
{
	int i;

	if (direction == CW || direction == CCW) {	/* rotate band */
		int newI;
		MballLoc temp1 = {0, 0}, temp2;

		for (i = 0; i < op->wedges; i++) {
			newI = (direction == CW) ? i : op->wedges - 1 - i;
			if (newI == ((direction == CW) ? 0 : op->wedges - 1)) {
				temp1 = op->mballLoc[newI][band];
				op->mballLoc[newI][band] = op->mballLoc
					[((direction == CW) ? op->wedges - 1 : 0)][band];
			} else {
				temp2 = temp1;
				temp1 = op->mballLoc[newI][band];
				op->mballLoc[newI][band] = temp2;
			}
			/*drawSector(op, newI + op->wedges * band, FALSE);*/
		}
	} else {		/* flip */
		int sphereDir = mapDirToWedge[(op->wedges - MIN_WEDGES) / 2][direction];
		int offset = op->wedges / 2;
		int wedge1, wedge2;

		for (i = 0; i < op->wedges / 2; i++)
			if (wedge == i + sphereDir)
				offset = 0;
		for (i = 0; i < (op->wedges + 2) / 4; i++) {
			wedge1 = (i + sphereDir + offset) % op->wedges;
			wedge2 = (op->wedges / 2 - 1 - i + sphereDir + offset) %
				op->wedges;
			swapPieces(op, wedge1, wedge2);
		}
	}
}

static void
moveControlCb(mballstruct *op, int wedge, int direction)
{
	int band;

	if (direction > COORD)
		for (band = 0; band < op->bands; band++) {
			movePieces(op, wedge, band, direction, True);
			/*setPuzzle(op, ACTION_CONTROL);*/
		}
	else {
		movePieces(op, 0, 0, direction, True);
		/*setPuzzle(op, ACTION_CONTROL);*/
		movePieces(op, op->wedges / 2, 0, direction, True);
		/*setPuzzle(op, ACTION_CONTROL);*/
	}
}

static void
moveMball(mballstruct *op, int wedge, int band, int direction, int control)
{
	if (control) {
		moveControlCb(op, wedge, direction);
	} else {
		movePieces(op, wedge, band, direction, False);
	}
}

static void
evalmovement(ModeInfo * mi, MballMove movement)
{
	mballstruct *op = &mball[MI_SCREEN(mi)];

	if (movement.wedge < 0 || movement.wedge >= MAX_WEDGES)
		return;
	moveMball(op, movement.wedge, movement.band, movement.direction,
		False);
}

static int
sphereSlices(mballstruct *op)
{
	return 120 / op->wedges;
}

static int
sphereStacks(mballstruct *op)
{
	if (op->bands < 120)
		return 120 / op->bands;
	else
		return op->bands;
}

static int
oppositeDirection(int direction)
{
	if (direction == CCW)
		return CW;
	else if (direction == CW)
		return CCW;
	else
		return (direction + (MAX_WEDGES / 2)) % MAX_WEDGES;
}

static Bool
shuffle(ModeInfo * mi)
{
	mballstruct *op = &mball[MI_SCREEN(mi)];
	int	 i, wedge, band;
	MballMove   move;

	if (MI_IS_FULLRANDOM(mi))
		op->wedges = NRAND((MAX_WEDGES - MIN_WEDGES) / 2 + 1) * 2 + MIN_WEDGES;
	else if (wedges % 2 == 0 && wedges >= MIN_WEDGES && wedges <= MAX_WEDGES)
		op->wedges = wedges;
	else
		op->wedges = NRAND((MAX_WEDGES - MIN_WEDGES) / 2 + 1) * 2 + MIN_WEDGES;
	i = MI_SIZE(mi);
	if (i < -MINSIZE) {
		i = NRAND(-i - MINSIZE + 1) + MINSIZE;
	} else if (i < MINSIZE)
		 i = MINSIZE;
	op->bands = i;
	op->slicesPerWedge = sphereSlices(op);
	op->stacksPerBand = sphereStacks(op);
	for (wedge = 0; wedge < MAX_WEDGES; wedge++) {
		if (op->mballLoc[wedge])
			free(op->mballLoc[wedge]);
		if ((op->mballLoc[wedge] = (MballLoc *)
				malloc(sizeof (MballLoc) *
					(size_t) op->bands)) == NULL) {
			return False;
		}
	}
	for (wedge = 0; wedge < op->wedges; wedge++)
		for (band = 0; band < op->bands; band++) {
			op->mballLoc[wedge][band].wedge = wedge;
			op->mballLoc[wedge][band].direction = DOWN;
		}
	op->storedmoves = MI_COUNT(mi);
	if (op->storedmoves < 0) {
		if (op->moves != NULL)
			free(op->moves);
		op->moves = (MballMove *) NULL;
		op->storedmoves = NRAND(-op->storedmoves) + 1;
	}
	if ((op->storedmoves) && (op->moves == NULL))
		if ((op->moves = (MballMove *) calloc(op->storedmoves + 1,
				sizeof (MballMove))) == NULL) {
			return False;
		}
	for (i = 0; i < op->storedmoves; i++) {
		int randomDirection, oldDirection = -1;
		int oldBand = -1;
		move.wedge = NRAND(op->wedges);
		move.band = NRAND(op->bands);
		/* since non oriented first scramble should be along a band */
		if (i == 0 || NRAND(2) == 0) {
			randomDirection = (NRAND(2) == 0) ? CW : CCW;
			if (oppositeDirection(randomDirection) == oldDirection && oldBand == move.band)
				randomDirection = oppositeDirection(randomDirection);
		} else {
			randomDirection = mapWedgeToDir[(op->wedges - MIN_WEDGES) / 2][NRAND(op->wedges)];
			if (randomDirection == oldDirection ||
				oppositeDirection(randomDirection) == oldDirection)
				randomDirection = (NRAND(2) == 0) ? CW : CCW;
		}
		oldDirection = randomDirection;
		oldBand = move.band;
		move.direction = randomDirection;
		if (hideshuffling)
			evalmovement(mi, move);
		op->moves[i] = move;
		/*(void) printf("move %d: %d %d %d\n", i, move.wedge, move.band, move.direction);*/
	}
	op->degreeTurn = 180;
	op->step = NRAND(op->degreeTurn);
	if (MI_CYCLES(mi) <= 1) {
		op->angleStep = op->degreeTurn;
	} else {
		op->angleStep = op->degreeTurn / (GLfloat) (MI_CYCLES(mi));
	}

	op->VX = 0.05;
	if (NRAND(100) < 50)
		op->VX *= -1;
	op->VY = 0.05;
	if (NRAND(100) < 50)
		op->VY *= -1;
	op->movement.wedge = NO_WEDGE;
	op->rotateStep = 0;
	op->action = hideshuffling ? ACTION_SOLVE : ACTION_SHUFFLE;
	op->shufflingmoves = 0;
	op->done = 0;
	return True;
}

static void
reshape_mball(ModeInfo * mi, int width, int height)
{
	mballstruct *op = &mball[MI_SCREEN(mi)];

	glViewport(0, 0, op->WindW = (GLint) width, op->WindH = (GLint) height);
	glFrustum(-1.0, 1.0, -1.0, 1.0, 5.0, 15.0);
	glMatrixMode(GL_MODELVIEW);

	op->AreObjectsDefined[ObjCubit] = False;
}

#ifdef STANDALONE
ENTRYPOINT Bool
mball_handle_event (ModeInfo *mi, XEvent *event)
{
	mballstruct *op = &mball[MI_SCREEN(mi)];

	if (gltrackball_event_handler (event, op->trackball,
			MI_WIDTH (mi), MI_HEIGHT (mi),
			&op->button_down_p))
		return True;
	else if (screenhack_event_helper (MI_DISPLAY(mi), MI_WINDOW(mi), event)) {
		op->done = 1;
		return True;
	}
	return False;
}
#endif

static Bool
pinit(ModeInfo * mi)
{
	glClearDepth(1.0);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glColor3f(1.0, 1.0, 1.0);

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position0);
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position1);
	glLightfv(GL_LIGHT2, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT2, GL_POSITION, position2);
	glLightfv(GL_LIGHT3, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT3, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT3, GL_POSITION, position3);
	glLightfv(GL_LIGHT4, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT4, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT4, GL_POSITION, position4);
	glLightfv(GL_LIGHT5, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT5, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT5, GL_POSITION, position5);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);
	glEnable(GL_LIGHT3);
	glEnable(GL_LIGHT4);
	glEnable(GL_LIGHT5);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_CULL_FACE);

	glShadeModel(GL_FLAT);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, front_shininess);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, front_specular);

	return (shuffle(mi));
}

static void
free_mball_screen(mballstruct *op)
{
	int wedge;

	if (op == NULL) {
		return;
	}
#ifdef STANDALONE
	if (op->trackball)
		gltrackball_free (op->trackball);
#endif
	for (wedge = 0; wedge < MAX_WEDGES; wedge++)
		if (op->mballLoc[wedge] != NULL) {
			free(op->mballLoc[wedge]);
			op->mballLoc[wedge] = (MballLoc *) NULL;
		}
	if (op->moves != NULL) {
		free(op->moves);
		op->moves = (MballMove *) NULL;
	}
	op = NULL;
}

ENTRYPOINT void
release_mball(ModeInfo * mi)
{
	if (mball != NULL) {
		int	 screen;

		for (screen = 0; screen < MI_NUM_SCREENS(mi); screen++)
			free_mball_screen(&mball[screen]);
		free(mball);
		mball = (mballstruct *) NULL;
	}
	FreeAllGL(mi);
}

ENTRYPOINT void
init_mball(ModeInfo * mi)
{
	mballstruct *op;

	MI_INIT(mi, mball);
	op = &mball[MI_SCREEN(mi)];

	op->PX = ((float) LRAND() / (float) MAXRAND) * 2.0 - 1.0;
	op->PY = ((float) LRAND() / (float) MAXRAND) * 2.0 - 1.0;

#ifdef STANDALONE
	op->trackball = gltrackball_init (True);
#endif

	if ((op->glx_context = init_GL(mi)) != NULL) {
		reshape_mball(mi, MI_WIDTH(mi), MI_HEIGHT(mi));
		glDrawBuffer(GL_BACK);
		if (!pinit(mi)) {
			free_mball_screen(op);
			if (MI_IS_VERBOSE(mi)) {
				 (void) fprintf(stderr,
					"Could not allocate memory for mball\n");
			}
		}
	} else {
		MI_CLEARWINDOW(mi);
	}
}

ENTRYPOINT void
draw_mball(ModeInfo * mi)
{
	Bool bounced = False;
	Display    *display = MI_DISPLAY(mi);
	Window      window = MI_WINDOW(mi);
	mballstruct *op;

	if (mball == NULL)
		return;
	op = &mball[MI_SCREEN(mi)];
	if ((op->storedmoves) && (op->moves == NULL))
		return;
	if (!op->glx_context)
		return;
	op->mono = MI_IS_MONO(mi);
	MI_IS_DRAWN(mi) = True;
#ifdef WIN32
	wglMakeCurrent(hdc, op->glx_context);
#else
	glXMakeCurrent(display, window, *(op->glx_context));
#endif
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	glTranslatef(0.0, 0.0, -10.0);

	op->PX += op->VX;
	op->PY += op->VY;

	if (op->PY < -1) {
		op->PY += (-1) - (op->PY);
		op->VY = -op->VY;
		bounced = True;
	}
	if (op->PY > 1) {
		op->PY -= (op->PY) - 1;
		op->VY = -op->VY;
		bounced = True;
	}
	if (op->PX < -1) {
		op->PX += (-1) - (op->PX);
		op->VX = -op->VX;
		bounced = True;
	}
	if (op->PX > 1) {
		op->PX -= (op->PX) - 1;
		op->VX = -op->VX;
		bounced = True;
	}
	if (bounced) {
		op->VX += ((float) LRAND() / (float) MAXRAND) * 0.02 - 0.01;
		op->VX += ((float) LRAND() / (float) MAXRAND) * 0.02 - 0.01;
		if (op->VX > 0.06)
			op->VX = 0.06;
		if (op->VY > 0.06)
			op->VY = 0.06;
		if (op->VX < -0.06)
			op->VX = -0.06;
		if (op->VY < -0.06)
			op->VY = -0.06;
	}
	if (!MI_IS_ICONIC(mi)) {
		glTranslatef(op->PX, op->PY, 0);
		glScalef(Scale4Window * op->WindH / op->WindW,
			Scale4Window, Scale4Window);
	} else {
		glScalef(Scale4Iconic * op->WindH / op->WindW,
			Scale4Iconic, Scale4Iconic);
	}

# ifdef HAVE_MOBILE     /* Keep it the same relative size when rotated. */
	{
		GLfloat h = MI_HEIGHT(mi) / (GLfloat) MI_WIDTH(mi);
		int o = (int) current_device_rotation();
		if (o != 0 && o != 180 && o != -180) {
			glScalef (1/h, h, 1); /* #### not quite right */
			h = 1.8;
			glScalef (h, h, h);
		}
	}
# endif

#ifdef STANDALONE
	gltrackball_rotate (op->trackball);
#endif

	glRotatef(op->step * 100, 1, 0, 0);
	glRotatef(op->step * 95, 0, 1, 0);
	glRotatef(op->step * 90, 0, 0, 1);
	drawSphere(op);
	if (MI_IS_FPS(mi)) {
		do_fps (mi);
	}
	glXSwapBuffers(display, window);
	if (op->action == ACTION_SHUFFLE) {
		if (op->done) {
			if (++op->rotateStep > DELAY_AFTER_SHUFFLING) {
				op->movement.wedge = NO_WEDGE;
				op->rotateStep = 0;
				op->action = ACTION_SOLVE;
				op->done = 0;
			}
		} else {
			if (op->movement.wedge == NO_WEDGE) {
				if (op->shufflingmoves < op->storedmoves) {
					op->rotateStep = 0;
					op->movement = op->moves[op->shufflingmoves];
				} else {
					op->rotateStep = 0;
					op->done = 1;
				}
			} else {
				if (op->rotateStep == 0) {
					;
				}
				op->rotateStep += op->angleStep;
				if (op->rotateStep > op->degreeTurn) {
					evalmovement(mi, op->movement);
					op->shufflingmoves++;
					op->movement.wedge = NO_WEDGE;
				}
			}
		}
	} else {
		if (op->done) {
			if (++op->rotateStep > DELAY_AFTER_SOLVING)
				if (!shuffle(mi)) {
					free_mball_screen(op);
					if (MI_IS_VERBOSE(mi)) {
						 (void) fprintf(stderr,
							"Could not allocate memory for mball\n");
					}
				}
		} else {
			if (op->movement.wedge == NO_WEDGE) {
				if (op->storedmoves > 0) {
					op->rotateStep = 0;
					op->movement = op->moves[op->storedmoves - 1];
					if (op->movement.direction >= COORD)
						op->movement.direction = 3 * COORD -
							op->movement.direction;
					else
						op->movement.direction = (op->movement.direction +
							(COORD / 2)) % COORD;
				} else {
					op->rotateStep = 0;
					op->done = 1;
				}
			} else {
				if (op->rotateStep == 0) {
					;
				}
				op->rotateStep += op->angleStep;
				if (op->rotateStep > op->degreeTurn) {
					evalmovement(mi, op->movement);
					op->storedmoves--;
					op->movement.wedge = NO_WEDGE;
				}
			}
		}
	}

	glPopMatrix();

	glFlush();

	op->step += 0.05;
}

#ifndef STANDALONE
ENTRYPOINT void
change_mball(ModeInfo * mi)
{
	mballstruct *op;

	if (mball == NULL)
		return;
	op = &mball[MI_SCREEN(mi)];

	if (!op->glx_context)
		return;
	if (!pinit(mi)) {
		free_mball_screen(op);
		if (MI_IS_VERBOSE(mi)) {
			 (void) fprintf(stderr,
				"Could not allocate memory for mball\n");
		}
	}
}
#endif

XSCREENSAVER_MODULE ("Mball", mball)

#endif /* MODE_mball */
