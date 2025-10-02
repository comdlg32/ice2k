/* -*- Mode: C; tab-width: 4 -*- */
/* skewb --- Shows an auto-solving Skewb */

#if 0
static const char sccsid[] = "@(#)skewb.c	5.00 2000/11/01 xlockmore";
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
 * This mode shows an auto-solving a skewb "puzzle".
 *
 * Thanks goes also to Brian Paul for making it possible and inexpensive
 * to use OpenGL at home.
 *
 * Based on rubik.c by Marcelo F. Vianna
 *
 * Revision History:
 * 05-Apr-2002: Removed all gllist uses (fix some bug with nvidia driver)
 * 01-Nov-2000: Allocation checks
 * 27-Apr-2000: Started writing, only have corners drawn and algorithm
 *              compiled in.
 */

/*-
 * Color labels mapping:
 * =====================
 *
 *        +------+
 *        |3    0|
 *        |      |
 *        | TOP  |
 *        | (0)  |
 *        |      |
 *        |2    1|
 * +------+------+------+
 * |3    0|3    0|3    0|
 * |      |      |      |
 * | LEFT |FRONT |RIGHT |
 * | (1)  | (2)  | (3)  |
 * |      |      |      |
 * |2    1|2    1|2    1|
 * +------+------+------+
 *        |3    0|
 *        |      |
 *        |BOTTOM|
 *        | (4)  |
 *        |      |
 *        |2    1|
 *        +------+         +------+
 *        |3    0|         |3 /\ 0|
 *        |      |         | /  \ |
 *        | BACK |         |/xxxx\|
 *        | (5)  |         |\(N) /|
 *        |      |         | \  / |
 *        |2    1|         |2 \/ 1|
 *        +------+         +------+
 *
 *  Map to 3d
 *  FRONT  => X, Y
 *  BACK   => X, Y
 *  LEFT   => Z, Y
 *  RIGHT  => Z, Y
 *  TOP    => X, Z
 *  BOTTOM => X, Z
 */

#ifdef VMS
#include <types.h>
#endif

#ifdef STANDALONE
# define MODE_skewb
# define DEFAULTS	"*delay: 100000 \n" \
			"*count: -30 \n" \
			"*cycles: 5 \n" \

# define free_skewb 0
# include "xlockmore.h"		/* from the xscreensaver distribution */
# include "gltrackball.h"

#else /* !STANDALONE */
#include "xlock.h"		/* from the xlockmore distribution */
#include "visgl.h"
#endif /* !STANDALONE */

#ifdef MODE_skewb

#include <GL/gl.h>
#define DEF_HIDESHUFFLING     "False"

static Bool hideshuffling;

