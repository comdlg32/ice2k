/* -*- Mode: C; tab-width: 4 -*- */
/* oct --- Shows an auto-solving Oct */

#if 0
static const char sccsid[] = "@(#)oct.c	5.77 2024/02/23 xlockmore";
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
 * This mode shows an auto-solving octahedron puzzle.
 *
 * Thanks goes also to Brian Paul for making it possible and inexpensive
 * to use OpenGL at home.
 *
 * Based on rubik.c by Marcelo F. Vianna
 *
 * Revision History:
 * 23-Feb-2024: Started writing, using xlock pyraminx.c and OctGL.c from
		xpuzzles.  Available are sizes 2-4.
 * 23-Mar-2024: Extended to size 5.
 * 13-Sep-2024: Extended to size 6.
 */


/* Blue Red Yellow Green */
/*-
 * Color labels mapping:
 * =====================
 *
 *  RED (0) (at top not visible)
 *        --+--
 *     ---0/0\0---
 *  ---1 2/1 2\1 2---
 * + 3   /3    \3    +
 * |MAGE/       \BLUE|
 * |NTA/  WHITE  \(1)|
 * |(3)/   (2)    \  |
 * | /             \ |
 * |/               \|
 * +-----------------+
 *  --   ORANGE    --
 *    --  (4)  3 --
 *      -- 2 1 --
 *        --0--
 *          +
 * Not visible
 * YELLOW (7)  PINK (2)
 *     GREEN (6)
 */

#ifdef VMS
#include <types.h>
#endif

#ifdef STANDALONE
# define MODE_oct
# define DEFAULTS	"*delay: 100000 \n" \
			"*count: -30 \n" \
			"*cycles: 5 \n" \

# define free_oct 0
# include "xlockmore.h"		/* from the xscreensaver distribution */
# include "gltrackball.h"

#else /* !STANDALONE */
#include "xlock.h"		/* from the xlockmore distribution */
#include "visgl.h"
#endif /* !STANDALONE */

#ifdef MODE_oct
#define glTranslate(x,y,z) glTranslatef((float) x, (float) y, (float) z)

#include <GL/gl.h>
#define DEF_HIDESHUFFLING     "False"
#define DEF_STICKY     "False"
#define DEF_PERIOD3     "False"

static Bool hideshuffling;
static Bool sticky;
static Bool period3;

static XrmOptionDescRec opts[] =
{
	{(char *) "-hideshuffling", (char *) ".oct.hideshuffling", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+hideshuffling", (char *) ".oct.hideshuffling", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-sticky", (char *) ".oct.sicky", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+sticky", (char *) ".oct.sticky", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-period3", (char *) ".oct.period3", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+period3", (char *) ".oct.period3", XrmoptionNoArg, (caddr_t) "off"}
};

static argtype vars[] =
{
	{(void *) & hideshuffling, (char *) "hideshuffling", (char *) "Hideshuffling", (char *) DEF_HIDESHUFFLING, t_Bool},
	{(void *) & sticky, (char *) "sticky", (char *) "Sticky", (char *) DEF_STICKY, t_Bool},
	{(void *) & period3, (char *) "period3", (char *) "Period3", (char *) DEF_PERIOD3, t_Bool}
};

static OptionStruct desc[] =
{
	{(char *) "-/+hideshuffling", (char *) "turn on/off hidden shuffle phase"},
	{(char *) "-/+sticky", (char *) "turn on/off sticky mode"},
	{(char *) "-/+period3", (char *) "turn on/off period 3 turns"}
};

ENTRYPOINT ModeSpecOpt oct_opts =
{sizeof opts / sizeof opts[0], opts, sizeof vars / sizeof vars[0], vars, desc};

#ifdef USE_MODULES
ModStruct   oct_description =
{"oct", "init_oct", "draw_oct", "release_oct",
 "draw_oct", "change_oct", "(char *) NULL", &oct_opts,
 100000, -30, 5, -6, 64, 1.0, "",
 "Shows an auto-solving Oct puzzle", 0, NULL};

#endif

#define ACTION_SOLVE    1
#define ACTION_SHUFFLE  0

#define DELAY_AFTER_SHUFFLING  5
#define DELAY_AFTER_SOLVING   20

/*************************************************************************/

#define MINSIZE 2

#define Scale4Window	       (0.9/op->size)
#define Scale4Iconic	       (2.1/op->size)

/* Directions relative to the face of a piece */
#define TOP 0
#define TR 1
#define RIGHT 2
#define BR 3
#define BOTTOM 4
#define BL 5
#define LEFT 6
#define TL 7
#define COORD 8
#define CW 9
#define CCW 15

#define PERIOD3 3
#define PERIOD4 4
#define BOTH 5

#define SAME 0
#define OPPOSITE 1
#define DOWN 0
#define UP 1
#define MAX_FACES 8
#define MAX_VIEWS 2
#define MAX_SIDES (MAX_FACES/MAX_VIEWS)
#define MAX_ORIENT (3*MAX_SIDES)
#define MAX_ROTATE 3

#define IGNORE_DIR (-1)

#define TOP_FACE 0
#define RIGHT_FACE 1
#define BOTTOM_FACE 2
#define LEFT_FACE 3
#define TOP_BACK 4
#define RIGHT_BACK 5
#define BOTTOM_BACK 6
#define LEFT_BACK 7
#define NO_FACE (MAX_FACES+1)
#define NO_POSITION (-1)
#define NO_ROTATION (2*MAX_ORIENT)

#define FACET_LENGTH (float) 0.5
#define SELECT_LENGTH2 (float) 0.5
#define STICKER_LENGTH2 (float) 0.485

#define FACET_TETRA (FACET_LENGTH-(float) 0.02)
#define STICKER_TETRA2 (FACET_TETRA-(float) 0.04)
#define STICKER_TETRA3 (STICKER_TETRA2-(float) 0.04)
#define SELECT_TETRA2 (STICKER_TETRA2-(float) 0.04)
#define SELECT_TETRA3 (STICKER_TETRA3-(float) 0.06)

#define BAR_WIDTH (FACET_LENGTH*(float) 0.05)
#define GAP_WIDTH (FACET_LENGTH*(float) 0.15)
#define GAP_WIDTH2 (FACET_LENGTH*(float) 0.2)

#define FACET_OCTA (FACET_LENGTH*(float) 1.90)
#define STICKER_OCTA2 (FACET_LENGTH*(float) 1.85)
#define SELECT_OCTA2 (FACET_LENGTH*(float) 1.81)

#define STICKER_LONG (FACET_TETRA-(float) 0.05)
#define STICKER_SHORT (STICKER_LONG-(float) 0.05)
#define STICKER_HALF ((STICKER_SHORT)*(float) 0.5)
#define STICKER_DEPTH (FACET_LENGTH+(float) 0.01)
#define SELECT_LONG (float) (4.5*FACET_TETRA/5.0-0.05)
#define SELECT_SHORT (float) (3.5*STICKER_LONG/5.0-0.05)
#define CUT (float) 0.04
#define CUT2 (float) 0.02
#define CUT_DEPTH (STICKER_DEPTH+(float) 0.001)
#define SELECT_DEPTH (STICKER_DEPTH+(float) 0.002)

#define ObjCubit	0
#define MaxObj	  1

typedef struct _OctLoc {
	int	 face;
	int	 rotation;  /* Not used yet */
} OctLoc;

typedef struct _OctLocPos {
	int	 face, position, direction;
} OctLocPos;

typedef struct _OctMove {
	int	 face, direction;
	int	 position;
	int	 style;
	int	 control; /* Not used yet */
} OctMove;

typedef struct _OctSlice {
	int fc; /* face or corner */
	int depth, rotation;
} OctSlice;

typedef struct {
#ifdef WIN32
	HGLRC       glx_context;
#else
	GLXContext *glx_context;
#endif
	GLint       WindH, WindW;
	GLfloat     step;
	OctMove  *moves;
	int	 storedmoves;
	int      degreeTurn;
	int	 shufflingmoves;
	int      size;
	int	 action;
	int	 done;
	GLfloat     angleStep;
	OctLoc *facetLoc[MAX_FACES];
	OctLoc *faceLoc;
	OctLoc *rowLoc[MAX_ORIENT / 2];
	OctMove   movement;
	GLfloat     rotateStep;
	GLfloat     PX, PY, VX, VY;
	Bool	AreObjectsDefined[2];
	Bool    period3, sticky;
#ifdef STANDALONE
	Bool button_down_p;
	trackball_state *trackball;
#endif
} octstruct;

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

static float MaterialRed[] =
{0.5, 0.0, 0.0, 1.0};
static float MaterialBlue[] =
{0.0, 0.0, 0.5, 1.0};
static float MaterialGreen[] =
{0.0, 0.5, 0.0, 1.0};
static float MaterialYellow[] =
{0.7, 0.7, 0.0, 1.0};
static float MaterialOrange[] =
{0.9, 0.45, 0.0, 1.0};
static float MaterialMagenta[] =
{0.7, 0.0, 0.7, 1.0};
static float MaterialPink[] =
{0.9, 0.5, 0.5, 1.0};
static float MaterialWhite[] =
{0.8, 0.8, 0.8, 1.0};

static float MaterialGray1[] =
{0.1, 0.1, 0.1, 1.0};
static float MaterialGray[] =
{0.2, 0.2, 0.2, 1.0};
static float MaterialGray3[] =
{0.3, 0.3, 0.3, 1.0};
static float MaterialGray4[] =
{0.4, 0.4, 0.4, 1.0};
static float MaterialGray5[] =
{0.5, 0.5, 0.5, 1.0};
static float MaterialGray6[] =
{0.6, 0.6, 0.6, 1.0};
static float MaterialGray7[] =
{0.7, 0.7, 0.7, 1.0};

static octstruct *oct = (octstruct *) NULL;

static void
pickColor(int c, int mono)
{
	switch (c) {
	case TOP_FACE:
		if (mono)
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray3);
		else
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialRed);
		break;
	case RIGHT_FACE:
		if (mono)
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray4);
		else
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialBlue);
		break;
	case LEFT_FACE:
		if (mono)
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray5);
		else
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialMagenta);
		break;
	case BOTTOM_FACE:
		if (mono)
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialWhite);
		else
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialWhite);
		break;
	case TOP_BACK:
		if (mono)
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray1);
		else
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialOrange);
		break;
	case RIGHT_BACK:
		if (mono)
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray);
		else
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialPink);
	break;
	case LEFT_BACK:
		if (mono)
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray6);
		else
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialYellow);
		break;
	case BOTTOM_BACK:
		if (mono)
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray7);
		else
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGreen);
		break;
	}
}

static void
drawStickerlessTetra(void)
{
	glPushMatrix();
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray);
	glBegin(GL_TRIANGLES);
	/* OPP GREEN FRONT */
	glNormal3f(-1.0, 1.0, 1.0);
	glVertex3f(-FACET_TETRA, FACET_LENGTH, FACET_LENGTH);
	glVertex3f(-FACET_LENGTH, FACET_LENGTH, FACET_TETRA);
	glVertex3f(-FACET_LENGTH, FACET_TETRA, FACET_LENGTH);
	/* OPP RED LEFT */
	glNormal3f(-1.0, -1.0, -1.0);
	glVertex3f(-FACET_LENGTH, -FACET_TETRA, -FACET_LENGTH);
	glVertex3f(-FACET_TETRA, -FACET_LENGTH, -FACET_LENGTH);
	glVertex3f(-FACET_LENGTH, -FACET_LENGTH, -FACET_TETRA);
	/* OPP YELLOW TOP */
	glNormal3f(1.0, 1.0, -1.0);
	glVertex3f(FACET_TETRA, FACET_LENGTH, -FACET_LENGTH);
	glVertex3f(FACET_LENGTH, FACET_LENGTH, -FACET_TETRA);
	glVertex3f(FACET_LENGTH, FACET_TETRA, -FACET_LENGTH);
	/* OPP BLUE RIGHT */
	glNormal3f(1.0, -1.0, 1.0);
	glVertex3f(FACET_LENGTH, -FACET_TETRA, FACET_LENGTH);
	glVertex3f(FACET_LENGTH, -FACET_LENGTH, FACET_TETRA);
	glVertex3f(FACET_TETRA, -FACET_LENGTH, FACET_LENGTH);

	/* TOP_FACE */
	glNormal3f(-1.0, 1.0, -1.0);
	glVertex3f(-FACET_LENGTH, -FACET_TETRA, -FACET_LENGTH);
	glVertex3f(-FACET_LENGTH, FACET_LENGTH, FACET_TETRA);
	glVertex3f(FACET_TETRA, FACET_LENGTH, -FACET_LENGTH);
	/* RIGHT_FACE */
	glNormal3f(1.0, 1.0, 1.0);
	glVertex3f(FACET_LENGTH, FACET_LENGTH, -FACET_TETRA);
	glVertex3f(-FACET_TETRA, FACET_LENGTH, FACET_LENGTH);
	glVertex3f(FACET_LENGTH, -FACET_TETRA, FACET_LENGTH);
	/* LEFT_FACE */
	glNormal3f(-1.0, -1.0, 1.0);
	glVertex3f(FACET_TETRA, -FACET_LENGTH, FACET_LENGTH);
	glVertex3f(-FACET_LENGTH, FACET_TETRA, FACET_LENGTH);
	glVertex3f(-FACET_LENGTH, -FACET_LENGTH, -FACET_TETRA);
	/* BOTTOM_FACE */
	glNormal3f(1.0, -1.0, -1.0);
	glVertex3f(FACET_LENGTH, FACET_TETRA, -FACET_LENGTH);
	glVertex3f(FACET_LENGTH, -FACET_LENGTH, FACET_TETRA);
	glVertex3f(-FACET_TETRA, -FACET_LENGTH, -FACET_LENGTH);
	glEnd();

	glBegin(GL_QUADS);
	/* LEFT BAR */
	glNormal3f(-1.0, 0.0, 0.0);
	glVertex3f(-FACET_LENGTH, -FACET_TETRA, -FACET_LENGTH);
	glVertex3f(-FACET_LENGTH, -FACET_LENGTH, -FACET_TETRA);
	glVertex3f(-FACET_LENGTH, FACET_TETRA, FACET_LENGTH);
	glVertex3f(-FACET_LENGTH, FACET_LENGTH, FACET_TETRA);
	/* TOP BAR */
	glNormal3f(1.0, 0.0, 0.0);
	glVertex3f(FACET_LENGTH, FACET_LENGTH, -FACET_TETRA);
	glVertex3f(FACET_TETRA, FACET_LENGTH, -FACET_LENGTH);
	glVertex3f(-FACET_LENGTH, FACET_LENGTH, FACET_TETRA);
	glVertex3f(-FACET_TETRA, FACET_LENGTH, FACET_LENGTH);
	/* RIGHT BAR */
	glNormal3f(0.0, 0.0, 1.0);
	glVertex3f(FACET_TETRA, -FACET_LENGTH, FACET_LENGTH);
	glVertex3f(FACET_LENGTH, -FACET_TETRA, FACET_LENGTH);
	glVertex3f(-FACET_TETRA, FACET_LENGTH, FACET_LENGTH);
	glVertex3f(-FACET_LENGTH, FACET_TETRA, FACET_LENGTH);
	/* FAR LEFT BAR */
	glNormal3f(0.0, 0.0, -1.0);
	glVertex3f(-FACET_TETRA, -FACET_LENGTH, -FACET_LENGTH);
	glVertex3f(-FACET_LENGTH, -FACET_TETRA, -FACET_LENGTH);
	glVertex3f(FACET_TETRA, FACET_LENGTH, -FACET_LENGTH);
	glVertex3f(FACET_LENGTH, FACET_TETRA, -FACET_LENGTH);
	/* FAR RIGHT BAR */
	glNormal3f(1.0, 0.0, 0.0);
	glVertex3f(FACET_LENGTH, FACET_TETRA, -FACET_LENGTH);
	glVertex3f(FACET_LENGTH, FACET_LENGTH, -FACET_TETRA);
	glVertex3f(FACET_LENGTH, -FACET_TETRA, FACET_LENGTH);
	glVertex3f(FACET_LENGTH, -FACET_LENGTH, FACET_TETRA);
	/* BOTTOM BAR */
	glNormal3f(0.0, -1.0, 0.0);
	glVertex3f(-FACET_LENGTH, -FACET_LENGTH, -FACET_TETRA);
	glVertex3f(-FACET_TETRA, -FACET_LENGTH, -FACET_LENGTH);
	glVertex3f(FACET_LENGTH, -FACET_LENGTH, FACET_TETRA);
	glVertex3f(FACET_TETRA, -FACET_LENGTH, FACET_LENGTH);
	glEnd();
	glPopMatrix();
}

static void
drawStickerlessInvTetra(void)
{
	glPushMatrix();
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray);
	glBegin(GL_TRIANGLES);
	/* OPP GREEN BACK */
	glNormal3f(1.0, -1.0, -1.0);
	glVertex3f(FACET_LENGTH, -FACET_TETRA, -FACET_LENGTH);
	glVertex3f(FACET_LENGTH, -FACET_LENGTH, -FACET_TETRA);
	glVertex3f(FACET_TETRA, -FACET_LENGTH, -FACET_LENGTH);
	/* OPP RED RIGHT */
	glNormal3f(1.0, 1.0, 1.0);
	glVertex3f(FACET_LENGTH, FACET_LENGTH, FACET_TETRA);
	glVertex3f(FACET_TETRA, FACET_LENGTH, FACET_LENGTH);
	glVertex3f(FACET_LENGTH, FACET_TETRA, FACET_LENGTH);
	/* OPP YELLOW BOTTOM */
	glNormal3f(-1.0, -1.0, 1.0);
	glVertex3f(-FACET_LENGTH, -FACET_TETRA, FACET_LENGTH);
	glVertex3f(-FACET_LENGTH, -FACET_LENGTH, FACET_TETRA);
	glVertex3f(-FACET_TETRA, -FACET_LENGTH, FACET_LENGTH);
	/* OPP BLUE LEFT */
	glNormal3f(-1.0, 1.0, -1.0);
	glVertex3f(-FACET_TETRA, FACET_LENGTH, -FACET_LENGTH);
	glVertex3f(-FACET_LENGTH, FACET_LENGTH, -FACET_TETRA);
	glVertex3f(-FACET_LENGTH, FACET_TETRA, -FACET_LENGTH);

	/* BOTTOM_FACE */
	glNormal3f(1.0, -1.0, 1.0);
	glVertex3f(-FACET_TETRA, -FACET_LENGTH, FACET_LENGTH);
	glVertex3f(FACET_LENGTH, -FACET_LENGTH, -FACET_TETRA);
	glVertex3f(FACET_LENGTH, FACET_TETRA, FACET_LENGTH);
	/* LEFT_FACE */
	glNormal3f(-1.0, -1.0, -1.0);
	glVertex3f(-FACET_LENGTH, FACET_TETRA, -FACET_LENGTH);
	glVertex3f(FACET_TETRA, -FACET_LENGTH, -FACET_LENGTH);
	glVertex3f(-FACET_LENGTH, -FACET_LENGTH, FACET_TETRA);
	/* RIGHT_FACE */
	glNormal3f(1.0, 1.0, -1.0);
	glVertex3f(FACET_LENGTH, FACET_LENGTH, FACET_TETRA);
	glVertex3f(FACET_LENGTH, -FACET_TETRA, -FACET_LENGTH);
	glVertex3f(-FACET_TETRA, FACET_LENGTH, -FACET_LENGTH);
	/* TOP_FACE */
	glNormal3f(-1.0, 1.0, 1.0);
	glVertex3f(FACET_TETRA, FACET_LENGTH, FACET_LENGTH);
	glVertex3f(-FACET_LENGTH, FACET_LENGTH, -FACET_TETRA);
	glVertex3f(-FACET_LENGTH, -FACET_TETRA, FACET_LENGTH);
	glEnd();

	glBegin(GL_QUADS);
	/* RIGHT BAR */
	glNormal3f(1.0, 0.0, 0.0);
	glVertex3f(FACET_LENGTH, -FACET_LENGTH, -FACET_TETRA);
	glVertex3f(FACET_LENGTH, -FACET_TETRA, -FACET_LENGTH);
	glVertex3f(FACET_LENGTH, FACET_LENGTH, FACET_TETRA);
	glVertex3f(FACET_LENGTH, FACET_TETRA, FACET_LENGTH);
	/* BOTTOM BAR */
	glNormal3f(-1.0, 0.0, 0.0);
	glVertex3f(FACET_TETRA, -FACET_LENGTH, -FACET_LENGTH);
	glVertex3f(FACET_LENGTH, -FACET_LENGTH, -FACET_TETRA);
	glVertex3f(-FACET_TETRA, -FACET_LENGTH, FACET_LENGTH);
	glVertex3f(-FACET_LENGTH, -FACET_LENGTH, FACET_TETRA);
	/* LEFT BAR */
	glNormal3f(0.0, 0.0, -1.0);
	glVertex3f(FACET_LENGTH, -FACET_TETRA, -FACET_LENGTH);
	glVertex3f(FACET_TETRA, -FACET_LENGTH, -FACET_LENGTH);
	glVertex3f(-FACET_LENGTH, FACET_TETRA, -FACET_LENGTH);
	glVertex3f(-FACET_TETRA, FACET_LENGTH, -FACET_LENGTH);

	/* FAR RIGHT BAR */
	glNormal3f(0.0, 0.0, 1.0);
	glVertex3f(-FACET_LENGTH, -FACET_TETRA, FACET_LENGTH);
	glVertex3f(-FACET_TETRA, -FACET_LENGTH, FACET_LENGTH);
	glVertex3f(FACET_LENGTH, FACET_TETRA, FACET_LENGTH);
	glVertex3f(FACET_TETRA, FACET_LENGTH, FACET_LENGTH);
	/* FAR LEFT BAR */
	glNormal3f(-1.0, 0.0, 0.0);
	glVertex3f(-FACET_LENGTH, FACET_LENGTH, -FACET_TETRA);
	glVertex3f(-FACET_LENGTH, FACET_TETRA, -FACET_LENGTH);
	glVertex3f(-FACET_LENGTH, -FACET_LENGTH, FACET_TETRA);
	glVertex3f(-FACET_LENGTH, -FACET_TETRA, FACET_LENGTH);
	/* TOP BAR */
	glNormal3f(0.0, 1.0, 0.0);
	glVertex3f(-FACET_TETRA, FACET_LENGTH, -FACET_LENGTH);
	glVertex3f(-FACET_LENGTH, FACET_LENGTH, -FACET_TETRA);
	glVertex3f(FACET_TETRA, FACET_LENGTH, FACET_LENGTH);
	glVertex3f(FACET_LENGTH, FACET_LENGTH, FACET_TETRA);
	glEnd();
	glPopMatrix();
}

static void
drawStickerlessOcta(int topFace, int rightFace, int bottomFace, int leftFace,
		int topBack, int rightBack, int bottomBack, int leftBack)
{
	glPushMatrix();
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray);
	glBegin(GL_QUADS);
	if (topFace != NO_POSITION || rightFace != NO_POSITION) {
		glNormal3f(1.0, 1.0, 0.0);
		glVertex3f(CUT, FACET_OCTA, BAR_WIDTH); /* TOP RIGHT BAR */
		glVertex3f(FACET_OCTA, CUT, BAR_WIDTH);
		glVertex3f(FACET_OCTA, CUT, -BAR_WIDTH);
		glVertex3f(CUT, FACET_OCTA, -BAR_WIDTH);
	}
	if (rightFace != NO_POSITION || bottomFace != NO_POSITION) {
		glNormal3f(0.0, 1.0, 1.0);
		glVertex3f(-BAR_WIDTH, CUT, FACET_OCTA); /* TOP FRONT RIGHT BAR */
		glVertex3f(BAR_WIDTH, CUT, FACET_OCTA);
		glVertex3f(BAR_WIDTH, FACET_OCTA, CUT);
		glVertex3f(-BAR_WIDTH, FACET_OCTA, CUT);
	}
	if (bottomFace != NO_POSITION || leftFace != NO_POSITION) {
		glNormal3f(-1.0, 1.0, 0.0);
		glVertex3f(-FACET_OCTA, CUT, BAR_WIDTH); /* TOP FRONT LEFT BAR */
		glVertex3f(-CUT, FACET_OCTA, BAR_WIDTH);
		glVertex3f(-CUT, FACET_OCTA, -BAR_WIDTH);
		glVertex3f(-FACET_OCTA, CUT, -BAR_WIDTH);
	}
	if (topFace != NO_POSITION || leftFace != NO_POSITION) {
		glNormal3f(0.0, 1.0, -1.0);
		glVertex3f(BAR_WIDTH, CUT, -FACET_OCTA); /* TOP LEFT BAR */
		glVertex3f(-BAR_WIDTH, CUT, -FACET_OCTA);
		glVertex3f(-BAR_WIDTH, FACET_OCTA, -CUT);
		glVertex3f(BAR_WIDTH, FACET_OCTA, -CUT);
	}

	if (topFace != NO_POSITION || bottomBack != NO_POSITION) {
		glNormal3f(1.0, 0.0, -1.0);
		glVertex3f(FACET_OCTA, BAR_WIDTH, -CUT); /* BACK BOTTOM BAR */
		glVertex3f(FACET_OCTA, -BAR_WIDTH, -CUT);
		glVertex3f(CUT, -BAR_WIDTH, -FACET_OCTA);
		glVertex3f(CUT, BAR_WIDTH, -FACET_OCTA);
	}
	if (rightFace != NO_POSITION || rightBack != NO_POSITION) {
		glNormal3f(1.0, 0.0, 1.0);
		glVertex3f(FACET_OCTA, -BAR_WIDTH, CUT); /* FAR RIGHT BAR */
		glVertex3f(FACET_OCTA, BAR_WIDTH, CUT);
		glVertex3f(CUT, BAR_WIDTH, FACET_OCTA);
		glVertex3f(CUT, -BAR_WIDTH, FACET_OCTA);
	}
	if (bottomFace != NO_POSITION || topBack != NO_POSITION) {
		glNormal3f(-1.0, 0.0, 1.0);
		glVertex3f(-FACET_OCTA, -BAR_WIDTH, CUT); /* BOTTOM BAR */
		glVertex3f(-CUT, -BAR_WIDTH, FACET_OCTA);
		glVertex3f(-CUT, BAR_WIDTH, FACET_OCTA);
		glVertex3f(-FACET_OCTA, BAR_WIDTH, CUT);
	}
	if (leftFace != NO_POSITION || leftBack != NO_POSITION) {
		glNormal3f(-1.0, 0.0, -1.0);
		glVertex3f(-FACET_OCTA, BAR_WIDTH, -CUT); /* FAR LEFT BAR */
		glVertex3f(-CUT, BAR_WIDTH, -FACET_OCTA);
		glVertex3f(-CUT, -BAR_WIDTH, -FACET_OCTA);
		glVertex3f(-FACET_OCTA, -BAR_WIDTH, -CUT);
	}

	if (topBack != NO_POSITION || rightBack != NO_POSITION) {
		glNormal3f(1.0, -1.0, 0.0);
		glVertex3f(FACET_OCTA, -CUT, -BAR_WIDTH); /* BACK RIGHT BAR */
		glVertex3f(FACET_OCTA, -CUT, BAR_WIDTH);
		glVertex3f(CUT, -FACET_OCTA, BAR_WIDTH);
		glVertex3f(CUT, -FACET_OCTA, -BAR_WIDTH);
	}
	if (rightBack != NO_POSITION || bottomBack != NO_POSITION) {
		glNormal3f(0.0, -1.0, 1.0);
		glVertex3f(-BAR_WIDTH, -CUT, FACET_OCTA); /* BACK UPPER RIGHT BAR */
		glVertex3f(-BAR_WIDTH, -FACET_OCTA, CUT);
		glVertex3f(BAR_WIDTH, -FACET_OCTA, CUT);
		glVertex3f(BAR_WIDTH, -CUT, FACET_OCTA);
	}
	if (bottomBack != NO_POSITION || leftBack != NO_POSITION) {
		glNormal3f(-1.0, -1.0, 0.0);
		glVertex3f(-CUT, -FACET_OCTA, -BAR_WIDTH); /* BACK UPPER LEFT BAR */
		glVertex3f(-CUT, -FACET_OCTA, BAR_WIDTH);
		glVertex3f(-FACET_OCTA, -CUT, BAR_WIDTH);
		glVertex3f(-FACET_OCTA, -CUT, -BAR_WIDTH);
	}
	if (topBack != NO_POSITION || leftBack != NO_POSITION) {
		glNormal3f(0.0, -1.0, -1.0);
		glVertex3f(BAR_WIDTH, -CUT, -FACET_OCTA); /* LEFT BAR */
		glVertex3f(BAR_WIDTH, -FACET_OCTA, -CUT);
		glVertex3f(-BAR_WIDTH, -FACET_OCTA, -CUT);
		glVertex3f(-BAR_WIDTH, -CUT, -FACET_OCTA);
	}
	glEnd();
	/* POINTS */
	if (bottomFace != NO_POSITION || leftFace != NO_POSITION
			|| topBack != NO_POSITION || leftBack != NO_POSITION) {
		glBegin(GL_POLYGON);
		glNormal3f(-1.0, 0.0, 0.0);
		glVertex3f(-FACET_OCTA, -CUT, -BAR_WIDTH); /* LEFT */
		glVertex3f(-FACET_OCTA, -CUT, BAR_WIDTH);
		glVertex3f(-FACET_OCTA, -BAR_WIDTH, CUT);
		glVertex3f(-FACET_OCTA, BAR_WIDTH, CUT);
		glVertex3f(-FACET_OCTA, CUT, BAR_WIDTH);
		glVertex3f(-FACET_OCTA, CUT, -BAR_WIDTH);
		glVertex3f(-FACET_OCTA, BAR_WIDTH, -CUT);
		glVertex3f(-FACET_OCTA, -BAR_WIDTH, -CUT);
		glEnd();
	    if (topFace == NO_POSITION && rightFace == NO_POSITION
			&& rightBack == NO_POSITION && bottomBack == NO_POSITION) {
		glBegin(GL_POLYGON);
		glNormal3f(1.0, 0.0, 0.0);
		glVertex3f(0, -BAR_WIDTH, -FACET_OCTA);
		glVertex3f(0, BAR_WIDTH, -FACET_OCTA);
		glVertex3f(0, FACET_OCTA, -BAR_WIDTH);
		glVertex3f(0, FACET_OCTA, BAR_WIDTH);
		glVertex3f(0, BAR_WIDTH, FACET_OCTA);
		glVertex3f(0, -BAR_WIDTH, FACET_OCTA);
		glVertex3f(0, -FACET_OCTA, BAR_WIDTH);
		glVertex3f(0, -FACET_OCTA, -BAR_WIDTH);
		glEnd();
	    }
	}
	if (topFace != NO_POSITION || rightFace != NO_POSITION
			|| rightBack != NO_POSITION || bottomBack != NO_POSITION) {
		glBegin(GL_POLYGON);
		glNormal3f(1.0, 0.0, 0.0);
		glVertex3f(FACET_OCTA, -BAR_WIDTH, -CUT); /* BACK RIGHT */
		glVertex3f(FACET_OCTA, BAR_WIDTH, -CUT);
		glVertex3f(FACET_OCTA, CUT, -BAR_WIDTH);
		glVertex3f(FACET_OCTA, CUT, BAR_WIDTH);
		glVertex3f(FACET_OCTA, BAR_WIDTH, CUT);
		glVertex3f(FACET_OCTA, -BAR_WIDTH, CUT);
		glVertex3f(FACET_OCTA, -CUT, BAR_WIDTH);
		glVertex3f(FACET_OCTA, -CUT, -BAR_WIDTH);
		glEnd();
	    if (bottomFace == NO_POSITION && leftFace == NO_POSITION
			&& topBack == NO_POSITION && leftBack == NO_POSITION) {
		glBegin(GL_POLYGON);
		glNormal3f(-1.0, 0.0, 0.0);
		glVertex3f(0, -FACET_OCTA, -BAR_WIDTH);
		glVertex3f(0, -FACET_OCTA, BAR_WIDTH);
		glVertex3f(0, -BAR_WIDTH, FACET_OCTA);
		glVertex3f(0, BAR_WIDTH, FACET_OCTA);
		glVertex3f(0, FACET_OCTA, BAR_WIDTH);
		glVertex3f(0, FACET_OCTA, -BAR_WIDTH);
		glVertex3f(0, BAR_WIDTH, -FACET_OCTA);
		glVertex3f(0, -BAR_WIDTH, -FACET_OCTA);
		glEnd();
	    }
	}
	if (topFace != NO_POSITION || rightFace != NO_POSITION
			|| bottomFace != NO_POSITION || leftFace != NO_POSITION) {
		glBegin(GL_POLYGON);
		glNormal3f(0.0, 1.0, 0.0);
		glVertex3f(-CUT, FACET_OCTA, -BAR_WIDTH); /* TOP */
		glVertex3f(-CUT, FACET_OCTA, BAR_WIDTH);
		glVertex3f(-BAR_WIDTH, FACET_OCTA, CUT);
		glVertex3f(BAR_WIDTH, FACET_OCTA, CUT);
		glVertex3f(CUT, FACET_OCTA, BAR_WIDTH);
		glVertex3f(CUT, FACET_OCTA, -BAR_WIDTH);
		glVertex3f(BAR_WIDTH, FACET_OCTA, -CUT);
		glVertex3f(-BAR_WIDTH, FACET_OCTA, -CUT);
		glEnd();
	    if (topBack == NO_POSITION && rightBack == NO_POSITION
			&& bottomBack == NO_POSITION && leftBack == NO_POSITION) {
		glBegin(GL_POLYGON);
		glNormal3f(0.0, -1.0, 0.0);
		glVertex3f(-BAR_WIDTH, 0, -FACET_OCTA);
		glVertex3f(BAR_WIDTH, 0, -FACET_OCTA);
		glVertex3f(FACET_OCTA, 0, -BAR_WIDTH);
		glVertex3f(FACET_OCTA, 0, BAR_WIDTH);
		glVertex3f(BAR_WIDTH, 0, FACET_OCTA);
		glVertex3f(-BAR_WIDTH, 0, FACET_OCTA);
		glVertex3f(-FACET_OCTA, 0, BAR_WIDTH);
		glVertex3f(-FACET_OCTA, 0, -BAR_WIDTH);
		glEnd();
	    }
	}
	if (topBack != NO_POSITION || rightBack != NO_POSITION
			|| bottomBack != NO_POSITION || leftBack != NO_POSITION) {
		glBegin(GL_POLYGON);
		glNormal3f(0.0, -1.0, 0.0);
		glVertex3f(-BAR_WIDTH, -FACET_OCTA, -CUT); /* BOTTOM */
		glVertex3f(BAR_WIDTH, -FACET_OCTA, -CUT);
		glVertex3f(CUT, -FACET_OCTA, -BAR_WIDTH);
		glVertex3f(CUT, -FACET_OCTA, BAR_WIDTH);
		glVertex3f(BAR_WIDTH, -FACET_OCTA, CUT);
		glVertex3f(-BAR_WIDTH, -FACET_OCTA, CUT);
		glVertex3f(-CUT, -FACET_OCTA, BAR_WIDTH);
		glVertex3f(-CUT, -FACET_OCTA, -BAR_WIDTH);
		glEnd();
	    if (topFace == NO_POSITION && rightFace == NO_POSITION
			&& bottomFace == NO_POSITION && leftFace == NO_POSITION) {
		glBegin(GL_POLYGON);
		glNormal3f(0.0, 1.0, 0.0);
		glVertex3f(-FACET_OCTA, 0, -BAR_WIDTH);
		glVertex3f(-FACET_OCTA, 0, BAR_WIDTH);
		glVertex3f(-BAR_WIDTH, 0, FACET_OCTA);
		glVertex3f(BAR_WIDTH, 0, FACET_OCTA);
		glVertex3f(FACET_OCTA, 0, BAR_WIDTH);
		glVertex3f(FACET_OCTA, 0, -BAR_WIDTH);
		glVertex3f(BAR_WIDTH, 0, -FACET_OCTA);
		glVertex3f(-BAR_WIDTH, 0, -FACET_OCTA);
		glEnd();
	    }
	}
	if (rightFace != NO_POSITION || bottomFace != NO_POSITION
			|| topBack != NO_POSITION || rightBack != NO_POSITION) {
		glBegin(GL_POLYGON);
		glNormal3f(0.0, 0.0, 1.0);
		glVertex3f(-BAR_WIDTH, -CUT, FACET_OCTA); /* RIGHT */
		glVertex3f(BAR_WIDTH, -CUT, FACET_OCTA);
		glVertex3f(CUT, -BAR_WIDTH, FACET_OCTA);
		glVertex3f(CUT, BAR_WIDTH, FACET_OCTA);
		glVertex3f(BAR_WIDTH, CUT, FACET_OCTA);
		glVertex3f(-BAR_WIDTH, CUT, FACET_OCTA);
		glVertex3f(-CUT, BAR_WIDTH, FACET_OCTA);
		glVertex3f(-CUT, -BAR_WIDTH, FACET_OCTA);
		glEnd();
	    if (topFace == NO_POSITION && leftFace == NO_POSITION
			&& bottomBack == NO_POSITION && leftBack == NO_POSITION) {
		glBegin(GL_POLYGON);
		glNormal3f(0.0, 0.0, -1.0);
		glVertex3f(-BAR_WIDTH, -FACET_OCTA, 0);
		glVertex3f(BAR_WIDTH, -FACET_OCTA, 0);
		glVertex3f(FACET_OCTA, -BAR_WIDTH, 0);
		glVertex3f(FACET_OCTA, BAR_WIDTH, 0);
		glVertex3f(BAR_WIDTH, FACET_OCTA, 0);
		glVertex3f(-BAR_WIDTH, FACET_OCTA, 0);
		glVertex3f(-FACET_OCTA, BAR_WIDTH, 0);
		glVertex3f(-FACET_OCTA, -BAR_WIDTH, 0);
		glEnd();
	    }
	}
	if (topFace != NO_POSITION || leftFace != NO_POSITION
			|| bottomBack != NO_POSITION || leftBack != NO_POSITION) {
		glBegin(GL_POLYGON);
		glNormal3f(0.0, 0.0, -1.0);
		glVertex3f(-CUT, -BAR_WIDTH, -FACET_OCTA); /* BACK LEFT */
		glVertex3f(-CUT, BAR_WIDTH, -FACET_OCTA);
		glVertex3f(-BAR_WIDTH, CUT, -FACET_OCTA);
		glVertex3f(BAR_WIDTH, CUT, -FACET_OCTA);
		glVertex3f(CUT, BAR_WIDTH, -FACET_OCTA);
		glVertex3f(CUT, -BAR_WIDTH, -FACET_OCTA);
		glVertex3f(BAR_WIDTH, -CUT, -FACET_OCTA);
		glVertex3f(-BAR_WIDTH, -CUT, -FACET_OCTA);
		glEnd();
	    if (rightFace == NO_POSITION && bottomFace == NO_POSITION
			&& topBack == NO_POSITION && rightBack == NO_POSITION) {
		glBegin(GL_POLYGON);
		glNormal3f(0.0, 0.0, 1.0);
		glVertex3f(-BAR_WIDTH, -FACET_OCTA, 0);
		glVertex3f(BAR_WIDTH, -FACET_OCTA, 0);
		glVertex3f(FACET_OCTA, -BAR_WIDTH, 0);
		glVertex3f(FACET_OCTA, BAR_WIDTH, 0);
		glVertex3f(BAR_WIDTH, FACET_OCTA, 0);
		glVertex3f(-BAR_WIDTH, FACET_OCTA, 0);
		glVertex3f(-FACET_OCTA, BAR_WIDTH, 0);
		glVertex3f(-FACET_OCTA, -BAR_WIDTH, 0);
		glEnd();
	    }
	}

	/* FACES */
	/* TOP BEHIND */
	if (topFace != NO_POSITION) {
		glBegin(GL_POLYGON);
		glNormal3f(1.0, 1.0, -1.0);
		glVertex3f(CUT, BAR_WIDTH, -FACET_OCTA);
		glVertex3f(BAR_WIDTH, CUT, -FACET_OCTA);
		glVertex3f(BAR_WIDTH, FACET_OCTA, -CUT);
		glVertex3f(CUT, FACET_OCTA, -BAR_WIDTH);
		glVertex3f(FACET_OCTA, CUT, -BAR_WIDTH);
		glVertex3f(FACET_OCTA, BAR_WIDTH, -CUT);
		glEnd();
	}
	/* TOP RIGHT */
	if (rightFace != NO_POSITION) {
		glBegin(GL_POLYGON);
		glNormal3f(1.0, 1.0, 1.0);
		glVertex3f(FACET_OCTA, BAR_WIDTH, CUT);
		glVertex3f(FACET_OCTA, CUT, BAR_WIDTH);
		glVertex3f(CUT, FACET_OCTA, BAR_WIDTH);
		glVertex3f(BAR_WIDTH, FACET_OCTA, CUT);
		glVertex3f(BAR_WIDTH, CUT, FACET_OCTA);
		glVertex3f(CUT, BAR_WIDTH, FACET_OCTA);
		glEnd();
	}
	/* FRONT */
	if (bottomFace != NO_POSITION) {
		glBegin(GL_POLYGON);
		glNormal3f(-1.0, 1.0, 1.0);
		glVertex3f(-CUT, BAR_WIDTH, FACET_OCTA); /* RIGHT */
		glVertex3f(-BAR_WIDTH, CUT, FACET_OCTA);
		glVertex3f(-BAR_WIDTH, FACET_OCTA, CUT); /* TOP */
		glVertex3f(-CUT, FACET_OCTA, BAR_WIDTH);
		glVertex3f(-FACET_OCTA, CUT, BAR_WIDTH); /* LEFT */
		glVertex3f(-FACET_OCTA, BAR_WIDTH, CUT);
		glEnd();
	}
	/* TOP LEFT */
	if (leftFace != NO_POSITION) {
		glBegin(GL_POLYGON);
		glNormal3f(-1.0, 1.0, -1.0);
		glVertex3f(-FACET_OCTA, BAR_WIDTH, -CUT); /* LEFT BOTTOM */
		glVertex3f(-FACET_OCTA, CUT, -BAR_WIDTH);
		glVertex3f(-CUT, FACET_OCTA, -BAR_WIDTH); /* TOP */
		glVertex3f(-BAR_WIDTH, FACET_OCTA, -CUT);
		glVertex3f(-BAR_WIDTH, CUT, -FACET_OCTA); /* FAR LEFT */
		glVertex3f(-CUT, BAR_WIDTH, -FACET_OCTA);
		glEnd();
	}
	/* BOTTOM */
	if (topBack != NO_POSITION) {
		glBegin(GL_POLYGON);
		glNormal3f(-1.0, -1.0, 1.0);
		glVertex3f(-FACET_OCTA, -BAR_WIDTH, CUT);
		glVertex3f(-FACET_OCTA, -CUT, BAR_WIDTH);
		glVertex3f(-CUT, -FACET_OCTA, BAR_WIDTH);
		glVertex3f(-BAR_WIDTH, -FACET_OCTA, CUT);
		glVertex3f(-BAR_WIDTH, -CUT, FACET_OCTA);
		glVertex3f(-CUT, -BAR_WIDTH, FACET_OCTA);
		glEnd();
	}
	/* BOTTOM RIGHT */
	if (rightBack != NO_POSITION) {
		glBegin(GL_POLYGON);
		glNormal3f(1.0, -1.0, 1.0);
		glVertex3f(CUT, -BAR_WIDTH, FACET_OCTA);
		glVertex3f(BAR_WIDTH, -CUT, FACET_OCTA);
		glVertex3f(BAR_WIDTH, -FACET_OCTA, CUT);
		glVertex3f(CUT, -FACET_OCTA, BAR_WIDTH);
		glVertex3f(FACET_OCTA, -CUT, BAR_WIDTH);
		glVertex3f(FACET_OCTA, -BAR_WIDTH, CUT);
		glEnd();
	}
	/* BACK */
	if (bottomBack != NO_POSITION) {
		glBegin(GL_POLYGON);
		glNormal3f(1.0, -1.0, -1.0);
		glVertex3f(FACET_OCTA, -BAR_WIDTH, -CUT);
		glVertex3f(FACET_OCTA, -CUT, -BAR_WIDTH);
		glVertex3f(CUT, -FACET_OCTA, -BAR_WIDTH);
		glVertex3f(BAR_WIDTH, -FACET_OCTA, -CUT);
		glVertex3f(BAR_WIDTH, -CUT, -FACET_OCTA);
		glVertex3f(CUT, -BAR_WIDTH, -FACET_OCTA);
		glEnd();
	}
	/* BOTTOM LEFT */
	if (leftBack != NO_POSITION) {
		glBegin(GL_POLYGON);
		glNormal3f(-1.0, -1.0, -1.0);
		glVertex3f(-CUT, -BAR_WIDTH, -FACET_OCTA);
		glVertex3f(-BAR_WIDTH, -CUT, -FACET_OCTA);
		glVertex3f(-BAR_WIDTH, -FACET_OCTA, -CUT);
		glVertex3f(-CUT, -FACET_OCTA, -BAR_WIDTH);
		glVertex3f(-FACET_OCTA, -CUT, -BAR_WIDTH);
		glVertex3f(-FACET_OCTA, -BAR_WIDTH, -CUT);
		glEnd();
	}
	glPopMatrix();
}

/* This helps to Map sizes for the piece faces */
static int truncUnit[MAX_FACES][6][3] =
{
	{ /* 0 inv Red */
		{1, 3, -2},
		{2, 3, -1},
		{3, 2, -1},
		{3, 1, -2},
		{2, 1, -3},
		{1, 2, -3}
	}, { /* 1 Blue */
		{2, 3, 1},
		{1, 3, 2},
		{1, 2, 3},
		{2, 1, 3},
		{3, 1, 2},
		{3, 2, 1}
	}, { /* 2 inv White */
		{-1, 3, 2},
		{-2, 3, 1},
		{-3, 2, 1},
		{-3, 1, 2},
		{-2, 1, 3},
		{-1, 2, 3}
	}, { /* 3 Magenta */
		{-2, 3, -1},
		{-1, 3, -2},
		{-1, 2, -3},
		{-2, 1, -3},
		{-3, 1, -2},
		{-3, 2, -1}
	}, { /* 4 Orange */
		{-2, -3, 1},
		{-1, -3, 2},
		{-1, -2, 3},
		{-2, -1, 3},
		{-3, -1, 2},
		{-3, -2, 1}
	}, { /* 5 inv Pink */
		{1, -3, 2},
		{2, -3, 1},
		{3, -2, 1},
		{3, -1, 2},
		{2, -1, 3},
		{1, -2, 3}
	}, { /* 6 Green */
		{2, -3, -1},
		{1, -3, -2},
		{1, -2, -3},
		{2, -1, -3},
		{3, -1, -2},
		{3, -2, -1}
	}, { /* 7 inv Yellow */
		{-1, -3, -2},
		{-2, -3, -1},
		{-3, -2, -1},
		{-3, -1, -2},
		{-2, -1, -3},
		{-1, -2, -3}
	}
};

static float stickerCoord[6][3];

static void setStickerCoord(int face, float a, float b, float c)
{
	int i, j, value;
	for (i = 0; i < 6; i++) {
		for (j = 0; j < 3; j++) {
			value = truncUnit[face][i][j];
			switch(value) {
			case -1:
				stickerCoord[i][j] = -a;
				break;
			case 1:
				stickerCoord[i][j] = a;
				break;
			case -2:
				stickerCoord[i][j] = -b;
				break;
			case 2:
				stickerCoord[i][j] = b;
				break;
			case -3:
				stickerCoord[i][j] = -c;
				break;
			case 3:
				stickerCoord[i][j] = c;
				break;
			}
		}
	}
}

static void
drawSticker(ModeInfo * mi, int face, int position, Bool tetra)
{
	int mono = MI_IS_MONO(mi);
	octstruct *op = &oct[MI_SCREEN(mi)];
	int i;

	glBegin(GL_POLYGON);
	pickColor(op->facetLoc[face][position].face, mono);
	if (tetra)
		setStickerCoord(face, -STICKER_TETRA3, STICKER_TETRA2, STICKER_LENGTH2);
	else
		setStickerCoord(face, CUT, GAP_WIDTH, STICKER_OCTA2);
	for (i = 0; i < 6; i++) {
		glVertex3f(stickerCoord[i][0], stickerCoord[i][1], stickerCoord[i][2]);
	}
	glEnd();
}

/* 2 different orientations of tetrahedron */
static void
drawRegTetraFacet(ModeInfo *mi, int leftFace, int rightFace, int topBack, int bottomBack)
{
	drawStickerlessTetra();
	glPushMatrix();
	if (leftFace != NO_POSITION) { /* 3 MAGENTA */
		glNormal3f(-1.0, 1.0, -1.0);
		drawSticker(mi, LEFT_FACE, leftFace, True);
	}
	if (rightFace != NO_POSITION) { /* 1 BLUE */
		glNormal3f(1.0, 1.0, 1.0);
		drawSticker(mi, RIGHT_FACE, rightFace, True);
	}
	if (topBack != NO_POSITION) { /* 4 ORANGE */
		glNormal3f(-1.0, -1.0, 1.0);
		drawSticker(mi, TOP_BACK, topBack, True);
	}
	if (bottomBack != NO_POSITION) { /* 6 GREEN */
		glNormal3f(1.0, -1.0, -1.0);
		drawSticker(mi, BOTTOM_BACK, bottomBack, True);
	}
	glPopMatrix();
}

static void
drawInvTetraFacet(ModeInfo *mi, int topFace, int bottomFace, int leftBack, int rightBack)
{
	drawStickerlessInvTetra();
	glPushMatrix();
	if (topFace != NO_POSITION) { /* 0 RED */
		glNormal3f(1.0, 1.0, -1.0);
		drawSticker(mi, TOP_FACE, topFace, True);
	}
	if (bottomFace != NO_POSITION) { /* 2 WHITE */
		glNormal3f(-1.0, 1.0, 1.0);
		drawSticker(mi, BOTTOM_FACE, bottomFace, True);
	}
	if (leftBack != NO_POSITION) { /* 7 YELLOW */
		glNormal3f(1.0, -1.0, 1.0);
		drawSticker(mi, LEFT_BACK, leftBack, True);
	}
	if (rightBack != NO_POSITION) { /* 5 PINK */
		glNormal3f(1.0, -1.0, 1.0);
		drawSticker(mi, RIGHT_BACK, rightBack, True);
	}
	glPopMatrix();
}


/* combine into one routine to be compatible with oct drawing */
static void
drawTetraFacet(ModeInfo *mi,
		int topFace, int rightFace, int bottomFace, int leftFace,
		int topBack, int rightBack, int bottomBack, int leftBack)
{
	if (topFace != -1 || bottomFace != -1
			|| leftBack != -1 || rightBack != -1)
		drawInvTetraFacet(mi, topFace, bottomFace, leftBack, rightBack);
	if (leftFace != -1 || rightFace != -1
			|| topBack != -1 || bottomBack != -1)
		drawRegTetraFacet(mi, leftFace, rightFace, topBack, bottomBack);
}

/* 4 faces not obscured */
static void
drawOctaFacet(ModeInfo *mi,
		int topFace, int rightFace, int bottomFace, int leftFace,
		int topBack, int rightBack, int bottomBack, int leftBack)
{
	octstruct *op = &oct[MI_SCREEN(mi)];
	glPushMatrix();
	if (op->movement.style == PERIOD4) /* draw visible parts of the octahedrons */
		drawStickerlessOcta(topFace, rightFace, bottomFace, leftFace,
			topBack, rightBack, bottomBack, leftBack);
	else
		drawStickerlessOcta(1, 1, 1, 1, 1, 1, 1, 1);
	/* TOP BEHIND */
	if (topFace != NO_POSITION) { /* RED */
		glNormal3f(1.0, 1.0, -1.0);
		drawSticker(mi, TOP_FACE, topFace, False);
	}
	/* TOP RIGHT */
	if (rightFace != NO_POSITION) { /* BLUE */
		glNormal3f(1.0, 1.0, 1.0);
		drawSticker(mi, RIGHT_FACE, rightFace, False);
	}
	/* FRONT */
	if (bottomFace != NO_POSITION) { /* WHITE */
		glNormal3f(-1.0, 1.0, 1.0);
		drawSticker(mi, BOTTOM_FACE, bottomFace, False);
	}
	/* TOP LEFT */
	if (leftFace != NO_POSITION) { /* MAGENTA */
		glNormal3f(-1.0, 1.0, -1.0);
		drawSticker(mi, LEFT_FACE, leftFace, False);
	}
	/* BOTTOM */
	if (topBack != NO_POSITION) { /* ORANGE */
		glNormal3f(-1.0, -1.0, 1.0);
		drawSticker(mi, TOP_BACK, topBack, False);
	}
	/* TOP RIGHT */
	if (rightBack != NO_POSITION) { /* PINK */
		glNormal3f(1.0, -1.0, 1.0);
		drawSticker(mi, RIGHT_BACK, rightBack, False);
	}
	/* FRONT */
	if (bottomBack != NO_POSITION) { /* GREEN */
		glNormal3f(1.0, -1.0, -1.0);
		drawSticker(mi, BOTTOM_BACK, bottomBack, False);
	}
	/* TOP LEFT */
	if (leftBack != NO_POSITION) { /* YELLOW */
		glNormal3f(1.0, -1.0, 1.0);
		drawSticker(mi, LEFT_BACK, leftBack, False);
	}
	glPopMatrix();
}

static void
drawFacet(ModeInfo *mi,
		int topFace, int rightFace, int bottomFace, int leftFace,
		int topBack, int rightBack, int bottomBack, int leftBack,
		Bool tetra)
{
	if (tetra)
		drawTetraFacet(mi, topFace, rightFace, bottomFace, leftFace,
			topBack, rightBack, bottomBack, leftBack);
	else
		drawOctaFacet(mi, topFace, rightFace, bottomFace, leftFace,
			topBack, rightBack, bottomBack, leftBack);
}

/* This is fast for small i, a -1 is returned for negative i. */
static int
SQRT(int i)
{
	int j = 0;

	while (j * j <= i)
		j++;
	return (j - 1);
}

typedef struct _RTT {
	int row, trbl, tlbr;
} RTT;

static void
toRTT(int position, RTT * rtt)
{
	rtt->row = SQRT(position);
	/* Passing row so there is no sqrt calculation again */
	rtt->trbl = (position - rtt->row * rtt->row) / 2;
	rtt->tlbr = (rtt->row * rtt->row + 2 * rtt->row - position) / 2;
}

#if 0
static int
toPosition(RTT rtt)
{
	return (rtt.row * rtt.row + rtt.row + rtt.trbl - rtt.tlbr);
}
#endif

static int
length(octstruct * op, int dir, int h)
{
	if (!(dir % 2) || dir > COORD)
		return (2 * h + 1);
	else
		return (2 * (op->size - h) - 1);
}

typedef struct _FaceDepth {
	int face, depth;
} FaceDepth;

static void
faceDepth(octstruct * op, int face, int position, int direction, FaceDepth *fd)
{
	RTT rtt;

	toRTT(position, &rtt);
	if ((face == 0 && (direction == BR || direction == TL)) ||
			(face == 1 && (direction == TOP || direction == BOTTOM)) ||
			(face == 2 && (direction == TR || direction == BL)) ||
			(face == 6 && (direction == BR || direction == TL)) ||
			(face == 7 && (direction == TOP || direction == BOTTOM)) ||
			(face == 4 && (direction == TR || direction == BL))) {
		fd->face = 3;
		if (face < 4)
			fd->depth = ((face == 1) ? rtt.row :
				((face == 0) ? rtt.tlbr : rtt.trbl));
		else
			fd->depth = op->size - 1 -
				((face == 7) ? rtt.row :
				((face == 4) ? rtt.trbl : rtt.tlbr));
	} else if ((face == 0 && (direction == TR || direction == BL)) ||
			(face == 3 && (direction == TOP || direction == BOTTOM)) ||
			(face == 2 && (direction == BR || direction == TL)) ||
			(face == 6 && (direction == TR || direction == BL)) ||
			(face == 5 && (direction == TOP || direction == BOTTOM)) ||
			(face == 4 && (direction == BR || direction == TL))) {
		fd->face = 1;
		if (face < 4)
			fd->depth = ((face == 3) ? rtt.row :
				((face == 0) ? rtt.trbl : rtt.tlbr));
		else
			fd->depth = op->size - 1 -
				((face == 5) ? rtt.row :
				((face == 4) ? rtt.tlbr : rtt.trbl));
	} else if ((face == 1 && (direction == TR || direction == BL)) ||
			(face == 2 && (direction == RIGHT || direction == LEFT)) ||
			(face == 3 && (direction == BR || direction == TL)) ||
			(face == 5 && (direction == TR || direction == BL)) ||
			(face == 6 && (direction == RIGHT || direction == LEFT)) ||
			(face == 7 && (direction == BR || direction == TL))) {
		fd->face = 0;
		if (face < 4)
			fd->depth = ((face == 2) ? rtt.row :
				((face == 1) ? rtt.tlbr : rtt.trbl));
		else
			fd->depth = op->size - 1 -
				((face == 6) ? rtt.row :
				((face == 5) ? rtt.tlbr : rtt.trbl));
	} else if ((face == 3 && (direction == TR || direction == BL)) ||
			(face == 0 && (direction == RIGHT || direction == LEFT)) ||
			(face == 1 && (direction == BR || direction == TL)) ||
			(face == 7 && (direction == TR || direction == BL)) ||
			(face == 4 && (direction == RIGHT || direction == LEFT)) ||
			(face == 5 && (direction == BR || direction == TL))) {
		fd->face = 2;
		if (face < 4)
			fd->depth = ((face == 0) ? rtt.row :
				((face == 1) ? rtt.trbl : rtt.tlbr));
		else
			fd->depth = op->size - 1 -
				((face == 4) ? rtt.row :
				((face == 5) ? rtt.trbl : rtt.tlbr));
	} else {
		fd->face = -1;
		fd->depth = -1;
	}
}