static XrmOptionDescRec opts[] =
{
	{(char *) "-hideshuffling", (char *) ".skewb.hideshuffling", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+hideshuffling", (char *) ".skewb.hideshuffling", XrmoptionNoArg, (caddr_t) "off"}
};

static argtype vars[] =
{
	{(void *) & hideshuffling, (char *) "hideshuffling", (char *) "Hideshuffling", (char *) DEF_HIDESHUFFLING, t_Bool}
};

static OptionStruct desc[] =
{
	{(char *) "-/+hideshuffling", (char *) "turn on/off hidden shuffle phase"}
};

ENTRYPOINT ModeSpecOpt skewb_opts =
{sizeof opts / sizeof opts[0], opts, sizeof vars / sizeof vars[0], vars, desc};

#ifdef USE_MODULES
ModStruct   skewb_description =
{"skewb", "init_skewb", "draw_skewb", "release_skewb",
 "draw_skewb", "change_skewb", "(char *) NULL", &skewb_opts,
 100000, -30, 5, 1, 64, 1.0, "",
 "Shows an auto-solving Skewb", 0, NULL};

#endif

#define VectMul(X1,Y1,Z1,X2,Y2,Z2) (Y1)*(Z2)-(Z1)*(Y2),(Z1)*(X2)-(X1)*(Z2),(X1)*(Y2)-(Y1)*(X2)
#define sqr(A)                     ((A)*(A))

#ifndef Pi
#define Pi                         M_PI
#endif


#define ACTION_SOLVE    1
#define ACTION_SHUFFLE  0

#define DELAY_AFTER_SHUFFLING  5
#define DELAY_AFTER_SOLVING   20

/*************************************************************************/

#define Scale4Window               (0.9/3.0)
#define Scale4Iconic               (2.1/3.0)

#define MAX_ORIENT 4		/* Number of orientations of a square */
#define MAX_FACES 6		/* Number of faces */

/* Directions relative to the face of a cubie */
#define TR 0
#define BR 1
#define BL 2
#define TL 3
#define STRT 4
#define CW 5
#define HALF 6
#define CCW 7
#define TOP 8
#define RIGHT 9
#define BOTTOM 10
#define LEFT 11
#define MAX_ROTATE 3
#define MAX_CUBES (MAX_ORIENT+1)
#define MINOR 0
#define MAJOR 1
#define MAX_FACES 6

#define TOP_FACE 0
#define LEFT_FACE 1
#define FRONT_FACE 2
#define RIGHT_FACE 3
#define BOTTOM_FACE 4
#define BACK_FACE 5
#define NO_FACE (MAX_FACES+1)
#define NO_CORNER 8
#define NO_ROTATION (2*MAX_ORIENT)

#define CUBE_LENGTH 0.5
#define CUBE_ROUND (CUBE_LENGTH-0.05)
#define STICKER_LONG (CUBE_ROUND-0.05)
#define STICKER_SHORT (STICKER_LONG-0.05)
#define STICKER_DEPTH (CUBE_LENGTH+0.01)
#define FACELEN (2.0*CUBE_LENGTH)
#define FACEROUND (2.0*CUBE_LENGTH-0.03)
#define FSTICKER_LONG (FACEROUND-0.05)
#define FSTICKER_SHORT (FSTICKER_LONG-0.05)
#define FSTICKER_DEPTH (FACELEN+0.01)

#define ObjCubit        0
#define ObjFacit        1
#define MaxObj          2

typedef struct _SkewbLoc {
	int         face;
	int         rotation;  /* Not used yet */
} SkewbLoc;

typedef struct _SkewbLocPos {
        int         face, position, direction;
} SkewbLocPos;

typedef struct _RowNext {
        int         face, direction, sideFace;
} RowNext;

typedef struct _SkewbMove {
        int         face, direction;
        int         position;
} SkewbMove;

typedef struct _SkewbSlice {
        int         corner, rotation;
} SkewbSlice;


/*-
 * Pick a face and a direction on face the next face and orientation
 * is then known.
 */
static SkewbLoc slideNextRow[MAX_FACES][MAX_ORIENT][MAX_ORIENT / 2] =
{
	{
		{
			{2, CW},
			{1, HALF}},
		{
			{5, CCW},
			{1, STRT}},
		{
			{3, STRT},
			{5, CW}},
		{
			{3, HALF},
			{2, CCW}}
	},
	{
		{
			{4, STRT},
			{5, CW}},
		{
			{0, STRT},
			{5, CCW}},
		{
			{2, CCW},
			{0, HALF}},
		{
			{2, CW},
			{4, HALF}}
	},
	{
		{
			{4, CW},
			{1, CCW}},
		{
			{0, CCW},
			{1, CW}},
		{
			{3, CCW},
			{0, CW}},
		{
			{3, CW},
			{4, CCW}}
	},
	{
		{
			{4, HALF},
			{2, CCW}},
		{
			{0, HALF},
			{2, CW}},
		{
			{5, CW},
			{0, STRT}},
		{
			{5, CCW},
			{4, STRT}}
	},
	{
		{
			{5, CW},
			{1, STRT}},
		{
			{2, CCW},
			{1, HALF}},
		{
			{3, HALF},
			{2, CW}},
		{
			{3, STRT},
			{5, CCW}}
	},
	{
		{
			{0, CW},
			{1, CW}},
		{
			{4, CCW},
			{1, CCW}},
		{
			{3, CW},
			{4, CW}},
		{
			{3, CCW},
			{0, CCW}}
	}
};
static SkewbLoc minToMaj[MAX_FACES][MAX_ORIENT] =
{				/* other equivalent mappings possible */
	{
		{3, CW},
		{2, STRT},
		{1, CCW},
		{5, STRT}},
	{
		{2, STRT},
		{4, CCW},
		{5, HALF},
		{0, CW}},
	{
		{3, STRT},
		{4, STRT},
		{1, STRT},
		{0, STRT}},
	{
		{5, HALF},
		{4, CW},
		{2, STRT},
		{0, CCW}},
	{
		{3, CCW},
		{5, STRT},
		{1, CW},
		{2, STRT}},
	{
		{3, HALF},
		{0, STRT},
		{1, HALF},
		{4, STRT}}
};

static SkewbLoc slideNextFace[MAX_FACES][MAX_ORIENT] =
{
	{
		{5, STRT},
		{3, CW},
		{2, STRT},
		{1, CCW}},
	{
		{0, CW},
		{2, STRT},
		{4, CCW},
		{5, HALF}},
	{
		{0, STRT},
		{3, STRT},
		{4, STRT},
		{1, STRT}},
	{
		{0, CCW},
		{5, HALF},
		{4, CW},
		{2, STRT}},
	{
		{2, STRT},
		{3, CCW},
		{5, STRT},
		{1, CW}},
	{
		{4, STRT},
		{3, HALF},
		{0, STRT},
		{1, HALF}}
};

static int  faceToRotate[MAX_FACES][MAX_ORIENT] =
{
	{3, 2, 1, 5},
	{2, 4, 5, 0},
	{3, 4, 1, 0},
	{5, 4, 2, 0},
	{3, 5, 1, 2},
	{3, 0, 1, 4}
};

typedef struct {
#ifdef WIN32
	HGLRC       glx_context;
#else
	GLXContext *glx_context;
#endif
	GLint       WindH, WindW;
	GLfloat     step;
	SkewbMove  *moves;
	int         storedmoves;
	int         shufflingmoves;
	int         action;
	int         done;
	GLfloat     angleStep;
	SkewbLoc    cubeLoc[MAX_FACES][MAX_CUBES];
	SkewbLoc    rowLoc[MAX_ORIENT][MAX_CUBES];
	SkewbLoc    minorLoc[MAX_ORIENT], majorLoc[MAX_ORIENT][MAX_ORIENT];
	SkewbMove   movement;
	GLfloat     rotateStep;
	GLfloat     PX, PY, VX, VY;
	Bool        AreObjectsDefined[2];
#ifdef STANDALONE
	Bool button_down_p;
	trackball_state *trackball;
#endif
} skewbstruct;

static float front_shininess[] =
{60.0};
static float front_specular[] =
{0.7, 0.7, 0.7, 1.0};
static float ambient[] =
{0.0, 0.0, 0.0, 1.0};
static float diffuse[] =
{1.0, 1.0, 1.0, 1.0};
static float position0[] =
{1.0, 1.0, 1.0, 0.0};
static float position1[] =
{-1.0, -1.0, 1.0, 0.0};
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
static float MaterialPink[] =
{0.9, 0.5, 0.5, 1.0};
static float MaterialYellow[] =
{0.7, 0.7, 0.0, 1.0};

static float MaterialWhite[] =
{0.8, 0.8, 0.8, 1.0};
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

static skewbstruct *skewb = (skewbstruct *) NULL;

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
	case LEFT_FACE:
		if (mono)
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray5);
		else
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialBlue);
		break;
	case FRONT_FACE:
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialWhite);
		break;
	case RIGHT_FACE:
		if (mono)
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray4);
		else
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGreen);
		break;
	case BOTTOM_FACE:
		if (mono)
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray7);
		else
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialPink);
		break;
	case BACK_FACE:
		if (mono)
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray6);
		else
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialYellow);
		break;
	}
}

static void
drawStickerlessCubit(void)
{
	glBegin(GL_QUADS);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray);
	/* Edge of cubit */
	glNormal3f(1.0, 1.0, 0.0);
	glVertex3f(CUBE_ROUND, CUBE_LENGTH, -CUBE_ROUND);
	glVertex3f(CUBE_ROUND, CUBE_LENGTH, CUBE_ROUND);
	glVertex3f(CUBE_LENGTH, CUBE_ROUND, CUBE_ROUND);
	glVertex3f(CUBE_LENGTH, CUBE_ROUND, -CUBE_ROUND);
	glNormal3f(0.0, 1.0, 1.0);
	glVertex3f(-CUBE_ROUND, CUBE_ROUND, CUBE_LENGTH);
	glVertex3f(CUBE_ROUND, CUBE_ROUND, CUBE_LENGTH);
	glVertex3f(CUBE_ROUND, CUBE_LENGTH, CUBE_ROUND);
	glVertex3f(-CUBE_ROUND, CUBE_LENGTH, CUBE_ROUND);
	glNormal3f(1.0, 0.0, 1.0);
	glVertex3f(CUBE_LENGTH, -CUBE_ROUND, CUBE_ROUND);
	glVertex3f(CUBE_LENGTH, CUBE_ROUND, CUBE_ROUND);
	glVertex3f(CUBE_ROUND, CUBE_ROUND, CUBE_LENGTH);
	glVertex3f(CUBE_ROUND, -CUBE_ROUND, CUBE_LENGTH);
	glEnd();
	glBegin(GL_TRIANGLES);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray);
	/* Put sticker here */
	glNormal3f(0.0, 0.0, 1.0);
	glVertex3f(CUBE_ROUND, -CUBE_ROUND, CUBE_LENGTH);
	glVertex3f(CUBE_ROUND, CUBE_ROUND, CUBE_LENGTH);
	glVertex3f(-CUBE_ROUND, CUBE_ROUND, CUBE_LENGTH);
	glNormal3f(1.0, 0.0, 0.0);
	glVertex3f(CUBE_LENGTH, CUBE_ROUND, -CUBE_ROUND);
	glVertex3f(CUBE_LENGTH, CUBE_ROUND, CUBE_ROUND);
	glVertex3f(CUBE_LENGTH, -CUBE_ROUND, CUBE_ROUND);
	glNormal3f(0.0, 1.0, 0.0);
	glVertex3f(-CUBE_ROUND, CUBE_LENGTH, CUBE_ROUND);
	glVertex3f(CUBE_ROUND, CUBE_LENGTH, CUBE_ROUND);
	glVertex3f(CUBE_ROUND, CUBE_LENGTH, -CUBE_ROUND);
	/* Corner of cubit */
	glNormal3f(1.0, 1.0, 1.0);
	glVertex3f(CUBE_ROUND, CUBE_ROUND, CUBE_LENGTH);
	glVertex3f(CUBE_LENGTH, CUBE_ROUND, CUBE_ROUND);
	glVertex3f(CUBE_ROUND, CUBE_LENGTH, CUBE_ROUND);

	/* Sharper corners of cubit */
	glNormal3f(-1.0, 1.0, 1.0);
	glVertex3f(-CUBE_LENGTH, CUBE_ROUND, CUBE_LENGTH);
	glVertex3f(-CUBE_LENGTH, CUBE_LENGTH, CUBE_ROUND);
	glVertex3f(-CUBE_LENGTH, CUBE_ROUND, CUBE_ROUND);
	glNormal3f(1.0, -1.0, 1.0);
	glVertex3f(CUBE_ROUND, -CUBE_LENGTH, CUBE_LENGTH);
	glVertex3f(CUBE_ROUND, -CUBE_LENGTH, CUBE_ROUND);
	glVertex3f(CUBE_LENGTH, -CUBE_LENGTH, CUBE_ROUND);
	glNormal3f(1.0, 1.0, -1.0);
	glVertex3f(CUBE_LENGTH, CUBE_ROUND, -CUBE_LENGTH);
	glVertex3f(CUBE_ROUND, CUBE_ROUND, -CUBE_LENGTH);
	glVertex3f(CUBE_ROUND, CUBE_LENGTH, -CUBE_LENGTH);
	glEnd();

	glBegin(GL_POLYGON);
	glNormal3f(-1.0, 1.0, 1.0);
	glVertex3f(-CUBE_ROUND, CUBE_ROUND,  CUBE_LENGTH);
	glVertex3f(-CUBE_ROUND, CUBE_LENGTH,  CUBE_ROUND);
	glVertex3f(CUBE_ROUND, CUBE_LENGTH, -CUBE_ROUND);
	glVertex3f(CUBE_LENGTH, CUBE_ROUND, -CUBE_ROUND);
	glVertex3f(CUBE_LENGTH, -CUBE_ROUND, CUBE_ROUND);
	glVertex3f(CUBE_ROUND, -CUBE_ROUND, CUBE_LENGTH);
	glEnd();
}

static void
drawStickerlessFacit(void)
{
	glBegin(GL_QUADS);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray);
	/* Edge of facit */
	glNormal3f(0.0, 0.0, 1.0);
	glNormal3f(0.0, 0.0, 1.0);
	glVertex3f(-FACEROUND, 0.0, FACELEN);
	glVertex3f(0.0, -FACEROUND, FACELEN);
	glVertex3f(FACEROUND, 0.0, FACELEN);
	glVertex3f(0.0, FACEROUND, FACELEN);
	glEnd();
	glBegin(GL_TRIANGLES);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray);
	glNormal3f(0.0, 0.0, 1.0);
	glVertex3f(0.0, -FACEROUND, FACELEN);
	glVertex3f(-FACEROUND, 0.0, FACELEN);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(FACEROUND, 0.0, FACELEN);
	glVertex3f(0.0, -FACEROUND, FACELEN);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, FACEROUND, FACELEN);
	glVertex3f(FACEROUND, 0.0, FACELEN);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(-FACEROUND, 0.0, FACELEN);
	glVertex3f(0.0, FACEROUND, FACELEN);
	glVertex3f(0.0, 0.0, 0.0);
	glEnd();
}

static void
drawFacit(ModeInfo * mi, int face)
{
	int	 mono = MI_IS_MONO(mi);
	skewbstruct *sp = &skewb[MI_SCREEN(mi)];

	pickColor(sp->cubeLoc[face][MAX_ORIENT].face, mono);
	switch (face) {
	case TOP_FACE:
		glBegin(GL_QUADS);
		glNormal3f(0.0, 1.0, 0.0);
		glVertex3f(0.0, FSTICKER_DEPTH, -FSTICKER_LONG);
		glVertex3f(-FSTICKER_LONG, FSTICKER_DEPTH, 0.0);
		glVertex3f(0.0, FSTICKER_DEPTH, FSTICKER_LONG);
		glVertex3f(FSTICKER_LONG, FSTICKER_DEPTH, 0.0);
		glEnd();
		break;
	case LEFT_FACE:
		glBegin(GL_QUADS);
		glNormal3f(-1.0, 0.0, 0.0);
		glVertex3f(-FSTICKER_DEPTH, 0.0, -FSTICKER_LONG);
		glVertex3f(-FSTICKER_DEPTH, -FSTICKER_LONG, 0.0);
		glVertex3f(-FSTICKER_DEPTH, 0.0, FSTICKER_LONG);
		glVertex3f(-FSTICKER_DEPTH, FSTICKER_LONG, 0.0);
		glEnd();
		break;
	case FRONT_FACE:
		glBegin(GL_QUADS);
		glNormal3f(0.0, 0.0, 1.0);
		glVertex3f(-FSTICKER_LONG, 0.0, FSTICKER_DEPTH);
		glVertex3f(0.0, -FSTICKER_LONG, FSTICKER_DEPTH);
		glVertex3f(FSTICKER_LONG, 0.0, FSTICKER_DEPTH);
		glVertex3f(0.0, FSTICKER_LONG, FSTICKER_DEPTH);
		glEnd();
		break;
	case RIGHT_FACE:
		glBegin(GL_QUADS);
		glNormal3f(1.0, 0.0, 0.0);
		glVertex3f(FSTICKER_DEPTH, FSTICKER_LONG, 0.0);
		glVertex3f(FSTICKER_DEPTH, 0.0, FSTICKER_LONG);
		glVertex3f(FSTICKER_DEPTH, -FSTICKER_LONG, 0.0);
		glVertex3f(FSTICKER_DEPTH, 0.0, -FSTICKER_LONG);
		glEnd();
		break;
	case BOTTOM_FACE:
		glBegin(GL_QUADS);
		glNormal3f(0.0, -1.0, 0.0);
		glVertex3f(FSTICKER_LONG, -FSTICKER_DEPTH, 0.0);
		glVertex3f(0.0, -FSTICKER_DEPTH, FSTICKER_LONG);
		glVertex3f(-FSTICKER_LONG, -FSTICKER_DEPTH, 0.0);
		glVertex3f(0.0, -FSTICKER_DEPTH, -FSTICKER_LONG);
		glEnd();
	       break;
	case BACK_FACE:
		glBegin(GL_QUADS);
		glNormal3f(0.0, 0.0, -1.0);
		glVertex3f(0.0, FSTICKER_LONG, -FSTICKER_DEPTH);
		glVertex3f(FSTICKER_LONG, 0.0, -FSTICKER_DEPTH);
		glVertex3f(0.0, -FSTICKER_LONG, -FSTICKER_DEPTH);
		glVertex3f(-FSTICKER_LONG, 0.0, -FSTICKER_DEPTH);
		glEnd();
		break;
	}
}