static void
drawFaces(ModeInfo * mi, int face, int position, int direction,
		Bool use, Bool all)
{
	octstruct *op = &oct[MI_SCREEN(mi)];
	int s0, s1, s2, i, j, k;
	int pos, col, row, sb, sl, sr, mb, ml, mr;
	int size = op->size, edge;
	Bool tetra, sticky = op->sticky;
	RTT rtt;
	FaceDepth fd;

	toRTT(position, &rtt);
	faceDepth(op, face, position, direction, &fd);

	if (size == 1 && (all || use))
		drawOctaFacet(mi, 0, 0, 0, 0, 0, 0, 0, 0);
	if (size > 1) { /* CORNERS */
		s0 = 2 * size - 2;
		j = (size - 1) * (size - 1);
		k = size * size - 1;
		if (all || (use && (fd.face == 0 || fd.face == 1) && fd.depth == 0)
				|| (use && (fd.face == 2 || fd.face == 3) && fd.depth == size - 1)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != 0)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != size - 1)) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, 0, 0);
			drawOctaFacet(mi, j, k, -1, -1, -1, j, k, -1); /* TR */
			glPopMatrix();
		}
		if (all || (use && (fd.face == 1 || fd.face == 2) && fd.depth == 0)
				|| (use && (fd.face == 0 || fd.face == 3) && fd.depth == size - 1)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != 0)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != size - 1)) {
			glPushMatrix();
			glTranslate(0, 0, s0 * FACET_LENGTH);
			drawOctaFacet(mi, -1, j, k, -1, j, k, -1, -1); /* BR */
			glPopMatrix();
		}
		if (all || (use && (fd.face == 2 || fd.face == 3) && fd.depth == 0)
				|| (use && (fd.face == 0 || fd.face == 1) && fd.depth == size - 1)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != 0)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != size - 1)) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, 0, 0);
			drawOctaFacet(mi, -1, -1, j, k, k, -1, -1, j); /* BL */
			glPopMatrix();
		}
		if (all || (use && (fd.face == 0 || fd.face == 3) && fd.depth == 0)
				|| (use && (fd.face == 1 || fd.face == 2) && fd.depth == size - 1)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != 0)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != size - 1)) {
			glPushMatrix();
			glTranslate(0, 0, -s0 * FACET_LENGTH);
			drawOctaFacet(mi, k, -1, -1, j, -1, -1, j, k); /* TL */
			glPopMatrix();
		}
		if (all || (use && fd.depth == 0)
				|| (!use && fd.depth != 0)) {
			glPushMatrix();
			glTranslate(0, s0 * FACET_LENGTH, 0);
			drawOctaFacet(mi, 0, 0, 0, 0, -1, -1, -1, -1); /* TOP */
			glPopMatrix();
		}
		if (all || (use && fd.depth == size - 1)
				|| (!use && fd.depth != size - 1)) {
			glPushMatrix();
			glTranslate(0, -s0 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, -1, -1, 0, 0, 0, 0); /* BOTTOM */
			glPopMatrix();
		}
	}
	if (size > 1 && size % 3 != 0 && (size < 4 || sticky)) { /* CENTERS */
		tetra = (size % 3 == 2); /* 2, 6, 12, 20, 30 */
		if (tetra) {
			s0 = ((size - 2) / 3) * 2 + 1;
		} else {
			s0 = ((size - 1) / 3) * 2;
		}
		s1 = size - 1 - s0;
		k = s0 * s0 + s0;
		if (all || (!sticky && ((use && fd.face == 0 && fd.depth == 0)
				|| (use && (fd.face == 1 || fd.face == 3) && fd.depth == s1)
				|| (use && fd.face == 2 && fd.depth == s0)
				|| (!use && fd.face == 0 && fd.depth != 0)
				|| (!use && (fd.face == 1 || fd.face == 3) && fd.depth != s1)
				|| (!use && fd.face == 2 && fd.depth != s0)))
			|| (sticky && ((use && fd.face == 0 && fd.depth == 0)
				|| (use && fd.face != 0 && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face == 0 && fd.depth != 0)
				|| (!use && fd.face != 0 && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, s0 * FACET_LENGTH, -s0 * FACET_LENGTH);
			drawFacet(mi, k, -1, -1, -1, -1, -1, -1, -1, tetra); /* 0 */
			glPopMatrix();
		}
		if (all || (!sticky && ((use && fd.face == 1 && fd.depth == 0)
				|| (use && (fd.face == 0 || fd.face == 2) && fd.depth == s1)
				|| (use && fd.face == 3 && fd.depth == s0)
				|| (!use && fd.face == 1 && fd.depth != 0)
				|| (!use && (fd.face == 0 || fd.face == 2) && fd.depth != s1)
				|| (!use && fd.face == 3 && fd.depth != s0)))
			|| (sticky && ((use && fd.face == 1 && fd.depth == 0)
				|| (use && fd.face != 1 && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face == 1 && fd.depth != 0)
				|| (!use && fd.face != 1 && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, s0 * FACET_LENGTH, s0 * FACET_LENGTH);
			drawFacet(mi, -1, k, -1, -1, -1, -1, -1, -1, tetra); /* 1 */
			glPopMatrix();
		}
		if (all || (!sticky && ((use && fd.face == 2 && fd.depth == 0)
				|| (use && (fd.face == 1 || fd.face == 3) && fd.depth == s1)
				|| (use && fd.face == 0 && fd.depth == s0)
				|| (!use && fd.face == 2 && fd.depth != 0)
				|| (!use && (fd.face == 1 || fd.face == 3) && fd.depth != s1)
				|| (!use && fd.face == 0 && fd.depth != s0)))
			|| (sticky && ((use && fd.face == 2 && fd.depth == 0)
				|| (use && fd.face != 2 && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face == 2 && fd.depth != 0)
				|| (!use && fd.face != 2 && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, s0 * FACET_LENGTH, s0 * FACET_LENGTH);
			drawFacet(mi, -1, -1, k, -1, -1, -1, -1, -1, tetra); /* 2 */
			glPopMatrix();
		}
		if (all || (!sticky && ((use && fd.face == 3 && fd.depth == 0)
				|| (use && (fd.face == 0 || fd.face == 2) && fd.depth == s1)
				|| (use && fd.face == 1 && fd.depth == s0)
				|| (!use && fd.face == 3 && fd.depth != 0)
				|| (!use && (fd.face == 0 || fd.face == 2) && fd.depth != s1)
				|| (!use && fd.face == 1 && fd.depth != s0)))
			|| (sticky && ((use && fd.face == 3 && fd.depth == 0)
				|| (use && fd.face != 3 && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face == 3 && fd.depth != 0)
				|| (!use && fd.face != 3 && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, s0 * FACET_LENGTH, -s0 * FACET_LENGTH);
			drawFacet(mi, -1, -1, -1, k, -1, -1, -1, -1, tetra); /* 3 */
			glPopMatrix();
		}
		if (all || (!sticky && ((use && fd.face == 0 && fd.depth == size - 1)
				|| (use && (fd.face == 1 || fd.face == 3) && fd.depth == s0)
				|| (use && fd.face == 2 && fd.depth == s1)
				|| (!use && fd.face == 0 && fd.depth != size - 1)
				|| (!use && (fd.face == 1 || fd.face == 3) && fd.depth != s0)
				|| (!use && fd.face == 2 && fd.depth != s1)))
			|| (sticky && ((use && fd.face == 0 && fd.depth == size - 1)
				|| (use && fd.face != 0 && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face == 0 && fd.depth != size - 1)
				|| (!use && fd.face != 0 && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, -s0 * FACET_LENGTH, s0 * FACET_LENGTH);
			drawFacet(mi, -1, -1, -1, -1, k, -1, -1, -1, tetra); /* 4 */
			glPopMatrix();
		}
		if (all || (!sticky && ((use && fd.face == 3 && fd.depth == size - 1)
				|| (use && (fd.face == 0 || fd.face == 2) && fd.depth == s0)
				|| (use && fd.face == 1 && fd.depth == s0)
				|| (!use && fd.face == 3 && fd.depth != size - 1)
				|| (!use && (fd.face == 0 || fd.face == 2) && fd.depth != s0)
				|| (!use && fd.face == 1 && fd.depth != s1)))
			|| (sticky && ((use && fd.face == 3 && fd.depth == size - 1)
				|| (use && fd.face != 3 && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face == 3 && fd.depth != size - 1)
				|| (!use && fd.face != 3 && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, -s0 * FACET_LENGTH, s0 * FACET_LENGTH);
			drawFacet(mi, -1, -1, -1, -1, -1, k, -1, -1, tetra); /* 5 */
			glPopMatrix();
		}
		if (all || (!sticky && ((use && fd.face == 2 && fd.depth == size - 1)
				|| (use && (fd.face == 1 || fd.face == 3) && fd.depth == s0)
				|| (use && fd.face == 0 && fd.depth == s1)
				|| (!use && fd.face == 2 && fd.depth != size - 1)
				|| (!use && (fd.face == 1 || fd.face == 3) && fd.depth != s0)
				|| (!use && fd.face == 0 && fd.depth != s1)))
			|| (sticky && ((use && fd.face == 2 && fd.depth == size - 1)
				|| (use && fd.face != 2 && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face == 2 && fd.depth != size - 1)
				|| (!use && fd.face != 2 && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, -s0 * FACET_LENGTH, -s0 * FACET_LENGTH);
			drawFacet(mi, -1, -1, -1, -1, -1, -1, k, -1, tetra); /* 6 */
			glPopMatrix();
		}
		if (all || (!sticky && ((use && fd.face == 1 && fd.depth == size - 1)
				|| (use && (fd.face == 0 || fd.face == 2) && fd.depth == s0)
				|| (use && fd.face == 3 && fd.depth == s1)
				|| (!use && fd.face == 1 && fd.depth != size - 1)
				|| (!use && (fd.face == 0 || fd.face == 2) && fd.depth != s0)
				|| (!use && fd.face == 3 && fd.depth != s1)))
			|| (sticky && ((use && fd.face == 1 && fd.depth == size - 1)
				|| (use && fd.face != 1 && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face == 1 && fd.depth != size - 1)
				|| (!use && fd.face != 1 && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, -s0 * FACET_LENGTH, -s0 * FACET_LENGTH);
			drawFacet(mi, -1, -1, -1, -1, -1, -1, -1, k, tetra); /* 7 */
			glPopMatrix();
		}
	}
	if (size > 2) { /* CORNERS - 1 */
		s0 = (size - 2) * 2 + 1;
		j = (size - 1) * (size - 1) + 1; /* 5, */
		k = size * size - 2; /* 7, */
		if (all || (!sticky && ((use && fd.face != 2 && fd.depth == 0)
				|| (use && fd.face == 2 && fd.depth == 1)
				|| (!use && fd.face != 2 && fd.depth != 0)
				|| (!use && fd.face == 2 && fd.depth != 1)))
			|| (sticky && ((use && fd.face != 2 && fd.depth == 0)
				|| (use && fd.face == 2 && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face != 2 && fd.depth != 0)
				|| (!use && fd.face == 2 && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(FACET_LENGTH, s0 * FACET_LENGTH, -FACET_LENGTH);
			drawTetraFacet(mi, 2, -1, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && (fd.face == 0 || fd.face == 1) && fd.depth == 0)
				|| (use && fd.face == 3 && fd.depth == size - 2)
				|| (use && fd.face == 2 && fd.depth == size - 1)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != 0)
				|| (!use && fd.face == 3 && fd.depth != size - 2)
				|| (!use && fd.face == 2 && fd.depth != size - 1)))
			|| (sticky && ((use && (fd.face == 0 || fd.face == 1) && fd.depth == 0)
				|| (use && fd.face == 3 && (fd.depth == 1 || fd.depth == 2))
				|| (use && fd.face == 2 && fd.depth == size - 1)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != 0)
				|| (!use && fd.face == 3 && fd.depth != 1 && fd.depth != 2)
				|| (!use && fd.face == 2 && fd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, FACET_LENGTH, -FACET_LENGTH);
			drawTetraFacet(mi, j, -1, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && (fd.face == 0 || fd.face == 3) && fd.depth == 0)
				|| (use && fd.face == 1 && fd.depth == size - 2)
				|| (use && fd.face == 2 && fd.depth == size - 1)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != 0)
				|| (!use && fd.face == 1 && fd.depth != size - 2)
				|| (!use && fd.face == 2 && fd.depth != size - 1)))
			|| (sticky && ((use && (fd.face == 0 || fd.face == 3) && fd.depth == 0)
				|| (use && fd.face == 1 && (fd.depth == 1 || fd.depth == 2))
				|| (use && fd.face == 2 && fd.depth == size - 1)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != 0)
				|| (!use && fd.face == 1 && fd.depth != 1 && fd.depth != 2)
				|| (!use && fd.face == 2 && fd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(FACET_LENGTH, FACET_LENGTH, -s0 * FACET_LENGTH);
			drawTetraFacet(mi, k, -1, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		/* 1 */
		if (all || (!sticky && ((use && fd.face != 3 && fd.depth == 0)
				|| (use && fd.face == 3 && fd.depth == 1)
				|| (!use && fd.face != 3 && fd.depth != 0)
				|| (!use && fd.face == 3 && fd.depth != 1)))
			|| (sticky && ((use && fd.face != 3 && fd.depth == 0)
				|| (use && fd.face == 3 && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face != 3 && fd.depth != 0)
				|| (!use && fd.face == 3 && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(FACET_LENGTH, s0 * FACET_LENGTH, FACET_LENGTH);
			drawTetraFacet(mi, -1, 2, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && (fd.face == 1 || fd.face == 2) && fd.depth == 0)
				|| (use && fd.face == 0 && fd.depth == size - 2)
				|| (use && fd.face == 3 && fd.depth == size - 1)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != 0)
				|| (!use && fd.face == 0 && fd.depth != size - 2)
				|| (!use && fd.face == 3 && fd.depth != size - 1)))
			|| (sticky && ((use && (fd.face == 1 || fd.face == 2) && fd.depth == 0)
				|| (use && fd.face == 0 && (fd.depth == 1 || fd.depth == 2))
				|| (use && fd.face == 3 && fd.depth == size - 1)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != 0)
				|| (!use && fd.face == 0 && fd.depth != 1 && fd.depth != 2)
				|| (!use && fd.face == 3 && fd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(FACET_LENGTH, FACET_LENGTH, s0 * FACET_LENGTH);
			drawTetraFacet(mi, -1, j, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && (fd.face == 0 || fd.face == 1) && fd.depth == 0)
				|| (use && fd.face == 2 && fd.depth == size - 2)
				|| (use && fd.face == 3 && fd.depth == size - 1)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != 0)
				|| (!use && fd.face == 2 && fd.depth != size - 2)
				|| (!use && fd.face == 3 && fd.depth != size - 1)))
			|| (sticky && ((use && (fd.face == 0 || fd.face == 1) && fd.depth == 0)
				|| (use && fd.face == 2 && (fd.depth == 1 || fd.depth == 2))
				|| (use && fd.face == 3 && fd.depth == size - 1)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != 0)
				|| (!use && fd.face == 2 && fd.depth != 1 && fd.depth != 2)
				|| (!use && fd.face == 3 && fd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, FACET_LENGTH, FACET_LENGTH);
			drawTetraFacet(mi, -1, k, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		/* 2 */
		if (all || (!sticky && ((use && fd.face != 0 && fd.depth == 0)
				|| (use && fd.face == 0 && fd.depth == 1)
				|| (!use && fd.face != 0 && fd.depth != 0)
				|| (!use && fd.face == 0 && fd.depth != 1)))
			|| (sticky && ((use && fd.face != 0 && fd.depth == 0)
				|| (use && fd.face == 0 && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face != 0 && fd.depth != 0)
				|| (!use && fd.face == 0 && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(-FACET_LENGTH, s0 * FACET_LENGTH, FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, 2, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && (fd.face == 2 || fd.face == 3) && fd.depth == 0)
				|| (use && fd.face == 1 && fd.depth == size - 2)
				|| (use && fd.face == 0 && fd.depth == size - 1)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != 0)
				|| (!use && fd.face == 1 && fd.depth != size - 2)
				|| (!use && fd.face == 0 && fd.depth != size - 1)))
			|| (sticky && ((use && (fd.face == 2 || fd.face == 3) && fd.depth == 0)
				|| (use && fd.face == 1 && (fd.depth == 1 || fd.depth == 2))
				|| (use && fd.face == 0 && fd.depth == size - 1)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != 0)
				|| (!use && fd.face == 1 && fd.depth != 1 && fd.depth != 2)
				|| (!use && fd.face == 0 && fd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, FACET_LENGTH, FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, j, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && (fd.face == 1 || fd.face == 2) && fd.depth == 0)
				|| (use && fd.face == 3 && fd.depth == size - 2)
				|| (use && fd.face == 0 && fd.depth == size - 1)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != 0)
				|| (!use && fd.face == 3 && fd.depth != size - 2)
				|| (!use && fd.face == 0 && fd.depth != size - 1)))
			|| (sticky && ((use && (fd.face == 1 || fd.face == 2) && fd.depth == 0)
				|| (use && fd.face == 3 && (fd.depth == 1 || fd.depth == 2))
				|| (use && fd.face == 0 && fd.depth == size - 1)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != 0)
				|| (!use && fd.face == 3 && fd.depth != 1 && fd.depth != 2)
				|| (!use && fd.face == 0 && fd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(-FACET_LENGTH, FACET_LENGTH, s0 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, k, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		/* 3 */
		if (all || (!sticky && ((use && fd.face != 1 && fd.depth == 0)
				|| (use && fd.face == 1 && fd.depth == 1)
				|| (!use && fd.face != 1 && fd.depth != 0)
				|| (!use && fd.face == 1 && fd.depth != 1)))
			|| (sticky && ((use && fd.face != 1 && fd.depth == 0)
				|| (use && fd.face == 1 && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face != 1 && fd.depth != 0)
				|| (!use && fd.face == 1 && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(-FACET_LENGTH, s0 * FACET_LENGTH, -FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, 2, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && (fd.face == 0 || fd.face == 3) && fd.depth == 0)
				|| (use && fd.face == 2 && fd.depth == size - 2)
				|| (use && fd.face == 1 && fd.depth == size - 1)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != 0)
				|| (!use && fd.face == 2 && fd.depth != size - 2)
				|| (!use && fd.face == 1 && fd.depth != size - 1)))
			|| (sticky && ((use && (fd.face == 0 || fd.face == 3) && fd.depth == 0)
				|| (use && fd.face == 2 && (fd.depth == 1 || fd.depth == 2))
				|| (use && fd.face == 1 && fd.depth == size - 1)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != 0)
				|| (!use && fd.face == 2 && fd.depth != 1 && fd.depth != 2)
				|| (!use && fd.face == 1 && fd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(-FACET_LENGTH, FACET_LENGTH, -s0 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, j, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && (fd.face == 2 || fd.face == 3) && fd.depth == 0)
				|| (use && fd.face == 0 && fd.depth == size - 2)
				|| (use && fd.face == 1 && fd.depth == size - 1)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != 0)
				|| (!use && fd.face == 0 && fd.depth != size - 2)
				|| (!use && fd.face == 1 && fd.depth != size - 1)))
			|| (sticky && ((use && (fd.face == 2 || fd.face == 3) && fd.depth == 0)
				|| (use && fd.face == 0 && (fd.depth == 1 || fd.depth == 2))
				|| (use && fd.face == 1 && fd.depth == size - 1)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != 0)
				|| (!use && fd.face == 0 && fd.depth != 1 && fd.depth != 2)
				|| (!use && fd.face == 1 && fd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, FACET_LENGTH, -FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, k, -1, -1, -1, -1);
			glPopMatrix();
		}
		/* 4 */
		if (all || (!sticky && ((use && fd.face != 2 && fd.depth == size - 1)
				|| (use && fd.face == 2 && fd.depth == size - 2)
				|| (!use && fd.face != 2 && fd.depth != size - 1)
				|| (!use && fd.face == 2 && fd.depth != size - 2)))
			|| (sticky && ((use && fd.face != 2 && fd.depth == size - 1)
				|| (use && fd.face == 2 && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face != 2 && fd.depth != size - 1)
				|| (!use && fd.face == 2 && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(-FACET_LENGTH, -s0 * FACET_LENGTH, FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, 2, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && (fd.face == 0 || fd.face == 3) && fd.depth == size - 1)
				|| (use && fd.face == 1 && fd.depth == 1)
				|| (use && fd.face == 2 && fd.depth == 0)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && fd.face == 1 && fd.depth != 1)
				|| (!use && fd.face == 2 && fd.depth != 0)))
			|| (sticky && ((use && (fd.face == 0 || fd.face == 3) && fd.depth == size - 1)
				|| (use && fd.face == 1 && (fd.depth == 1 || fd.depth == 2))
				|| (use && fd.face == 2 && fd.depth == 0)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && fd.face == 1 && fd.depth != 1 && fd.depth != 2)
				|| (!use && fd.face == 2 && fd.depth != 0)))) {
			glPushMatrix();
			glTranslate(-FACET_LENGTH, -FACET_LENGTH, s0 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, j, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && (fd.face == 0 || fd.face == 1) && fd.depth == size - 1)
				|| (use && fd.face == 3 && fd.depth == 1)
				|| (use && fd.face == 2 && fd.depth == 0)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != size - 1)
				|| (!use && fd.face == 3 && fd.depth != 1)
				|| (!use && fd.face == 2 && fd.depth != 0)))
			|| (sticky && ((use && (fd.face == 0 || fd.face == 1) && fd.depth == size - 1)
				|| (use && fd.face == 3 && (fd.depth == 1 || fd.depth == 2))
				|| (use && fd.face == 2 && fd.depth == 0)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != size - 1)
				|| (!use && fd.face == 3 && fd.depth != 1 && fd.depth != 2)
				|| (!use && fd.face == 2 && fd.depth != 0)))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, -FACET_LENGTH, FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, k, -1, -1, -1);
			glPopMatrix();
		}
		/* 5 */
		if (all || (!sticky && ((use && fd.face != 1 && fd.depth == size - 1)
				|| (use && fd.face == 1 && fd.depth == size - 2)
				|| (!use && fd.face != 1 && fd.depth != size - 1)
				|| (!use && fd.face == 1 && fd.depth != size - 2)))
			|| (sticky && ((use && fd.face != 1 && fd.depth == size - 1)
				|| (use && fd.face == 1 && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face != 1 && fd.depth != size - 1)
				|| (!use && fd.face == 1 && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(FACET_LENGTH, -s0 * FACET_LENGTH, FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, 2, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && (fd.face == 2 || fd.face == 3) && fd.depth == size - 1)
				|| (use && fd.face == 0 && fd.depth == 1)
				|| (use && fd.face == 1 && fd.depth == 0)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && fd.face == 0 && fd.depth != 1)
				|| (!use && fd.face == 1 && fd.depth != 0)))
			|| (sticky && ((use && (fd.face == 2 || fd.face == 3) && fd.depth == size - 1)
				|| (use && fd.face == 0 && (fd.depth == 1 || fd.depth == 2))
				|| (use && fd.face == 1 && fd.depth == 0)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && fd.face == 0 && fd.depth != 1 && fd.depth != 2)
				|| (!use && fd.face == 1 && fd.depth != 0)))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, -FACET_LENGTH, FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, j, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && (fd.face == 0 || fd.face == 3) && fd.depth == size - 1)
				|| (use && fd.face == 2 && fd.depth == 1)
				|| (use && fd.face == 1 && fd.depth == 0)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && fd.face == 2 && fd.depth != 1)
				|| (!use && fd.face == 1 && fd.depth != 0)))
			|| (sticky && ((use && (fd.face == 0 || fd.face == 3) && fd.depth == size - 1)
				|| (use && fd.face == 2 && (fd.depth == 1 || fd.depth == 2))
				|| (use && fd.face == 1 && fd.depth == 0)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && fd.face == 2 && fd.depth != 1 && fd.depth != 2)
				|| (!use && fd.face == 1 && fd.depth != 0)))) {
			glPushMatrix();
			glTranslate(FACET_LENGTH, -FACET_LENGTH, s0 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, k, -1, -1);
			glPopMatrix();
		}
		/* 6 */
		if (all || (!sticky && ((use && fd.face != 0 && fd.depth == size - 1)
				|| (use && fd.face == 0 && fd.depth == size - 2)
				|| (!use && fd.face != 0 && fd.depth != size - 1)
				|| (!use && fd.face == 0 && fd.depth != size - 2)))
			|| (sticky && ((use && fd.face != 0 && fd.depth == size - 1)
				|| (use && fd.face == 0 && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face != 0 && fd.depth != size - 1)
				|| (!use && fd.face == 0 && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(FACET_LENGTH, -s0 * FACET_LENGTH, -FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, 2, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && (fd.face == 1 || fd.face == 2) && fd.depth == size - 1)
				|| (use && fd.face == 3 && fd.depth == 1)
				|| (use && fd.face == 0 && fd.depth == 0)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != size - 1)
				|| (!use && fd.face == 3 && fd.depth != 1)
				|| (!use && fd.face == 0 && fd.depth != 0)))
			|| (sticky && ((use && (fd.face == 1 || fd.face == 2) && fd.depth == size - 1)
				|| (use && fd.face == 3 && (fd.depth == 1 || fd.depth == 2))
				|| (use && fd.face == 0 && fd.depth == 0)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != size - 1)
				|| (!use && fd.face == 3 && fd.depth != 1 && fd.depth != 2)
				|| (!use && fd.face == 0 && fd.depth != 0)))) {
			glPushMatrix();
			glTranslate(FACET_LENGTH, -FACET_LENGTH, -s0 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, j, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && (fd.face == 2 || fd.face == 3) && fd.depth == size - 1)
				|| (use && fd.face == 1 && fd.depth == 1)
				|| (use && fd.face == 0 && fd.depth == 0)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && fd.face == 1 && fd.depth != 1)
				|| (!use && fd.face == 0 && fd.depth != 0)))
			|| (sticky && ((use && (fd.face == 2 || fd.face == 3) && fd.depth == size - 1)
				|| (use && fd.face == 1 && (fd.depth == 1 || fd.depth == 2))
				|| (use && fd.face == 0 && fd.depth == 0)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && fd.face == 1 && fd.depth != 1 && fd.depth != 2)
				|| (!use && fd.face == 0 && fd.depth != 0)))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, -FACET_LENGTH, -FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, k, -1);
			glPopMatrix();
		}
		/* 7 */
		if (all || (!sticky && ((use && fd.face != 3 && fd.depth == size - 1)
				|| (use && fd.face == 3 && fd.depth == size - 2)
				|| (!use && fd.face != 3 && fd.depth != size - 1)
				|| (!use && fd.face == 3 && fd.depth != size - 2)))
			|| (sticky && ((use && fd.face != 3 && fd.depth == size - 1)
				|| (use && fd.face == 3 && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face != 3 && fd.depth != size - 1)
				|| (!use && fd.face == 3 && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(-FACET_LENGTH, -s0 * FACET_LENGTH, -FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, -1, 2);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && (fd.face == 0 || fd.face == 1) && fd.depth == size - 1)
				|| (use && fd.face == 2 && fd.depth == 1)
				|| (use && fd.face == 3 && fd.depth == 0)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != size - 1)
				|| (!use && fd.face == 2 && fd.depth != 1)
				|| (!use && fd.face == 3 && fd.depth != 0)))
			|| (sticky && ((use && (fd.face == 0 || fd.face == 1) && fd.depth == size - 1)
				|| (use && fd.face == 2 && (fd.depth == 1 || fd.depth == 2))
				|| (use && fd.face == 3 && fd.depth == 0)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != size - 1)
				|| (!use && fd.face == 2 && fd.depth != 1 && fd.depth != 2)
				|| (!use && fd.face == 3 && fd.depth != 0)))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, -FACET_LENGTH, -FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, -1, j);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && (fd.face == 1 || fd.face == 2) && fd.depth == size - 1)
				|| (use && fd.face == 0 && fd.depth == 1)
				|| (use && fd.face == 3 && fd.depth == 0)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != size - 1)
				|| (!use && fd.face == 0 && fd.depth != 1)
				|| (!use && fd.face == 3 && fd.depth != 0)))
			|| (sticky && ((use && (fd.face == 1 || fd.face == 2) && fd.depth == size - 1)
				|| (use && fd.face == 0 && (fd.depth == 1 || fd.depth == 2))
				|| (use && fd.face == 3 && fd.depth == 0)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != size - 1)
				|| (!use && fd.face == 0 && fd.depth != 1 && fd.depth != 2)
				|| (!use && fd.face == 3 && fd.depth != 0)))) {
			glPushMatrix();
			glTranslate(-FACET_LENGTH, -FACET_LENGTH, -s0* FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, -1, k);
			glPopMatrix();
		}
	}
	if (size > 2 && size % 2 == 1) {
		/* OUTSIDE MIDDLE EDGES */ /* 1, 3, 6; 5, 7, 12; 4, 8, 20 */
		s1 = size - 1;
		i = (s1 / 2) * (s1 / 2);
		j = (s1 / 2 + 1) * (s1 / 2 + 1) - 1;
		k = size * size - size;
		if (all || (use && (fd.face == 0 || fd.face == 1) && fd.depth == 0)
				|| (use && (fd.face == 2 || fd.face == 3) && fd.depth == s1 / 2)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != 0)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != s1 / 2)) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, s1 * FACET_LENGTH, 0);
			drawOctaFacet(mi, i, j, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 1 || fd.face == 2) && fd.depth == 0)
				|| (use && (fd.face == 0 || fd.face == 3) && fd.depth == s1 / 2)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != 0)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != s1 / 2)) {
			glPushMatrix();
			glTranslate(0, s1 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, i, j, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 2 || fd.face == 3) && fd.depth == 0)
				|| (use && (fd.face == 0 || fd.face == 1) && fd.depth == s1 / 2)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != 0)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != s1 / 2)) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, s1 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, i, j, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 0 || fd.face == 3) && fd.depth == 0)
				|| (use && (fd.face == 1 || fd.face == 2) && fd.depth == s1 / 2)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != 0)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != s1 / 2)) {
			glPushMatrix();
			glTranslate(0, s1 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawOctaFacet(mi, j, -1, -1, i, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 0 || fd.face == 3) && fd.depth == size - 1)
				|| (use && (fd.face == 1 || fd.face == 2) && fd.depth == s1 / 2)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != s1 / 2)) {
			glPushMatrix();
			glTranslate(0, -s1 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, -1, i, j, -1, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 2 || fd.face == 3) && fd.depth == size - 1)
				|| (use && (fd.face == 0 || fd.face == 1) && fd.depth == s1 / 2)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != s1 / 2)) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, -s1 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, -1, -1, -1, i, j, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 1 || fd.face == 2) && fd.depth == size - 1)
				|| (use && (fd.face == 0 || fd.face == 3) && fd.depth == s1 / 2)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != size - 1)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != s1 / 2)) {
			glPushMatrix();
			glTranslate(0, -s1 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, -1, -1, -1, i, j);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 0 || fd.face == 1) && fd.depth == size - 1)
				|| (use && (fd.face == 2 || fd.face == 3) && fd.depth == s1 / 2)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != size - 1)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != s1 / 2)) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, -s1 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, -1, -1, j, -1, -1, i);
			glPopMatrix();
		}
		if (all || (use && fd.face == 0 && fd.depth == 0)
				|| (use && (fd.face == 1 || fd.face == 3) && fd.depth == s1 / 2)
				|| (use && fd.face == 2 && fd.depth == size - 1)
				|| (!use && fd.face == 0 && fd.depth != 0)
				|| (!use && (fd.face == 1 || fd.face == 3) && fd.depth != s1 / 2)
				|| (!use && fd.face == 2 && fd.depth != size - 1)) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, 0, -s1 * FACET_LENGTH);
			drawOctaFacet(mi, k, -1, -1, -1, -1, -1, k, -1);
			glPopMatrix();
		}
		if (all || (use && fd.face == 1 && fd.depth == 0)
				|| (use && (fd.face == 0 || fd.face == 2) && fd.depth == s1 / 2)
				|| (use && fd.face == 3 && fd.depth == size - 1)
				|| (!use && fd.face == 1 && fd.depth != 0)
				|| (!use && (fd.face == 0 || fd.face == 2) && fd.depth != s1 / 2)
				|| (!use && fd.face == 3 && fd.depth != size - 1)) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, 0, s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, k, -1, -1, -1, k, -1, -1);
			glPopMatrix();
		}
		if (all || (use && fd.face == 2 && fd.depth == 0)
				|| (use && (fd.face == 1 || fd.face == 3) && fd.depth == s1 / 2)
				|| (use && fd.face == 0 && fd.depth == size - 1)
				|| (!use && fd.face == 2 && fd.depth != 0)
				|| (!use && (fd.face == 1 || fd.face == 3) && fd.depth != s1 / 2)
				|| (!use && fd.face == 0 && fd.depth != size - 1)) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, 0, s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, k, -1, k, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && fd.face == 3 && fd.depth == 0)
				|| (use && (fd.face == 0 || fd.face == 2) && fd.depth == s1 / 2)
				|| (use && fd.face == 1 && fd.depth == size - 1)
				|| (!use && fd.face == 3 && fd.depth != 0)
				|| (!use && (fd.face == 0 || fd.face == 2) && fd.depth != s1 / 2)
				|| (!use && fd.face == 1 && fd.depth != size - 1)) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, 0, -s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, k, -1, -1, -1, k);
			glPopMatrix();
		}
	} else if (size > 2 && size % 2 == 0) {
		/* INSIDE MIDDLE EDGES - 1 */ /* (5, 7, 12), */
		int m = size / 2;
		s1 = size - 1;
		i = (size / 2) * (size / 2) + 1;
		j = (size / 2 + 1) * (size / 2 + 1) - 2;
		k = size * size - size;
		/* 0 */
		if (all || (!sticky && ((use && (fd.face == 0 || fd.face == 1) && fd.depth == 0)
				|| (use && fd.face == 2 && fd.depth == m)
				|| (use && fd.face == 3 && fd.depth == m - 1)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != 0)
				|| (!use && fd.face == 2 && fd.depth != m)
				|| (!use && fd.face == 3 && fd.depth != m - 1)))
			|| (sticky && ((use && (fd.face == 0 || fd.face == 1) && fd.depth == 0)
				|| (use && (fd.face == 2 || fd.face == 3) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != 0)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, s1 * FACET_LENGTH, -FACET_LENGTH);
			drawTetraFacet(mi, i, -1, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && (fd.face == 0 || fd.face == 3) && fd.depth == 0)
				|| (use && fd.face == 2 && fd.depth == m)
				|| (use && fd.face == 1 && fd.depth == m - 1)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != 0)
				|| (!use && fd.face == 2 && fd.depth != m)
				|| (!use && fd.face == 1 && fd.depth != m - 1)))
			|| (sticky && ((use && (fd.face == 0 || fd.face == 3) && fd.depth == 0)
				|| (use && (fd.face == 1 || fd.face == 2) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != 0)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(FACET_LENGTH, s1 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, j, -1, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && fd.face == 0 && fd.depth == 0)
				|| (use && fd.face == 2 && fd.depth == size - 1)
				|| (use && (fd.face == 1 || fd.face == 3) && fd.depth == m - 1)
				|| (!use && fd.face == 0 && fd.depth != 0)
				|| (!use && fd.face == 2 && fd.depth != size - 1)
				|| (!use && (fd.face == 1 || fd.face == 3) && fd.depth != m - 1)))
			|| (sticky && ((use && fd.face == 0 && fd.depth == 0)
				|| (use && fd.face == 2 && fd.depth == size - 1)
				|| (use && (fd.face == 1 || fd.face == 3) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face == 0 && fd.depth != 0)
				|| (!use && fd.face == 2 && fd.depth != size - 1)
				|| (!use && (fd.face == 1 || fd.face == 3) && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, k, -1, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		/* 1 */
		if (all || (!sticky && ((use && (fd.face == 1 || fd.face == 2) && fd.depth == 0)
				|| (use && fd.face == 3 && fd.depth == m)
				|| (use && fd.face == 0 && fd.depth == m - 1)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != 0)
				|| (!use && fd.face == 3 && fd.depth != m)
				|| (!use && fd.face == 0 && fd.depth != m - 1)))
			|| (sticky && ((use && (fd.face == 1 || fd.face == 2) && fd.depth == 0)
				|| (use && (fd.face == 0 || fd.face == 3) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != 0)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(FACET_LENGTH, s1 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, i, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && (fd.face == 0 || fd.face == 1) && fd.depth == 0)
				|| (use && fd.face == 3 && fd.depth == m)
				|| (use && fd.face == 2 && fd.depth == m - 1)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != 0)
				|| (!use && fd.face == 3 && fd.depth != m)
				|| (!use && fd.face == 2 && fd.depth != m - 1)))
			|| (sticky && ((use && (fd.face == 0 || fd.face == 1) && fd.depth == 0)
				|| (use && (fd.face == 2 || fd.face == 3) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != 0)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, s1 * FACET_LENGTH, FACET_LENGTH);
			drawTetraFacet(mi, -1, j, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && fd.face == 1 && fd.depth == 0)
				|| (use && fd.face == 3 && fd.depth == size - 1)
				|| (use && (fd.face == 0 || fd.face == 2) && fd.depth == m - 1)
				|| (!use && fd.face == 1 && fd.depth != 0)
				|| (!use && fd.face == 3 && fd.depth != size - 1)
				|| (!use && (fd.face == 0 || fd.face == 2) && fd.depth != m - 1)))
			|| (sticky && ((use && fd.face == 1 && fd.depth == 0)
				|| (use && fd.face == 3 && fd.depth == size - 1)
				|| (use && (fd.face == 0 || fd.face == 2) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face == 1 && fd.depth != 0)
				|| (!use && fd.face == 3 && fd.depth != size - 1)
				|| (!use && (fd.face == 0 || fd.face == 2) && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, k, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		/* 2 */
		if (all || (!sticky && ((use && (fd.face == 2 || fd.face == 3) && fd.depth == 0)
				|| (use && fd.face == 0 && fd.depth == m)
				|| (use && fd.face == 1 && fd.depth == m - 1)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != 0)
				|| (!use && fd.face == 0 && fd.depth != m)
				|| (!use && fd.face == 1 && fd.depth != m - 1)))
			|| (sticky && ((use && (fd.face == 2 || fd.face == 3) && fd.depth == 0)
				|| (use && (fd.face == 0 || fd.face == 1) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != 0)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, s1 * FACET_LENGTH, FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, i, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && (fd.face == 1 || fd.face == 2) && fd.depth == 0)
				|| (use && fd.face == 0 && fd.depth == m)
				|| (use && fd.face == 3 && fd.depth == m - 1)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != 0)
				|| (!use && fd.face == 0 && fd.depth != m)
				|| (!use && fd.face == 3 && fd.depth != m - 1)))
			|| (sticky && ((use && (fd.face == 1 || fd.face == 2) && fd.depth == 0)
				|| (use && (fd.face == 0 || fd.face == 3) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != 0)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(-FACET_LENGTH, s1 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, j, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && fd.face == 2 && fd.depth == 0)
				|| (use && fd.face == 0 && fd.depth == size - 1)
				|| (use && (fd.face == 1 || fd.face == 3) && fd.depth == m - 1)
				|| (!use && fd.face == 2 && fd.depth != 0)
				|| (!use && fd.face == 0 && fd.depth != size - 1)
				|| (!use && (fd.face == 1 || fd.face == 3) && fd.depth != m - 1)))
			|| (sticky && ((use && fd.face == 2 && fd.depth == 0)
				|| (use && fd.face == 0 && fd.depth == size - 1)
				|| (use && (fd.face == 1 || fd.face == 3) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face == 2 && fd.depth != 0)
				|| (!use && fd.face == 0 && fd.depth != size - 1)
				|| (!use && (fd.face == 1 || fd.face == 3) && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, k, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		/* 3 */
		if (all || (!sticky && ((use && (fd.face == 0 || fd.face == 3) && fd.depth == 0)
				|| (use && fd.face == 1 && fd.depth == m)
				|| (use && fd.face == 2 && fd.depth == m - 1)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != 0)
				|| (!use && fd.face == 1 && fd.depth != m)
				|| (!use && fd.face == 2 && fd.depth != m - 1)))
			|| (sticky && ((use && (fd.face == 0 || fd.face == 3) && fd.depth == 0)
				|| (use && (fd.face == 1 || fd.face == 2) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != 0)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(-FACET_LENGTH, s1 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, i, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && (fd.face == 2 || fd.face == 3) && fd.depth == 0)
				|| (use && fd.face == 1 && fd.depth == m)
				|| (use && fd.face == 0 && fd.depth == m - 1)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != 0)
				|| (!use && fd.face == 1 && fd.depth != m)
				|| (!use && fd.face == 0 && fd.depth != m - 1)))
			|| (sticky && ((use && (fd.face == 2 || fd.face == 3) && fd.depth == 0)
				|| (use && (fd.face == 0 || fd.face == 1) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != 0)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, s1 * FACET_LENGTH, -FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, j, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && fd.face == 3 && fd.depth == 0)
				|| (use && fd.face == 1 && fd.depth == size - 1)
				|| (use && (fd.face == 0 || fd.face == 2) && fd.depth == m - 1)
				|| (!use && fd.face == 3 && fd.depth != 0)
				|| (!use && fd.face == 1 && fd.depth != size - 1)
				|| (!use && (fd.face == 0 || fd.face == 2) && fd.depth != m - 1)))
			|| (sticky && ((use && fd.face == 3 && fd.depth == 0)
				|| (use && fd.face == 1 && fd.depth == size - 1)
				|| (use && (fd.face == 0 || fd.face == 2) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face == 3 && fd.depth != 0)
				|| (!use && fd.face == 1 && fd.depth != size - 1)
				|| (!use && (fd.face == 0 || fd.face == 2) && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, k, -1, -1, -1, -1);
			glPopMatrix();
		}
		/* 4 */
		if (all || (!sticky && ((use && (fd.face == 0 || fd.face == 3) && fd.depth == size - 1)
				|| (use && fd.face == 1 && fd.depth == m)
				|| (use && fd.face == 2 && fd.depth == m - 1)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && fd.face == 1 && fd.depth != m)
				|| (!use && fd.face == 2 && fd.depth != m - 1)))
			|| (sticky && ((use && (fd.face == 0 || fd.face == 3) && fd.depth == size - 1)
				|| (use && (fd.face == 1 || fd.face == 2) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(-FACET_LENGTH, -s1 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, i, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && (fd.face == 0 || fd.face == 1) && fd.depth == size - 1)
				|| (use && fd.face == 3 && fd.depth == m)
				|| (use && fd.face == 2 && fd.depth == m - 1)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != size - 1)
				|| (!use && fd.face == 3 && fd.depth != m)
				|| (!use && fd.face == 2 && fd.depth != m - 1)))
			|| (sticky && ((use && (fd.face == 0 || fd.face == 1) && fd.depth == size - 1)
				|| (use && (fd.face == 2 || fd.face == 3) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != size - 1)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, -s1 * FACET_LENGTH, FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, j, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && fd.face == 0 && fd.depth == size - 1)
				|| (use && fd.face == 2 && fd.depth == 0)
				|| (use && (fd.face == 1 || fd.face == 3) && fd.depth == m)
				|| (!use && fd.face == 0 && fd.depth != size - 1)
				|| (!use && fd.face == 2 && fd.depth != 0)
				|| (!use && (fd.face == 1 || fd.face == 3) && fd.depth != m)))
			|| (sticky && ((use && fd.face == 0 && fd.depth == size - 1)
				|| (use && fd.face == 2 && fd.depth == 0)
				|| (use && (fd.face == 1 || fd.face == 3) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face == 0 && fd.depth != size - 1)
				|| (!use && fd.face == 2 && fd.depth != 0)
				|| (!use && (fd.face == 1 || fd.face == 3) && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, -FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, k, -1, -1, -1);
			glPopMatrix();
		}
		/* 5 */
		if (all || (!sticky && ((use && (fd.face == 2 || fd.face == 3) && fd.depth == size - 1)
				|| (use && fd.face == 0 && fd.depth == m)
				|| (use && fd.face == 1 && fd.depth == m - 1)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && fd.face == 0 && fd.depth != m)
				|| (!use && fd.face == 1 && fd.depth != m - 1)))
			|| (sticky && ((use && (fd.face == 2 || fd.face == 3) && fd.depth == size - 1)
				|| (use && (fd.face == 0 || fd.face == 1) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, -s1 * FACET_LENGTH, FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, i, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && (fd.face == 0 || fd.face == 3) && fd.depth == size - 1)
				|| (use && fd.face == 2 && fd.depth == m)
				|| (use && fd.face == 1 && fd.depth == m - 1)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && fd.face == 2 && fd.depth != m)
				|| (!use && fd.face == 1 && fd.depth != m - 1)))
			|| (sticky && ((use && (fd.face == 0 || fd.face == 3) && fd.depth == size - 1)
				|| (use && (fd.face == 1 || fd.face == 2) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(FACET_LENGTH, -s1 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, j, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && fd.face == 3 && fd.depth == size - 1)
				|| (use && fd.face == 1 && fd.depth == 0)
				|| (use && (fd.face == 0 || fd.face == 2) && fd.depth == m)
				|| (!use && fd.face == 3 && fd.depth != size - 1)
				|| (!use && fd.face == 1 && fd.depth != 0)
				|| (!use && (fd.face == 0 || fd.face == 2) && fd.depth != m)))
			|| (sticky && ((use && fd.face == 3 && fd.depth == size - 1)
				|| (use && fd.face == 1 && fd.depth == 0)
				|| (use && (fd.face == 0 || fd.face == 2) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face == 3 && fd.depth != size - 1)
				|| (!use && fd.face == 1 && fd.depth != 0)
				|| (!use && (fd.face == 0 || fd.face == 2) && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, -FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, k, -1, -1);
			glPopMatrix();
		}
		/* 6 */
		if (all || (!sticky && ((use && (fd.face == 1 || fd.face == 2) && fd.depth == size - 1)
				|| (use && fd.face == 3 && fd.depth == m)
				|| (use && fd.face == 0 && fd.depth == m - 1)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != size - 1)
				|| (!use && fd.face == 3 && fd.depth != m)
				|| (!use && fd.face == 0 && fd.depth != m - 1)))
			|| (sticky && ((use && (fd.face == 1 || fd.face == 2) && fd.depth == size - 1)
				|| (use && (fd.face == 0 || fd.face == 3) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != size - 1)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(FACET_LENGTH, -s1 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, i, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && (fd.face == 2 || fd.face == 3) && fd.depth == size - 1)
				|| (use && fd.face == 1 && fd.depth == m)
				|| (use && fd.face == 0 && fd.depth == m - 1)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && fd.face == 1 && fd.depth != m)
				|| (!use && fd.face == 0 && fd.depth != m - 1)))
			|| (sticky && ((use && (fd.face == 2 || fd.face == 3) && fd.depth == size - 1)
				|| (use && (fd.face == 0 || fd.face == 1) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, -s1 * FACET_LENGTH, -FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, j, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && fd.face == 2 && fd.depth == size - 1)
				|| (use && fd.face == 0 && fd.depth == 0)
				|| (use && (fd.face == 1 || fd.face == 3) && fd.depth == m)
				|| (!use && fd.face == 2 && fd.depth != size - 1)
				|| (!use && fd.face == 0 && fd.depth != 0)
				|| (!use && (fd.face == 1 || fd.face == 3) && fd.depth != m)))
			|| (sticky && ((use && fd.face == 2 && fd.depth == size - 1)
				|| (use && fd.face == 0 && fd.depth == 0)
				|| (use && (fd.face == 1 || fd.face == 3) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face == 2 && fd.depth != size - 1)
				|| (!use && fd.face == 0 && fd.depth != 0)
				|| (!use && (fd.face == 1 || fd.face == 3) && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, -FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, k, -1);
			glPopMatrix();
		}
		/* 7 */
		if (all || (!sticky && ((use && (fd.face == 0 || fd.face == 1) && fd.depth == size - 1)
				|| (use && fd.face == 2 && fd.depth == m)
				|| (use && fd.face == 3 && fd.depth == m - 1)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != size - 1)
				|| (!use && fd.face == 2 && fd.depth != m)
				|| (!use && fd.face == 3 && fd.depth != m - 1)))
			|| (sticky && ((use && (fd.face == 0 || fd.face == 1) && fd.depth == size - 1)
				|| (use && (fd.face == 2 || fd.face == 3) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != size - 1)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, -s1 * FACET_LENGTH, -FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, -1, i);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && (fd.face == 1 || fd.face == 2) && fd.depth == size - 1)
				|| (use && fd.face == 0 && fd.depth == m)
				|| (use && fd.face == 3 && fd.depth == m - 1)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != size - 1)
				|| (!use && fd.face == 0 && fd.depth != m)
				|| (!use && fd.face == 3 && fd.depth != m - 1)))
			|| (sticky && ((use && (fd.face == 1 || fd.face == 2) && fd.depth == size - 1)
				|| (use && (fd.face == 0 || fd.face == 3) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != size - 1)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(-FACET_LENGTH, -s1 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, -1, j);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && fd.face == 1 && fd.depth == size - 1)
				|| (use && fd.face == 3 && fd.depth == 0)
				|| (use && (fd.face == 0 || fd.face == 2) && fd.depth == m)
				|| (!use && fd.face == 1 && fd.depth != size - 1)
				|| (!use && fd.face == 3 && fd.depth != 0)
				|| (!use && (fd.face == 0 || fd.face == 2) && fd.depth != m)))
			|| (sticky && ((use && fd.face == 1 && fd.depth == size - 1)
				|| (use && fd.face == 3 && fd.depth == 0)
				|| (use && (fd.face == 0 || fd.face == 2) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face == 1 && fd.depth != size - 1)
				|| (!use && fd.face == 3 && fd.depth != 0)
				|| (!use && (fd.face == 0 || fd.face == 2) && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, -FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, -1, k);
			glPopMatrix();
		}
	}
	if (sticky) { /* OUTSIDE EDGE 1 */
		int a, b, c, d, e, f;
		edge = size / 2 - 1;
		s0 = 2 * size - 2 * edge - 2; 
		s1 = 2 * edge; /* 2, 4 */
		a = edge * edge; /* 1, 4 */
		b = (edge + 1) * (edge + 1) - 1; /* 3, 8 */
		c = (size - edge - 1) * (size - edge - 1); /* 4, 9, 16 */
		d = (size - edge) * (size - edge) - 1; /* 8, 15 */
		e = (size - 1) * (size - 1) + 2 * edge; /* 11, */
		f = size * size - 2 * edge - 1; /* 13, */
		if (all || ((use && (fd.face == 0 || fd.face == 1) && fd.depth == 0)
				|| (use && (fd.face == 2 || fd.face == 3) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != 0)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != 1 && fd.depth != 2))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, s0 * FACET_LENGTH, 0);
			drawOctaFacet(mi, a, b, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 1 || fd.face == 2) && fd.depth == 0)
				|| (use && (fd.face == 0 || fd.face == 3) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != 0)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != 1 && fd.depth != 2))) {
			glPushMatrix();
			glTranslate(0, s0 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, a, b, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 2 || fd.face == 3) && fd.depth == 0)
				|| (use && (fd.face == 0 || fd.face == 1) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != 0)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != 1 && fd.depth != 2))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, s0 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, a, b, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 0 || fd.face == 3) && fd.depth == 0)
				|| (use && (fd.face == 1 || fd.face == 2) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != 0)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != 1 && fd.depth != 2))) {
			glPushMatrix();
			glTranslate(0, s0 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawOctaFacet(mi, b, -1, -1, a, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 0 || fd.face == 3) && fd.depth == size - 1)
				|| (use && (fd.face == 1 || fd.face == 2) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != 1 && fd.depth != 2))) {
			glPushMatrix();
			glTranslate(0, -s0 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, -1, a, b, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 2 || fd.face == 3) && fd.depth == size - 1)
				|| (use && (fd.face == 0 || fd.face == 1) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != 1 && fd.depth != 2))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, -s0 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, -1, -1, -1, a, b, -1);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 1 || fd.face == 2) && fd.depth == size - 1)
				|| (use && (fd.face == 0 || fd.face == 3) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != size - 1)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != 1 && fd.depth != 2))) {
			glPushMatrix();
			glTranslate(0, -s0 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, -1, -1, -1, a, b);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 0 || fd.face == 1) && fd.depth == size - 1)
				|| (use && (fd.face == 2 || fd.face == 3) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != size - 1)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != 1 && fd.depth != 2))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, -s0 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, -1, -1, b, -1, -1, a);
			glPopMatrix();
		}

		if (all || (sticky && ((use && (fd.face == 0 || fd.face == 1) && fd.depth == 0)
				|| (use && (fd.face == 2 || fd.face == 3) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != 0)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != 1 && fd.depth != 2)))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, s1 * FACET_LENGTH, 0);
			drawOctaFacet(mi, c, d, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 1 || fd.face == 2) && fd.depth == 0)
				|| (use && (fd.face == 0 || fd.face == 3) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != 0)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != 1 && fd.depth != 2))) {
			glPushMatrix();
			glTranslate(0, s1 * FACET_LENGTH, s0 * FACET_LENGTH);
			drawOctaFacet(mi, -1, c, d, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 2 || fd.face == 3) && fd.depth == 0)
				|| (use && (fd.face == 0 || fd.face == 1) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != 0)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != 1 && fd.depth != 2))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, s1 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, c, d, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 0 || fd.face == 3) && fd.depth == 0)
				|| (use && (fd.face == 1 || fd.face == 2) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != 0)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != 1 && fd.depth != 2))) {
			glPushMatrix();
			glTranslate(0, s1 * FACET_LENGTH, -s0 * FACET_LENGTH);
			drawOctaFacet(mi, d, -1, -1, c, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 0 || fd.face == 3) && fd.depth == size - 1)
				|| (use && (fd.face == 1 || fd.face == 2) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != 1 && fd.depth != 2))) {
			glPushMatrix();
			glTranslate(0, -s1 * FACET_LENGTH, s0 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, -1, c, d, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 2 || fd.face == 3) && fd.depth == size - 1)
				|| (use && (fd.face == 0 || fd.face == 1) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != 1 && fd.depth != 2))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, -s1 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, -1, -1, -1, c, d, -1);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 1 || fd.face == 2) && fd.depth == size - 1)
				|| (use && (fd.face == 0 || fd.face == 3) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != size - 1)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != 1 && fd.depth != 2))) {
			glPushMatrix();
			glTranslate(0, -s1 * FACET_LENGTH, -s0 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, -1, -1, -1, c, d);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 0 || fd.face == 1) && fd.depth == size - 1)
				|| (use && (fd.face == 2 || fd.face == 3) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != size - 1)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != 1 && fd.depth != 2))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, -s1 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, -1, -1, d, -1, -1, c);
			glPopMatrix();
		}

		if (all || ((use && fd.face == 0 && fd.depth == 0)
				|| (use && fd.face == 2 && fd.depth == size - 1)
				|| (use && (fd.face == 1 || fd.face == 3) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face == 0 && fd.depth != 0)
				|| (!use && fd.face == 2 && fd.depth != size - 1)
				|| (!use && (fd.face == 1 || fd.face == 3) && fd.depth != 1 && fd.depth != 2))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, 0, -s1 * FACET_LENGTH);
			drawOctaFacet(mi, e, -1, -1, -1, -1, -1, f, -1);
			glPopMatrix();
		}
		if (all || ((use && fd.face == 1 && fd.depth == 0)
				|| (use && fd.face == 3 && fd.depth == size - 1)
				|| (use && (fd.face == 0 || fd.face == 2) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face == 1 && fd.depth != 0)
				|| (!use && fd.face == 3 && fd.depth != size - 1)
				|| (!use && (fd.face == 0 || fd.face == 2) && fd.depth != 1 && fd.depth != 2))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, 0, s0 * FACET_LENGTH);
			drawOctaFacet(mi, -1, e, -1, -1, -1, f, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && fd.face == 2 && fd.depth == 0)
				|| (use && fd.face == 0 && fd.depth == size - 1)
				|| (use && (fd.face == 1 || fd.face == 3) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face == 2 && fd.depth != 0)
				|| (!use && fd.face == 0 && fd.depth != size - 1)
				|| (!use && (fd.face == 1 || fd.face == 3) && fd.depth != 1 && fd.depth != 2))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, 0, s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, e, -1, f, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && fd.face == 3 && fd.depth == 0)
				|| (use && fd.face == 1 && fd.depth == size - 1)
				|| (use && (fd.face == 0 || fd.face == 2) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face == 3 && fd.depth != 0)
				|| (!use && fd.face == 1 && fd.depth != size - 1)
				|| (!use && (fd.face == 0 || fd.face == 2) && fd.depth != 1 && fd.depth != 2))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, 0, -s0 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, e, -1, -1, -1, f);
			glPopMatrix();
		}
		if (all || ((use && fd.face == 2 && fd.depth == 0)
				|| (use && fd.face == 0 && fd.depth == size - 1)
				|| (use && (fd.face == 1 || fd.face == 3) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face == 2 && fd.depth != 0)
				|| (!use && fd.face == 0 && fd.depth != size - 1)
				|| (!use && (fd.face == 1 || fd.face == 3) && fd.depth != 1 && fd.depth != 2))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, 0, s0 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, f, -1, e, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && fd.face == 1 && fd.depth == 0)
				|| (use && fd.face == 3 && fd.depth == size - 1)
				|| (use && (fd.face == 0 || fd.face == 2) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face == 1 && fd.depth != 0)
				|| (!use && fd.face == 3 && fd.depth != size - 1)
				|| (!use && (fd.face == 0 || fd.face == 2) && fd.depth != 1 && fd.depth != 2))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, 0, s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, f, -1, -1, -1, e, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && fd.face == 0 && fd.depth == 0)
				|| (use && fd.face == 2 && fd.depth == size - 1)
				|| (use && (fd.face == 1 || fd.face == 3) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face == 0 && fd.depth != 0)
				|| (!use && fd.face == 2 && fd.depth != size - 1)
				|| (!use && (fd.face == 1 || fd.face == 3) && fd.depth != 1 && fd.depth != 2))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, 0, -s0 * FACET_LENGTH);
			drawOctaFacet(mi, f, -1, -1, -1, -1, -1, e, -1);
			glPopMatrix();
		}
		if (all || ((use && fd.face == 3 && fd.depth == 0)
				|| (use && fd.face == 1 && fd.depth == size - 1)
				|| (use && (fd.face == 0 || fd.face == 2) && (fd.depth == 1 || fd.depth == 2))
				|| (!use && fd.face == 3 && fd.depth != 0)
				|| (!use && fd.face == 1 && fd.depth != size - 1)
				|| (!use && (fd.face == 0 || fd.face == 2) && fd.depth != 1 && fd.depth != 2))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, 0, -s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, f, -1, -1, -1, e);
			glPopMatrix();
		}
	}
	for (edge = 1; edge < (size - 1) / 2; edge++) {
		/*if (size > 4) INSIDE EDGE 2 */
		int s3, m1, m2;
		int a, b, c, d, e, f;
		s1 = 1;
		s2 = 2 * size - 2 * edge - 3; /* 2 * size - 5 */
		s3 = 2 * edge + 1; /* 3 */
		a = (edge + 1) * (edge + 1) + 1; /* 5 */
		b = (edge + 2) * (edge + 2) - 2; /* 7 */
		c = (size - edge - 1) * (size - edge - 1) + 1;/* (size - 2) * (size - 2) + 1 */
		d = (size - edge) * (size - edge) - 2; /* (size - 1) * (size - 1) - 2 */
		e = (size - 1) * ( size - 1) + 2 * edge + 1; /* (size - 1) * (size - 1) + 3 */
		f = size * size - 2 * edge - 2; /* size * size - 4 */
		m1 = edge;
		m2 = edge + 1;
		if (all || (use && (fd.face == 0 || fd.face == 1) && fd.depth == 0)
				|| (use && fd.face == 3 && fd.depth == m1)
				|| (use && fd.face == 2 && fd.depth == m2)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != 0)
				|| (!use && fd.face == 3 && fd.depth != m1)
				|| (!use && fd.face == 2 && fd.depth != m2)) {
			glPushMatrix();
			glTranslate(s3 * FACET_LENGTH, s2 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, a, -1, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 0 || fd.face == 1) && fd.depth == 0)
				|| (use && fd.face == 2 && fd.depth == m1)
				|| (use && fd.face == 3 && fd.depth == m2)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != 0)
				|| (!use && fd.face == 2 && fd.depth != m1)
				|| (!use && fd.face == 3 && fd.depth != m2)) {
			glPushMatrix();
			glTranslate(s3 * FACET_LENGTH, s2 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, b, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 1 || fd.face == 2) && fd.depth == 0)
				|| (use && fd.face == 0 && fd.depth == m1)
				|| (use && fd.face == 3 && fd.depth == m2)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != 0)
				|| (!use && fd.face == 0 && fd.depth != m1)
				|| (!use && fd.face == 3 && fd.depth != m2)) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, s2 * FACET_LENGTH, s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, a, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 1 || fd.face == 2) && fd.depth == 0)
				|| (use && fd.face == 3 && fd.depth == m1)
				|| (use && fd.face == 0 && fd.depth == m2)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != 0)
				|| (!use && fd.face == 3 && fd.depth != m1)
				|| (!use && fd.face == 0 && fd.depth != m2)) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, s2 * FACET_LENGTH, s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, b, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 2 || fd.face == 3) && fd.depth == 0)
				|| (use && fd.face == 1 && fd.depth == m1)
				|| (use && fd.face == 0 && fd.depth == m2)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != 0)
				|| (!use && fd.face == 1 && fd.depth != m1)
				|| (!use && fd.face == 0 && fd.depth != m2)) {
			glPushMatrix();
			glTranslate(-s3 * FACET_LENGTH, s2 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, a, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 2 || fd.face == 3) && fd.depth == 0)
				|| (use && fd.face == 0 && fd.depth == m1)
				|| (use && fd.face == 1 && fd.depth == m2)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != 0)
				|| (!use && fd.face == 0 && fd.depth != m1)
				|| (!use && fd.face == 1 && fd.depth != m2)) {
			glPushMatrix();
			glTranslate(-s3 * FACET_LENGTH, s2 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, b, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 0 || fd.face == 3) && fd.depth == 0)
				|| (use && fd.face == 2 && fd.depth == m1)
				|| (use && fd.face == 1 && fd.depth == m2)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != 0)
				|| (!use && fd.face == 2 && fd.depth != m1)
				|| (!use && fd.face == 1 && fd.depth != m2)) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, s2 * FACET_LENGTH, -s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, a, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 0 || fd.face == 3) && fd.depth == 0)
				|| (use && fd.face == 1 && fd.depth == m1)
				|| (use && fd.face == 2 && fd.depth == m2)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != 0)
				|| (!use && fd.face == 1 && fd.depth != m1)
				|| (!use && fd.face == 2 && fd.depth != m2)) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, s2 * FACET_LENGTH, -s3 * FACET_LENGTH);
			drawTetraFacet(mi, b, -1, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}

		if (all || (use && (fd.face == 0 || fd.face == 3) && fd.depth == size - 1)
				|| (use && fd.face == 1 && fd.depth == size - 1 - m1)
				|| (use && fd.face == 2 && fd.depth == size - 1 - m2)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && fd.face == 1 && fd.depth != size - 1 - m1)
				|| (!use && fd.face == 2 && fd.depth != size - 1 - m2)) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, -s2 * FACET_LENGTH, s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, a, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 0 || fd.face == 3) && fd.depth == size - 1)
				|| (use && fd.face == 2 && fd.depth == size - 1 - m1)
				|| (use && fd.face == 1 && fd.depth == size - 1 - m2)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && fd.face == 2 && fd.depth != size - 1 - m1)
				|| (!use && fd.face == 1 && fd.depth != size - 1 - m2)) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, -s2 * FACET_LENGTH, s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, b, -1, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 2 || fd.face == 3) && fd.depth == size - 1)
				|| (use && fd.face == 0 && fd.depth == size - 1 - m1)
				|| (use && fd.face == 1 && fd.depth == size - 1 - m2)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && fd.face == 0 && fd.depth != size - 1 - m1)
				|| (!use && fd.face == 1 && fd.depth != size - 1 - m2)) {
			glPushMatrix();
			glTranslate(s3 * FACET_LENGTH, -s2 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, a, -1, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 2 || fd.face == 3) && fd.depth == size - 1)
				|| (use && fd.face == 1 && fd.depth == size - 1 - m1)
				|| (use && fd.face == 0 && fd.depth == size - 1 - m2)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && fd.face == 1 && fd.depth != size - 1 - m1)
				|| (!use && fd.face == 0 && fd.depth != size - 1 - m2)) {
			glPushMatrix();
			glTranslate(s3 * FACET_LENGTH, -s2 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, b, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 1 || fd.face == 2) && fd.depth == size - 1)
				|| (use && fd.face == 3 && fd.depth == size - 1 - m1)
				|| (use && fd.face == 0 && fd.depth == size - 1 - m2)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != size - 1)
				|| (!use && fd.face == 3 && fd.depth != size - 1 - m1)
				|| (!use && fd.face == 0 && fd.depth != size - 1 - m2)) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, -s2 * FACET_LENGTH, -s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, a, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 1 || fd.face == 2) && fd.depth == size - 1)
				|| (use && fd.face == 0 && fd.depth == size - 1 - m1)
				|| (use && fd.face == 3 && fd.depth == size - 1 - m2)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != size - 1)
				|| (!use && fd.face == 0 && fd.depth != size - 1 - m1)
				|| (!use && fd.face == 3 && fd.depth != size - 1 - m2)) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, -s2 * FACET_LENGTH, -s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, -1, b);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 0 || fd.face == 1) && fd.depth == size - 1)
				|| (use && fd.face == 2 && fd.depth == size - 1 - m1)
				|| (use && fd.face == 3 && fd.depth == size - 1 - m2)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != size - 1)
				|| (!use && fd.face == 2 && fd.depth != size - 1 - m1)
				|| (!use && fd.face == 3 && fd.depth != size - 1 - m2)) {
			glPushMatrix();
			glTranslate(-s3 * FACET_LENGTH, -s2 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, -1, a);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 0 || fd.face == 1) && fd.depth == size - 1)
				|| (use && fd.face == 3 && fd.depth == size - 1 - m1)
				|| (use && fd.face == 2 && fd.depth == size - 1 - m2)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != size - 1)
				|| (!use && fd.face == 3 && fd.depth != size - 1 - m1)
				|| (!use && fd.face == 2 && fd.depth != size - 1 - m2)) {
			glPushMatrix();
			glTranslate(-s3 * FACET_LENGTH, -s2 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, b, -1, -1, -1);
			glPopMatrix();
		}

		if (all || (use && (fd.face == 0 || fd.face == 1) && fd.depth == 0)
				|| (use && fd.face == 2 && fd.depth == size - 1 - m1)
				|| (use && fd.face == 3 && fd.depth == size - 1 - m2)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != 0)
				|| (!use && fd.face == 2 && fd.depth != size - 1 - m1)
				|| (!use && fd.face == 3 && fd.depth != size - 1 - m2)) {
			glPushMatrix();
			glTranslate(s2 * FACET_LENGTH, s3 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, c, -1, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 0 || fd.face == 1) && fd.depth == 0)
				|| (use && fd.face == 3 && fd.depth == size - 1 - m1)
				|| (use && fd.face == 2 && fd.depth == size - 1 - m2)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != 0)
				|| (!use && fd.face == 3 && fd.depth != size - 1 - m1)
				|| (!use && fd.face == 2 && fd.depth != size - 1 - m2)) {
			glPushMatrix();
			glTranslate(s2 * FACET_LENGTH, s3 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, d, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 1 || fd.face == 2) && fd.depth == 0)
				|| (use && fd.face == 3 && fd.depth == size - 1 - m1)
				|| (use && fd.face == 0 && fd.depth == size - 1 - m2)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != 0)
				|| (!use && fd.face == 3 && fd.depth != size - 1 - m1)
				|| (!use && fd.face == 0 && fd.depth != size - 1 - m2)) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, s3 * FACET_LENGTH, s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, c, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 1 || fd.face == 2) && fd.depth == 0)
				|| (use && fd.face == 0 && fd.depth == size - 1 - m1)
				|| (use && fd.face == 3 && fd.depth == size - 1 - m2)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != 0)
				|| (!use && fd.face == 0 && fd.depth != size - 1 - m1)
				|| (!use && fd.face == 3 && fd.depth != size - 1 - m2)) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, s3 * FACET_LENGTH, s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, d, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 2 || fd.face == 3) && fd.depth == 0)
				|| (use && fd.face == 0 && fd.depth == size - 1 - m1)
				|| (use && fd.face == 1 && fd.depth == size - 1 - m2)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != 0)
				|| (!use && fd.face == 0 && fd.depth != size - 1 - m1)
				|| (!use && fd.face == 1 && fd.depth != size - 1 - m2)) {
			glPushMatrix();
			glTranslate(-s2 * FACET_LENGTH, s3 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, c, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 2 || fd.face == 3) && fd.depth == 0)
				|| (use && fd.face == 1 && fd.depth == size - 1 - m1)
				|| (use && fd.face == 0 && fd.depth == size - 1 - m2)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != 0)
				|| (!use && fd.face == 1 && fd.depth != size - 1 - m1)
				|| (!use && fd.face == 0 && fd.depth != size - 1 - m2)) {
			glPushMatrix();
			glTranslate(-s2 * FACET_LENGTH, s3 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, d, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 0 || fd.face == 3) && fd.depth == 0)
				|| (use && fd.face == 1 && fd.depth == size - 1 - m1)
				|| (use && fd.face == 2 && fd.depth == size - 1 - m2)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != 0)
				|| (!use && fd.face == 1 && fd.depth != size - 1 - m1)
				|| (!use && fd.face == 2 && fd.depth != size - 1 - m2)) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, s3 * FACET_LENGTH, -s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, c, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 0 || fd.face == 3) && fd.depth == 0)
				|| (use && fd.face == 2 && fd.depth == size - 1 - m1)
				|| (use && fd.face == 1 && fd.depth == size - 1 - m2)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != 0)
				|| (!use && fd.face == 2 && fd.depth != size - 1 - m1)
				|| (!use && fd.face == 1 && fd.depth != size - 1 - m2)) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, s3 * FACET_LENGTH, -s2 * FACET_LENGTH);
			drawTetraFacet(mi, d, -1, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 0 || fd.face == 3) && fd.depth == size - 1)
				|| (use && fd.face == 2 && fd.depth == m1)
				|| (use && fd.face == 1 && fd.depth == m2)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && fd.face == 2 && fd.depth != m1)
				|| (!use && fd.face == 1 && fd.depth != m2)) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, -s3 * FACET_LENGTH, s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, c, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 0 || fd.face == 3) && fd.depth == size - 1)
				|| (use && fd.face == 1 && fd.depth == m1)
				|| (use && fd.face == 2 && fd.depth == m2)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && fd.face == 1 && fd.depth != m1)
				|| (!use && fd.face == 2 && fd.depth != m2)) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, -s3 * FACET_LENGTH, s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, d, -1, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 2 || fd.face == 3) && fd.depth == size - 1)
				|| (use && fd.face == 1 && fd.depth == m1)
				|| (use && fd.face == 0 && fd.depth == m2)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && fd.face == 1 && fd.depth != m1)
				|| (!use && fd.face == 0 && fd.depth != m2)) {
			glPushMatrix();
			glTranslate(s2 * FACET_LENGTH, -s3 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, c, -1, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 2 || fd.face == 3) && fd.depth == size - 1)
				|| (use && fd.face == 0 && fd.depth == m1)
				|| (use && fd.face == 1 && fd.depth == m2)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && fd.face == 0 && fd.depth != m1)
				|| (!use && fd.face == 1 && fd.depth != m2)) {
			glPushMatrix();
			glTranslate(s2 * FACET_LENGTH, -s3 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, d, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 1 || fd.face == 2) && fd.depth == size - 1)
				|| (use && fd.face == 0 && fd.depth == m1)
				|| (use && fd.face == 3 && fd.depth == m2)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != size - 1)
				|| (!use && fd.face == 0 && fd.depth != m1)
				|| (!use && fd.face == 3 && fd.depth != m2)) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, -s3 * FACET_LENGTH, -s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, c, -1);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 1 || fd.face == 2) && fd.depth == size - 1)
				|| (use && fd.face == 3 && fd.depth == m1)
				|| (use && fd.face == 0 && fd.depth == m2)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != size - 1)
				|| (!use && fd.face == 3 && fd.depth != m1)
				|| (!use && fd.face == 0 && fd.depth != m2)) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, -s3 * FACET_LENGTH, -s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, -1, d);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 0 || fd.face == 1) && fd.depth == size - 1)
				|| (use && fd.face == 3 && fd.depth == m1)
				|| (use && fd.face == 2 && fd.depth == m2)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != size - 1)
				|| (!use && fd.face == 3 && fd.depth != m1)
				|| (!use && fd.face == 2 && fd.depth != m2)) {
			glPushMatrix();
			glTranslate(-s2 * FACET_LENGTH, -s3 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, -1, c);
			glPopMatrix();
		}
		if (all || (use && (fd.face == 0 || fd.face == 1) && fd.depth == size - 1)
				|| (use && fd.face == 2 && fd.depth == m1)
				|| (use && fd.face == 3 && fd.depth == m2)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != size - 1)
				|| (!use && fd.face == 2 && fd.depth != m1)
				|| (!use && fd.face == 3 && fd.depth != m2)) {
			glPushMatrix();
			glTranslate(-s2 * FACET_LENGTH, -s3 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, d, -1, -1, -1);
			glPopMatrix();
		}

		if (all || (use && fd.face == 0 && fd.depth == 0)
				|| (use && fd.face == 2 && fd.depth == size - 1)
				|| (use && fd.face == 1 && fd.depth == m1)
				|| (use && fd.face == 3 && fd.depth == size - 1 - m2)
				|| (!use && fd.face == 0 && fd.depth != 0)
				|| (!use && fd.face == 2 && fd.depth != size - 1)
				|| (!use && fd.face == 1 && fd.depth != m1)
				|| (!use && fd.face == 3 && fd.depth != size - m2 - 1)) {
			glPushMatrix();
			glTranslate(s2 * FACET_LENGTH, s1 * FACET_LENGTH, -s3 * FACET_LENGTH);
			drawTetraFacet(mi, e, -1, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && fd.face == 0 && fd.depth == 0)
				|| (use && fd.face == 2 && fd.depth == size - 1)
				|| (use && fd.face == 3 && fd.depth == size - 1 - m1)
				|| (use && fd.face == 1 && fd.depth == m2)
				|| (!use && fd.face == 0 && fd.depth != 0)
				|| (!use && fd.face == 2 && fd.depth != size - 1)
				|| (!use && fd.face == 3 && fd.depth != size - 1 - m1)
				|| (!use && fd.face == 1 && fd.depth != m2)) {
			glPushMatrix();
			glTranslate(s2 * FACET_LENGTH, -s1 * FACET_LENGTH, -s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, f, -1);
			glPopMatrix();
		}
		if (all || (use && fd.face == 1 && fd.depth == 0)
				|| (use && fd.face == 3 && fd.depth == size - 1)
				|| (use && fd.face == 2 && fd.depth == m1)
				|| (use && fd.face == 0 && fd.depth == size - 1 - m2)
				|| (!use && fd.face == 1 && fd.depth != 0)
				|| (!use && fd.face == 3 && fd.depth != size - 1)
				|| (!use && fd.face == 2 && fd.depth != m1)
				|| (!use && fd.face == 0 && fd.depth != size - 1 - m2)) {
			glPushMatrix();
			glTranslate(s3 * FACET_LENGTH, s1 * FACET_LENGTH, s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, e, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && fd.face == 1 && fd.depth == 0)
				|| (use && fd.face == 3 && fd.depth == size - 1)
				|| (use && fd.face == 0 && fd.depth == size - 1 - m1)
				|| (use && fd.face == 2 && fd.depth == m2)
				|| (!use && fd.face == 1 && fd.depth != 0)
				|| (!use && fd.face == 3 && fd.depth != size - 1)
				|| (!use && fd.face == 0 && fd.depth != size - 1 - m1)
				|| (!use && fd.face == 2 && fd.depth != m2)) {
			glPushMatrix();
			glTranslate(s3 * FACET_LENGTH, -s1 * FACET_LENGTH, s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, f, -1, -1);
			glPopMatrix();
		}
		if (all || (use && fd.face == 2 && fd.depth == 0)
				|| (use && fd.face == 0 && fd.depth == size - 1)
				|| (use && fd.face == 3 && fd.depth == m1)
				|| (use && fd.face == 1 && fd.depth == size - 1 - m2)
				|| (!use && fd.face == 2 && fd.depth != 0)
				|| (!use && fd.face == 0 && fd.depth != size - 1)
				|| (!use && fd.face == 3 && fd.depth != m1)
				|| (!use && fd.face == 1 && fd.depth != size - 1 - m2)) {
			glPushMatrix();
			glTranslate(-s2 * FACET_LENGTH, s1 * FACET_LENGTH, s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, e, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && fd.face == 2 && fd.depth == 0)
				|| (use && fd.face == 0 && fd.depth == size - 1)
				|| (use && fd.face == 1 && fd.depth == size - 1 - m1)
				|| (use && fd.face == 3 && fd.depth == m2)
				|| (!use && fd.face == 2 && fd.depth != 0)
				|| (!use && fd.face == 0 && fd.depth != size - 1)
				|| (!use && fd.face == 1 && fd.depth != size - 1 - m1)
				|| (!use && fd.face == 3 && fd.depth != m2)) {
			glPushMatrix();
			glTranslate(-s2 * FACET_LENGTH, -s1 * FACET_LENGTH, s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, f, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && fd.face == 3 && fd.depth == 0)
				|| (use && fd.face == 1 && fd.depth == size - 1)
				|| (use && fd.face == 0 && fd.depth == m1)
				|| (use && fd.face == 2 && fd.depth == size - 1 - m2)
				|| (!use && fd.face == 3 && fd.depth != 0)
				|| (!use && fd.face == 1 && fd.depth != size - 1)
				|| (!use && fd.face == 0 && fd.depth != m1)
				|| (!use && fd.face == 2 && fd.depth != size - 1 - m2)) {
			glPushMatrix();
			glTranslate(-s3 * FACET_LENGTH, s1 * FACET_LENGTH, -s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, e, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && fd.face == 3 && fd.depth == 0)
				|| (use && fd.face == 1 && fd.depth == size - 1)
				|| (use && fd.face == 2 && fd.depth == size - 1 - m1)
				|| (use && fd.face == 0 && fd.depth == m2)
				|| (!use && fd.face == 3 && fd.depth != 0)
				|| (!use && fd.face == 1 && fd.depth != size - 1)
				|| (!use && fd.face == 2 && fd.depth != size - 1 - m1)
				|| (!use && fd.face == 0 && fd.depth != m2)) {
			glPushMatrix();
			glTranslate(-s3 * FACET_LENGTH, -s1 * FACET_LENGTH, -s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, -1, f);
			glPopMatrix();
		}
		if (all || (use && fd.face == 2 && fd.depth == 0)
				|| (use && fd.face == 0 && fd.depth == size - 1)
				|| (use && fd.face == 3 && fd.depth == size - 1 - m1)
				|| (use && fd.face == 1 && fd.depth == m2)
				|| (!use && fd.face == 2 && fd.depth != 0)
				|| (!use && fd.face == 0 && fd.depth != size - 1)
				|| (!use && fd.face == 3 && fd.depth != size - 1 - m1)
				|| (!use && fd.face == 1 && fd.depth != m2)) {
			glPushMatrix();
			glTranslate(-s3 * FACET_LENGTH, -s1 * FACET_LENGTH, s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, e, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && fd.face == 2 && fd.depth == 0)
				|| (use && fd.face == 0 && fd.depth == size - 1)
				|| (use && fd.face == 1 && fd.depth == m1)
				|| (use && fd.face == 3 && fd.depth == size - 1 - m2)
				|| (!use && fd.face == 2 && fd.depth != 0)
				|| (!use && fd.face == 0 && fd.depth != size - 1)
				|| (!use && fd.face == 1 && fd.depth != m1)
				|| (!use && fd.face == 3 && fd.depth != size - 1 - m2)) {
			glPushMatrix();
			glTranslate(-s3 * FACET_LENGTH, s1 * FACET_LENGTH, s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, f, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && fd.face == 1 && fd.depth == 0)
				|| (use && fd.face == 3 && fd.depth == size - 1)
				|| (use && fd.face == 2 && fd.depth == size - 1 - m1)
				|| (use && fd.face == 0 && fd.depth == m2)
				|| (!use && fd.face == 1 && fd.depth != 0)
				|| (!use && fd.face == 3 && fd.depth != size - 1)
				|| (!use && fd.face == 2 && fd.depth != size - 1 - m1)
				|| (!use && fd.face == 0 && fd.depth != m2)) {
			glPushMatrix();
			glTranslate(s2 * FACET_LENGTH, -s1 * FACET_LENGTH, s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, e, -1, -1);
			glPopMatrix();
		}
		if (all || (use && fd.face == 1 && fd.depth == 0)
				|| (use && fd.face == 3 && fd.depth == size - 1)
				|| (use && fd.face == 0 && fd.depth == m1)
				|| (use && fd.face == 2 && fd.depth == size - 1 - m2)
				|| (!use && fd.face == 1 && fd.depth != 0)
				|| (!use && fd.face == 3 && fd.depth != size - 1)
				|| (!use && fd.face == 0 && fd.depth != m1)
				|| (!use && fd.face == 2 && fd.depth != size - 1 - m2)) {
			glPushMatrix();
			glTranslate(s2 * FACET_LENGTH, s1 * FACET_LENGTH, s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, f, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && fd.face == 0 && fd.depth == 0)
				|| (use && fd.face == 2 && fd.depth == size - 1)
				|| (use && fd.face == 1 && fd.depth == size - 1 - m1)
				|| (use && fd.face == 3 && fd.depth == m2)
				|| (!use && fd.face == 0 && fd.depth != 0)
				|| (!use && fd.face == 2 && fd.depth != size - 1)
				|| (!use && fd.face == 1 && fd.depth != size - 1 - m1)
				|| (!use && fd.face == 3 && fd.depth != m2)) {
			glPushMatrix();
			glTranslate(s3 * FACET_LENGTH, -s1 * FACET_LENGTH, -s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, e, -1);
			glPopMatrix();
		}
		if (all || (use && fd.face == 0 && fd.depth == 0)
				|| (use && fd.face == 2 && fd.depth == size - 1)
				|| (use && fd.face == 3 && fd.depth == m1)
				|| (use && fd.face == 1 && fd.depth == size - 1 - m2)
				|| (!use && fd.face == 0 && fd.depth != 0)
				|| (!use && fd.face == 2 && fd.depth != size - 1)
				|| (!use && fd.face == 3 && fd.depth != m1)
				|| (!use && fd.face == 1 && fd.depth != size - 1 - m2)) {
			glPushMatrix();
			glTranslate(s3 * FACET_LENGTH, s1 * FACET_LENGTH, -s2 * FACET_LENGTH);
			drawTetraFacet(mi, f, -1, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && fd.face == 3 && fd.depth == 0)
				|| (use && fd.face == 1 && fd.depth == size - 1)
				|| (use && fd.face == 0 && fd.depth == size - 1 - m1)
				|| (use && fd.face == 2 && fd.depth == m2)
				|| (!use && fd.face == 3 && fd.depth != 0)
				|| (!use && fd.face == 1 && fd.depth != size - 1)
				|| (!use && fd.face == 0 && fd.depth != size - 1 - m1)
				|| (!use && fd.face == 2 && fd.depth != m2)) {
			glPushMatrix();
			glTranslate(-s2 * FACET_LENGTH, -s1 * FACET_LENGTH, -s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, -1, e);
			glPopMatrix();
		}
		if (all || (use && fd.face == 3 && fd.depth == 0)
				|| (use && fd.face == 1 && fd.depth == size - 1)
				|| (use && fd.face == 2 && fd.depth == m1)
				|| (use && fd.face == 0 && fd.depth == size - 1 - m2)
				|| (!use && fd.face == 3 && fd.depth != 0)
				|| (!use && fd.face == 1 && fd.depth != size - 1)
				|| (!use && fd.face == 2 && fd.depth != m1)
				|| (!use && fd.face == 0 && fd.depth != size - 1 - m2)) {
			glPushMatrix();
			glTranslate(-s2 * FACET_LENGTH, s1 * FACET_LENGTH, -s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, f, -1, -1, -1, -1);
			glPopMatrix();
		}
	}
	if (!sticky)
	    for (edge = 1; edge < size / 2; edge++) {
		/*if (size > 5) OUTSIDE EDGE 1 */
		int a, b, c, d, e, f;
		s0 = 2 * size - 2 * edge - 2; 
		s1 = 2 * edge; /* 2, 4 */
		a = edge * edge; /* 1, 4 */
		b = (edge + 1) * (edge + 1) - 1; /* 3, 8 */
		c = (size - edge - 1) * (size - edge - 1); /* 4, 9, 16 */
		d = (size - edge) * (size - edge) - 1; /* 8, 15 */
		e = (size - 1) * (size - 1) + 2 * edge; /* 11, */
		f = size * size - 2 * edge - 1; /* 13, */
		if (all || ((use && (fd.face == 0 || fd.face == 1) && fd.depth == 0)
				|| (use && (fd.face == 2 || fd.face == 3) && fd.depth == edge)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != 0)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != edge))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, s0 * FACET_LENGTH, 0);
			drawOctaFacet(mi, a, b, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 1 || fd.face == 2) && fd.depth == 0)
				|| (use && (fd.face == 0 || fd.face == 3) && fd.depth == edge)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != 0)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != edge))) {
			glPushMatrix();
			glTranslate(0, s0 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, a, b, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 2 || fd.face == 3) && fd.depth == 0)
				|| (use && (fd.face == 0 || fd.face == 1) && fd.depth == edge)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != 0)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != edge))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, s0 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, a, b, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 0 || fd.face == 3) && fd.depth == 0)
				|| (use && (fd.face == 1 || fd.face == 2) && fd.depth == edge)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != 0)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != edge))) {
			glPushMatrix();
			glTranslate(0, s0 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawOctaFacet(mi, b, -1, -1, a, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 0 || fd.face == 3) && fd.depth == size - 1)
				|| (use && (fd.face == 1 || fd.face == 2) && fd.depth == size - 1 - edge)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != size - 1 - edge))) {
			glPushMatrix();
			glTranslate(0, -s0 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, -1, a, b, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 2 || fd.face == 3) && fd.depth == size - 1)
				|| (use && (fd.face == 0 || fd.face == 1) && fd.depth == size - 1 - edge)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != size - 1 - edge))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, -s0 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, -1, -1, -1, a, b, -1);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 1 || fd.face == 2) && fd.depth == size - 1)
				|| (use && (fd.face == 0 || fd.face == 3) && fd.depth == size - 1 - edge)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != size - 1)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != size - 1 - edge))) {
			glPushMatrix();
			glTranslate(0, -s0 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, -1, -1, -1, a, b);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 0 || fd.face == 1) && fd.depth == size - 1)
				|| (use && (fd.face == 2 || fd.face == 3) && fd.depth == size - 1 - edge)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != size - 1)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != size - 1 - edge))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, -s0 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, -1, -1, b, -1, -1, a);
			glPopMatrix();
		}

		if (all || ((use && (fd.face == 0 || fd.face == 1) && fd.depth == 0)
				|| (use && (fd.face == 2 || fd.face == 3) && fd.depth == size - 1 - edge)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != 0)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != size - 1 - edge))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, s1 * FACET_LENGTH, 0);
			drawOctaFacet(mi, c, d, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 1 || fd.face == 2) && fd.depth == 0)
				|| (use && (fd.face == 0 || fd.face == 3) && fd.depth == size - 1 - edge)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != 0)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != size - 1 - edge))) {
			glPushMatrix();
			glTranslate(0, s1 * FACET_LENGTH, s0 * FACET_LENGTH);
			drawOctaFacet(mi, -1, c, d, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 2 || fd.face == 3) && fd.depth == 0)
				|| (use && (fd.face == 0 || fd.face == 1) && fd.depth == size - 1 - edge)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != 0)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != size - 1 - edge))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, s1 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, c, d, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 0 || fd.face == 3) && fd.depth == 0)
				|| (use && (fd.face == 1 || fd.face == 2) && fd.depth == size - 1 - edge)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != 0)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != size - 1 - edge))) {
			glPushMatrix();
			glTranslate(0, s1 * FACET_LENGTH, -s0 * FACET_LENGTH);
			drawOctaFacet(mi, d, -1, -1, c, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 0 || fd.face == 3) && fd.depth == size - 1)
				|| (use && (fd.face == 1 || fd.face == 2) && fd.depth == edge)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != edge))) {
			glPushMatrix();
			glTranslate(0, -s1 * FACET_LENGTH, s0 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, -1, c, d, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 2 || fd.face == 3) && fd.depth == size - 1)
				|| (use && (fd.face == 0 || fd.face == 1) && fd.depth == edge)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != size - 1)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != edge))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, -s1 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, -1, -1, -1, c, d, -1);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 1 || fd.face == 2) && fd.depth == size - 1)
				|| (use && (fd.face == 0 || fd.face == 3) && fd.depth == edge)
				|| (!use && (fd.face == 1 || fd.face == 2) && fd.depth != size - 1)
				|| (!use && (fd.face == 0 || fd.face == 3) && fd.depth != edge))) {
			glPushMatrix();
			glTranslate(0, -s1 * FACET_LENGTH, -s0 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, -1, -1, -1, c, d);
			glPopMatrix();
		}
		if (all || ((use && (fd.face == 0 || fd.face == 1) && fd.depth == size - 1)
				|| (use && (fd.face == 2 || fd.face == 3) && fd.depth == edge)
				|| (!use && (fd.face == 0 || fd.face == 1) && fd.depth != size - 1)
				|| (!use && (fd.face == 2 || fd.face == 3) && fd.depth != edge))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, -s1 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, -1, -1, d, -1, -1, c);
			glPopMatrix();
		}

		if (all || ((use && fd.face == 0 && fd.depth == 0)
				|| (use && fd.face == 2 && fd.depth == size - 1)
				|| (use && fd.face == 1 && fd.depth == edge)
				|| (use && fd.face == 3 && fd.depth == size - 1 - edge)
				|| (!use && fd.face == 0 && fd.depth != 0)
				|| (!use && fd.face == 2 && fd.depth != size - 1)
				|| (!use && fd.face == 1 && fd.depth != edge)
				|| (!use && fd.face == 3 && fd.depth != size - 1 - edge))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, 0, -s1 * FACET_LENGTH);
			drawOctaFacet(mi, e, -1, -1, -1, -1, -1, f, -1);
			glPopMatrix();
		}
		if (all || ((use && fd.face == 1 && fd.depth == 0)
				|| (use && fd.face == 3 && fd.depth == size - 1)
				|| (use && fd.face == 2 && fd.depth == edge)
				|| (use && fd.face == 0 && fd.depth == size - 1 - edge)
				|| (!use && fd.face == 1 && fd.depth != 0)
				|| (!use && fd.face == 3 && fd.depth != size - 1)
				|| (!use && fd.face == 2 && fd.depth != edge)
				|| (!use && fd.face == 0 && fd.depth != size - 1 - edge))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, 0, s0 * FACET_LENGTH);
			drawOctaFacet(mi, -1, e, -1, -1, -1, f, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && fd.face == 2 && fd.depth == 0)
				|| (use && fd.face == 0 && fd.depth == size - 1)
				|| (use && fd.face == 3 && fd.depth == edge)
				|| (use && fd.face == 1 && fd.depth == size - 1 - edge)
				|| (!use && fd.face == 2 && fd.depth != 0)
				|| (!use && fd.face == 0 && fd.depth != size - 1)
				|| (!use && fd.face == 3 && fd.depth != edge)
				|| (!use && fd.face == 1 && fd.depth != size - 1 - edge))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, 0, s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, e, -1, f, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && fd.face == 3 && fd.depth == 0)
				|| (use && fd.face == 1 && fd.depth == size - 1)
				|| (use && fd.face == 0 && fd.depth == edge)
				|| (use && fd.face == 2 && fd.depth == size - 1 - edge)
				|| (!use && fd.face == 3 && fd.depth != 0)
				|| (!use && fd.face == 1 && fd.depth != size - 1)
				|| (!use && fd.face == 0 && fd.depth != edge)
				|| (!use && fd.face == 2 && fd.depth != size - 1 - edge))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, 0, -s0 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, e, -1, -1, -1, f);
			glPopMatrix();
		}
		if (all || ((use && fd.face == 2 && fd.depth == 0)
				|| (use && fd.face == 0 && fd.depth == size - 1)
				|| (use && fd.face == 1 && fd.depth == edge)
				|| (use && fd.face == 3 && fd.depth == size - 1 - edge)
				|| (!use && fd.face == 2 && fd.depth != 0)
				|| (!use && fd.face == 0 && fd.depth != size - 1)
				|| (!use && fd.face == 1 && fd.depth != edge)
				|| (!use && fd.face == 3 && fd.depth != size - 1 - edge))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, 0, s0 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, f, -1, e, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && fd.face == 1 && fd.depth == 0)
				|| (use && fd.face == 3 && fd.depth == size - 1)
				|| (use && fd.face == 0 && fd.depth == edge)
				|| (use && fd.face == 2 && fd.depth == size - 1 - edge)
				|| (!use && fd.face == 1 && fd.depth != 0)
				|| (!use && fd.face == 3 && fd.depth != size - 1)
				|| (!use && fd.face == 0 && fd.depth != edge)
				|| (!use && fd.face == 2 && fd.depth != size - 1 - edge))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, 0, s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, f, -1, -1, -1, e, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && fd.face == 0 && fd.depth == 0)
				|| (use && fd.face == 2 && fd.depth == size - 1)
				|| (use && fd.face == 3 && fd.depth == edge)
				|| (use && fd.face == 1 && fd.depth == size - 1 - edge)
				|| (!use && fd.face == 0 && fd.depth != 0)
				|| (!use && fd.face == 2 && fd.depth != size - 1)
				|| (!use && fd.face == 3 && fd.depth != edge)
				|| (!use && fd.face == 1 && fd.depth != size - 1 - edge))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, 0, -s0 * FACET_LENGTH);
			drawOctaFacet(mi, f, -1, -1, -1, -1, -1, e, -1);
			glPopMatrix();
		}
		if (all || ((use && fd.face == 3 && fd.depth == 0)
				|| (use && fd.face == 1 && fd.depth == size - 1)
				|| (use && fd.face == 2 && fd.depth == edge)
				|| (use && fd.face == 0 && fd.depth == size - 1 - edge)
				|| (!use && fd.face == 3 && fd.depth != 0)
				|| (!use && fd.face == 1 && fd.depth != size - 1)
				|| (!use && fd.face == 2 && fd.depth != edge)
				|| (!use && fd.face == 0 && fd.depth != size - 1 - edge))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, 0, -s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, f, -1, -1, -1, e);
			glPopMatrix();
		}
	}
	if (!sticky) {
	  pos = 1;
	  for (row = 0; row < size - 3; row++) {
	    pos += 4;
	    for (col = 0; col < 2 * row + 1; col++) {
		pos++;
		tetra = ((col % 2) == 1);
		sb = 2 * size - 2 * row - ((tetra) ? 0 : 1) - 5;
		sl = col + 2;
		sr = 2 * row - col + 2;
		mb = row + 2;
		ml = size - 2 - col / 2;
		mr = size - row - 2 + (col + 1) / 2;
		if (all || (use && fd.face == 0 && fd.depth == 0)
				|| (use && fd.face == 1 && fd.depth == size - 1 - ml)
				|| (use && fd.face == 2 && fd.depth == mb)
				|| (use && fd.face == 3 && fd.depth == size - 1 - mr)
				|| (!use && fd.face == 0 && fd.depth != 0)
				|| (!use && fd.face == 1 && fd.depth != size - 1 - ml)
				|| (!use && fd.face == 2 && fd.depth != mb)
				|| (!use && fd.face == 3 && fd.depth != size - 1 - mr)) {
			glPushMatrix(); /* 0 */
			glTranslate(sr * FACET_LENGTH, sb * FACET_LENGTH, -sl * FACET_LENGTH);
			drawFacet(mi, pos, -1, -1, -1, -1, -1, -1, -1, tetra);
			glPopMatrix();
		}
		if (all || (use && fd.face == 1 && fd.depth == 0)
				|| (use && fd.face == 0 && fd.depth == size - 1 - mr)
				|| (use && fd.face == 3 && fd.depth == mb)
				|| (use && fd.face == 2 && fd.depth == size - 1 - ml)
				|| (!use && fd.face == 1 && fd.depth != 0)
				|| (!use && fd.face == 0 && fd.depth != size - 1 - mr)
				|| (!use && fd.face == 3 && fd.depth != mb)
				|| (!use && fd.face == 2 && fd.depth != size - 1 - ml)) {
			glPushMatrix(); /* 1 */
			glTranslate(sl * FACET_LENGTH, sb * FACET_LENGTH, sr * FACET_LENGTH);
			drawFacet(mi, -1, pos, -1, -1, -1, -1, -1, -1, tetra);
			glPopMatrix();
		}
		if (all || (use && fd.face == 2 && fd.depth == 0)
				|| (use && fd.face == 1 && fd.depth == size - 1 - mr)
				|| (use && fd.face == 0 && fd.depth == mb)
				|| (use && fd.face == 3 && fd.depth == size - 1 - ml)
				|| (!use && fd.face == 2 && fd.depth != 0)
				|| (!use && fd.face == 1 && fd.depth != size - 1 - mr)
				|| (!use && fd.face == 0 && fd.depth != mb)
				|| (!use && fd.face == 3 && fd.depth != size - 1 - ml)) {
			glPushMatrix(); /* 2 */
			glTranslate(-sr * FACET_LENGTH, sb * FACET_LENGTH, sl * FACET_LENGTH);
			drawFacet(mi, -1, -1, pos, -1, -1, -1, -1, -1, tetra);
			glPopMatrix();
		}
		if (all || (use && fd.face == 3 && fd.depth == 0)
				|| (use && fd.face == 0 && fd.depth == size - 1 - ml)
				|| (use && fd.face == 1 && fd.depth == mb)
				|| (use && fd.face == 2 && fd.depth == size - 1 - mr)
				|| (!use && fd.face == 3 && fd.depth != 0)
				|| (!use && fd.face == 0 && fd.depth != size - 1 - ml)
				|| (!use && fd.face == 1 && fd.depth != mb)
				|| (!use && fd.face == 2 && fd.depth != size - 1 - mr)) {
			glPushMatrix(); /* 3 */
			glTranslate(-sl * FACET_LENGTH, sb * FACET_LENGTH, -sr * FACET_LENGTH);
			drawFacet(mi, -1, -1, -1, pos, -1, -1, -1, -1, tetra);
			glPopMatrix();
		}
		if (all || (use && fd.face == 0 && fd.depth == size - 1)
				|| (use && fd.face == 1 && fd.depth == mr)
				|| (use && fd.face == 2 && fd.depth == size - 1 - mb)
				|| (use && fd.face == 3 && fd.depth == ml)
				|| (!use && fd.face == 0 && fd.depth != size - 1)
				|| (!use && fd.face == 1 && fd.depth != mr)
				|| (!use && fd.face == 2 && fd.depth != size - 1 - mb)
				|| (!use && fd.face == 3 && fd.depth != ml)) {
			glPushMatrix(); /* 4 */
			glTranslate(-sl * FACET_LENGTH, -sb * FACET_LENGTH, sr * FACET_LENGTH);
			drawFacet(mi, -1, -1, -1, -1, pos, -1, -1, -1, tetra);
			glPopMatrix();
		}
		if (all || (use && fd.face == 3 && fd.depth == size - 1)
				|| (use && fd.face == 0 && fd.depth == mr)
				|| (use && fd.face == 1 && fd.depth == size - 1 - mb)
				|| (use && fd.face == 2 && fd.depth == ml)
				|| (!use && fd.face == 3 && fd.depth != size - 1)
				|| (!use && fd.face == 0 && fd.depth != mr)
				|| (!use && fd.face == 1 && fd.depth != size - 1 - mb)
				|| (!use && fd.face == 2 && fd.depth != ml)) {
			glPushMatrix(); /* 5 */
			glTranslate(sr * FACET_LENGTH, -sb * FACET_LENGTH, sl * FACET_LENGTH);
			drawFacet(mi, -1, -1, -1, -1, -1, pos, -1, -1, tetra);
			glPopMatrix();
		}
		if (all || (use && fd.face == 2 && fd.depth == size - 1)
				|| (use && fd.face == 1 && fd.depth == ml)
				|| (use && fd.face == 0 && fd.depth == size - 1 - mb)
				|| (use && fd.face == 3 && fd.depth == mr)
				|| (!use && fd.face == 2 && fd.depth != size - 1)
				|| (!use && fd.face == 1 && fd.depth != ml)
				|| (!use && fd.face == 0 && fd.depth != size - 1 - mb)
				|| (!use && fd.face == 3 && fd.depth != mr)) {
			glPushMatrix(); /* 6 */
			glTranslate(sl * FACET_LENGTH, -sb * FACET_LENGTH, -sr * FACET_LENGTH);
			drawFacet(mi, -1, -1, -1, -1, -1, -1, pos, -1, tetra);
			glPopMatrix(); 
		}
		if (all || (use && fd.face == 1 && fd.depth == size - 1)
				|| (use && fd.face == 0 && fd.depth == ml)
				|| (use && fd.face == 3 && fd.depth == size - 1 - mb)
				|| (use && fd.face == 2 && fd.depth == mr)
				|| (!use && fd.face == 1 && fd.depth != size - 1)
				|| (!use && fd.face == 0 && fd.depth != ml)
				|| (!use && fd.face == 3 && fd.depth != size - 1 - mb)
				|| (!use && fd.face == 2 && fd.depth != mr)) {
			glPushMatrix(); /* 7 */
			glTranslate(-sr * FACET_LENGTH, -sb * FACET_LENGTH, -sl * FACET_LENGTH);
			drawFacet(mi, -1, -1, -1, -1, -1, -1, -1, pos, tetra);
			glPopMatrix();
		}
	    }
	  }
	}
}