static void
drawCubit(ModeInfo * mi,
	   int back, int front, int left, int right, int bottom, int top)
{
	/* skewbstruct *sp = &skewb[MI_SCREEN(mi)]; */
	int	 mono = MI_IS_MONO(mi);

	if (back != NO_FACE) {
		glBegin(GL_POLYGON);
		pickColor(back, mono);
		glNormal3f(0.0, 0.0, -1.0);
		if (top != NO_FACE) {
			glVertex3f(-STICKER_SHORT, STICKER_LONG, -STICKER_DEPTH);
			glVertex3f(STICKER_SHORT, STICKER_LONG, -STICKER_DEPTH);
		}
		if (left != NO_FACE) {
			glVertex3f(-STICKER_LONG, -STICKER_SHORT, -STICKER_DEPTH);
			glVertex3f(-STICKER_LONG, STICKER_SHORT, -STICKER_DEPTH);
		}
		if (bottom != NO_FACE) {
			glVertex3f(STICKER_SHORT, -STICKER_LONG, -STICKER_DEPTH);
			glVertex3f(-STICKER_SHORT, -STICKER_LONG, -STICKER_DEPTH);
		}
		if (right != NO_FACE) {
			glVertex3f(STICKER_LONG, STICKER_SHORT, -STICKER_DEPTH);
			glVertex3f(STICKER_LONG, -STICKER_SHORT, -STICKER_DEPTH);
		}
		glEnd();
	}
	if (front != NO_FACE) {
		glBegin(GL_POLYGON);
		pickColor(front, mono);
		glNormal3f(0.0, 0.0, 1.0);
		if (top != NO_FACE) {
			glVertex3f(STICKER_SHORT, STICKER_LONG, STICKER_DEPTH);
			glVertex3f(-STICKER_SHORT, STICKER_LONG, STICKER_DEPTH);
		}
		if (left != NO_FACE) {
			glVertex3f(-STICKER_LONG, STICKER_SHORT, STICKER_DEPTH);
			glVertex3f(-STICKER_LONG, -STICKER_SHORT, STICKER_DEPTH);
		}
		if (bottom != NO_FACE) {
			glVertex3f(-STICKER_SHORT, -STICKER_LONG, STICKER_DEPTH);
			glVertex3f(STICKER_SHORT, -STICKER_LONG, STICKER_DEPTH);
		}
		if (right != NO_FACE) {
			glVertex3f(STICKER_LONG, -STICKER_SHORT, STICKER_DEPTH);
			glVertex3f(STICKER_LONG, STICKER_SHORT, STICKER_DEPTH);
		}
		glEnd();
	}
	if (left != NO_FACE) {
		glBegin(GL_POLYGON);
		pickColor(left, mono);
		glNormal3f(-1.0, 0.0, 0.0);
		if (front != NO_FACE) {
			glVertex3f(-STICKER_DEPTH, -STICKER_SHORT, STICKER_LONG);
			glVertex3f(-STICKER_DEPTH, STICKER_SHORT, STICKER_LONG);
		}
		if (top != NO_FACE) {
			glVertex3f(-STICKER_DEPTH, STICKER_LONG, STICKER_SHORT);
			glVertex3f(-STICKER_DEPTH, STICKER_LONG, -STICKER_SHORT);
		}
		if (back != NO_FACE) {
			glVertex3f(-STICKER_DEPTH, STICKER_SHORT, -STICKER_LONG);
			glVertex3f(-STICKER_DEPTH, -STICKER_SHORT, -STICKER_LONG);
		}
		if (bottom != NO_FACE) {
			glVertex3f(-STICKER_DEPTH, -STICKER_LONG, -STICKER_SHORT);
			glVertex3f(-STICKER_DEPTH, -STICKER_LONG, STICKER_SHORT);
		}
		glEnd();
	}
	if (right != NO_FACE) { /* Green */
		glBegin(GL_POLYGON);
		pickColor(right, mono);
		glNormal3f(1.0, 0.0, 0.0);
		if (front != NO_FACE) {
			glVertex3f(STICKER_DEPTH, STICKER_SHORT, STICKER_LONG);
			glVertex3f(STICKER_DEPTH, -STICKER_SHORT, STICKER_LONG);
		}
		if (top != NO_FACE) {
			glVertex3f(STICKER_DEPTH, STICKER_LONG, -STICKER_SHORT);
			glVertex3f(STICKER_DEPTH, STICKER_LONG, STICKER_SHORT);
		}
		if (back != NO_FACE) {
			glVertex3f(STICKER_DEPTH, -STICKER_SHORT, -STICKER_LONG);
			glVertex3f(STICKER_DEPTH, STICKER_SHORT, -STICKER_LONG);
		}
		if (bottom != NO_FACE) {
			glVertex3f(STICKER_DEPTH, -STICKER_LONG, STICKER_SHORT);
			glVertex3f(STICKER_DEPTH, -STICKER_LONG, -STICKER_SHORT);
		}
		glEnd();
	}
	if (bottom != NO_FACE) { /* Pink */
		glBegin(GL_POLYGON);
		pickColor(bottom, mono);
		glNormal3f(0.0, -1.0, 0.0);
		if (left != NO_FACE) {
			glVertex3f(-STICKER_LONG, -STICKER_DEPTH, STICKER_SHORT);
			glVertex3f(-STICKER_LONG, -STICKER_DEPTH, -STICKER_SHORT);
		}
		if (front != NO_FACE) {
			glVertex3f(STICKER_SHORT, -STICKER_DEPTH, STICKER_LONG);
			glVertex3f(-STICKER_SHORT, -STICKER_DEPTH, STICKER_LONG);
		}
		if (right != NO_FACE) {
			glVertex3f(STICKER_LONG, -STICKER_DEPTH, -STICKER_SHORT);
			glVertex3f(STICKER_LONG, -STICKER_DEPTH, STICKER_SHORT);
		}
		if (back != NO_FACE) {
			glVertex3f(-STICKER_SHORT, -STICKER_DEPTH, -STICKER_LONG);
			glVertex3f(STICKER_SHORT, -STICKER_DEPTH, -STICKER_LONG);
		}
		glEnd();
	}
	if (top != NO_FACE) {
		glBegin(GL_POLYGON);
		pickColor(top, mono);
		glNormal3f(0.0, 1.0, 0.0);
		if (left != NO_FACE) {
			glVertex3f(-STICKER_LONG, STICKER_DEPTH, -STICKER_SHORT);
			glVertex3f(-STICKER_LONG, STICKER_DEPTH, STICKER_SHORT);
		}
		if (front != NO_FACE) {
			glVertex3f(-STICKER_SHORT, STICKER_DEPTH, STICKER_LONG);
			glVertex3f(STICKER_SHORT, STICKER_DEPTH, STICKER_LONG);
		}
		if (right != NO_FACE) {
			glVertex3f(STICKER_LONG, STICKER_DEPTH, STICKER_SHORT);
			glVertex3f(STICKER_LONG, STICKER_DEPTH, -STICKER_SHORT);
		}
		if (back != NO_FACE) {
			glVertex3f(STICKER_SHORT, STICKER_DEPTH, -STICKER_LONG);
			glVertex3f(-STICKER_SHORT, STICKER_DEPTH, -STICKER_LONG);
		}
		glEnd();
	}
}