typedef struct _CornerDepth {
	int corner, depth;
} CornerDepth;

static void
cornerDepth(octstruct * op, int face, int position, int direction, CornerDepth *cd)
{
	RTT rtt;

	toRTT(position, &rtt);
	switch (face) {
	case 0:
		if (direction == BR || direction == TL) {
			cd->corner = 0;
			cd->depth = op->size - 1 - rtt.tlbr;
		} else if (direction == TR || direction == BL) {
			cd->corner = 1;
			cd->depth = op->size - 1 + rtt.trbl;
		} else {
			cd->corner = 2;
			cd->depth = rtt.row;
		}
		break;
	case 1:
		if (direction == BR || direction == TL) {
			cd->corner = 0;
			cd->depth = op->size - 1 - rtt.trbl;
		} else if (direction == TR || direction == BL) {
			cd->corner = 1;
			cd->depth = op->size - 1 - rtt.tlbr;
		} else {
			cd->corner = 2;
			cd->depth = rtt.row;
		}
		break;
	case 2:
		if (direction == BR || direction == TL) {
			cd->corner = 0;
			cd->depth = op->size - 1 + rtt.tlbr;
		} else if (direction == TR || direction == BL) {
			cd->corner = 1;
			cd->depth = op->size - 1 - rtt.trbl;
		} else {
			cd->corner = 2;
			cd->depth = rtt.row;
		}
		break;
	case 3:
		if (direction == BR || direction == TL) {
			cd->corner = 0;
			cd->depth = op->size - 1 + rtt.trbl;
		} else if (direction == TR || direction == BL) {
			cd->corner = 1;
			cd->depth = op->size - 1 + rtt.tlbr;
		} else {
			cd->corner = 2;
			cd->depth = rtt.row;
		}
		break;
	case 4:
		if (direction == TR || direction == BL) {
			cd->corner = 0;
			cd->depth = op->size - 1 + rtt.trbl;
		} else if (direction == BR || direction == TL) {
			cd->corner = 1;
			cd->depth = op->size - 1 - rtt.tlbr;
		} else {
			cd->corner = 2;
			cd->depth = 2 * (op->size - 1) - rtt.row;
		}
		break;
	case 5:
		if (direction == TR || direction == BL) {
			cd->corner = 0;
			cd->depth = op->size - 1 - rtt.tlbr;
		} else if (direction == BR || direction == TL) {
			cd->corner = 1;
			cd->depth = op->size - 1 - rtt.trbl;
		} else {
			cd->corner = 2;
			cd->depth = 2 * (op->size - 1) - rtt.row;
		}
		break;
	case 6:
		if (direction == TR || direction == BL) {
			cd->corner = 0;
			cd->depth = op->size - 1 - rtt.trbl;
		} else if (direction == BR || direction == TL) {
			cd->corner = 1;
			cd->depth = op->size - 1 + rtt.tlbr;
		} else {
			cd->corner = 2;
			cd->depth = 2 * (op->size - 1) - rtt.row;
		}
		break;
	case 7:
		if (direction == TR || direction == BL) {
			cd->corner = 0;
			cd->depth = op->size - 1 + rtt.tlbr;
		} else if (direction == BR || direction == TL) {
			cd->corner = 1;
			cd->depth = op->size - 1 + rtt.trbl;
		} else {
			cd->corner = 2;
			cd->depth = 2 * (op->size - 1) - rtt.row;
		}
		break;
	default:
		cd->corner = -1;
		cd->depth = -1;
	}
}