static void
drawWholeFacit(ModeInfo *mi, int x, int y, int z)
{
	if (x < 0) {
		/* LEFT_FACE, Blue, 1 */
		glPushMatrix();
		glRotatef(-90.0, 0, 1, 0);
		drawStickerlessFacit();
		glPopMatrix();
		glPushMatrix();
		drawFacit(mi, 1);
		glPopMatrix();
	} else if (x > 0) {
		 /* RIGHT_FACE, Green, 3 */
		glPushMatrix();
		glRotatef(90.0, 0, 1, 0);
		drawStickerlessFacit();
		glPopMatrix();
		glPushMatrix();
		drawFacit(mi, 3);
		glPopMatrix();
	} else if (y < 0) {
		/* BOTTOM_FACE, Pink 4 */
		glPushMatrix();
		glRotatef(90.0, 1, 0, 0);
		drawStickerlessFacit();
		glPopMatrix();
		glPushMatrix();
		drawFacit(mi, 4);
		glPopMatrix();
	} else if (y > 0) {
		/* TOP_FACE, Red 0 */
		glPushMatrix();
		glRotatef(-90.0, 1, 0, 0);
		drawStickerlessFacit();
		glPopMatrix();
		glPushMatrix();
		drawFacit(mi, 0);
		glPopMatrix();
	} else if (z < 0) {
		/* BACK_FACE, Yellow 5 */
		glPushMatrix();
		glRotatef(180.0, 0, 1, 0);
		drawStickerlessFacit();
		glPopMatrix();
		glPushMatrix();
		drawFacit(mi, 5);
		glPopMatrix();
	} else if (z > 0) {
		/* FRONT_FACE, White, 2 */
		glPushMatrix();
		drawStickerlessFacit();
		glPopMatrix();
		glPushMatrix();
		drawFacit(mi, 2);
		glPopMatrix();
	}
}

static void
drawWholeCubit(ModeInfo * mi, int x, int y, int z)
{
	skewbstruct *sp = &skewb[MI_SCREEN(mi)];
	const float h = 0.5;

	if (x < 0) {
		if (y < 0) {
			if (z < 0) {
				glPushMatrix();
				glTranslatef(-h, -h, -h);
				glPushMatrix();
				glRotatef(90.0, 0, 1, 0);
				glRotatef(180.0, 1, 0, 0);
				drawStickerlessCubit();
				glPopMatrix();
				drawCubit(mi, /* BLL */
					sp->cubeLoc[BACK_FACE][3].face,
					NO_FACE,
					sp->cubeLoc[LEFT_FACE][2].face,
					NO_FACE,
					sp->cubeLoc[BOTTOM_FACE][2].face,
					NO_FACE);
				glPopMatrix();
			} else if (z > 0) {
				glPushMatrix();
				glTranslatef(-h, -h, h);
				glPushMatrix();
				glRotatef(180.0, 0, 0, 1);
				drawStickerlessCubit();
				glPopMatrix();
				drawCubit(mi, /* FLL */
					NO_FACE,
					sp->cubeLoc[FRONT_FACE][2].face,
					sp->cubeLoc[LEFT_FACE][1].face,
					NO_FACE,
					sp->cubeLoc[BOTTOM_FACE][3].face,
					NO_FACE);
				glPopMatrix();
			}
		} else if (y > 0) {
			if (z < 0) {
				glPushMatrix();
				glTranslatef(-h, h, -h);
				glPushMatrix();
				glRotatef(90.0, 0, -1, 0);
				glRotatef(90.0, 0, 0, 1);
				drawStickerlessCubit();
				glPopMatrix();
				drawCubit(mi, /* BUL */
					sp->cubeLoc[BACK_FACE][2].face,
					NO_FACE,
					sp->cubeLoc[LEFT_FACE][3].face,
					NO_FACE,
					NO_FACE,
					sp->cubeLoc[TOP_FACE][3].face);
				glPopMatrix();
			} else if (z > 0) {
				glPushMatrix();
				glTranslatef(-h, h, h);
				glPushMatrix();
				glRotatef(90.0, 0, 0, 1);
				drawStickerlessCubit();
				glPopMatrix();
				drawCubit(mi, /* FUL */
					NO_FACE,
					sp->cubeLoc[FRONT_FACE][3].face,
					sp->cubeLoc[LEFT_FACE][0].face,
					NO_FACE,
					NO_FACE,
					sp->cubeLoc[TOP_FACE][2].face);
				glPopMatrix();
			}
		}
	} else if (x > 0) {
		if (y < 0) {
			if (z < 0) {
				glPushMatrix();
				glTranslatef(h, -h, -h);
				glPushMatrix();
				glRotatef(90.0, 0, 1, 0);
				glRotatef(90.0, 1, 0, 0);
				drawStickerlessCubit();
				glPopMatrix();
				drawCubit(mi, /* BLR */
					sp->cubeLoc[BACK_FACE][0].face,
					NO_FACE,
					NO_FACE,
					sp->cubeLoc[RIGHT_FACE][1].face,
					sp->cubeLoc[BOTTOM_FACE][1].face,
					NO_FACE);
				glPopMatrix();
			} else if (z > 0) {
				glPushMatrix();
				glTranslatef(h, -h, h);
				glPushMatrix();
				glRotatef(90.0, 1, 0, 0);
				drawStickerlessCubit();
				glPopMatrix();
				drawCubit(mi, /* FLR */
					NO_FACE,
					sp->cubeLoc[FRONT_FACE][1].face,
					NO_FACE,
					sp->cubeLoc[RIGHT_FACE][2].face,
					sp->cubeLoc[BOTTOM_FACE][0].face,
					NO_FACE);
				glPopMatrix();
			}
		} else if (y > 0) {
			if (z < 0) {
				glPushMatrix();
				glTranslatef(h, h, -h);
				glPushMatrix();
				glRotatef(90.0, 0, 1, 0);
				drawStickerlessCubit();
				glPopMatrix();
				drawCubit(mi, /* BUR */
					sp->cubeLoc[BACK_FACE][1].face,
					NO_FACE,
					NO_FACE,
					sp->cubeLoc[RIGHT_FACE][0].face,
					NO_FACE,
					sp->cubeLoc[TOP_FACE][0].face);
				glPopMatrix();
			} else if (z > 0) {
				glPushMatrix();
				glTranslatef(h, h, h);
				glPushMatrix();
				drawStickerlessCubit();
				glPopMatrix();
				drawCubit(mi, /* FUR */
					NO_FACE,
					sp->cubeLoc[FRONT_FACE][0].face,
					NO_FACE,
					sp->cubeLoc[RIGHT_FACE][3].face,
					NO_FACE,
					sp->cubeLoc[TOP_FACE][1].face);
				glPopMatrix();
			}
		}
	}
}

static int nextCorner[MAX_FACES][MAX_ORIENT] =
{
	{4, 5, 1, 0},
	{7, 5, 4, 6},
	{6, 4, 0, 2},
	{2, 0, 1, 3},
	{7, 6, 2, 3},
	{5, 7, 3, 1}
};