/* A drawing error was noticed as the octahedrons turn through the face
   of the puzzle (it looks like a small shadow).  Octahedrons are cut
   up to pyramids and half pyramids to fix this. */
static void
drawCorners(ModeInfo * mi, int face, int position, int direction,
		Bool use, Bool all)
{
	octstruct *op = &oct[MI_SCREEN(mi)];
	int s0, s1, s2, i, j, k;
	int pos, col, row, sb, sl, sr, mb, ml, mr;
	int size = op->size, edge;
	Bool tetra, sticky = op->sticky;
	RTT rtt;
	CornerDepth cd;

	toRTT(position, &rtt);
	cornerDepth(op, face, position, direction, &cd);

	if (size == 1 && (all || use))
		drawOctaFacet(mi, 0, 0, 0, 0, 0, 0, 0, 0);
	if (size > 1) { /* CORNERS */
		s0 = 2 * size - 2;
		j = (size - 1) * (size - 1);
		k = size * size - 1;
		if (all || (!sticky && ((use && cd.corner == 0 && cd.depth == 0)
				|| (use && cd.corner != 0 && cd.depth == size - 1)
				|| (!use && cd.corner == 0 && cd.depth != 0)
				|| (!use && cd.corner != 0 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 0 && cd.depth < size - 1)
				|| (use && cd.corner != 0 && cd.depth == size - 1)
				|| (!use && cd.corner == 0 && cd.depth >= size - 1)
				|| (!use && cd.corner != 0 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, 0, 0);
			drawOctaFacet(mi, j, k, -1, -1, -1, j, k, -1); /* TR */
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 1 && cd.depth == 0)
				|| (use && cd.corner != 1 && cd.depth == size - 1)
				|| (!use && cd.corner == 1 && cd.depth != 0)
				|| (!use && cd.corner != 1 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 1 && cd.depth < size - 1)
				|| (use && cd.corner != 1 && cd.depth == size - 1)
				|| (!use && cd.corner == 1 && cd.depth >= size - 1)
				|| (!use && cd.corner != 1 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(0, 0, s0 * FACET_LENGTH);
			drawOctaFacet(mi, -1, j, k, -1, j, k, -1, -1); /* BR */
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 0 && cd.depth == 2 * size - 2)
				|| (use && cd.corner != 0 && cd.depth == size - 1)
				|| (!use && cd.corner == 0 && cd.depth != 2 * size - 2)
				|| (!use && cd.corner != 0 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 0 && cd.depth > size - 1)
				|| (use && cd.corner != 0 && cd.depth == size - 1)
				|| (!use && cd.corner == 0 && cd.depth <= size - 1)
				|| (!use && cd.corner != 0 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, 0, 0);
			drawOctaFacet(mi, -1, -1, j, k, k, -1, -1, j); /* BL */
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 1 && cd.depth == 2 * size - 2)
				|| (use && cd.corner != 1 && cd.depth == size - 1)
				|| (!use && cd.corner == 1 && cd.depth != 2 * size - 2)
				|| (!use && cd.corner != 1 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 1 && cd.depth > size - 1)
				|| (use && cd.corner != 1 && cd.depth == size - 1)
				|| (!use && cd.corner == 1 && cd.depth <= size - 1)
				|| (!use && cd.corner != 1 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(0, 0, -s0 * FACET_LENGTH);
			drawOctaFacet(mi, k, -1, -1, j, -1, -1, j, k); /* TL */
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 2 && cd.depth == 0)
				|| (use && cd.corner != 2 && cd.depth == size - 1)
				|| (!use && cd.corner == 2 && cd.depth != 0)
				|| (!use && cd.corner != 2 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 2 && cd.depth < size - 1)
				|| (use && cd.corner != 2 && cd.depth == size - 1)
				|| (!use && cd.corner == 2 && cd.depth >= size - 1)
				|| (!use && cd.corner != 2 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(0, s0 * FACET_LENGTH, 0);
			drawOctaFacet(mi, 0, 0, 0, 0, -1, -1, -1, -1); /* TOP */
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 2 && cd.depth == 2 * size - 2)
				|| (use && cd.corner != 2 && cd.depth == size - 1)
				|| (!use && cd.corner == 2 && cd.depth != 2 * size - 2)
				|| (!use && cd.corner != 2 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 2 && cd.depth > size - 1)
				|| (use && cd.corner != 2 && cd.depth == size - 1)
				|| (!use && cd.corner == 2 && cd.depth <= size - 1)
				|| (!use && cd.corner != 2 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(0, -s0 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, -1, -1, 0, 0, 0, 0); /* BOTTOM */
			glPopMatrix();
		}
	}
	if (size > 1 && size % 3 != 0 && (size < 4 || sticky)) { /* CENTERS */
		tetra = (size % 3 == 2); /* 2, 6, 12, 20, 30 */
		if (tetra) {
			s0 = ((size - 2) / 3) * 2 + 1;
		} else {
			s0 = ((size - 1) / 3) * 2;
		}
		s1 = size - 1 - s0;
		k = s0 * s0 + s0;
		if (all || (!sticky && ((use && cd.corner != 1 && cd.depth == size - 1 - s1)
				|| (use && cd.corner == 1 && cd.depth == size - 1 + s1)
				|| (!use && cd.corner != 1 && cd.depth != size - 1 - s1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1 + s1)))
			|| (sticky && ((use && cd.corner != 1 && cd.depth < size - 1)
				|| (use && cd.corner == 1 && cd.depth > size - 1)
				|| (!use && cd.corner != 1 && cd.depth >= size - 1)
				|| (!use && cd.corner == 1 && cd.depth <= size - 1)))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, s0 * FACET_LENGTH, -s0 * FACET_LENGTH);
			drawFacet(mi, k, -1, -1, -1, -1, -1, -1, -1, tetra); /* 0 */
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.depth == size - 1 - s1)
				|| (!use && cd.depth != size - 1 - s1)))
			|| (sticky && ((use && cd.depth < size - 1)
				|| (!use && cd.depth >= size - 1)))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, s0 * FACET_LENGTH, s0 * FACET_LENGTH);
			drawFacet(mi, -1, k, -1, -1, -1, -1, -1, -1, tetra); /* 1 */
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner != 0 && cd.depth == size - 1 - s1)
				|| (use && cd.corner == 0 && cd.depth == size - 1 + s1)
				|| (!use && cd.corner != 0 && cd.depth != size - 1 - s1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1 + s1)))
			|| (sticky && ((use && cd.corner != 0 && cd.depth < size - 1)
				|| (use && cd.corner == 0 && cd.depth > size - 1)
				|| (!use && cd.corner != 0 && cd.depth >= size - 1)
				|| (!use && cd.corner == 0 && cd.depth <= size - 1)))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, s0 * FACET_LENGTH, s0 * FACET_LENGTH);
			drawFacet(mi, -1, -1, k, -1, -1, -1, -1, -1, tetra); /* 2 */
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner != 2 && cd.depth == size - 1 + s1)
				|| (use && cd.corner == 2 && cd.depth == size - 1 - s1)
				|| (!use && cd.corner != 2 && cd.depth != size - 1 + s1)
				|| (!use && cd.corner == 2 && cd.depth != size - 1 - s1)))
			|| (sticky && ((use && cd.corner != 2 && cd.depth > size - 1)
				|| (use && cd.corner == 2 && cd.depth < size - 1)
				|| (!use && cd.corner != 2 && cd.depth <= size - 1)
				|| (!use && cd.corner == 2 && cd.depth >= size - 1)))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, s0 * FACET_LENGTH, -s0 * FACET_LENGTH);
			drawFacet(mi, -1, -1, -1, k, -1, -1, -1, -1, tetra); /* 3 */
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner != 1 && cd.depth == size - 1 + s1)
				|| (use && cd.corner == 1 && cd.depth == size - 1 - s1)
				|| (!use && cd.corner != 1 && cd.depth != size - 1 + s1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1 - s1)))
			|| (sticky && ((use && cd.corner != 1 && cd.depth > size - 1)
				|| (use && cd.corner == 1 && cd.depth < size - 1)
				|| (!use && cd.corner != 1 && cd.depth <= size - 1)
				|| (!use && cd.corner == 1 && cd.depth >= size - 1)))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, -s0 * FACET_LENGTH, s0 * FACET_LENGTH);
			drawFacet(mi, -1, -1, -1, -1, k, -1, -1, -1, tetra); /* 4 */
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner != 2 && cd.depth == size - 1 - s1)
				|| (use && cd.corner == 2 && cd.depth == size - 1 + s1)
				|| (!use && cd.corner != 2 && cd.depth != size - 1 - s1)
				|| (!use && cd.corner == 2 && cd.depth != size - 1 + s1)))
			|| (sticky && ((use && cd.corner != 2 && cd.depth < size - 1)
				|| (use && cd.corner == 2 && cd.depth > size - 1)
				|| (!use && cd.corner != 2 && cd.depth >= size - 1)
				|| (!use && cd.corner == 2 && cd.depth <= size - 1)))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, -s0 * FACET_LENGTH, s0 * FACET_LENGTH);
			drawFacet(mi, -1, -1, -1, -1, -1, k, -1, -1, tetra); /* 5 */
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner != 0 && cd.depth == size - 1 + s1)
				|| (use && cd.corner == 0 && cd.depth == size - 1 - s1)
				|| (!use && cd.corner != 0 && cd.depth != size - 1 + s1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1 - s1)))
			|| (sticky && ((use && cd.corner != 0 && cd.depth > size - 1)
				|| (use && cd.corner == 0 && cd.depth < size - 1)
				|| (!use && cd.corner != 0 && cd.depth <= size - 1)
				|| (!use && cd.corner == 0 && cd.depth >= size - 1)))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, -s0 * FACET_LENGTH, -s0 * FACET_LENGTH);
			drawFacet(mi, -1, -1, -1, -1, -1, -1, k, -1, tetra); /* 6 */
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.depth == size - 1 + s1)
				|| (!use && cd.depth != size - 1 + s1)))
			|| (sticky && ((use && cd.depth > size - 1)
				|| (!use && cd.depth <= size - 1)))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, -s0 * FACET_LENGTH, -s0 * FACET_LENGTH);
			drawFacet(mi, -1, -1, -1, -1, -1, -1, -1, k, tetra); /* 7 */
			glPopMatrix();
		}
	}
	if (size > 2) { /* CORNERS - 1 */
		s0 = 2 * size - 3;
		j = (size - 1) * (size - 1) + 1; /* 5, */
		k = size * size - 2; /* 7, */
		/* 0 */
		if (all || (!sticky && ((use && cd.corner == 2 && cd.depth == 1)
				|| (use && cd.corner != 2 && cd.depth == size - 1)
				|| (!use && cd.corner == 2 && cd.depth != 1)
				|| (!use && cd.corner != 2 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 2 && cd.depth < size - 1)
				|| (use && cd.corner != 2 && cd.depth == size - 1)
				|| (!use && cd.corner == 2 && cd.depth >= size - 1)
				|| (!use && cd.corner != 2 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(FACET_LENGTH, s0 * FACET_LENGTH, -FACET_LENGTH);
			drawTetraFacet(mi, 2, -1, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 0 && cd.depth == 1)
				|| (use && cd.corner != 0 && cd.depth == size - 1)
				|| (!use && cd.corner == 0 && cd.depth != 1)
				|| (!use && cd.corner != 0 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 0 && cd.depth < size - 1)
				|| (use && cd.corner != 0 && cd.depth == size - 1)
				|| (!use && cd.corner == 0 && cd.depth >= size - 1)
				|| (!use && cd.corner != 0 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, FACET_LENGTH, -FACET_LENGTH);
			drawTetraFacet(mi, j, -1, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 1 && cd.depth == 2 * size - 3)
				|| (use && cd.corner != 1 && cd.depth == size - 1)
				|| (!use && cd.corner == 1 && cd.depth != 2 * size - 3)
				|| (!use && cd.corner != 1 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 1 && cd.depth > size - 1)
				|| (use && cd.corner != 1 && cd.depth == size - 1)
				|| (!use && cd.corner == 1 && cd.depth <= size - 1)
				|| (!use && cd.corner != 1 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(FACET_LENGTH, FACET_LENGTH, -s0 * FACET_LENGTH);
			drawTetraFacet(mi, k, -1, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		/* 1 */
		if (all || (!sticky && ((use && cd.corner == 2 && cd.depth == 1)
				|| (use && cd.corner != 2 && cd.depth == size - 1)
				|| (!use && cd.corner == 2 && cd.depth != 1)
				|| (!use && cd.corner != 2 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 2 && cd.depth < size - 1)
				|| (use && cd.corner != 2 && cd.depth == size - 1)
				|| (!use && cd.corner == 2 && cd.depth >= size - 1)
				|| (!use && cd.corner != 2 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(FACET_LENGTH, s0 * FACET_LENGTH, FACET_LENGTH);
			drawTetraFacet(mi, -1, 2, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 1 && cd.depth == 1)
				|| (use && cd.corner != 1 && cd.depth == size - 1)
				|| (!use && cd.corner == 1 && cd.depth != 1)
				|| (!use && cd.corner != 1 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 1 && cd.depth < size - 1)
				|| (use && cd.corner != 1 && cd.depth == size - 1)
				|| (!use && cd.corner == 1 && cd.depth >= size - 1)
				|| (!use && cd.corner != 1 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(FACET_LENGTH, FACET_LENGTH, s0 * FACET_LENGTH);
			drawTetraFacet(mi, -1, j, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 0 && cd.depth == 1)
				|| (use && cd.corner != 0 && cd.depth == size - 1)
				|| (!use && cd.corner == 0 && cd.depth != 1)
				|| (!use && cd.corner != 0 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 0 && cd.depth < size - 1)
				|| (use && cd.corner != 0 && cd.depth == size - 1)
				|| (!use && cd.corner == 0 && cd.depth >= size - 1)
				|| (!use && cd.corner != 0 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, FACET_LENGTH, FACET_LENGTH);
			drawTetraFacet(mi, -1, k, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		/* 2 */
		if (all || (!sticky && ((use && cd.corner == 2 && cd.depth == 1)
				|| (use && cd.corner != 2 && cd.depth == size - 1)
				|| (!use && cd.corner == 2 && cd.depth != 1)
				|| (!use && cd.corner != 2 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 2 && cd.depth < size - 1)
				|| (use && cd.corner != 2 && cd.depth == size - 1)
				|| (!use && cd.corner == 2 && cd.depth >= size - 1)
				|| (!use && cd.corner != 2 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(-FACET_LENGTH, s0 * FACET_LENGTH, FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, 2, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 0 && cd.depth == 2 * size - 3)
				|| (use && cd.corner != 0 && cd.depth == size - 1)
				|| (!use && cd.corner == 0 && cd.depth != 2 * size - 3)
				|| (!use && cd.corner != 0 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 0 && cd.depth > size - 1)
				|| (use && cd.corner != 0 && cd.depth == size - 1)
				|| (!use && cd.corner == 0 && cd.depth <= size - 1)
				|| (!use && cd.corner != 0 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, FACET_LENGTH, FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, j, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 1 && cd.depth == 1)
				|| (use && cd.corner != 1 && cd.depth == size - 1)
				|| (!use && cd.corner == 1 && cd.depth != 1)
				|| (!use && cd.corner != 1 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 1 && cd.depth < size - 1)
				|| (use && cd.corner != 1 && cd.depth == size - 1)
				|| (!use && cd.corner == 1 && cd.depth >= size - 1)
				|| (!use && cd.corner != 1 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(-FACET_LENGTH, FACET_LENGTH, s0 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, k, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		/* 3 */
		if (all || (!sticky && ((use && cd.corner == 2 && cd.depth == 1)
				|| (use && cd.corner != 2 && cd.depth == size - 1)
				|| (!use && cd.corner == 2 && cd.depth != 1)
				|| (!use && cd.corner != 2 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 2 && cd.depth < size - 1)
				|| (use && cd.corner != 2 && cd.depth == size - 1)
				|| (!use && cd.corner == 2 && cd.depth >= size - 1)
				|| (!use && cd.corner != 2 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(-FACET_LENGTH, s0 * FACET_LENGTH, -FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, 2, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 1 && cd.depth == 2 * size - 3)
				|| (use && cd.corner != 1 && cd.depth == size - 1)
				|| (!use && cd.corner == 1 && cd.depth != 2 * size - 3)
				|| (!use && cd.corner != 1 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 1 && cd.depth > size - 1)
				|| (use && cd.corner != 1 && cd.depth == size - 1)
				|| (!use && cd.corner == 1 && cd.depth <= size - 1)
				|| (!use && cd.corner != 1 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(-FACET_LENGTH, FACET_LENGTH, -s0 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, j, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 0 && cd.depth == 2 * size - 3)
				|| (use && cd.corner != 0 && cd.depth == size - 1)
				|| (!use && cd.corner == 0 && cd.depth != 2 * size - 3)
				|| (!use && cd.corner != 0 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 0 && cd.depth > size - 1)
				|| (use && cd.corner != 0 && cd.depth == size - 1)
				|| (!use && cd.corner == 0 && cd.depth <= size - 1)
				|| (!use && cd.corner != 0 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, FACET_LENGTH, -FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, k, -1, -1, -1, -1);
			glPopMatrix();
		}
		/* 4 */
		if (all || (!sticky && ((use && cd.corner == 2 && cd.depth == 2 * size - 3)
				|| (use && cd.corner != 2 && cd.depth == size - 1)
				|| (!use && cd.corner == 2 && cd.depth != 2 * size - 3)
				|| (!use && cd.corner != 2 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 2 && cd.depth > size - 1)
				|| (use && cd.corner != 2 && cd.depth == size - 1)
				|| (!use && cd.corner == 2 && cd.depth <= size - 1)
				|| (!use && cd.corner != 2 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(-FACET_LENGTH, -s0 * FACET_LENGTH, FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, 2, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 1 && cd.depth == 1)
				|| (use && cd.corner != 1 && cd.depth == size - 1)
				|| (!use && cd.corner == 1 && cd.depth != 1)
				|| (!use && cd.corner != 1 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 1 && cd.depth < size - 1)
				|| (use && cd.corner != 1 && cd.depth == size - 1)
				|| (!use && cd.corner == 1 && cd.depth >= size - 1)
				|| (!use && cd.corner != 1 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(-FACET_LENGTH, -FACET_LENGTH, s0 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, j, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 0 && cd.depth == 2 * size - 3)
				|| (use && cd.corner != 0 && cd.depth == size - 1)
				|| (!use && cd.corner == 0 && cd.depth != 2 * size - 3)
				|| (!use && cd.corner != 0 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 0 && cd.depth > size - 1)
				|| (use && cd.corner != 0 && cd.depth == size - 1)
				|| (!use && cd.corner == 0 && cd.depth <= size - 1)
				|| (!use && cd.corner != 0 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, -FACET_LENGTH, FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, k, -1, -1, -1);
			glPopMatrix();
		}
		/* 5 */
		if (all || (!sticky && ((use && cd.corner == 2 && cd.depth == 2 * size - 3)
				|| (use && cd.corner != 2 && cd.depth == size - 1)
				|| (!use && cd.corner == 2 && cd.depth != 2 * size - 3)
				|| (!use && cd.corner != 2 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 2 && cd.depth > size - 1)
				|| (use && cd.corner != 2 && cd.depth == size - 1)
				|| (!use && cd.corner == 2 && cd.depth <= size - 1)
				|| (!use && cd.corner != 2 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(FACET_LENGTH, -s0 * FACET_LENGTH, FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, 2, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 0 && cd.depth == 1)
				|| (use && cd.corner != 0 && cd.depth == size - 1)
				|| (!use && cd.corner == 0 && cd.depth != 1)
				|| (!use && cd.corner != 0 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 0 && cd.depth < size - 1)
				|| (use && cd.corner != 0 && cd.depth == size - 1)
				|| (!use && cd.corner == 0 && cd.depth >= size - 1)
				|| (!use && cd.corner != 0 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, -FACET_LENGTH, FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, j, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 1 && cd.depth == 1)
				|| (use && cd.corner != 1 && cd.depth == size - 1)
				|| (!use && cd.corner == 1 && cd.depth != 1)
				|| (!use && cd.corner != 1 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 1 && cd.depth < size - 1)
				|| (use && cd.corner != 1 && cd.depth == size - 1)
				|| (!use && cd.corner == 1 && cd.depth >= size - 1)
				|| (!use && cd.corner != 1 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(FACET_LENGTH, -FACET_LENGTH, s0 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, k, -1, -1);
			glPopMatrix();
		}
		/* 6 */
		if (all || (!sticky && ((use && cd.corner == 2 && cd.depth == 2 * size - 3)
				|| (use && cd.corner != 2 && cd.depth == size - 1)
				|| (!use && cd.corner == 2 && cd.depth != 2 * size - 3)
				|| (!use && cd.corner != 2 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 2 && cd.depth > size - 1)
				|| (use && cd.corner != 2 && cd.depth == size - 1)
				|| (!use && cd.corner == 2 && cd.depth <= size - 1)
				|| (!use && cd.corner != 2 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(FACET_LENGTH, -s0 * FACET_LENGTH, -FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, 2, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 1 && cd.depth == 2 * size - 3)
				|| (use && cd.corner != 1 && cd.depth == size - 1)
				|| (!use && cd.corner == 1 && cd.depth != 2 * size - 3)
				|| (!use && cd.corner != 1 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 1 && cd.depth > size - 1)
				|| (use && cd.corner != 1 && cd.depth == size - 1)
				|| (!use && cd.corner == 1 && cd.depth <= size - 1)
				|| (!use && cd.corner != 1 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(FACET_LENGTH, -FACET_LENGTH, -s0 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, j, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 0 && cd.depth == 1)
				|| (use && cd.corner != 0 && cd.depth == size - 1)
				|| (!use && cd.corner == 0 && cd.depth != 1)
				|| (!use && cd.corner != 0 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 0 && cd.depth < size - 1)
				|| (use && cd.corner != 0 && cd.depth == size - 1)
				|| (!use && cd.corner == 0 && cd.depth >= size - 1)
				|| (!use && cd.corner != 0 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, -FACET_LENGTH, -FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, k, -1);
			glPopMatrix();
		}
		/* 7 */
		if (all || (!sticky && ((use && cd.corner == 2 && cd.depth == 2 * size - 3)
				|| (use && cd.corner != 2 && cd.depth == size - 1)
				|| (!use && cd.corner == 2 && cd.depth != 2 * size - 3)
				|| (!use && cd.corner != 2 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 2 && cd.depth > size - 1)
				|| (use && cd.corner != 2 && cd.depth == size - 1)
				|| (!use && cd.corner == 2 && cd.depth <= size - 1)
				|| (!use && cd.corner != 2 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(-FACET_LENGTH, -s0 * FACET_LENGTH, -FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, -1, 2);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 0 && cd.depth == 2 * size - 3)
				|| (use && cd.corner != 0 && cd.depth == size - 1)
				|| (!use && cd.corner == 0 && cd.depth != 2 * size - 3)
				|| (!use && cd.corner != 0 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 0 && cd.depth > size - 1)
				|| (use && cd.corner != 0 && cd.depth == size - 1)
				|| (!use && cd.corner == 0 && cd.depth <= size - 1)
				|| (!use && cd.corner != 0 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, -FACET_LENGTH, -FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, -1, j);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 1 && cd.depth == 2 * size - 3)
				|| (use && cd.corner != 1 && cd.depth == size - 1)
				|| (!use && cd.corner == 1 && cd.depth != 2 * size - 3)
				|| (!use && cd.corner != 1 && cd.depth != size - 1)))
			|| (sticky && ((use && cd.corner == 1 && cd.depth > size - 1)
				|| (use && cd.corner != 1 && cd.depth == size - 1)
				|| (!use && cd.corner == 1 && cd.depth <= size - 1)
				|| (!use && cd.corner != 1 && cd.depth != size - 1)))) {
			glPushMatrix();
			glTranslate(-FACET_LENGTH, -FACET_LENGTH, -s0* FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, -1, k);
			glPopMatrix();
		}
	}
	if (size > 2 && size % 2 == 1) {
		/* OUTSIDE MIDDLE EDGES */ /* 1, 3, 6; 5, 7, 12; 4, 8, 20 */
		int m1 = (size - 1) / 2;
		int e = (3 * size - 3) / 2; /* 3, 6, 9 */
		s1 = size - 1;
		i = (s1 / 2) * (s1 / 2);
		j = (s1 / 2 + 1) * (s1 / 2 + 1) - 1;
		k = size * size - size;
		if (all || (use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner != 1 && cd.depth == m1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner != 1 && cd.depth != m1)) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, s1 * FACET_LENGTH, 0);
			drawOctaFacet(mi, i, j, -1, -1, -1, -1, -1, -1); /* 0 */
			glPopMatrix();
		}
		if (all || (use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner != 0 && cd.depth == m1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner != 0 && cd.depth != m1)) {
			glPushMatrix();
			glTranslate(0, s1 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, i, j, -1, -1, -1, -1, -1); /* 1 */
			glPopMatrix();
		}
		if (all || (use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 2 && cd.depth == m1)
				|| (use && cd.corner == 0 && cd.depth == e)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 2 && cd.depth != m1)
				|| (!use && cd.corner == 0 && cd.depth != e)) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, s1 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, i, j, -1, -1, -1, -1); /* 2 */
			glPopMatrix();
		}
		if (all || (use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 2 && cd.depth == m1)
				|| (use && cd.corner == 1 && cd.depth == e)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 2 && cd.depth != m1)
				|| (!use && cd.corner == 1 && cd.depth != e)) {
			glPushMatrix();
			glTranslate(0, s1 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawOctaFacet(mi, j, -1, -1, i, -1, -1, -1, -1); /* 3 */
			glPopMatrix();
		}
		if (all || (use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == m1)
				|| (use && cd.corner == 2 && cd.depth == e)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != m1)
				|| (!use && cd.corner == 2 && cd.depth != e)) {
			glPushMatrix();
			glTranslate(0, -s1 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, -1, i, j, -1, -1); /* 9 */
			glPopMatrix();
		}
		if (all || (use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == m1)
				|| (use && cd.corner == 2 && cd.depth == e)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != m1)
				|| (!use && cd.corner == 2 && cd.depth != e)) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, -s1 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, -1, -1, -1, i, j, -1); /* 10 */
			glPopMatrix();
		}
		if (all || (use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner != 0 && cd.depth == e)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner != 0 && cd.depth != e)) {
			glPushMatrix();
			glTranslate(0, -s1 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, -1, -1, -1, i, j); /* 11 */
			glPopMatrix();
		}
		if (all || (use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner != 1 && cd.depth == e)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner != 1 && cd.depth != e)) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, -s1 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, -1, -1, j, -1, -1, i); /* 12 */
			glPopMatrix();
		}
		if (all || (use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == m1)
				|| (use && cd.corner == 1 && cd.depth == e)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != m1)
				|| (!use && cd.corner == 1 && cd.depth != e)) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, 0, -s1 * FACET_LENGTH);
			drawOctaFacet(mi, k, -1, -1, -1, -1, -1, k, -1); /* 4 */
			glPopMatrix();
		}
		if (all || (use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner != 2 && cd.depth == m1)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner != 2 && cd.depth != m1)) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, 0, s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, k, -1, -1, -1, k, -1, -1); /* 5 */
			glPopMatrix();
		}
		if (all || (use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == m1)
				|| (use && cd.corner == 0 && cd.depth == e)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != m1)
				|| (!use && cd.corner == 0 && cd.depth != e)) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, 0, s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, k, -1, k, -1, -1, -1); /* 6 */
			glPopMatrix();
		}
		if (all || (use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner != 2 && cd.depth == e)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner != 2 && cd.depth != e)) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, 0, -s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, k, -1, -1, -1, k); /* 7 */
			glPopMatrix();
		}
	} else if (size > 2 && size % 2 == 0) {
		/* INSIDE MIDDLE EDGE - 1 */
		int m = size / 2;
		int e = 2 * size - 2 - m;
		s1 = size - 1;
		i = (size / 2) * (size / 2) + 1;
		j = (size / 2 + 1) * (size / 2 + 1) - 2;
		k = size * size - size;
		/* 0 */
		if (all || (!sticky && ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner != 1 && cd.depth == m)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner != 1 && cd.depth != m)))
			|| (sticky && ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner != 1 && cd.depth < size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner != 1 && cd.depth >= size - 1)))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, s1 * FACET_LENGTH, -FACET_LENGTH);
			drawTetraFacet(mi, i, -1, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == e)
				|| (use && cd.corner == 2 && cd.depth == m)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != e)
				|| (!use && cd.corner == 2 && cd.depth != m)))
			|| (sticky && ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth > size - 1)
				|| (use && cd.corner == 2 && cd.depth < size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth <= size - 1)
				|| (!use && cd.corner == 2 && cd.depth >= size - 1)))) {
			glPushMatrix();
			glTranslate(FACET_LENGTH, s1 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, j, -1, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == e)
				|| (use && cd.corner == 0 && cd.depth == m)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != e)
				|| (!use && cd.corner == 0 && cd.depth != m)))
			|| (sticky && ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth > size - 1)
				|| (use && cd.corner == 0 && cd.depth < size - 1)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth <= size - 1)
				|| (!use && cd.corner == 0 && cd.depth >= size - 1)))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, k, -1, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		/* 1 */
		if (all || (!sticky && ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner != 0 && cd.depth == m)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner != 0 && cd.depth != m)))
			|| (sticky && ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner != 0 && cd.depth < size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner != 0 && cd.depth >= size - 1)))) {
			glPushMatrix();
			glTranslate(FACET_LENGTH, s1 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, i, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner != 1 && cd.depth == m)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner != 1 && cd.depth != m)))
			|| (sticky && ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner != 1 && cd.depth < size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner != 1 && cd.depth >= size - 1)))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, s1 * FACET_LENGTH, FACET_LENGTH);
			drawTetraFacet(mi, -1, j, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner != 2 && cd.depth == m)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner != 2 && cd.depth != m)))
			|| (sticky && ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner != 2 && cd.depth < size - 1)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner != 2 && cd.depth >= size - 1)))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, k, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		/* 2 */
		if (all || (!sticky && ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == e)
				|| (use && cd.corner == 2 && cd.depth == m)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != e)
				|| (!use && cd.corner == 2 && cd.depth != m)))
			|| (sticky && ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth > size - 1)
				|| (use && cd.corner == 2 && cd.depth < size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth <= size - 1)
				|| (!use && cd.corner == 2 && cd.depth >= size - 1)))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, s1 * FACET_LENGTH, FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, i, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner != 0 && cd.depth == m)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner != 0 && cd.depth != m)))
			|| (sticky && ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner != 0 && cd.depth < size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner != 0 && cd.depth >= size - 1)))) {
			glPushMatrix();
			glTranslate(-FACET_LENGTH, s1 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, j, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == e)
				|| (use && cd.corner == 1 && cd.depth == m)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != e)
				|| (!use && cd.corner == 1 && cd.depth != m)))
			|| (sticky && ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth > size - 1)
				|| (use && cd.corner == 1 && cd.depth < size - 1)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth <= size - 1)
				|| (!use && cd.corner == 1 && cd.depth >= size - 1)))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, k, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		/* 3 */
		if (all || (!sticky && ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == e)
				|| (use && cd.corner == 2 && cd.depth == m)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != e)
				|| (!use && cd.corner == 2 && cd.depth != m)))
			|| (sticky && ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth > size - 1)
				|| (use && cd.corner == 2 && cd.depth < size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth <= size - 1)
				|| (!use && cd.corner == 2 && cd.depth >= size - 1)))) {
			glPushMatrix();
			glTranslate(-FACET_LENGTH, s1 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, i, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == e)
				|| (use && cd.corner == 2 && cd.depth == m)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != e)
				|| (!use && cd.corner == 2 && cd.depth != m)))
			|| (sticky && ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth > size - 1)
				|| (use && cd.corner == 2 && cd.depth < size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth <= size - 1)
				|| (!use && cd.corner == 2 && cd.depth >= size - 1)))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, s1 * FACET_LENGTH, -FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, j, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner != 2 && cd.depth == e)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner != 2 && cd.depth != e)))
			|| (sticky && ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner != 2 && cd.depth > size - 1)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner != 2 && cd.depth <= size - 1)))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, k, -1, -1, -1, -1);
			glPopMatrix();
		}
		/* 4 */
		if (all || (!sticky && ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 2 && cd.depth == e)
				|| (use && cd.corner == 1 && cd.depth == m)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 2 && cd.depth != e)
				|| (!use && cd.corner == 1 && cd.depth != m)))
			|| (sticky && ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 2 && cd.depth > size - 1)
				|| (use && cd.corner == 1 && cd.depth < size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 2 && cd.depth <= size - 1)
				|| (!use && cd.corner == 1 && cd.depth >= size - 1)))) {
			glPushMatrix();
			glTranslate(-FACET_LENGTH, -s1 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, i, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner != 1 && cd.depth == e)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner != 1 && cd.depth != e)))
			|| (sticky && ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner != 1 && cd.depth > size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner != 1 && cd.depth <= size - 1)))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, -s1 * FACET_LENGTH, FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, j, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == e)
				|| (use && cd.corner == 1 && cd.depth == m)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != e)
				|| (!use && cd.corner == 1 && cd.depth != m)))
			|| (sticky && ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth > size - 1)
				|| (use && cd.corner == 1 && cd.depth < size - 1)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth <= size - 1)
				|| (!use && cd.corner == 1 && cd.depth >= size - 1)))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, -FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, k, -1, -1, -1);
			glPopMatrix();
		}
		/* 5 */
		if (all || (!sticky && ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 2 && cd.depth == e)
				|| (use && cd.corner == 0 && cd.depth == m)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 2 && cd.depth != e)
				|| (!use && cd.corner == 0 && cd.depth != m)))
			|| (sticky && ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 2 && cd.depth > size - 1)
				|| (use && cd.corner == 0 && cd.depth < size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 2 && cd.depth <= size - 1)
				|| (!use && cd.corner == 0 && cd.depth >= size - 1)))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, -s1 * FACET_LENGTH, FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, i, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 2 && cd.depth == e)
				|| (use && cd.corner == 1 && cd.depth == m)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 2 && cd.depth != e)
				|| (!use && cd.corner == 1 && cd.depth != m)))
			|| (sticky && ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 2 && cd.depth > size - 1)
				|| (use && cd.corner == 1 && cd.depth < size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 2 && cd.depth <= size - 1)
				|| (!use && cd.corner == 1 && cd.depth >= size - 1)))) {
			glPushMatrix();
			glTranslate(FACET_LENGTH, -s1 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, j, -1, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner != 2 && cd.depth == m)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner != 2 && cd.depth != m)))
			|| (sticky && ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner != 2 && cd.depth < size - 1)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner != 2 && cd.depth >= size - 1)))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, -FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, k, -1, -1);
			glPopMatrix();
		}
		/* 6 */
		if (all || (!sticky && ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner != 0 && cd.depth == e)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner != 0 && cd.depth != e)))
			|| (sticky && ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner != 0 && cd.depth > size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner != 0 && cd.depth <= size - 1)))) {
			glPushMatrix();
			glTranslate(FACET_LENGTH, -s1 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, i, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 2 && cd.depth == e)
				|| (use && cd.corner == 0 && cd.depth == m)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 2 && cd.depth != e)
				|| (!use && cd.corner == 0 && cd.depth != m)))
			|| (sticky && ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 2 && cd.depth > size - 1)
				|| (use && cd.corner == 0 && cd.depth < size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 2 && cd.depth <= size - 1)
				|| (!use && cd.corner == 0 && cd.depth >= size - 1)))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, -s1 * FACET_LENGTH, -FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, j, -1);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == e)
				|| (use && cd.corner == 0 && cd.depth == m)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != e)
				|| (!use && cd.corner == 0 && cd.depth != m)))
			|| (sticky && ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth > size - 1)
				|| (use && cd.corner == 0 && cd.depth < size - 1)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth <= size - 1)
				|| (!use && cd.corner == 0 && cd.depth >= size - 1)))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, -FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, k, -1);
			glPopMatrix();
		}
		/* 7 */
		if (all || (!sticky && ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner != 1 && cd.depth == e)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner != 1 && cd.depth != e)))
			|| (sticky && ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner != 1 && cd.depth > size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner != 1 && cd.depth <= size - 1)))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, -s1 * FACET_LENGTH, -FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, -1, i);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner != 0 && cd.depth == e)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner != 0 && cd.depth != e)))
			|| (sticky && ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner != 0 && cd.depth > size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner != 0 && cd.depth <= size - 1)))) {
			glPushMatrix();
			glTranslate(-FACET_LENGTH, -s1 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, -1, j);
			glPopMatrix();
		}
		if (all || (!sticky && ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner != 2 && cd.depth == e)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner != 2 && cd.depth != e)))
			|| (sticky && ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner != 2 && cd.depth > size - 1)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner != 2 && cd.depth <= size - 1)))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, -FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, -1, k);
			glPopMatrix();
		}
	}
	if (sticky) { /* OUTSIDE EDGE 1 */
		int a, b, c, d, e, f;
		edge = size / 2 - 1;
		s0 = 2 * size - 2 * edge - 2; 
		s1 = 2 * edge; /* 2, 4 */
		a = edge * edge; /* 1, 4 */
		b = (edge + 1) * (edge + 1) - 1; /* 3, 8 */
		c = (size - edge - 1) * (size - edge - 1); /* 4, 9, 16 */
		d = (size - edge) * (size - edge) - 1; /* 8, 15 */
		e = (size - 1) * (size - 1) + 2 * edge; /* 11, */
		f = size * size - 2 * edge - 1; /* 13, */
		if (all || ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner != 1 && cd.depth < size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner != 1 && cd.depth >= size - 1))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, s0 * FACET_LENGTH, 0);
			drawOctaFacet(mi, a, b, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner != 0 && cd.depth < size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner != 0 && cd.depth >= size - 1))) {
			glPushMatrix();
			glTranslate(0, s0 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, a, b, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth > size - 1)
				|| (use && cd.corner == 2 && cd.depth < size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth <= size - 1)
				|| (!use && cd.corner == 2 && cd.depth >= size - 1))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, s0 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, a, b, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth > size - 1)
				|| (use && cd.corner == 2 && cd.depth < size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth <= size - 1)
				|| (!use && cd.corner == 2 && cd.depth >= size - 1))) {
			glPushMatrix();
			glTranslate(0, s0 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawOctaFacet(mi, b, -1, -1, a, -1, -1, -1, -1);
			glPopMatrix();
		}

		if (all || ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth < size - 1)
				|| (use && cd.corner == 2 && cd.depth > size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth >= size - 1)
				|| (!use && cd.corner == 2 && cd.depth <= size - 1))) {
			glPushMatrix();
			glTranslate(0, -s0 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, -1, a, b, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth < size - 1)
				|| (use && cd.corner == 2 && cd.depth > size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth >= size - 1)
				|| (!use && cd.corner == 2 && cd.depth <= size - 1))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, -s0 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, -1, -1, -1, a, b, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner != 0 && cd.depth > size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner != 0 && cd.depth <= size - 1))) {
			glPushMatrix();
			glTranslate(0, -s0 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, -1, -1, -1, a, b);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner != 1 && cd.depth > size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner != 1 && cd.depth <= size - 1))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, -s0 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, -1, -1, b, -1, -1, a);
			glPopMatrix();
		}

		if (all || ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner != 1 && cd.depth < size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner != 1 && cd.depth >= size - 1))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, s1 * FACET_LENGTH, 0);
			drawOctaFacet(mi, c, d, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner != 0 && cd.depth < size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner != 0 && cd.depth >= size - 1))) {
			glPushMatrix();
			glTranslate(0, s1 * FACET_LENGTH, s0 * FACET_LENGTH);
			drawOctaFacet(mi, -1, c, d, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth > size - 1)
				|| (use && cd.corner == 2 && cd.depth < size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth <= size - 1)
				|| (!use && cd.corner == 2 && cd.depth >= size - 1))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, s1 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, c, d, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth > size - 1)
				|| (use && cd.corner == 2 && cd.depth < size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth <= size - 1)
				|| (!use && cd.corner == 2 && cd.depth >= size - 1))) {
			glPushMatrix();
			glTranslate(0, s1 * FACET_LENGTH, -s0 * FACET_LENGTH);
			drawOctaFacet(mi, d, -1, -1, c, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth < size - 1)
				|| (use && cd.corner == 2 && cd.depth > size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth >= size - 1)
				|| (!use && cd.corner == 2 && cd.depth <= size - 1))) {
			glPushMatrix();
			glTranslate(0, -s1 * FACET_LENGTH, s0 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, -1, c, d, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth < size - 1)
				|| (use && cd.corner == 2 && cd.depth > size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth >= size - 1)
				|| (!use && cd.corner == 2 && cd.depth <= size - 1))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, -s1 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, -1, -1, -1, c, d, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner != 0 && cd.depth > size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner != 0 && cd.depth <= size - 1))) {
			glPushMatrix();
			glTranslate(0, -s1 * FACET_LENGTH, -s0 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, -1, -1, -1, c, d);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner != 1 && cd.depth > size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner != 1 && cd.depth <= size - 1))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, -s1 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, -1, -1, d, -1, -1, c);
			glPopMatrix();
		}

		if (all || ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth > size - 1)
				|| (use && cd.corner == 0 && cd.depth < size - 1)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth <= size - 1)
				|| (!use && cd.corner == 0 && cd.depth >= size - 1))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, 0, -s1 * FACET_LENGTH);
			drawOctaFacet(mi, e, -1, -1, -1, -1, -1, f, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner != 2 && cd.depth < size - 1)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner != 2 && cd.depth >= size - 1))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, 0, s0 * FACET_LENGTH);
			drawOctaFacet(mi, -1, e, -1, -1, -1, f, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth < size - 1)
				|| (use && cd.corner == 0 && cd.depth > size - 1)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth >= size - 1)
				|| (!use && cd.corner == 0 && cd.depth <= size - 1))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, 0, s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, e, -1, f, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner != 2 && cd.depth > size - 1)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner != 2 && cd.depth <= size - 1))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, 0, -s0 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, e, -1, -1, -1, f);
			glPopMatrix();
		}

		if (all || ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth > size - 1)
				|| (use && cd.corner == 1 && cd.depth < size - 1)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth <= size - 1)
				|| (!use && cd.corner == 1 && cd.depth >= size - 1))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, 0, s0 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, f, -1, e, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner != 2 && cd.depth < size - 1)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner != 2 && cd.depth >= size - 1))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, 0, s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, f, -1, -1, -1, e, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth < size - 1)
				|| (use && cd.corner == 1 && cd.depth > size - 1)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth >= size - 1)
				|| (!use && cd.corner == 1 && cd.depth <= size - 1))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, 0, -s0 * FACET_LENGTH);
			drawOctaFacet(mi, f, -1, -1, -1, -1, -1, e, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner != 2 && cd.depth > size - 1)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner != 2 && cd.depth <= size - 1))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, 0, -s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, f, -1, -1, -1, f);
			glPopMatrix();
		}
	}
	for (edge = 1; edge < (size - 1) / 2; edge++) {
	    /*if (size > 4) INSIDE EDGE 2 */
		int s3, m1, m2;
		int a, b, c, d, e, f;
		s1 = 1;
		s2 = 2 * size - 2 * edge - 3; /* 2 * size - 5 */
		s3 = 2 * edge + 1; /* 3 */
		a = (edge + 1) * (edge + 1) + 1; /* 5 */
		b = (edge + 2) * (edge + 2) - 2; /* 7 */
		c = (size - edge - 1) * (size - edge - 1) + 1; /* (size - 2) * (size - 2) + 1 */
		d = (size - edge) * (size - edge) - 2; /* (size - 1) * (size - 1) - 2 */
		e = (size - 1) * ( size - 1) + 2 * edge + 1; /* (size - 1) * (size - 1) + 3 */
		f = size * size - 2 * edge - 2; /* size * size - 4 */
		m1 = edge;
		m2 = edge + 1;
		if (all || (use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == size - 1 - m1)
				|| (use && cd.corner == 2 && cd.depth == m2)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1 - m1)
				|| (!use && cd.corner == 2 && cd.depth != m2)) {
			glPushMatrix();
			glTranslate(s3 * FACET_LENGTH, s2 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, a, -1, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
			glPushMatrix();
			glTranslate(s3 * FACET_LENGTH, s2 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, b, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == size - 1 - m1)
				|| (use && cd.corner == 2 && cd.depth == m2)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1 - m1)
				|| (!use && cd.corner == 2 && cd.depth != m2)) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, s2 * FACET_LENGTH, s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, a, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, s2 * FACET_LENGTH, s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, b, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == size - 1 + m1)
				|| (use && cd.corner == 2 && cd.depth == m2)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1 + m1)
				|| (!use && cd.corner == 2 && cd.depth != m2)) {
			glPushMatrix();
			glTranslate(-s3 * FACET_LENGTH, s2 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, a, -1, -1, -1, -1, -1);
			glPopMatrix();
			glPushMatrix();
			glTranslate(-s3 * FACET_LENGTH, s2 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, b, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == size - 1 + m1)
				|| (use && cd.corner == 2 && cd.depth == m2)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1 + m1)
				|| (!use && cd.corner == 2 && cd.depth != m2)) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, s2 * FACET_LENGTH, -s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, a, -1, -1, -1, -1);
			glPopMatrix();
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, s2 * FACET_LENGTH, -s3 * FACET_LENGTH);
			drawTetraFacet(mi, b, -1, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}

		if (all || (use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == size - 1 - m1)
				|| (use && cd.corner == 2 && cd.depth == 2 * size - 2 - m2)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1 - m1)
				|| (!use && cd.corner == 2 && cd.depth != 2 * size - 2 - m2)) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, -s2 * FACET_LENGTH, s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, a, -1, -1, -1);
			glPopMatrix();
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, -s2 * FACET_LENGTH, s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, b, -1, -1);
			glPopMatrix();
		}
		if (all || (use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == size - 1 - m1)
				|| (use && cd.corner == 2 && cd.depth == 2 * size - 2 - m2)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1 - m1)
				|| (!use && cd.corner == 2 && cd.depth != 2 * size - 2 - m2)) {
			glPushMatrix();
			glTranslate(s3 * FACET_LENGTH, -s2 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, a, -1, -1);
			glPopMatrix();
			glPushMatrix();
			glTranslate(s3 * FACET_LENGTH, -s2 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, b, -1);
			glPopMatrix();
		}
		if (all || (use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == size - 1 + m1)
				|| (use && cd.corner == 2 && cd.depth == 2 * size - 2 - m2)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1 + m1)
				|| (!use && cd.corner == 2 && cd.depth != 2 * size - 2 - m2)) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, -s2 * FACET_LENGTH, -s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, a, -1);
			glPopMatrix();
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, -s2 * FACET_LENGTH, -s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, -1, b);
			glPopMatrix();
		}
		if (all || (use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == size - 1 + m1)
				|| (use && cd.corner == 2 && cd.depth == 2 * size - 2 - m2)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1 + m1)
				|| (!use && cd.corner == 2 && cd.depth != 2 * size - 2 - m2)) {
			glPushMatrix();
			glTranslate(-s3 * FACET_LENGTH, -s2 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, -1, a);
			glPopMatrix();
			glPushMatrix();
			glTranslate(-s3 * FACET_LENGTH, -s2 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, b, -1, -1, -1);
			glPopMatrix();
		}

		if (all || (use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == m2)
				|| (use && cd.corner == 2 && cd.depth == size - 1 - m1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != m2)
				|| (!use && cd.corner == 2 && cd.depth != size - 1 - m1)) {
			glPushMatrix();
			glTranslate(s2 * FACET_LENGTH, s3 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, c, -1, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
			glPushMatrix();
			glTranslate(s2 * FACET_LENGTH, s3 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, d, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == m2)
				|| (use && cd.corner == 2 && cd.depth == size - 1 - m1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != m2)
				|| (!use && cd.corner == 2 && cd.depth != size - 1 - m1)) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, s3 * FACET_LENGTH, s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, c, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, s3 * FACET_LENGTH, s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, d, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == 2 * size - 2 - m2)
				|| (use && cd.corner == 2 && cd.depth == size - 1 - m1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != 2 * size - 2 - m2)
				|| (!use && cd.corner == 2 && cd.depth != size - 1 - m1)) {
			glPushMatrix();
			glTranslate(-s2 * FACET_LENGTH, s3 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, c, -1, -1, -1, -1, -1);
			glPopMatrix();
			glPushMatrix();
			glTranslate(-s2 * FACET_LENGTH, s3 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, d, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == 2 * size - 2 - m2)
				|| (use && cd.corner == 2 && cd.depth == size - 1 - m1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != 2 * size - 2 - m2)
				|| (!use && cd.corner == 2 && cd.depth != size - 1 - m1)) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, s3 * FACET_LENGTH, -s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, c, -1, -1, -1, -1);
			glPopMatrix();
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, s3 * FACET_LENGTH, -s2 * FACET_LENGTH);
			drawTetraFacet(mi, d, -1, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == m2)
				|| (use && cd.corner == 2 && cd.depth == size - 1 + m1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != m2)
				|| (!use && cd.corner == 2 && cd.depth != size - 1 + m1)) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, -s3 * FACET_LENGTH, s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, c, -1, -1, -1);
			glPopMatrix();
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, -s3 * FACET_LENGTH, s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, d, -1, -1);
			glPopMatrix();
		}
		if (all || (use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == m2)
				|| (use && cd.corner == 2 && cd.depth == size - 1 + m1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != m2)
				|| (!use && cd.corner == 2 && cd.depth != size - 1 + m1)) {
			glPushMatrix();
			glTranslate(s2 * FACET_LENGTH, -s3 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, c, -1, -1);
			glPopMatrix();
			glPushMatrix();
			glTranslate(s2 * FACET_LENGTH, -s3 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, d, -1);
			glPopMatrix();
		}
		if (all || (use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == 2 * size - 2 - m2)
				|| (use && cd.corner == 2 && cd.depth == size - 1 + m1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != 2 * size - 2 - m2)
				|| (!use && cd.corner == 2 && cd.depth != size - 1 + m1)) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, -s3 * FACET_LENGTH, -s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, c, -1);
			glPopMatrix();
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, -s3 * FACET_LENGTH, -s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, -1, d);
			glPopMatrix();
		}
		if (all || (use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == 2 * size - 2 - m2)
				|| (use && cd.corner == 2 && cd.depth == size - 1 + m1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != 2 * size - 2 - m2)
				|| (!use && cd.corner == 2 && cd.depth != size - 1 + m1)) {
			glPushMatrix();
			glTranslate(-s2 * FACET_LENGTH, -s3 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, -1, c);
			glPopMatrix();
			glPushMatrix();
			glTranslate(-s2 * FACET_LENGTH, -s3 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, d, -1, -1, -1);
			glPopMatrix();
		}

		if (all || (use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == size - 1 + m1)
				|| (use && cd.corner == 0 && cd.depth == m2)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1 + m1)
				|| (!use && cd.corner == 0 && cd.depth != m2)) {
			glPushMatrix();
			glTranslate(s2 * FACET_LENGTH, s1 * FACET_LENGTH, -s3 * FACET_LENGTH);
			drawTetraFacet(mi, e, -1, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
			glPushMatrix();
			glTranslate(s2 * FACET_LENGTH, -s1 * FACET_LENGTH, -s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, f, -1);
			glPopMatrix();
		}
		if (all || (use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == size - 1 - m1)
				|| (use && cd.corner == 1 && cd.depth == m2)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1 - m1)
				|| (!use && cd.corner == 1 && cd.depth != m2)) {
			glPushMatrix();
			glTranslate(s3 * FACET_LENGTH, s1 * FACET_LENGTH, s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, e, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
			glPushMatrix();
			glTranslate(s3 * FACET_LENGTH, -s1 * FACET_LENGTH, s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, f, -1, -1);
			glPopMatrix();
		}
		if (all || (use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == size - 1 - m1)
				|| (use && cd.corner == 0 && cd.depth == 2 * size - 2 - m2)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1 - m1)
				|| (!use && cd.corner == 0 && cd.depth != 2 * size - 2 - m2)) {
			glPushMatrix();
			glTranslate(-s2 * FACET_LENGTH, s1 * FACET_LENGTH, s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, e, -1, -1, -1, -1, -1);
			glPopMatrix();
			glPushMatrix();
			glTranslate(-s2 * FACET_LENGTH, -s1 * FACET_LENGTH, s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, f, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == size - 1 + m1)
				|| (use && cd.corner == 1 && cd.depth == 2 * size - 2 - m2)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1 + m1)
				|| (!use && cd.corner == 1 && cd.depth != 2 * size - 2 - m2)) {
			glPushMatrix();
			glTranslate(-s3 * FACET_LENGTH, s1 * FACET_LENGTH, -s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, e, -1, -1, -1, -1);
			glPopMatrix();
			glPushMatrix();
			glTranslate(-s3 * FACET_LENGTH, -s1 * FACET_LENGTH, -s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, -1, f);
			glPopMatrix();
		}
		if (all || (use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == size - 1 + m1)
				|| (use && cd.corner == 1 && cd.depth == m2)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1 + m1)
				|| (!use && cd.corner == 1 && cd.depth != m2)) {
			glPushMatrix();
			glTranslate(-s3 * FACET_LENGTH, -s1 * FACET_LENGTH, s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, e, -1, -1, -1);
			glPopMatrix();
			glPushMatrix();
			glTranslate(-s3 * FACET_LENGTH, s1 * FACET_LENGTH, s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, f, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == size - 1 - m1)
				|| (use && cd.corner == 0 && cd.depth == m2)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1 - m1)
				|| (!use && cd.corner == 0 && cd.depth != m2)) {
			glPushMatrix();
			glTranslate(s2 * FACET_LENGTH, -s1 * FACET_LENGTH, s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, e, -1, -1);
			glPopMatrix();
			glPushMatrix();
			glTranslate(s2 * FACET_LENGTH, s1 * FACET_LENGTH, s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, f, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == size - 1 - m1)
				|| (use && cd.corner == 1 && cd.depth == 2 * size - 2 - m2)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1 - m1)
				|| (!use && cd.corner == 1 && cd.depth != 2 * size - 2 - m2)) {
			glPushMatrix();
			glTranslate(s3 * FACET_LENGTH, -s1 * FACET_LENGTH, -s2 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, e, -1);
			glPopMatrix();
			glPushMatrix();
			glTranslate(s3 * FACET_LENGTH, s1 * FACET_LENGTH, -s2 * FACET_LENGTH);
			drawTetraFacet(mi, f, -1, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || (use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == size - 1 + m1)
				|| (use && cd.corner == 0 && cd.depth == 2 * size - 2 - m2)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1 + m1)
				|| (!use && cd.corner == 0 && cd.depth != 2 * size - 2 - m2)) {
			glPushMatrix();
			glTranslate(-s2 * FACET_LENGTH, -s1 * FACET_LENGTH, -s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, -1, -1, -1, -1, e);
			glPopMatrix();
			glPushMatrix();
			glTranslate(-s2 * FACET_LENGTH, s1 * FACET_LENGTH, -s3 * FACET_LENGTH);
			drawTetraFacet(mi, -1, -1, -1, f, -1, -1, -1, -1);
			glPopMatrix();
		}
	}
	if (!sticky)
	    for (edge = 1; edge < size / 2; edge++) {
		/*if (size > 3) OUTSIDE EDGE 1 */
		int a, b, c, d, e, f;
		s0 = 2 * size - 2 * edge - 2;
		s1 = 2 * edge; /* 2, 4 */
		a = edge * edge; /* 1, 4 */
		b = (edge + 1) * (edge + 1) - 1; /* 3, 8 */
		c = (size - edge - 1) * (size - edge - 1); /* 4, 9, 16 */
		d = (size - edge) * (size - edge) - 1; /* 8, 15 */
		e = (size - 1) * (size - 1) + 2 * edge; /* 11, */
		f = size * size - 2 * edge - 1; /* 13, */
		if (all || ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == size - 1 - edge)
				|| (use && cd.corner == 2 && cd.depth == edge)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1 - edge)
				|| (!use && cd.corner == 2 && cd.depth != edge))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, s0 * FACET_LENGTH, 0);
			drawOctaFacet(mi, a, b, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == size - 1 - edge)
				|| (use && cd.corner == 2 && cd.depth == edge)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1 - edge)
				|| (!use && cd.corner == 2 && cd.depth != edge))) {
			glPushMatrix();
			glTranslate(0, s0 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, a, b, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == size - 1 + edge)
				|| (use && cd.corner == 2 && cd.depth == edge)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1 + edge)
				|| (!use && cd.corner == 2 && cd.depth != edge))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, s0 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, a, b, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == size - 1 + edge)
				|| (use && cd.corner == 2 && cd.depth == edge)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1 + edge)
				|| (!use && cd.corner == 2 && cd.depth != edge))) {
			glPushMatrix();
			glTranslate(0, s0 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawOctaFacet(mi, b, -1, -1, a, -1, -1, -1, -1);
			glPopMatrix();
		}

		if (all || ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == size - 1 - edge)
				|| (use && cd.corner == 2 && cd.depth == 2 * size - 2 - edge)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1 - edge)
				|| (!use && cd.corner == 2 && cd.depth != 2 * size - 2 - edge))) {
			glPushMatrix();
			glTranslate(0, -s0 * FACET_LENGTH, s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, -1, a, b, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == size - 1 - edge)
				|| (use && cd.corner == 2 && cd.depth == 2 * size - 2 - edge)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1 - edge)
				|| (!use && cd.corner == 2 && cd.depth != 2 * size - 2 - edge))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, -s0 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, -1, -1, -1, a, b, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == size - 1 + edge)
				|| (use && cd.corner == 2 && cd.depth == 2 * size - 2 - edge)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1 + edge)
				|| (!use && cd.corner == 2 && cd.depth != 2 * size - 2 - edge))) {
			glPushMatrix();
			glTranslate(0, -s0 * FACET_LENGTH, -s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, -1, -1, -1, a, b);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == size - 1 + edge)
				|| (use && cd.corner == 2 && cd.depth == 2 * size - 2 - edge)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1 + edge)
				|| (!use && cd.corner == 2 && cd.depth != 2 * size - 2 - edge))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, -s0 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, -1, -1, b, -1, -1, a);
			glPopMatrix();
		}

		if (all || ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == edge)
				|| (use && cd.corner == 2 && cd.depth == size - 1 - edge)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != edge)
				|| (!use && cd.corner == 2 && cd.depth != size - 1 - edge))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, s1 * FACET_LENGTH, 0);
			drawOctaFacet(mi, c, d, -1, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == edge)
				|| (use && cd.corner == 2 && cd.depth == size - 1 - edge)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != edge)
				|| (!use && cd.corner == 2 && cd.depth != size - 1 - edge))) {
			glPushMatrix();
			glTranslate(0, s1 * FACET_LENGTH, s0 * FACET_LENGTH);
			drawOctaFacet(mi, -1, c, d, -1, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == 2 * size - 2 - edge)
				|| (use && cd.corner == 2 && cd.depth == size - 1 - edge)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != 2 * size - 2 - edge)
				|| (!use && cd.corner == 2 && cd.depth != size - 1 - edge))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, s1 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, c, d, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == 2 * size - 2 - edge)
				|| (use && cd.corner == 2 && cd.depth == size - 1 - edge)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != 2 * size - 2 - edge)
				|| (!use && cd.corner == 2 && cd.depth != size - 1 - edge))) {
			glPushMatrix();
			glTranslate(0, s1 * FACET_LENGTH, -s0 * FACET_LENGTH);
			drawOctaFacet(mi, d, -1, -1, c, -1, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == edge)
				|| (use && cd.corner == 2 && cd.depth == size - 1 + edge)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != edge)
				|| (!use && cd.corner == 2 && cd.depth != size - 1 + edge))) {
			glPushMatrix();
			glTranslate(0, -s1 * FACET_LENGTH, s0 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, -1, c, d, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == edge)
				|| (use && cd.corner == 2 && cd.depth == size - 1 + edge)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != edge)
				|| (!use && cd.corner == 2 && cd.depth != size - 1 + edge))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, -s1 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, -1, -1, -1, c, d, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 0 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == 2 * size - 2 - edge)
				|| (use && cd.corner == 2 && cd.depth == size - 1 + edge)
				|| (!use && cd.corner == 0 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != 2 * size - 2 - edge)
				|| (!use && cd.corner == 2 && cd.depth != size - 1 + edge))) {
			glPushMatrix();
			glTranslate(0, -s1 * FACET_LENGTH, -s0 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, -1, -1, -1, c, d);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 1 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == 2 * size - 2 - edge)
				|| (use && cd.corner == 2 && cd.depth == size - 1 + edge)
				|| (!use && cd.corner == 1 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != 2 * size - 2 - edge)
				|| (!use && cd.corner == 2 && cd.depth != size - 1 + edge))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, -s1 * FACET_LENGTH, 0);
			drawOctaFacet(mi, -1, -1, -1, -1, d, -1, -1, c);
			glPopMatrix();
		}

		if (all || ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == size - 1 + edge)
				|| (use && cd.corner == 0 && cd.depth == edge)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1 + edge)
				|| (!use && cd.corner == 0 && cd.depth != edge))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, 0, -s1 * FACET_LENGTH);
			drawOctaFacet(mi, e, -1, -1, -1, -1, -1, f, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == size - 1 - edge)
				|| (use && cd.corner == 1 && cd.depth == edge)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1 - edge)
				|| (!use && cd.corner == 1 && cd.depth != edge))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, 0, s0 * FACET_LENGTH);
			drawOctaFacet(mi, -1, e, -1, -1, -1, f, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == size - 1 - edge)
				|| (use && cd.corner == 0 && cd.depth == 2 * size - 2 - edge)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1 - edge)
				|| (!use && cd.corner == 0 && cd.depth != 2 * size - 2 - edge))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, 0, s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, e, -1, f, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == size - 1 + edge)
				|| (use && cd.corner == 1 && cd.depth == 2 * size - 2 - edge)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1 + edge)
				|| (!use && cd.corner == 1 && cd.depth != 2 * size - 2 - edge))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, 0, -s0 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, e, -1, -1, -1, f);
			glPopMatrix();
		}

		if (all || ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == size - 1 + edge)
				|| (use && cd.corner == 1 && cd.depth == edge)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1 + edge)
				|| (!use && cd.corner == 1 && cd.depth != edge))) {
			glPushMatrix();
			glTranslate(-s1 * FACET_LENGTH, 0, s0 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, f, -1, e, -1, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == size - 1 - edge)
				|| (use && cd.corner == 0 && cd.depth == edge)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1 - edge)
				|| (!use && cd.corner == 0 && cd.depth != edge))) {
			glPushMatrix();
			glTranslate(s0 * FACET_LENGTH, 0, s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, f, -1, -1, -1, e, -1, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 0 && cd.depth == size - 1 - edge)
				|| (use && cd.corner == 1 && cd.depth == 2 * size - 2 - edge)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 0 && cd.depth != size - 1 - edge)
				|| (!use && cd.corner == 1 && cd.depth != 2 * size - 2 - edge))) {
			glPushMatrix();
			glTranslate(s1 * FACET_LENGTH, 0, -s0 * FACET_LENGTH);
			drawOctaFacet(mi, f, -1, -1, -1, -1, -1, e, -1);
			glPopMatrix();
		}
		if (all || ((use && cd.corner == 2 && cd.depth == size - 1)
				|| (use && cd.corner == 1 && cd.depth == size - 1 + edge)
				|| (use && cd.corner == 0 && cd.depth == 2 * size - 2 - edge)
				|| (!use && cd.corner == 2 && cd.depth != size - 1)
				|| (!use && cd.corner == 1 && cd.depth != size - 1 + edge)
				|| (!use && cd.corner == 0 && cd.depth != 2 * size - 2 - edge))) {
			glPushMatrix();
			glTranslate(-s0 * FACET_LENGTH, 0, -s1 * FACET_LENGTH);
			drawOctaFacet(mi, -1, -1, -1, f, -1, -1, -1, e);
			glPopMatrix();
		}
	}
	if (!sticky) {
	  pos = 1;
	  for (row = 0; row < size - 3; row++) {
	    pos += 4;
	    for (col = 0; col < 2 * row + 1; col++) {
		pos++;
		tetra = ((col % 2) == 1);
		sb = 2 * size - 2 * row - ((tetra) ? 0 : 1) - 5;
		sl = col + 2;
		sr = 2 * row - col + 2;
		mb = row + 2;
		ml = size - 2 - col / 2;
		mr = size - row - 2 + (col + 1) / 2;
		if (all || (use && cd.corner == 0 && cd.depth == mr)
				|| (use && cd.corner == 1 && cd.depth == 2 * size - 2 - ml)
				|| (use && cd.corner == 2 && cd.depth == mb)
				|| (!use && cd.corner == 0 && cd.depth != mr)
				|| (!use && cd.corner == 1 && cd.depth != 2 * size - 2 - ml)
				|| (!use && cd.corner == 2 && cd.depth != mb)) {
			glPushMatrix(); /* 0 */
			glTranslate(sr * FACET_LENGTH, sb * FACET_LENGTH, -sl * FACET_LENGTH);
			drawFacet(mi, pos, -1, -1, -1, -1, -1, -1, -1, tetra);
			glPopMatrix();
		}
		if (all || (use && cd.corner == 0 && cd.depth == ml)
				|| (use && cd.corner == 1 && cd.depth == mr)
				|| (use && cd.corner == 2 && cd.depth == mb)
				|| (!use && cd.corner == 0 && cd.depth != ml)
				|| (!use && cd.corner == 1 && cd.depth != mr)
				|| (!use && cd.corner == 2 && cd.depth != mb)) {
			glPushMatrix(); /* 1 */
			glTranslate(sl * FACET_LENGTH, sb * FACET_LENGTH, sr * FACET_LENGTH);
			drawFacet(mi, -1, pos, -1, -1, -1, -1, -1, -1, tetra);
			glPopMatrix();
		}
		if (all || (use && cd.corner == 0 && cd.depth == 2 * size - 2 - mr)
				|| (use && cd.corner == 1 && cd.depth == ml)
				|| (use && cd.corner == 2 && cd.depth == mb)
				|| (!use && cd.corner == 0 && cd.depth != 2 * size - 2 - mr)
				|| (!use && cd.corner == 1 && cd.depth != ml)
				|| (!use && cd.corner == 2 && cd.depth != mb)) {
			glPushMatrix(); /* 2 */
			glTranslate(-sr * FACET_LENGTH, sb * FACET_LENGTH, sl * FACET_LENGTH);
			drawFacet(mi, -1, -1, pos, -1, -1, -1, -1, -1, tetra);
			glPopMatrix();
		}
		if (all || (use && cd.corner == 0 && cd.depth == 2 * size - 2 - ml)
				|| (use && cd.corner == 1 && cd.depth == 2 * size - 2 - mr)
				|| (use && cd.corner == 2 && cd.depth == mb)
				|| (!use && cd.corner == 0 && cd.depth != 2 * size - 2 - ml)
				|| (!use && cd.corner == 1 && cd.depth != 2 * size - 2 - mr)
				|| (!use && cd.corner == 2 && cd.depth != mb)) {
			glPushMatrix(); /* 3 */
			glTranslate(-sl * FACET_LENGTH, sb * FACET_LENGTH, -sr * FACET_LENGTH);
			drawFacet(mi, -1, -1, -1, pos, -1, -1, -1, -1, tetra);
			glPopMatrix();
		}
		if (all || (use && cd.corner == 0 && cd.depth == 2 * size - 2 - ml)
				|| (use && cd.corner == 1 && cd.depth == mr)
				|| (use && cd.corner == 2 && cd.depth == 2 * size - 2 - mb)
				|| (!use && cd.corner == 0 && cd.depth != 2 * size - 2 - ml)
				|| (!use && cd.corner == 1 && cd.depth != mr)
				|| (!use && cd.corner == 2 && cd.depth != 2 * size - 2 - mb)) {
			glPushMatrix(); /* 4 */
			glTranslate(-sl * FACET_LENGTH, -sb * FACET_LENGTH, sr * FACET_LENGTH);
			drawFacet(mi, -1, -1, -1, -1, pos, -1, -1, -1, tetra);
			glPopMatrix();
		}
		if (all || (use && cd.corner == 0 && cd.depth == mr)
				|| (use && cd.corner == 1 && cd.depth == ml)
				|| (use && cd.corner == 2 && cd.depth == 2 * size - 2 - mb)
				|| (!use && cd.corner == 0 && cd.depth != mr)
				|| (!use && cd.corner == 1 && cd.depth != ml)
				|| (!use && cd.corner == 2 && cd.depth != 2 * size - 2 - mb)) {
			glPushMatrix(); /* 5 */
			glTranslate(sr * FACET_LENGTH, -sb * FACET_LENGTH, sl * FACET_LENGTH);
			drawFacet(mi, -1, -1, -1, -1, -1, pos, -1, -1, tetra);
			glPopMatrix();
		}
		if (all || (use && cd.corner == 0 && cd.depth == ml)
				|| (use && cd.corner == 1 && cd.depth == 2 * size - 2 - mr)
				|| (use && cd.corner == 2 && cd.depth == 2 * size - 2 - mb)
				|| (!use && cd.corner == 0 && cd.depth != ml)
				|| (!use && cd.corner == 1 && cd.depth != 2 * size - 2 - mr)
				|| (!use && cd.corner == 2 && cd.depth != 2 * size - 2 - mb)) {
			glPushMatrix(); /* 6 */
			glTranslate(sl * FACET_LENGTH, -sb * FACET_LENGTH, -sr * FACET_LENGTH);
			drawFacet(mi, -1, -1, -1, -1, -1, -1, pos, -1, tetra);
			glPopMatrix(); 
		}
		if (all || (use && cd.corner == 0 && cd.depth == 2 * size - 2 - mr)
				|| (use && cd.corner == 1 && cd.depth == 2 * size - 2 - ml)
				|| (use && cd.corner == 2 && cd.depth == 2 * size - 2 - mb)
				|| (!use && cd.corner == 0 && cd.depth != 2 * size - 2 - mr)
				|| (!use && cd.corner == 1 && cd.depth != 2 * size - 2 - ml)
				|| (!use && cd.corner == 2 && cd.depth != 2 * size - 2 - mb)) {
			glPushMatrix(); /* 7 */
			glTranslate(-sr * FACET_LENGTH, -sb * FACET_LENGTH, -sl * FACET_LENGTH);
			drawFacet(mi, -1, -1, -1, -1, -1, -1, -1, pos, tetra);
			glPopMatrix();
		}
	    }
	  }
	}
}
/*-
 * This rotates whole octahedron.
 */