static int oppCorner[8] =
{7, 6, 5, 4, 3, 2, 1, 0};

#if 0
static int nextFace[MAX_FACES][MAX_ORIENT] =
{
	{-4, -1, 4, 1},
	{-1, 2, 1, -2},
	{-4, 2, 4, -2},
	{1, 2, -1, -2},
	{-4, 1, 4, -1},
	{-4, -2, 4, 2},
};
#endif

static void
convertMove(SkewbMove move, SkewbSlice * slice)
{
	if ((move.position + 3) % MAX_ORIENT == move.direction) {
		(*slice).corner = nextCorner[move.face][move.position];
		(*slice).rotation = CW;
	} else if ((move.position + 1) % MAX_ORIENT == move.direction) {
		(*slice).corner = nextCorner[move.face][move.position];
		(*slice).rotation = CCW;
	} else {
		(*slice).corner = NO_CORNER;
		(*slice).rotation = CW;
	}
}

/*-
 * If you want to rotate a slice, you can draw that at the angleStep,
  then the without angleStep.
 * There is a sequence for drawing cubies for each axis being moved...
 */
static void
skewRotate(ModeInfo *mi, int corner, GLfloat rotateStep)
{
#define X1 (((corner & 4) != 0) ? 1 : -1)
#define Y1 (((corner & 2) != 0) ? 1 : -1)
#define Z1 (((corner & 1) != 0) ? 1 : -1)
#define Xn (((corner & 4) != 0) ? -1 : 1)
#define Yn (((corner & 2) != 0) ? -1 : 1)
#define Zn (((corner & 1) != 0) ? -1 : 1)
	glPushMatrix();
	glRotatef(rotateStep, X1, Y1, Z1);
	drawWholeFacit(mi, X1, 0, 0);
	drawWholeFacit(mi, 0, Y1, 0);
	drawWholeFacit(mi, 0, 0, Z1);
	drawWholeCubit(mi, X1, Y1, Z1);
	drawWholeCubit(mi, Xn, Y1, Z1);
	drawWholeCubit(mi, X1, Yn, Z1);
	drawWholeCubit(mi, X1, Y1, Zn);
	glPopMatrix();

	drawWholeFacit(mi, Xn, 0, 0);
	drawWholeFacit(mi, 0, Yn, 0);
	drawWholeFacit(mi, 0, 0, Zn);
	drawWholeCubit(mi, Xn, Yn, Zn);
	drawWholeCubit(mi, X1, Yn, Zn);
	drawWholeCubit(mi, Xn, Y1, Zn);
	drawWholeCubit(mi, Xn, Yn, Z1);
}

static void
drawCube(ModeInfo * mi)
{
	skewbstruct *sp = &skewb[MI_SCREEN(mi)];
	SkewbSlice  slice;
	GLfloat     rotateStep = 0.0;
	/* int	 i, j, k; */

	if (sp->movement.face == NO_FACE) {
		slice.corner = NO_CORNER;
		slice.rotation = NO_ROTATION;
		skewRotate(mi, slice.corner, rotateStep);
	} else {
		convertMove(sp->movement, &slice);
		rotateStep = (slice.rotation == CCW) ? sp->rotateStep :
			-sp->rotateStep;
		skewRotate(mi, slice.corner, rotateStep);
	}
}

/* From David Bagley's xskewb.  Used by permission. ;)  */
static void
rotateFace(skewbstruct * sp, int face, int direction)
{
	SkewbLoc faceLoc[MAX_CUBES];
	int corner;

	/* Read Face */
	for (corner = 0; corner < MAX_ORIENT; corner++)
		faceLoc[corner] = sp->cubeLoc[face][corner];
	/* Write Face */
	for (corner = 0; corner < MAX_ORIENT; corner++) {
		sp->cubeLoc[face][corner] = (direction == CW) ?
			faceLoc[(corner + MAX_ORIENT - 1) % MAX_ORIENT] :
			faceLoc[(corner + 1) % MAX_ORIENT];
		sp->cubeLoc[face][corner].rotation =
			(sp->cubeLoc[face][corner].rotation + direction) %
			MAX_ORIENT;
		/* drawTriangle(face, corner); */
	}
	sp->cubeLoc[face][MAX_ORIENT].rotation =
		(sp->cubeLoc[face][MAX_ORIENT].rotation + direction) %
		MAX_ORIENT;
	/* drawDiamond(face); */
}

static void
readFace(skewbstruct * sp, int face, int h)
{
	int position;

	for (position = 0; position < MAX_CUBES; position++)
		sp->rowLoc[h][position] = sp->cubeLoc[face][position];
}

static void
writeFace(skewbstruct * sp, int face, int rotate, int h)
{
	int corner, newCorner;

	for (corner = 0; corner < MAX_ORIENT; corner++) {
		newCorner = (corner + rotate) % MAX_ORIENT;
		sp->cubeLoc[face][newCorner] = sp->rowLoc[h][corner];
		sp->cubeLoc[face][newCorner].rotation =
			(sp->cubeLoc[face][newCorner].rotation + rotate) %
			MAX_ORIENT;
		/* drawTriangle(face, (corner + rotate) % MAX_ORIENT); */
	}
	sp->cubeLoc[face][MAX_ORIENT] = sp->rowLoc[h][MAX_ORIENT];
	sp->cubeLoc[face][MAX_ORIENT].rotation =
		(sp->cubeLoc[face][MAX_ORIENT].rotation + rotate) % MAX_ORIENT;
	/* drawDiamond(face); */
}

static void
readDiagonal(skewbstruct *sp, int face, int corner, int orient, int size)
{
	int g;

	if (size == MINOR)
		sp->minorLoc[orient] = sp->cubeLoc[face][corner];
	else {			/* size == MAJOR */
		for (g = 1; g < MAX_ORIENT; g++)
			sp->majorLoc[orient][g - 1] =
				sp->cubeLoc[face][(corner + g) % MAX_ORIENT];
		sp->majorLoc[orient][MAX_ORIENT - 1] =
			sp->cubeLoc[face][MAX_ORIENT];
	}
}

static void
rotateDiagonal(skewbstruct *sp, int rotate, int orient, int size)
{
	int g;

	if (size == MINOR)
		sp->minorLoc[orient].rotation =
			(sp->minorLoc[orient].rotation + rotate) % MAX_ORIENT;
	else			/* size == MAJOR */
		for (g = 0; g < MAX_ORIENT; g++)
			sp->majorLoc[orient][g].rotation =
				(sp->majorLoc[orient][g].rotation + rotate) %
				MAX_ORIENT;
}

static void
writeDiagonal(skewbstruct *sp, int face, int corner, int orient, int size)
{
	int g, h;

	if (size == MINOR) {
		sp->cubeLoc[face][corner] = sp->minorLoc[orient];
		/* drawTriangle(face, corner); */
	} else {		/* size == MAJOR */
		sp->cubeLoc[face][MAX_ORIENT] =
			sp->majorLoc[orient][MAX_ORIENT - 1];
		/* drawDiamond(face); */
		for (g = 1; g < MAX_ORIENT; g++) {
			h = (corner + g) % MAX_ORIENT;
			sp->cubeLoc[face][h] = sp->majorLoc[orient][g - 1];
			/* drawTriangle(face, h); */
		}
	}
}

#ifdef HACK
static Boolean
checkMoveDir(int position1, int position2, int *direction)
{
	if (!((position1 - position2 + MAX_ORIENT) % 2))
		return False;
	switch (position1) {
		case 0:
			*direction = (position2 == 1) ? 2 : 3;
			break;
		case 1:
			*direction = (position2 == 2) ? 3 : 0;
			break;
		case 2:
			*direction = (position2 == 3) ? 0 : 1;
			break;
		case 3:
			*direction = (position2 == 0) ? 1 : 2;
			break;
		default:
			return False;
	}
	*direction += 2 * MAX_ORIENT;
	return True;
}
#endif

static void
moveSkewb(skewbstruct * sp, int face, int position, int direction)
{
	int newFace, newDirection, newCorner, k, size, rotate;

	if (direction < 2 * MAX_ORIENT) {
		/* position as MAX_ORIENT is ambiguous */
		for (size = MINOR; size <= MAJOR; size++) {
			readDiagonal(sp, face, position, 0, size);
			for (k = 1; k <= MAX_ROTATE; k++) {
				newFace = slideNextRow[face][position][direction / 2].face;
				rotate = slideNextRow[face][position][direction / 2].rotation %
					MAX_ORIENT;
				newDirection = (rotate + direction) % MAX_ORIENT;
				newCorner = (rotate + position) % MAX_ORIENT;
				if (k != MAX_ROTATE)
					readDiagonal(sp, newFace, newCorner, k, size);
				rotateDiagonal(sp, rotate, k - 1, size);
				writeDiagonal(sp, newFace, newCorner, k - 1, size);
				face = newFace;
				position = newCorner;
				direction = newDirection;
			}
			if (size == MINOR) {
				newFace = minToMaj[face][position].face;
				rotate = minToMaj[face][position].rotation % MAX_ORIENT;
				direction = (rotate + direction) % MAX_ORIENT;
				position = (position + rotate + 2) % MAX_ORIENT;
				face = newFace;
			}
		}
	} else {
		rotateFace(sp, faceToRotate[face][direction % MAX_ORIENT], CW);
		rotateFace(sp, faceToRotate[face][(direction + 2) % MAX_ORIENT], CCW);
		readFace(sp, face, 0);
		for (k = 1; k <= MAX_ORIENT; k++) {
			newFace = slideNextFace[face][direction % MAX_ORIENT].face;
			rotate = slideNextFace[face][direction % MAX_ORIENT].rotation;
			newDirection = (rotate + direction) % MAX_ORIENT;
			if (k != MAX_ORIENT)
				readFace(sp, newFace, k);
			writeFace(sp, newFace, rotate, k - 1);
			face = newFace;
			direction = newDirection;
		}
	}
}

#ifdef DEBUG
static void
printCube(skewbstruct * sp)
{
	int	 face, position;

	for (face = 0; face < MAX_FACES; face++) {
		for (position = 0; position < MAX_CUBES; position++) {
			(void) printf("%d %d  ", sp->cubeLoc[face][position].face,
			sp->cubeLoc[face][position].rotation);
		}
		(void) printf("\n");
	}
	(void) printf("\n");
}

#endif

static void
evalmovement(ModeInfo * mi, SkewbMove movement)
{
	skewbstruct *sp = &skewb[MI_SCREEN(mi)];

#ifdef DEBUG
	printCube(sp);
#endif
	if (movement.face < 0 || movement.face >= MAX_FACES)
		return;

	moveSkewb(sp, movement.face, movement.position, movement.direction);

}

static Bool
compareMoves(SkewbMove move1, SkewbMove move2)
{
	SkewbSlice slice1, slice2;

	convertMove(move1, &slice1);
	convertMove(move2, &slice2);
	return (slice1.corner == slice2.corner
		|| slice1.corner == oppCorner[slice2.corner]);
}

static Bool
shuffle(ModeInfo * mi)
{
	skewbstruct *sp = &skewb[MI_SCREEN(mi)];
	int	 i, face, position;
	SkewbMove   move;

	for (face = 0; face < MAX_FACES; face++) {
		for (position = 0; position < MAX_CUBES; position++) {
			sp->cubeLoc[face][position].face = face;
			sp->cubeLoc[face][position].rotation = TOP;
		}
	}
	sp->storedmoves = MI_COUNT(mi);
	if (sp->storedmoves < 0) {
		if (sp->moves != NULL)
			free(sp->moves);
		sp->moves = (SkewbMove *) NULL;
		sp->storedmoves = NRAND(-sp->storedmoves) + 1;
	}
	if ((sp->storedmoves) && (sp->moves == NULL))
		if ((sp->moves = (SkewbMove *) calloc(sp->storedmoves + 1,
				sizeof (SkewbMove))) == NULL) {
			return False;
		}
	if (MI_CYCLES(mi) <= 1) {
		sp->angleStep = 120.0;
	} else {
		sp->angleStep = 120.0 / (GLfloat) (MI_CYCLES(mi));
	}

	for (i = 0; i < sp->storedmoves; i++) {
		Bool condition;

		do {
			move.face = NRAND(MAX_FACES);
			move.position = NRAND(MAX_ORIENT);
			move.direction = ((move.position +
				((NRAND(2) == 0) ? 1 : 3)) % MAX_ORIENT);
			condition = True;
			if (i > 0 && compareMoves(move, sp->moves[i - 1])) {
				condition = False;
			}
		} while (!condition);
		if (hideshuffling)
			evalmovement(mi, move);
		sp->moves[i] = move;
	}
	sp->VX = 0.05;
	if (NRAND(100) < 50)
		sp->VX *= -1;
	sp->VY = 0.05;
	if (NRAND(100) < 50)
		sp->VY *= -1;
	sp->movement.face = NO_FACE;
	sp->rotateStep = 0;
	sp->action = hideshuffling ? ACTION_SOLVE : ACTION_SHUFFLE;
	sp->shufflingmoves = 0;
	sp->done = 0;
	return True;
}

static void
reshape_skewb(ModeInfo * mi, int width, int height)
{
	skewbstruct *sp = &skewb[MI_SCREEN(mi)];

	glViewport(0, 0, sp->WindW = (GLint) width, sp->WindH = (GLint) height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-1.0, 1.0, -1.0, 1.0, 5.0, 15.0);
	glMatrixMode(GL_MODELVIEW);

	sp->AreObjectsDefined[ObjFacit] = False;
	sp->AreObjectsDefined[ObjCubit] = False;
}

#ifdef STANDALONE
ENTRYPOINT Bool
skewb_handle_event (ModeInfo *mi, XEvent *event)
{
	skewbstruct *sp = &skewb[MI_SCREEN(mi)];

	if (gltrackball_event_handler (event, sp->trackball,
			MI_WIDTH (mi), MI_HEIGHT (mi),
			&sp->button_down_p))
		return True;
	else if (screenhack_event_helper (MI_DISPLAY(mi), MI_WINDOW(mi), event)) {
		sp->done = 1;
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
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_CULL_FACE);

	glShadeModel(GL_FLAT);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, front_shininess);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, front_specular);

	return (shuffle(mi));
}