static void
rotateAllFaces(ModeInfo * mi)
{
	drawFaces(mi, 0, 0, 0, False, True);
}

static void
rotateFaceSlice(ModeInfo * mi, GLfloat rotateStep,
		GLfloat x, GLfloat y, GLfloat z,
		int face, int position, int direction)
{
	octstruct *op = &oct[MI_SCREEN(mi)];
	glPushMatrix();
	glRotatef(rotateStep, x, y, z);
	if (op->movement.control)
		drawFaces(mi, face, 0, direction, False, True);
	else
		drawFaces(mi, face, position, direction, True, False);
	glPopMatrix();
	if (!op->movement.control)
		drawFaces(mi, face, position, direction, False, False);
}

/*-
 * This rotates whole octahedron.
 */
static void
rotateAllCorners(ModeInfo * mi)
{
	drawCorners(mi, 0, 0, 0, False, True);
}

static void
rotateCornerSlice(ModeInfo * mi, GLfloat rotateStep,
		GLfloat x, GLfloat y, GLfloat z,
		int face, int position, int direction)
{
	octstruct *op = &oct[MI_SCREEN(mi)];
	glPushMatrix();
	glRotatef(rotateStep, x, y, z);
	if (op->movement.control)
		drawCorners(mi, face, 0, direction, False, True);
	else
		drawCorners(mi, face, position, direction, True, False);
	glPopMatrix();
	if (!op->movement.control)
		drawCorners(mi, face, position, direction, False, False);
}

static int
getSliceDirection(OctMove move)
{
	if (move.face == NO_FACE || move.direction == IGNORE_DIR) {
		return -1;
	} else if (move.style == PERIOD3) {
		switch (move.face) {
		case 0:
		case 6:
			switch (move.direction) {
			case TR:
			case RIGHT:
			case BR:
				return CW;
			case BL:
			case LEFT:
			case TL:
				return CCW;
			default:
				return -1;
			}
			break;
		case 1:
		case 7:
			switch (move.direction) {
			case TL:
			case TOP:
			case TR:
				return CCW;
			case BR:
			case BOTTOM:
			case BL:
				return CW;
			default:
				return -1;
			}
			break;
		case 2:
		case 4:
			switch (move.direction) {
			case TR:
			case RIGHT:
			case BR:
				return CCW;
			case BL:
			case LEFT:
			case TL:
				return CW;
			default:
				return -1;
			}
			break;
		case 3:
		case 5:
			switch (move.direction) {
			case TL:
			case TOP:
			case TR:
				return CW;
			case BR:
			case BOTTOM:
			case BL:
				return CCW;
			default:
				return -1;
			}
			break;
		default:
			return -1;
		}
	} else /* (style == PERIOD4) */ {
		if (move.face < 4) {
			switch (move.direction) {
			case TL:
			case TR:
			case CW:
				return CW;
			case BR:
			case BL:
			case CCW:
				return CCW;
			default:
				return -1;
			}
		} else {
			switch (move.direction) {
			case TL:
			case TR:
			case CCW:
				return CW;
			case BR:
			case BL:
			case CW:
				return CCW;
			default:
				return - 1;
			}
		}
	}
}

/* Convert move to weird general notation */
static void
convertMove(octstruct * op, OctMove move, OctSlice * slice)
{
	if (move.style == PERIOD3) {
		FaceDepth fd;
		faceDepth(op, move.face, move.position, move.direction, &fd);
		slice->fc = fd.face;
		slice->depth = fd.depth;
		slice->rotation = getSliceDirection(move);
	} else if (move.style == PERIOD4) {
		CornerDepth cd;
		cornerDepth(op, move.face, move.position, move.direction, &cd);
		slice->fc = cd.corner;
		slice->depth = cd.depth;
		slice->rotation = getSliceDirection(move);
	} else {
		slice->fc = -1;
		slice->depth = -1;
	}
}

static void
draw_octa(ModeInfo * mi)
{
	octstruct *op = &oct[MI_SCREEN(mi)];
	int face = op->movement.face;
	int position = op->movement.position;
	int direction = op->movement.direction;
	int style = op->movement.style;
	int control = 0; /* op->movement.control; */

	op->movement.control = control;
	if (face == NO_FACE || direction == IGNORE_DIR) {
		if (style == PERIOD3)
			rotateAllFaces(mi);
		else
			rotateAllCorners(mi);
/* 8 face/corner rotate */
/*	  1-1-1  */
/*	111      */
/* 1 -1 -1 rotate front faces CCW <=> 0 LEFT */
/* -1 1 -1 rotate right face CCW <=> 0 BL */
/* -1 -1 1 rotate top face CCW <=> 1 TR */
/* 1 1 1 rotate left face CCW <=> 0 TOP */
/* -1 1 1 rotate front faces CW <=> 0 RIGHT */
/* 1 -1 1 rotate right face CW <=> 0 TR */
/* 1 1 -1 rotate top face CW <=> 1 BL */
/* -1 -1 -1 rotate left face CW <=> BOTTOM */
	} else if (style == PERIOD3) {
		switch (face) {
		case 0:
			switch (direction) {
			case TR:
				rotateFaceSlice(mi, op->rotateStep, -1, -1, -1, face, position, direction);
				break;
			case RIGHT:
				rotateFaceSlice(mi, op->rotateStep, 1, -1, -1, face, position, direction);
				break;
			case BR:
				rotateFaceSlice(mi, op->rotateStep, 1, -1, 1, face, position, direction);
				break;
			case BL:
				rotateFaceSlice(mi, op->rotateStep, 1, 1, 1, face, position, direction);
				break;
			case LEFT:
				rotateFaceSlice(mi, op->rotateStep, -1, 1, 1, face, position, direction);
				break;
			case TL:
				rotateFaceSlice(mi, op->rotateStep, -1, 1, -1, face, position, direction);
				break;
			default:
				rotateAllFaces(mi);
			}
			break;
		case 1:
			switch (direction) {
			case TOP:
				rotateFaceSlice(mi, op->rotateStep, -1, 1, -1, face, position, direction);
				break;
			case TR:
				rotateFaceSlice(mi, op->rotateStep, 1, 1, -1, face, position, direction);
				break;
			case BR:
				rotateFaceSlice(mi, op->rotateStep, 1, -1, -1, face, position, direction);
				break;
			case BOTTOM:
				rotateFaceSlice(mi, op->rotateStep, 1, -1, 1, face, position, direction);
				break;
			case BL:
				rotateFaceSlice(mi, op->rotateStep, -1, -1, 1, face, position, direction);
				break;
			case TL:
				rotateFaceSlice(mi, op->rotateStep, -1, 1, 1, face, position, direction);
				break;
			default:
				rotateAllFaces(mi);
			}
			break;
		case 2:
			switch (direction) {
			case TR:
				rotateFaceSlice(mi, op->rotateStep, -1, 1, -1, face, position, direction);
				break;
			case RIGHT:
				rotateFaceSlice(mi, op->rotateStep, 1, 1, -1, face, position, direction);
				break;
			case BR:
				rotateFaceSlice(mi, op->rotateStep, 1, 1, 1, face, position, direction);
				break;
			case BL:
				rotateFaceSlice(mi, op->rotateStep, 1, -1, 1, face, position, direction);
				break;
			case LEFT:
				rotateFaceSlice(mi, op->rotateStep, -1, -1, 1, face, position, direction);
				break;
			case TL:
				rotateFaceSlice(mi, op->rotateStep, -1, -1, -1, face, position, direction);
				break;
			default:
				rotateAllFaces(mi);
			}
			break;
		case 3:
			switch (direction) {
			case TOP:
				rotateFaceSlice(mi, op->rotateStep, -1, -1, -1, face, position, direction);
				break;
			case TR:
				rotateFaceSlice(mi, op->rotateStep, 1, -1, -1, face, position, direction);
				break;
			case BR:
				rotateFaceSlice(mi, op->rotateStep, 1, 1, -1, face, position, direction);
				break;
			case BOTTOM:
				rotateFaceSlice(mi, op->rotateStep, 1, 1, 1, face, position, direction);
				break;
			case BL:
				rotateFaceSlice(mi, op->rotateStep, -1, 1, 1, face, position, direction);
				break;
			case TL:
				rotateFaceSlice(mi, op->rotateStep, -1, -1, 1, face, position, direction);
				break;
			default:
				rotateAllFaces(mi);
			}
			break;
		case 4:
			switch (direction) {
			case TR:
				rotateFaceSlice(mi, op->rotateStep, -1, 1, -1, face, position, direction);
				break;
			case RIGHT:
				rotateFaceSlice(mi, op->rotateStep, -1, 1, 1, face, position, direction);
				break;
			case BR:
				rotateFaceSlice(mi, op->rotateStep, 1, 1, 1, face, position, direction);
				break;
			case BL:
				rotateFaceSlice(mi, op->rotateStep, 1, -1, 1, face, position, direction);
				break;
			case LEFT:
				rotateFaceSlice(mi, op->rotateStep, 1, -1, -1, face, position, direction);
				break;
			case TL:
				rotateFaceSlice(mi, op->rotateStep, -1, -1, -1, face, position, direction);
				break;
			default:
				rotateAllFaces(mi);
			}
			break;
		case 5:
			switch (direction) {
			case TOP:
				rotateFaceSlice(mi, op->rotateStep, -1, -1, -1, face, position, direction);
				break;
			case TR:
				rotateFaceSlice(mi, op->rotateStep, -1, -1, 1, face, position, direction);
				break;
			case BR:
				rotateFaceSlice(mi, op->rotateStep, -1, 1, 1, face, position, direction);
				break;
			case BOTTOM:
				rotateFaceSlice(mi, op->rotateStep, 1, 1, 1, face, position, direction);
				break;
			case BL:
				rotateFaceSlice(mi, op->rotateStep, 1, 1, -1, face, position, direction);
				break;
			case TL:
				rotateFaceSlice(mi, op->rotateStep, 1, -1, -1, face, position, direction);
				break;
			default:
				rotateAllFaces(mi);
			}
			break;
		case 6:
			switch (direction) {
			case TR:
				rotateFaceSlice(mi, op->rotateStep, -1, -1, -1, face, position, direction);
				break;
			case RIGHT:
				rotateFaceSlice(mi, op->rotateStep, -1, -1, 1, face, position, direction);
				break;
			case BR:
				rotateFaceSlice(mi, op->rotateStep, 1, -1, 1, face, position, direction);
				break;
			case BL:
				rotateFaceSlice(mi, op->rotateStep, 1, 1, 1, face, position, direction);
				break;
			case LEFT:
				rotateFaceSlice(mi, op->rotateStep, 1, 1, -1, face, position, direction);
				break;
			case TL:
				rotateFaceSlice(mi, op->rotateStep, -1, 1, -1, face, position, direction);
				break;
			default:
				rotateAllFaces(mi);
			}
			break;
		case 7:
			switch (direction) {
			case TOP:
				rotateFaceSlice(mi, op->rotateStep, -1, 1, -1, face, position, direction);
				break;
			case TR:
				rotateFaceSlice(mi, op->rotateStep, -1, 1, 1, face, position, direction);
				break;
			case BR:
				rotateFaceSlice(mi, op->rotateStep, -1, -1, 1, face, position, direction);
				break;
			case BOTTOM:
				rotateFaceSlice(mi, op->rotateStep, 1, -1, 1, face, position, direction);
				break;
			case BL:
				rotateFaceSlice(mi, op->rotateStep, 1, -1, -1, face, position, direction);
				break;
			case TL:
				rotateFaceSlice(mi, op->rotateStep, 1, 1, -1, face, position, direction);
				break;
			default:
				rotateAllFaces(mi);
			}
			break;
		default:
			rotateAllFaces(mi);
		}
/* 6 edge */
/* 0 1 0 rotate front edge CCW <=> 0 BL */
/* 0 -1 0 rotate front edge CW <=> 0 TR */
/* 1 0 0 rotate left edge CW <=> 0 BOTTOM */
/* -1 0 0 rotate left edge CCW <=> 0 TOP */
/* 0 0 1 rotate right edge CCW <=> 0 LEFT */
/* 0 0 -1 rotate right edge CW <=> 0 RIGHT */
	} else /* (style == PERIOD4) */ {
		/*if ((face == 1 || face == 2) && (dir == TR || dir == BL))
			dir = (dir + MAX_ORIENT / 2) % MAX_ORIENT;*/
		if (face < 4) {
			switch (direction) {
			case TR:
				rotateCornerSlice(mi, op->rotateStep, 0, 0, -1, face, position, direction);
				break;
			case BR:
				rotateCornerSlice(mi, op->rotateStep, 1, 0, 0, face, position, direction);
				break;
			case BL:
				rotateCornerSlice(mi, op->rotateStep, 0, 0, 1, face, position, direction);
				break;
			case TL:
				rotateCornerSlice(mi, op->rotateStep, -1, 0, 0, face, position, direction);
				break;
			case CW:
				rotateCornerSlice(mi, op->rotateStep, 0, -1, 0, face, position, direction);
				break;
			case CCW:
				rotateCornerSlice(mi, op->rotateStep, 0, 1, 0, face, position, direction);
				break;
			default:
				rotateAllCorners(mi);
			}
		} else {
			switch (direction) {
			case TR:
				rotateCornerSlice(mi, op->rotateStep, -1, 0, 0, face, position, direction);
				break;
			case BR:
				rotateCornerSlice(mi, op->rotateStep, 0, 0, 1, face, position, direction);
				break;
			case BL:
				rotateCornerSlice(mi, op->rotateStep, 1, 0, 0, face, position, direction);
				break;
			case TL:
				rotateCornerSlice(mi, op->rotateStep, 0, 0, -1, face, position, direction);
				break;
			case CW:
				rotateCornerSlice(mi, op->rotateStep, 0, 1, 0, face, position, direction);
				break;
			case CCW:
				rotateCornerSlice(mi, op->rotateStep, 0, -1, 0, face, position, direction);
				break;
			default:
				rotateAllCorners(mi);
			}
		}
	}
}