static void
free_skewb_screen(skewbstruct *sp)
{
	if (sp == NULL) {
		return;
	}
#ifdef STANDALONE
	if (sp->trackball)
		gltrackball_free (sp->trackball);
#endif
	if (sp->moves != NULL) {
		free(sp->moves);
		sp->moves = (SkewbMove *) NULL;
	}
	sp = NULL;
}

ENTRYPOINT void
release_skewb(ModeInfo * mi)
{
	if (skewb != NULL) {
		int	 screen;

		for (screen = 0; screen < MI_NUM_SCREENS(mi); screen++)
			free_skewb_screen(&skewb[screen]);
		free(skewb);
		skewb = (skewbstruct *) NULL;
	}
	FreeAllGL(mi);
}

ENTRYPOINT void
init_skewb(ModeInfo * mi)
{
	skewbstruct *sp;

	MI_INIT(mi, skewb);
	sp = &skewb[MI_SCREEN(mi)];

	sp->step = NRAND(180);
	sp->PX = ((float) LRAND() / (float) MAXRAND) * 2.0 - 1.0;
	sp->PY = ((float) LRAND() / (float) MAXRAND) * 2.0 - 1.0;

#ifdef STANDALONE
	sp->trackball = gltrackball_init (True);
#endif

	if ((sp->glx_context = init_GL(mi)) != NULL) {
		reshape_skewb(mi, MI_WIDTH(mi), MI_HEIGHT(mi));
		glDrawBuffer(GL_BACK);
		if (!pinit(mi)) {
			free_skewb_screen(sp);
			if (MI_IS_VERBOSE(mi)) {
				 (void) fprintf(stderr,
					"Could not allocate memory for skewb\n");
			}
		}
	} else {
		MI_CLEARWINDOW(mi);
	}
}

ENTRYPOINT void
draw_skewb(ModeInfo * mi)
{
	Bool bounced = False;
	Display    *display = MI_DISPLAY(mi);
	Window      window = MI_WINDOW(mi);
	skewbstruct *sp;

	if (skewb == NULL)
		return;
	sp = &skewb[MI_SCREEN(mi)];
	if ((sp->storedmoves) && (sp->moves == NULL))
		return;
	if (!sp->glx_context)
		return;
	MI_IS_DRAWN(mi) = True;
#ifdef WIN32
	wglMakeCurrent(hdc, sp->glx_context);
#else
	glXMakeCurrent(display, window, *(sp->glx_context));
#endif
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	glTranslatef(0.0, 0.0, -10.0);

	sp->PX += sp->VX;
	sp->PY += sp->VY;

	if (sp->PY < -1) {
		sp->PY += (-1) - (sp->PY);
		sp->VY = -sp->VY;
		bounced = True;
	}
	if (sp->PY > 1) {
		sp->PY -= (sp->PY) - 1;
		sp->VY = -sp->VY;
		bounced = True;
	}
	if (sp->PX < -1) {
		sp->PX += (-1) - (sp->PX);
		sp->VX = -sp->VX;
		bounced = True;
	}
	if (sp->PX > 1) {
		sp->PX -= (sp->PX) - 1;
		sp->VX = -sp->VX;
		bounced = True;
	}
	if (bounced) {
		sp->VX += ((float) LRAND() / (float) MAXRAND) * 0.02 - 0.01;
		sp->VX += ((float) LRAND() / (float) MAXRAND) * 0.02 - 0.01;
		if (sp->VX > 0.06)
			sp->VX = 0.06;
		if (sp->VY > 0.06)
			sp->VY = 0.06;
		if (sp->VX < -0.06)
			sp->VX = -0.06;
		if (sp->VY < -0.06)
			sp->VY = -0.06;
	}
	if (!MI_IS_ICONIC(mi)) {
		glTranslatef(sp->PX, sp->PY, 0);
		glScalef(Scale4Window * sp->WindH / sp->WindW,
			Scale4Window, Scale4Window);
	} else {
		glScalef(Scale4Iconic * sp->WindH / sp->WindW,
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
	gltrackball_rotate (sp->trackball);
#endif

	glRotatef(sp->step * 100, 1, 0, 0);
	glRotatef(sp->step * 95, 0, 1, 0);
	glRotatef(sp->step * 90, 0, 0, 1);
	drawCube(mi);
	if (MI_IS_FPS(mi)) {
		do_fps (mi);
	}
	glXSwapBuffers(display, window);
	if (sp->action == ACTION_SHUFFLE) {
		if (sp->done) {
			if (++sp->rotateStep > DELAY_AFTER_SHUFFLING) {
				sp->movement.face = NO_FACE;
				sp->rotateStep = 0;
				sp->action = ACTION_SOLVE;
				sp->done = 0;
			}
		} else {
			if (sp->movement.face == NO_FACE) {
				if (sp->shufflingmoves < sp->storedmoves) {
					sp->rotateStep = 0;
					sp->movement = sp->moves[sp->shufflingmoves];
				} else {
					sp->rotateStep = 0;
					sp->done = 1;
				}
			} else {
				if (sp->rotateStep == 0) {
					;
				}
				sp->rotateStep += sp->angleStep;
				if (sp->rotateStep > 120) {
					evalmovement(mi, sp->movement);
					sp->shufflingmoves++;
					sp->movement.face = NO_FACE;
				}
			}
		}
	} else {
		if (sp->done) {
			if (++sp->rotateStep > DELAY_AFTER_SOLVING)
				if (!shuffle(mi)) {
					free_skewb_screen(sp);
					if (MI_IS_VERBOSE(mi)) {
						 (void) fprintf(stderr,
							"Could not allocate memory for skewb\n");
					}
				}
		} else {
			if (sp->movement.face == NO_FACE) {
				if (sp->storedmoves > 0) {
					sp->rotateStep = 0;
					sp->movement = sp->moves[sp->storedmoves - 1];
					sp->movement.direction = (sp->movement.direction +
						(MAX_ORIENT / 2)) % MAX_ORIENT;
				} else {
					sp->rotateStep = 0;
					sp->done = 1;
				}
			} else {
				if (sp->rotateStep == 0) {
					;
				}
				sp->rotateStep += sp->angleStep;
				if (sp->rotateStep > 120) {
					evalmovement(mi, sp->movement);
					sp->storedmoves--;
					sp->movement.face = NO_FACE;
				}
			}
		}
	}

	glPopMatrix();

	glFlush();

	sp->step += 0.05;
}

#ifndef STANDALONE
ENTRYPOINT void
change_skewb(ModeInfo * mi)
{
	skewbstruct *sp;

	if (skewb == NULL)
		return;
	sp = &skewb[MI_SCREEN(mi)];

	if (!sp->glx_context)
		return;
	if (!pinit(mi)) {
		free_skewb_screen(sp);
		if (MI_IS_VERBOSE(mi)) {
			 (void) fprintf(stderr,
				"Could not allocate memory for skewb\n");
		}
	}
}
#endif

XSCREENSAVER_MODULE ("Skewb", skewb)

#endif /* MODE_skewb */