/* From David Bagley's xoct.  Used by permission. ;)  */
typedef struct _RowNextP3 {
	int viewChanged, face, direction;
} RowNextP3;

static OctLoc rowToRotate[MAX_FACES][COORD] =
{
	{
		{IGNORE_DIR, IGNORE_DIR},
		{1, CW},
		{2, CW},
		{3, CW},
		{IGNORE_DIR, IGNORE_DIR},
		{1, CCW},
		{2, CCW},
		{3, CCW}
	},
	{
		{3, CCW},
		{0, CCW},
		{IGNORE_DIR, IGNORE_DIR},
		{2, CW},
		{3, CW},
		{0, CW},
		{IGNORE_DIR, IGNORE_DIR},
		{2, CCW}
	},
	{
		{IGNORE_DIR, IGNORE_DIR},
		{3, CCW},
		{0, CCW},
		{1, CCW},
		{IGNORE_DIR, IGNORE_DIR},
		{3, CW},
		{0, CW},
		{1, CW}
	},
	{
		{1, CW},
		{2, CW},
		{IGNORE_DIR, IGNORE_DIR},
		{0, CCW},
		{1, CCW},
		{2, CCW},
		{IGNORE_DIR, IGNORE_DIR},
		{0, CW}
	},
	{
		{IGNORE_DIR, IGNORE_DIR},
		{3, CCW},
		{2, CCW},
		{1, CCW},
		{IGNORE_DIR, IGNORE_DIR},
		{3, CW},
		{2, CW},
		{1, CW}
	},
	{
		{1, CW},
		{0, CW},
		{IGNORE_DIR, IGNORE_DIR},
		{2, CCW},
		{1, CCW},
		{0, CCW},
		{IGNORE_DIR, IGNORE_DIR},
		{2, CW}
	},
	{
		{IGNORE_DIR, IGNORE_DIR},
		{1, CW},
		{0, CW},
		{3, CW},
		{IGNORE_DIR, IGNORE_DIR},
		{1, CCW},
		{0, CCW},
		{3, CCW}
	},
	{
		{3, CCW},
		{2, CCW},
		{IGNORE_DIR, IGNORE_DIR},
		{0, CW},
		{3, CW},
		{2, CW},
		{IGNORE_DIR, IGNORE_DIR},
		{0, CCW}
	}
};

static RowNextP3 slideNextRowP3[MAX_SIDES][COORD] =
{
	{
		{IGNORE_DIR, IGNORE_DIR, IGNORE_DIR},
		{True, 2, TR},
		{False, 1, BR},
		{False, 1, BOTTOM},
		{IGNORE_DIR, IGNORE_DIR, IGNORE_DIR},
		{False, 3, BOTTOM},
		{False, 3, BL},
		{True, 2, TL}
	},
	{
		{False, 0, TL},
		{True, 1, BL},
		{IGNORE_DIR, IGNORE_DIR, IGNORE_DIR},
		{True, 1, TL},
		{False, 2, BL},
		{False, 2, LEFT},
		{IGNORE_DIR, IGNORE_DIR, IGNORE_DIR},
		{False, 0, LEFT}
	},
	{
		{IGNORE_DIR, IGNORE_DIR, IGNORE_DIR},
		{False, 1, TOP},
		{False, 1, TR},
		{True, 0, BR},
		{IGNORE_DIR, IGNORE_DIR, IGNORE_DIR},
		{True, 0, BL},
		{False, 3, TL},
		{False, 3, TOP},
	},
	{
		{False, 0, TR},
		{False, 0, RIGHT},
		{IGNORE_DIR, IGNORE_DIR, IGNORE_DIR},
		{False, 2, RIGHT},
		{False, 2, BR},
		{True, 3, TR},
		{IGNORE_DIR, IGNORE_DIR, IGNORE_DIR},
		{True, 3, BR}
	}
};
static int reverseP3[MAX_SIDES][COORD] =
{
	{IGNORE_DIR, False, False, True, IGNORE_DIR, True, True, True},
	{True, True, IGNORE_DIR, False, False, True, IGNORE_DIR, True},
	{IGNORE_DIR, True, True, True, IGNORE_DIR, False, False, True},
	{False, True, IGNORE_DIR, True, True, True, IGNORE_DIR, False}
};
static int rotateOrientP3[MAX_SIDES][COORD] =
{
	{IGNORE_DIR, 11, 7, 2, IGNORE_DIR, 10, 5, 1},
	{11, 7, IGNORE_DIR, 5, 1, 2, IGNORE_DIR, 10},
	{IGNORE_DIR, 10, 5, 1, IGNORE_DIR, 11, 7, 2},
	{1, 2, IGNORE_DIR, 10, 11, 7, IGNORE_DIR, 5}
};

static void
rotateFace(octstruct * op, int view, int side, int direction)
{
	int g, square;
	int i = 0, j = 0, k, i_1, j_1, k_1, position;

	/* Read Face */
	k = -1;
	square = 0;
	for (g = 0; g < op->size * op->size; g++) {
#if 0
		/* This is the old algorithm, its now more efficient */
		k = SQRT(g);
		j = (g - k * k) / 2;
		i = ((k + 1) * (k + 1) - g - 1) / 2;
#endif
		if (square <= g) {
			k++;
			square = (k + 1) * (k + 1);
			j = -1;
			i = k;
		}
		if (((square - g) & 1) == 0)
			i--;
		else
			j++;
		if (direction == CW) {
			k_1 = op->size - 1 - i;
			i_1 = j;
			j_1 = op->size - 1 - k;
		} else {	/* (direction == CCW) */
			k_1 = op->size - 1 - j;
			j_1 = i;
			i_1 = op->size - 1 - k;
		}
		position = k_1 * k_1 + 2 * j_1 + ((j_1 == k_1 - i_1) ? 0 : 1);
		op->faceLoc[position] = op->facetLoc[view * MAX_SIDES + side][g];
	}
	/* Write Face */
	square = 1;
	for (g = 0; g < op->size * op->size; g++) {
		op->facetLoc[view * MAX_SIDES + side][g] = op->faceLoc[g];
		op->facetLoc[view * MAX_SIDES + side][g].rotation =
			(direction == CW) ?
			(op->facetLoc[view * MAX_SIDES + side][g].rotation + 4) % MAX_ORIENT :
			(op->facetLoc[view * MAX_SIDES + side][g].rotation + 8) % MAX_ORIENT;
		/* drawTriangle(view * MAX_SIDES + side, g, FALSE); */
		if (g == square * square - 1) {
			++square;
		}
	}
}

static void
readRTT(octstruct * op, int view, int side, int dir, int h, int len, int orient)
{
	int f, g, s;
	int base = h * h;

	if (((dir & 1) == 0) || dir > COORD)
		for (g = 0; g < len; g++)
			op->rowLoc[orient][g] =
				op->facetLoc[view * MAX_SIDES + side][base + g];
	else if (((dir / 2 + side) & 1) == 1) {
		f = -1;
		for (g = 0; g < len; g++) {
			s = g & 1;
			op->rowLoc[orient][g] =
				op->facetLoc[view * MAX_SIDES + side][base + f + 1 - s];
			if (s == SAME)
				f += g + 2 * (h + 1) + 1;
		}
	} else {
		base += 2 * h;
		f = 1;
		for (g = 0; g < len; g++) {
			s = g & 1;
			op->rowLoc[orient][g] =
				op->facetLoc[view * MAX_SIDES + side][base + f - 1 + s];
			if (s == SAME)
				f += g + 2 * h + 1;
		}
	}
}

static void
rotateRTT(octstruct * op, int rotate, int len, int orient)
{
	int g;

	for (g = 0; g < len; g++)
		op->rowLoc[orient][g].rotation =
			(op->rowLoc[orient][g].rotation + rotate) %
			MAX_ORIENT;
}

static void
reverseRTT(octstruct * op, int len, int orient)
{
	int g;
	OctLoc temp;

	for (g = 0; g < (len - 1) / 2; g++) {
		temp = op->rowLoc[orient][len - 1 - g];
		op->rowLoc[orient][len - 1 - g] = op->rowLoc[orient][g];
		op->rowLoc[orient][g] = temp;
	}
}

static void
writeRTT(octstruct * op, int view, int side, int dir, int h, int len, int orient)
{
	int f, g, s;
	int base = h * h;

	if (((dir & 1) == 0) || dir > COORD) {  /* CW || CCW */
		for (g = 0; g < len; g++) {
			s = g & 1;
			op->facetLoc[view * MAX_SIDES + side][base + g] =
				op->rowLoc[orient][g];
			/*drawTriangle(mi, view * MAX_SIDES + side, base + g, FALSE);*/
		}
	} else if (((dir / 2 + side) & 1) == 1) {
		f = -1;
		for (g = 0; g < len; g++) {
			s = g & 1;
			op->facetLoc[view * MAX_SIDES + side][base + f + 1 - s] =
				op->rowLoc[orient][g];
			/*drawTriangle(mi, view * MAX_SIDES + side,
				base + f + 1 - s, FALSE);*/
			if (s == SAME)
				f += g + 2 * (h + 1) + 1;
		}
	} else {
		base += 2 * h;
		f = 1;
		for (g = 0; g < len; g++) {
			s = g & 1;
			op->facetLoc[view * MAX_SIDES + side][base + f - 1 + s] =
				op->rowLoc[orient][g];
			/*drawTriangle(mi, view * MAX_SIDES + side,
				base + f - 1 + s, FALSE);*/
			if (s == SAME)
				f += g + 2 * h + 1;
		}
	}
}


static void
movePieces(octstruct *op, int face, RTT rtt, int direction, int style,
		Bool control)
{
	Bool bound;
	int view, side, orient, newView, rotate, g, h, len;
	int newSide, oldSide, newDirection, l = 0;
	int dir = direction;

	view = face / MAX_SIDES;
	side = face % MAX_SIDES;
	if (style == PERIOD3) {
		if (dir == CW || dir == CCW) { /* Remap to row movement */
			side = (side + 2) % MAX_SIDES;
			dir = ((side + dir) % MAX_SIDES) * 2;
			/*face = side + MAX_SIDES * ((view == UP) ? DOWN : UP;*/
			rtt.row = rtt.trbl = rtt.tlbr = 0;
		}
		if (((rtt.row == 0) && ((dir & 1) == 0)) ||
				((rtt.trbl == 0) && ((dir & 1) == 1) &&
				(((side + dir / 2) & 1) == 0)) ||
				((rtt.tlbr == 0) && ((dir & 1) == 1) &&
				(((side + dir / 2) & 1) == 1)))
			rotateFace(op, view, rowToRotate[side][dir].face,
				rowToRotate[side][dir].rotation);
		if ((rtt.row == op->size - 1 && ((dir & 1) == 0)) ||
				(rtt.trbl == op->size - 1 &&
				((dir & 1) == 1) &&
				(((side + dir / 2) & 1) == 0)) ||
				(rtt.tlbr == op->size - 1 &&
				((dir & 1) == 1) &&
				(((side + dir / 2) & 1) == 1)))
			rotateFace(op, (view == UP) ? DOWN : UP,
				rowToRotate[MAX_SIDES + side][dir].face,
				rowToRotate[MAX_SIDES + side][dir].rotation);
		if ((dir & 1) == 0)
			h = rtt.row;
		else if (((dir / 2 + side) & 1) == 1)
			h = rtt.tlbr;
		else	    /* (((dir / 2 + side) & 1) == 0) */
			h = rtt.trbl;
		if (op->sticky && (h == 1 || h == 2)) {
			l = 0;
			bound = True;
			h = 1;
		} else
			bound = False;
		newView = view;
		newSide = side;
		newDirection = dir;
		do {
			len = length(op, dir, h);
			rotate = rotateOrientP3[side][dir];
			readRTT(op, view, side, dir, h, len, 0);
			if (reverseP3[side][dir] == True)
				reverseRTT(op, len, 0);
			rotateRTT(op, rotate, len, 0);
			for (orient = 1; orient < 8; orient++) {
				if (slideNextRowP3[side][dir].viewChanged) {
					view = (view == UP) ? DOWN : UP;
					h = op->size - 1 - h;
				}
				oldSide = slideNextRowP3[side][dir].face;
				dir = slideNextRowP3[side][dir].direction;
				side = oldSide;
				len = length(op, dir, h);
				rotate = rotateOrientP3[side][dir];
				if (orient < 6) {
					readRTT(op, view, side, dir, h, len, orient);
					if (reverseP3[side][dir] == True)
						reverseRTT(op, len, orient);
					rotateRTT(op, rotate, len, orient);
				}
				if (orient >= 2)
					writeRTT(op, view, side, dir, h, len, orient - 2);
			}
			l++;
			h = 2;
			view = newView;
			side = newSide;
			dir = newDirection;
		} while (bound && l < 2);
	} else {		/* style == PERIOD4 */
		if (dir > TL)
			h = rtt.row;
		else if (((dir / 2 + side) & 1) == 1)
			h = rtt.tlbr;
		else    /* (((dir / 2 + side) & 1) == 0) */
			h = rtt.trbl;

		if (op->sticky &&
				!((dir > TL && h == op->size - 1) ||
				(dir <= TL && h == 0))) {
			l = 0;
			h = (dir <= TL) ? 1 : 0;
			bound = True;
		} else
			bound = False;
		g = 0;
		do {	    /* In case this is on an edge */
			len = length(op, dir, h);
			if (g == 1) {
				if (dir > TL) {
					dir = (dir == CW) ? CCW : CW;
					view = (view == UP) ? DOWN : UP;
				} else
					side = (side + 2) % MAX_SIDES;
			}
			readRTT(op, view, side, dir, h, len, 0);
			for (orient = 1; orient <= 4; orient++) {
				if (dir <= TL) {
					if ((side - dir / 2 + COORD) %
							MAX_SIDES < 2) {
						newView = (view == UP) ? DOWN : UP;
						newSide = ((side & 1) == 1) ?
							side : (side + 2) % MAX_SIDES;
						newDirection = (((dir / 2) & 1) == 0) ?
							((dir + 6) % MAX_FACES) : ((dir + 2) % MAX_FACES);
						if ((side & 1) == 1)
							rotate = (((newDirection - dir) / 2 + MAX_SIDES) %
								MAX_SIDES == 1) ? 4 : MAX_ORIENT - 4;
						else
							rotate = (((newDirection - dir) / 2 + MAX_SIDES) %
								MAX_SIDES == 1) ? 2 : MAX_ORIENT - 2;
					} else {	/* Next is on same view */
						newView = view;
						newSide = MAX_SIDES - side - 1;
						if (((dir / 2) & 1) == 1)
							newSide = (newSide + 2) % MAX_SIDES;
						newDirection = dir;
						rotate = ((side - newSide + MAX_SIDES) % MAX_SIDES == 1) ?
							1 : MAX_ORIENT - 1;
					}
				} else {	/* dir == CW || dir == CCW */
					newView = view;
					newSide = (side + dir) % MAX_SIDES;
					newDirection = dir;
					rotate = 3 * newDirection;
				}
				if (orient != 4)
					readRTT(op, newView, newSide, newDirection, h, len, orient);
				rotateRTT(op, rotate, len, orient - 1);
				if (dir <= TL)
					reverseRTT(op, len, orient - 1);
				writeRTT(op, newView, newSide, newDirection, h, len, orient - 1);
				view = newView;
				side = newSide;
				dir = newDirection;
			}
			l++;
			if (op->sticky &&
					!((dir > TL && h == op->size - 1) ||
					(dir <= TL && h == 0)))
				h++;
			else
				g++;
		} while ((bound && l < op->size - 1) ||
				(((dir > TL && h == op->size - 1) ||
				(dir <= TL && h == 0)) && g < 2 && !bound));
	}
}

static void
moveControlCb(octstruct * op, int face, int direction, int style)
{
	int i, j, newFace, newDirection;
	RTT rtt;

	rtt.row = 0;
	rtt.trbl = 0;
	rtt.tlbr = 0;
	if (op->sticky && style == PERIOD4) {
		for (i = 0; i < 3; i++) {
			if (i == 2) {
				newFace = (1 - face / MAX_SIDES) *
					MAX_SIDES + (((face & 1) == 1) ?
					(face + MAX_SIDES / 2) % MAX_SIDES :
					face % MAX_SIDES);
				if (direction == CW || direction == CCW) {
					j = i - 2;
					rtt.row = j;
					rtt.trbl = j;
					rtt.tlbr = j;
					newDirection = (direction == CW) ? CCW : CW;
				} else {
					j = i + 1;
					rtt.row = j;
					rtt.trbl = j;
					rtt.tlbr = j;
					newDirection = ((direction / 2) & 1) ?
						(direction + 2) % MAX_FACES :
						(direction + 6) % MAX_FACES;
				}
			} else {
				if (i == 1)
					j = i + 2;
				else
					j = i;
				rtt.row = j;
				rtt.trbl = j;
				rtt.tlbr = j;
				newFace = face;
				newDirection = direction;
			}
			movePieces(op, newFace, rtt, newDirection,
				style, True);
		}
	} else if (style == PERIOD3) {
		for (i = 0; i < op->size; i++) {
			if (op->sticky && i == 2)
				i++;
			if (direction == CW || direction == CCW) {
				if (face >= MAX_SIDES)
					direction = (direction == CW) ? CCW : CW;
				if (face == 5)
					face = 7;
				else if (face == 7)
					face = 5;
				newFace = (face + 2) % MAX_SIDES;
				newDirection = ((face + direction) % MAX_SIDES) * 2;
				rtt.row = i;
				rtt.trbl = i;
				rtt.tlbr = i;
			} else if (direction % 2 == 0) {
				newFace = face;
				newDirection = direction;
				rtt.row = i;
				rtt.trbl = i;
				rtt.tlbr = i;
			} else if (((direction == TR || direction == BL) &&
					face % 2 == 0) ||
					((direction == TL || direction == BR) &&
					face % 2 == 1)) {
				newFace = face;
				newDirection = direction;
				rtt.row = i;
				rtt.trbl = i;
				rtt.tlbr = 0;
			} else {
				newFace = face;
				newDirection = direction;
				rtt.row = i;
				rtt.trbl = 0;
				rtt.tlbr = i;
			}
			movePieces(op, newFace, rtt, newDirection, style, True);
		}
	} else  { /* (style == PERIOD4) */
		for (i = 0; i < 2 * op->size - 1; i++) {
			if (i < op->size) {
				if (direction == CW || direction == CCW  || direction % 2 == 0) {
					rtt.row = i;
					rtt.trbl = i;
					rtt.tlbr = i;
					newFace = face;
					newDirection = direction;
				} else if (((direction == TR || direction == BL) &&
						face % 2 == 0) ||
						((direction == TL || direction == BR) &&
						face % 2 == 1)) {
					newFace = face;
					newDirection = direction;
					rtt.row = i;
					rtt.trbl = i;
					rtt.tlbr = 0;
				} else {
					newFace = face;
					newDirection = direction;
					rtt.row = i;
					rtt.trbl = 0;
					rtt.tlbr = i;
				}
			} else {
				if (direction == CW || direction == CCW  || direction % 2 == 0) {
					newFace = (1 - face / MAX_SIDES) *
						MAX_SIDES + (((face & 1) == 1) ?
						(face + MAX_SIDES / 2) % MAX_SIDES :
						face % MAX_SIDES);
					if (direction == CW || direction == CCW) {
						j = i - op->size;
						rtt.row = j;
						rtt.trbl = j;
						rtt.tlbr = j;
						newDirection = (direction == CW) ? CCW : CW;
					} else {
						j = i - op->size + 1;
						rtt.row = j;
						rtt.trbl = j;
						rtt.tlbr = j;
						newDirection = (((direction / 2) & 1) == 1) ?
							(direction + 2) % MAX_FACES :
							(direction + 6) % MAX_FACES;
					}
				} else if (((direction == TR || direction == BL) &&
						face % 2 == 0) ||
						((direction == TL || direction == BR) &&
						face % 2 == 1)) {
					newFace = ((face / 4) * 4) + ((face % 4) + 2) % 4;
					newDirection = direction;
					j = i - op->size + 1;
					rtt.row = j;
					rtt.trbl = j;
					rtt.tlbr = 0;
				} else {
					newFace = ((face / 4) * 4) + ((face % 4) + 2) % 4;
					newDirection = direction;
					j = i - op->size + 1;
					rtt.row = j;
					rtt.trbl = 0;
					rtt.tlbr = j;
				}
			}
			movePieces(op, newFace, rtt, newDirection,
				style, True);
		}
	}
}

static void
moveOct(octstruct * op, int face, int position, int direction,
		int style, int control)
{
	if (control) {
		moveControlCb(op, face, direction, style);
	} else {
		RTT rtt;

		toRTT(position, &rtt);
		movePieces(op, face, rtt, direction, style, False);
	}
}

static void
evalmovement(ModeInfo * mi, OctMove movement)
{
	octstruct *op = &oct[MI_SCREEN(mi)];

	if (movement.face < 0 || movement.face >= MAX_FACES)
		return;
	moveOct(op, movement.face, movement.position, movement.direction,
		movement.style, False);

}

static Bool
compareMoves(octstruct * op, OctMove move1, OctMove move2)
{
	OctSlice slice1, slice2;

	convertMove(op, move1, &slice1);
	convertMove(op, move2, &slice2);
	if (move1.style != move2.style)
		return False;
	if (slice1.fc == slice2.fc && slice1.depth == slice2.depth) {
		if (move1.style == PERIOD3)
			return True;
		else if (slice1.rotation != slice2.rotation) /* CW or CCW */
			return True;
	}
	return False;
}

static Bool
shuffle(ModeInfo * mi)
{
	octstruct *op = &oct[MI_SCREEN(mi)];
	int	 i, face, position, orient;
	OctMove   move;

	/* Period 4 needs work on the turning */
	if (MI_IS_FULLRANDOM(mi)) {
		op->period3 = (Bool) (LRAND() & 1);
	} else {
		op->period3 = period3;
		op->sticky = sticky;
	}
	if (op->period3) {
		op->degreeTurn = 120;
		op->movement.style = PERIOD3;
	} else {
		op->degreeTurn = 90;
		op->movement.style = PERIOD4;
	}
	op->step = NRAND(op->degreeTurn);

	i = MI_SIZE(mi);
	if (MI_IS_FULLRANDOM(mi))
		op->sticky = False;
	else
		op->sticky = sticky;
	if (i < -MINSIZE) {
		i = NRAND(-i - MINSIZE + 1) + MINSIZE;
		if (i == MINSIZE && NRAND(4) != 0) {
			op->sticky = True;
		}
	} else if (i < MINSIZE)
		 i = MINSIZE;
	op->size = i;
	if (op->sticky)
		op->size = 4;
	for (face = 0; face < MAX_FACES; face++) {
		if (op->facetLoc[face] != NULL)
			free(op->facetLoc[face]);
		if ((op->facetLoc[face] = (OctLoc *) malloc(op->size * op->size *
				sizeof (OctLoc))) == NULL) {
			return False;
		}
		for (position = 0; position < op->size * op->size; position++) {
			op->facetLoc[face][position].face = face;
			op->facetLoc[face][position].rotation =
				3 * face % MAX_ORIENT;
		}
	}
	for (orient = 0; orient < MAX_ORIENT / 2; orient++) {
		if (op->rowLoc[orient] != NULL)
			free(op->rowLoc[orient]);
		if ((op->rowLoc[orient] = (OctLoc *) malloc((2 * op->size - 1) *
				sizeof (OctLoc))) == NULL) {
			return False;
		}
	}
	if (op->faceLoc != NULL)
		free(op->faceLoc);
	if ((op->faceLoc = (OctLoc *) malloc(op->size * op->size *
			sizeof (OctLoc))) == NULL) {
		return False;
	}
	op->storedmoves = MI_COUNT(mi);
	if (op->storedmoves < 0) {
		if (op->moves != NULL)
			free(op->moves);
		op->moves = (OctMove *) NULL;
		op->storedmoves = NRAND(-op->storedmoves) + 1;
	}
	if ((op->storedmoves) && (op->moves == NULL))
		if ((op->moves = (OctMove *) calloc(op->storedmoves + 1,
				sizeof (OctMove))) == NULL) {
			return False;
		}
	if (MI_CYCLES(mi) <= 1) {
		op->angleStep = op->degreeTurn;
	} else {
		op->angleStep = op->degreeTurn / (GLfloat) (MI_CYCLES(mi));
	}

	for (i = 0; i < op->storedmoves; i++) {
		Bool condition;
		int randomDirection;

		do {
			if (op->period3)
				move.style = PERIOD3;
			else
				move.style = PERIOD4;
			move.face = NRAND(MAX_FACES);
			if (op->sticky) {
				if (NRAND(2) == 1)
					move.position = 6; /* center */
				else
					move.position = (NRAND(2) == 1) ? 0 : 12;
			} else /* (!w->oct.sticky) */
				move.position = NRAND(op->size * op->size);
			randomDirection = NRAND(MAX_ORIENT / 2);
			if (randomDirection == 4) {
				if (move.style == PERIOD3) {
					if ((move.face & 1) == 1)
						randomDirection = BOTTOM;
					else
						randomDirection = RIGHT;
				} else /* style == PERIOD4 */
					randomDirection = CW;
			} else if (randomDirection == 5) {
				if (move.style == PERIOD3) {
					if ((move.face & 1) == 1)
						randomDirection = TOP;
					else
						randomDirection = LEFT;
				} else /* style == PERIOD4 */
					randomDirection = CCW;
			} else
				randomDirection = randomDirection * 2 + 1;
			move.direction = randomDirection;
			condition = True;
			if (i > 0 && compareMoves(op, move, op->moves[i - 1])) {
				condition = False;
			}
		} while (!condition);
		if (hideshuffling)
			evalmovement(mi, move);
		op->moves[i] = move;
	}
	op->VX = 0.05;
	if (NRAND(100) < 50)
		op->VX *= -1;
	op->VY = 0.05;
	if (NRAND(100) < 50)
		op->VY *= -1;
	op->movement.face = NO_FACE;
	op->rotateStep = 0;
	op->action = hideshuffling ? ACTION_SOLVE : ACTION_SHUFFLE;
	op->shufflingmoves = 0;
	op->done = 0;
	return True;
}

static void
reshape_oct(ModeInfo * mi, int width, int height)
{
	octstruct *op = &oct[MI_SCREEN(mi)];

	glViewport(0, 0, op->WindW = (GLint) width, op->WindH = (GLint) height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-1.0, 1.0, -1.0, 1.0, 5.0, 15.0);
	glMatrixMode(GL_MODELVIEW);

	op->AreObjectsDefined[ObjCubit] = False;
}

#ifdef STANDALONE
ENTRYPOINT Bool
oct_handle_event (ModeInfo *mi, XEvent *event)
{
	octstruct *op = &oct[MI_SCREEN(mi)];

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
free_oct_screen(octstruct *op)
{
	int face, orient;

	if (op == NULL) {
		return;
	}
#ifdef STANDALONE
	if (op->trackball)
		gltrackball_free (op->trackball);
#endif
	for (face = 0; face < MAX_FACES; face++)
		if (op->facetLoc[face] != NULL) {
			free(op->facetLoc[face]);
			op->facetLoc[face] = (OctLoc *) NULL;
		}
	for (orient = 0; orient < MAX_ORIENT / 2; orient++)
		if (op->rowLoc[orient] != NULL) {
			free(op->rowLoc[orient]);
			op->rowLoc[orient] = (OctLoc *) NULL;
		}
	if (op->faceLoc != NULL)
		free(op->faceLoc);
	op->faceLoc = (OctLoc *) NULL;
	if (op->moves != NULL) {
		free(op->moves);
		op->moves = (OctMove *) NULL;
	}
	op = NULL;
}

ENTRYPOINT void
release_oct(ModeInfo * mi)
{
	if (oct != NULL) {
		int	 screen;

		for (screen = 0; screen < MI_NUM_SCREENS(mi); screen++)
			free_oct_screen(&oct[screen]);
		free(oct);
		oct = (octstruct *) NULL;
	}
	FreeAllGL(mi);
}

ENTRYPOINT void
init_oct(ModeInfo * mi)
{
	octstruct *op;

	MI_INIT(mi, oct);
	op = &oct[MI_SCREEN(mi)];

	op->PX = ((float) LRAND() / (float) MAXRAND) * 2.0 - 1.0;
	op->PY = ((float) LRAND() / (float) MAXRAND) * 2.0 - 1.0;

#ifdef STANDALONE
	op->trackball = gltrackball_init (True);
#endif

	if ((op->glx_context = init_GL(mi)) != NULL) {
		reshape_oct(mi, MI_WIDTH(mi), MI_HEIGHT(mi));
		glDrawBuffer(GL_BACK);
		if (!pinit(mi)) {
			free_oct_screen(op);
			if (MI_IS_VERBOSE(mi)) {
				 (void) fprintf(stderr,
					"Could not allocate memory for oct\n");
			}
		}
	} else {
		MI_CLEARWINDOW(mi);
	}
}

ENTRYPOINT void
draw_oct(ModeInfo * mi)
{
	Bool bounced = False;
	Display    *display = MI_DISPLAY(mi);
	Window      window = MI_WINDOW(mi);
	octstruct *op;

	if (oct == NULL)
		return;
	op = &oct[MI_SCREEN(mi)];
	if ((op->storedmoves) && (op->moves == NULL))
		return;
	if (!op->glx_context)
		return;
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
	draw_octa(mi);
	if (MI_IS_FPS(mi)) {
		do_fps (mi);
	}
	glXSwapBuffers(display, window);
	if (op->action == ACTION_SHUFFLE) {
		if (op->done) {
			if (++op->rotateStep > DELAY_AFTER_SHUFFLING) {
				op->movement.face = NO_FACE;
				op->rotateStep = 0;
				op->action = ACTION_SOLVE;
				op->done = 0;
			}
		} else {
			if (op->movement.face == NO_FACE) {
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
					op->movement.face = NO_FACE;
				}
			}
		}
	} else {
		if (op->done) {
			if (++op->rotateStep > DELAY_AFTER_SOLVING)
				if (!shuffle(mi)) {
					free_oct_screen(op);
					if (MI_IS_VERBOSE(mi)) {
						 (void) fprintf(stderr,
							"Could not allocate memory for oct\n");
					}
				}
		} else {
			if (op->movement.face == NO_FACE) {
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
					op->movement.face = NO_FACE;
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
change_oct(ModeInfo * mi)
{
	octstruct *op;

	if (oct == NULL)
		return;
	op = &oct[MI_SCREEN(mi)];

	if (!op->glx_context)
		return;
	if (!pinit(mi)) {
		free_oct_screen(op);
		if (MI_IS_VERBOSE(mi)) {
			 (void) fprintf(stderr,
				"Could not allocate memory for oct\n");
		}
	}
}
#endif

XSCREENSAVER_MODULE ("Oct", oct)

#endif /* MODE_oct */
