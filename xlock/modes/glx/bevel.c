/* -*- Mode: C; tab-width: 4 -*- */
/* bevel --- Shows an auto-solving Bevel */

#if 0
static const char sccsid[] = "@(#)bevel.c	5.73 2023/10/23 xlockmore";
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
 * This mode shows an auto-solving a bevel "puzzle" or Helicopter Cube.
 * (I liked the shorter name for typing  :) ).
 *
 * Thanks goes also to Brian Paul for making it possible and inexpensive
 * to use OpenGL at home.
 *
 * Based on rubik.c by Marcelo F. Vianna
 *
 * Revision History:
 * 23-Oct-2023: Started writing, using xlock skewb.c and BevelGL.c from
                xpuzzles (currently unreleased).  The physical puzzle
                is more popularly known as the Helicopter Cube.
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
 *        | RED  |
 *        |2    1|
 * +------+------+------+
 * |3    0|3    0|3    0|
 * |      |      |      |
 * | LEFT |FRONT |RIGHT |
 * | (1)  | (2)  | (3)  |
 * |YELLOW| BLUE |ORANGE|
 * |2    1|2    1|2    1|
 * +------+------+------+
 *        |3    0|
 *        |      |
 *        |BOTTOM|
 *        | (4)  |
 *        |WHITE |
 *        |2    1|
 *        +------+       +-------+
 *        |3    0|       |3 /|\ 0|
 *        |      |       | / | \ |
 *        | BACK |       |/ 7|4 \|
 *        | (5)  |       |-------|
 *        |GREEN |       |\ 6|5 /|
 *        |2    1|       | \ | / |
 *        +------+       |2 \|/ 1|
 *                       +-------+
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
# define MODE_bevel
# define DEFAULTS	"*delay: 100000 \n" \
			"*count: -30 \n" \
			"*cycles: 5 \n" \

# define free_bevel 0
# include "xlockmore.h"		/* from the xscreensaver distribution */
# include "gltrackball.h"

#else /* !STANDALONE */
#include "xlock.h"		/* from the xlockmore distribution */
#include "visgl.h"
#endif /* !STANDALONE */

#ifdef MODE_bevel

#include <GL/gl.h>
#define DEF_HIDESHUFFLING     "False"

static Bool hideshuffling;

static XrmOptionDescRec opts[] =
{
	{(char *) "-hideshuffling", (char *) ".bevel.hideshuffling", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+hideshuffling", (char *) ".bevel.hideshuffling", XrmoptionNoArg, (caddr_t) "off"}
};

static argtype vars[] =
{
	{(void *) & hideshuffling, (char *) "hideshuffling", (char *) "Hideshuffling", (char *) DEF_HIDESHUFFLING, t_Bool}
};

static OptionStruct desc[] =
{
	{(char *) "-/+hideshuffling", (char *) "turn on/off hidden shuffle phase"}
};

ENTRYPOINT ModeSpecOpt bevel_opts =
{sizeof opts / sizeof opts[0], opts, sizeof vars / sizeof vars[0], vars, desc};

#ifdef USE_MODULES
ModStruct   bevel_description =
{"bevel", "init_bevel", "draw_bevel", "release_bevel",
 "draw_bevel", "change_bevel", "(char *) NULL", &bevel_opts,
 100000, -30, 5, 1, 64, 1.0, "",
 "Shows an auto-solving Bevel or Helicopter Cube", 0, NULL};

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
#define MAX_ROTATE 2
#define MAX_CUBES (2*MAX_ORIENT)
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
#define NO_EDGE 8
#define NO_ROTATION (2*MAX_ORIENT)

#define CUBE_LENGTH 0.5
#define CUBE_ROUND2 0.61
#define CUBE_ROUND (CUBE_LENGTH-0.02)
#define STICKER_LONG (CUBE_ROUND-0.05)
#define STICKER_SHORT (STICKER_LONG-0.05)
#define STICKER_DEPTH (CUBE_LENGTH+0.01)
#define FACELEN (2.0*CUBE_LENGTH)
#define FACEROUND (2.0*CUBE_LENGTH-0.06)
#define FSTICKER_LONG (FACEROUND-0.05)
#define FSTICKER_SHORT (FSTICKER_LONG-0.05)
#define FSTICKER_DEPTH (FACELEN+0.01)
#define CUT (float) 0.04
#define CUT2 (float) 0.02
#define CUT_DEPTH (STICKER_DEPTH+(float) 0.001)
#define TENTH (float) 0.1

#define ObjCubit        0
#define ObjFacit        1
#define MaxObj          2

typedef struct _BevelLoc {
	int         face;
	int         rotation;  /* Not used yet */
} BevelLoc;

typedef struct _BevelLocPos {
        int         face, position, direction;
} BevelLocPos;

typedef struct _RowNext {
        int         face, direction, sideFace;
} RowNext;

typedef struct _BevelMove {
        int         face, direction;
        int         position;
} BevelMove;

typedef struct _BevelSlice {
        int         edge, rotation;
} BevelSlice;


static BevelLoc slideNextFace[MAX_FACES][MAX_ORIENT] =
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

static int nextFace[MAX_FACES][MAX_ORIENT] =
{
	{-4, -1, 4, 1},
	{-1, 2, 1, -2},
	{-4, 2, 4, -2},
	{1, 2, -1, -2},
	{-4, 1, 4, -1},
	{-4, -2, 4, 2}
};

static BevelLocPos matchFaceSide[MAX_FACES][MAX_ORIENT] =
{
	{
		{5, 2, HALF},
		{3, 0, CCW},
		{2, 0, HALF},
		{1, 0, CW}
	},
	{
		{0, 3, CCW},
		{2, 3, HALF},
		{4, 3, CW},
		{5, 3, STRT}
	},
	{
		{0, 2, HALF},
		{3, 3, CCW},
		{4, 0, HALF},
		{1, 1, CW}
	},
	{
		{0, 1, CCW},
		{5, 1, HALF},
		{4, 1, CW},
		{2, 1, STRT}
	},
	{
		{2, 2, HALF},
		{3, 2, CW},
		{5, 0, HALF},
		{1, 2, CCW}
	},
	{
		{4, 2, HALF},
		{3, 1, STRT},
		{0, 0, HALF},
		{1, 3, STRT}
	}
};

static BevelLocPos majToMin[MAX_FACES][MAX_ORIENT] =
{			       /* other equivalent mappings possible */
	{
		{3, 0, 3},
		{2, 0, 1},
		{1, 0, 3},
		{5, 2, 3}
	},
	{
		{2, 3, 2},
		{4, 3, 2},
		{5, 3, 2},
		{0, 3, 2}
	},
	{
		{3, 3, 0},
		{4, 0, 1},
		{1, 1, 2},
		{0, 2, 3}
	},
	{
		{5, 1, 0},
		{4, 1, 0},
		{2, 1, 0},
		{0, 1, 0}
	},
	{
		{3, 2, 1},
		{5, 0, 1},
		{1, 2, 1},
		{2, 2, 3}
	},
	{
		{3, 1, 2},
		{0, 0, 1},
		{1, 3, 0},
		{4, 2, 3}
	}
};

typedef struct {
#ifdef WIN32
	HGLRC       glx_context;
#else
	GLXContext *glx_context;
#endif
	GLint       WindH, WindW;
	GLfloat     step;
	BevelMove  *moves;
	int         storedmoves;
	int         shufflingmoves;
	int         action;
	int         done;
	GLfloat     angleStep;
	BevelLoc    cubeLoc[MAX_FACES][MAX_CUBES];
	BevelLoc    rowLoc[MAX_ORIENT][MAX_CUBES];
	BevelLoc    minorLoc[MAX_ROTATE], majorLoc[MAX_ROTATE][MAX_CUBES];
	BevelMove   movement;
	GLfloat     rotateStep;
	GLfloat     PX, PY, VX, VY;
	Bool        AreObjectsDefined[2];
#ifdef STANDALONE
	Bool button_down_p;
	trackball_state *trackball;
#endif
} bevelstruct;

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
static float MaterialOrange[] =
{0.9, 0.5, 0.0, 1.0};
static float MaterialYellow[] =
{0.7, 0.7, 0.0, 1.0};
static float MaterialGreen[] =
{0.0, 0.5, 0.0, 1.0};

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

static bevelstruct *bevel = (bevelstruct *) NULL;

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
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialYellow);
		break;
	case FRONT_FACE:
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialBlue);
		break;
	case RIGHT_FACE:
		if (mono)
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray4);
		else
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialOrange);
		break;
	case BOTTOM_FACE:
		if (mono)
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray7);
		else
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialWhite);
		break;
	case BACK_FACE:
		if (mono)
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray6);
		else
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGreen);
		break;
}
}

static void
drawOuterStickerlessCubit(void)
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
	glVertex3f(-CUBE_ROUND, CUBE_ROUND, CUBE_LENGTH);
	glVertex3f(-CUBE_ROUND, CUBE_LENGTH, CUBE_ROUND);
	glVertex3f(CUBE_ROUND, CUBE_LENGTH, -CUBE_ROUND);
	glVertex3f(CUBE_LENGTH, CUBE_ROUND, -CUBE_ROUND);
	glVertex3f(CUBE_LENGTH, -CUBE_ROUND, CUBE_ROUND);
	glVertex3f(CUBE_ROUND, -CUBE_ROUND, CUBE_LENGTH);
	glEnd();
}

static void
drawInnerStickerlessCubit(void)
{
	glBegin(GL_TRIANGLES);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray);
	glNormal3f(0.0, 0.0, 1.0);
	glVertex3f(FACEROUND, CUT2, FACELEN);
	glVertex3f(CUT2, FACEROUND, FACELEN);
	glVertex3f(CUT2, CUT2, FACELEN);

	glVertex3f(FACEROUND, CUT2, FACELEN);
	glVertex3f(CUT2, FACEROUND, FACELEN);
	glVertex3f(CUT2, CUT2, CUBE_ROUND);

	glVertex3f(CUT2, FACEROUND, FACELEN);
	glVertex3f(CUT2, CUT2, FACELEN);
	glVertex3f(CUT2, CUT2, CUBE_ROUND);

	glVertex3f(CUT2, CUT2, FACELEN);
	glVertex3f(FACEROUND, CUT2, FACELEN);
	glVertex3f(CUT2, CUT2, CUBE_ROUND);
	glEnd();
}

static void
drawInnerCubit(ModeInfo * mi, int face, int position)
{
	bevelstruct *bp = &bevel[MI_SCREEN(mi)];
	int      mono = MI_IS_MONO(mi);

	pickColor(bp->cubeLoc[face][position + MAX_ORIENT].face, mono);
#ifdef DEBUG
		(void) printf("face=%d position=%d\n", face, position);
#endif
	switch (face) {
	case TOP_FACE:
		switch (position) {
		case 0:
			glBegin(GL_TRIANGLES);
			glNormal3f(0.0, 1.0, 0.0);
			glVertex3f(FSTICKER_LONG, FSTICKER_DEPTH, -CUT);
			glVertex3f(CUT, FSTICKER_DEPTH, -FSTICKER_LONG);
			glVertex3f(CUT, FSTICKER_DEPTH, -CUT);
			glEnd();
			break;
		case 1:
			glBegin(GL_TRIANGLES);
			glNormal3f(0.0, 1.0, 0.0);
			glVertex3f(CUT, FSTICKER_DEPTH, FSTICKER_LONG);
			glVertex3f(FSTICKER_LONG, FSTICKER_DEPTH, CUT);
			glVertex3f(CUT, FSTICKER_DEPTH, CUT);
			glEnd();
			break;
		case 2:
			glBegin(GL_TRIANGLES);
			glNormal3f(0.0, 1.0, 0.0);
			glVertex3f(-FSTICKER_LONG, FSTICKER_DEPTH, CUT);
			glVertex3f(-CUT, FSTICKER_DEPTH, FSTICKER_LONG);
			glVertex3f(-CUT, FSTICKER_DEPTH, CUT);
			glEnd();
			break;
		case 3:
			glBegin(GL_TRIANGLES);
			glNormal3f(0.0, 1.0, 0.0);
			glVertex3f(-CUT, FSTICKER_DEPTH, -FSTICKER_LONG);
			glVertex3f(-FSTICKER_LONG, FSTICKER_DEPTH, -CUT);
			glVertex3f(-CUT, FSTICKER_DEPTH, -CUT);
			glEnd();
			break;
		}
		break;
	case LEFT_FACE:
		switch (position) {
		case 0:
			glBegin(GL_TRIANGLES);
			glNormal3f(-1.0, 0.0, 0.0);
			glVertex3f(-FSTICKER_DEPTH, CUT, FSTICKER_LONG);
			glVertex3f(-FSTICKER_DEPTH, FSTICKER_LONG, CUT);
			glVertex3f(-FSTICKER_DEPTH, CUT, CUT);
			glEnd();
			break;
		case 1:
			glBegin(GL_TRIANGLES);
			glNormal3f(-1.0, 0.0, 0.0);
			glVertex3f(-FSTICKER_DEPTH, -FSTICKER_LONG, CUT);
			glVertex3f(-FSTICKER_DEPTH, -CUT, FSTICKER_LONG);
			glVertex3f(-FSTICKER_DEPTH, -CUT, CUT);
			glEnd();
			break;
		case 2:
			glBegin(GL_TRIANGLES);
			glNormal3f(-1.0, 0.0, 0.0);
			glVertex3f(-FSTICKER_DEPTH, -CUT, -FSTICKER_LONG);
			glVertex3f(-FSTICKER_DEPTH, -FSTICKER_LONG, -CUT);
			glVertex3f(-FSTICKER_DEPTH, -CUT, -CUT);
			glEnd();
			break;
		case 3:
			glBegin(GL_TRIANGLES);
			glNormal3f(-1.0, 0.0, 0.0);
			glVertex3f(-FSTICKER_DEPTH, FSTICKER_LONG, -CUT);
			glVertex3f(-FSTICKER_DEPTH, CUT, -FSTICKER_LONG);
			glVertex3f(-FSTICKER_DEPTH, CUT, -CUT);
			glEnd();
			break;
		}
		break;
	case FRONT_FACE:
		switch (position) {
		case 0:
			glBegin(GL_TRIANGLES);
			glNormal3f(0.0, 0.0, 1.0);
			glVertex3f(FSTICKER_LONG, CUT, FSTICKER_DEPTH);
			glVertex3f(CUT, FSTICKER_LONG, FSTICKER_DEPTH);
			glVertex3f(CUT, CUT, FSTICKER_DEPTH);
			glEnd();
			break;
		case 1:
			glBegin(GL_TRIANGLES);
			glNormal3f(0.0, 0.0, 1.0);
			glVertex3f(CUT, -FSTICKER_LONG, FSTICKER_DEPTH);
			glVertex3f(FSTICKER_LONG, -CUT, FSTICKER_DEPTH);
			glVertex3f(CUT, -CUT, FSTICKER_DEPTH);
			glEnd();
			break;
		case 2:
			glBegin(GL_TRIANGLES);
			glNormal3f(0.0, 0.0, 1.0);
			glVertex3f(-FSTICKER_LONG, -CUT, FSTICKER_DEPTH);
			glVertex3f(-CUT, -FSTICKER_LONG, FSTICKER_DEPTH);
			glVertex3f(-CUT, -CUT, FSTICKER_DEPTH);
			glEnd();
			break;
		case 3:
			glBegin(GL_TRIANGLES);
			glNormal3f(0.0, 0.0, 1.0);
			glVertex3f(-CUT, FSTICKER_LONG, FSTICKER_DEPTH);
			glVertex3f(-FSTICKER_LONG, CUT, FSTICKER_DEPTH);
			glVertex3f(-CUT, CUT, FSTICKER_DEPTH);
			glEnd();
			break;
		}
		break;
	case RIGHT_FACE:
		switch (position) {
		case 0:
			glBegin(GL_TRIANGLES);
			glNormal3f(1.0, 0.0, 0.0);
			glVertex3f(FSTICKER_DEPTH, CUT, -FSTICKER_LONG);
			glVertex3f(FSTICKER_DEPTH, FSTICKER_LONG, -CUT);
			glVertex3f(FSTICKER_DEPTH, CUT, -CUT);
			glEnd();
			break;
		case 1:
			glBegin(GL_TRIANGLES);
			glNormal3f(1.0, 0.0, 0.0);
			glVertex3f(FSTICKER_DEPTH, -FSTICKER_LONG, -CUT);
			glVertex3f(FSTICKER_DEPTH, -CUT, -FSTICKER_LONG);
			glVertex3f(FSTICKER_DEPTH, -CUT, -CUT);
			glEnd();
			break;
		case 2:
			glBegin(GL_TRIANGLES);
			glNormal3f(1.0, 0.0, 0.0);
			glVertex3f(FSTICKER_DEPTH, -CUT, FSTICKER_LONG);
			glVertex3f(FSTICKER_DEPTH, -FSTICKER_LONG, CUT);
			glVertex3f(FSTICKER_DEPTH, -CUT, CUT);
			glEnd();
			break;
		case 3:
			glBegin(GL_TRIANGLES);
			glNormal3f(1.0, 0.0, 0.0);
			glVertex3f(FSTICKER_DEPTH, FSTICKER_LONG, CUT);
			glVertex3f(FSTICKER_DEPTH, CUT, FSTICKER_LONG);
			glVertex3f(FSTICKER_DEPTH, CUT, CUT);
			glEnd();
			break;
		}
		break;
	case BOTTOM_FACE:
		switch (position) {
		case 0:
			glBegin(GL_TRIANGLES);
			glNormal3f(0.0, -1.0, 0.0);
			glVertex3f(FSTICKER_LONG, -FSTICKER_DEPTH, CUT);
			glVertex3f(CUT, -FSTICKER_DEPTH, FSTICKER_LONG);
			glVertex3f(CUT, -FSTICKER_DEPTH, CUT);
			glEnd();
			break;
		case 1:
			glBegin(GL_TRIANGLES);
			glNormal3f(0.0, -1.0, 0.0);
			glVertex3f(CUT, -FSTICKER_DEPTH, -FSTICKER_LONG);
			glVertex3f(FSTICKER_LONG, -FSTICKER_DEPTH, -CUT);
			glVertex3f(CUT, -FSTICKER_DEPTH, -CUT);
			glEnd();
			break;
		case 2:
			glBegin(GL_TRIANGLES);
			glNormal3f(0.0, -1.0, 0.0);
			glVertex3f(-FSTICKER_LONG, -FSTICKER_DEPTH, -CUT);
			glVertex3f(-CUT, -FSTICKER_DEPTH, -FSTICKER_LONG);
			glVertex3f(-CUT, -FSTICKER_DEPTH, -CUT);
			glEnd();
			break;
		case 3:
			glBegin(GL_TRIANGLES);
			glNormal3f(0.0, -1.0, 0.0);
			glVertex3f(-CUT, -FSTICKER_DEPTH, FSTICKER_LONG);
			glVertex3f(-FSTICKER_LONG, -FSTICKER_DEPTH, CUT);
			glVertex3f(-CUT, -FSTICKER_DEPTH, CUT);
			glEnd();
			break;
		}
		break;
	case BACK_FACE:
		switch (position) {
		case 0:
			glBegin(GL_TRIANGLES);
			glNormal3f(0.0, 0.0, -1.0);
			glVertex3f(FSTICKER_LONG, -CUT, -FSTICKER_DEPTH);
			glVertex3f(CUT, -FSTICKER_LONG, -FSTICKER_DEPTH);
			glVertex3f(CUT, -CUT, -FSTICKER_DEPTH);
			glEnd();
			break;
		case 1:
			glBegin(GL_TRIANGLES);
			glNormal3f(0.0, 0.0, -1.0);
			glVertex3f(CUT, FSTICKER_LONG, -FSTICKER_DEPTH);
			glVertex3f(FSTICKER_LONG, CUT, -FSTICKER_DEPTH);
			glVertex3f(CUT, CUT, -FSTICKER_DEPTH);
			glEnd();
			break;
		case 2:
			glBegin(GL_TRIANGLES);
			glNormal3f(0.0, 0.0, -1.0);
			glVertex3f(-FSTICKER_LONG, CUT, -FSTICKER_DEPTH);
			glVertex3f(-CUT, FSTICKER_LONG, -FSTICKER_DEPTH);
			glVertex3f(-CUT, CUT, -FSTICKER_DEPTH);
			glEnd();
			break;
		case 3:
			glBegin(GL_TRIANGLES);
			glNormal3f(0.0, 0.0, -1.0);
			glVertex3f(-CUT, -FSTICKER_LONG, -FSTICKER_DEPTH);
			glVertex3f(-FSTICKER_LONG, -CUT, -FSTICKER_DEPTH);
			glVertex3f(-CUT, -CUT, -FSTICKER_DEPTH);
			glEnd();
			break;
		}
		break;
	}
}

/* so puzzle will not fall apart */
static void
drawCenter(void)
{
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);
	glVertex3f(-CUBE_ROUND2, -CUBE_ROUND2, CUBE_ROUND2);
	glVertex3f(CUBE_ROUND2, -CUBE_ROUND2, CUBE_ROUND2);
	glVertex3f(CUBE_ROUND2, CUBE_ROUND2, CUBE_ROUND2);
	glVertex3f(-CUBE_ROUND2, CUBE_ROUND2, CUBE_ROUND2);
	glNormal3f(0.0, 0.0, -1.0);
	glVertex3f(-CUBE_ROUND2, CUBE_ROUND2, -CUBE_ROUND2);
	glVertex3f(CUBE_ROUND2, CUBE_ROUND2, -CUBE_ROUND2);
	glVertex3f(CUBE_ROUND2, -CUBE_ROUND2, -CUBE_ROUND2);
	glVertex3f(-CUBE_ROUND2, -CUBE_ROUND2, -CUBE_ROUND2);
	glNormal3f(-1.0, 0.0, 0.0);
	glVertex3f(-CUBE_ROUND2, -CUBE_ROUND2, CUBE_ROUND2);
	glVertex3f(-CUBE_ROUND2, CUBE_ROUND2, CUBE_ROUND2);
	glVertex3f(-CUBE_ROUND2, CUBE_ROUND2, -CUBE_ROUND2);
	glVertex3f(-CUBE_ROUND2, -CUBE_ROUND2, -CUBE_ROUND2);
	glNormal3f(1.0, 0.0, 0.0);
	glVertex3f(CUBE_ROUND2, -CUBE_ROUND2, -CUBE_ROUND2);
	glVertex3f(CUBE_ROUND2, CUBE_ROUND2, -CUBE_ROUND2);
	glVertex3f(CUBE_ROUND2, CUBE_ROUND2, CUBE_ROUND2);
	glVertex3f(CUBE_ROUND2, -CUBE_ROUND2, CUBE_ROUND2);
	glNormal3f(0.0, -1.0, 0.0);
	glVertex3f(CUBE_ROUND2, -CUBE_ROUND2, -CUBE_ROUND2);
	glVertex3f(CUBE_ROUND2, -CUBE_ROUND2, CUBE_ROUND2);
	glVertex3f(-CUBE_ROUND2, -CUBE_ROUND2, CUBE_ROUND2);
	glVertex3f(-CUBE_ROUND2, -CUBE_ROUND2, -CUBE_ROUND2);
	glNormal3f(0.0, 1.0, 0.0);
	glVertex3f(-CUBE_ROUND2, CUBE_ROUND2, -CUBE_ROUND2);
	glVertex3f(-CUBE_ROUND2, CUBE_ROUND2, CUBE_ROUND2);
	glVertex3f(CUBE_ROUND2, CUBE_ROUND2, CUBE_ROUND2);
	glVertex3f(CUBE_ROUND2, CUBE_ROUND2, -CUBE_ROUND2);
	glEnd();
}

static void
drawOuterCubit(ModeInfo * mi,
		int back, int front, int left, int right, int bottom, int top)
{
	int      mono = MI_IS_MONO(mi);

	if (back != NO_FACE) { /* Green */
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
	if (front != NO_FACE) { /* Blue */
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
	if (left != NO_FACE) { /* Yellow */
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
	if (right != NO_FACE) { /* Orange */
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
	if (bottom != NO_FACE) { /* White */
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
	if (top != NO_FACE) { /* Red */
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
drawWholeInnerCubit(ModeInfo * mi, int x, int y, int z, int x1, int y1, int z1)
{
	if (x < 0) {
		if (y1 > 0 && z1 > 0) {
			/* LEFT_FACE, Yellow, 1 (red blue) */
			glPushMatrix();
			glRotatef(-90.0, 0, 1, 0);
			drawInnerStickerlessCubit();
			glPopMatrix();
			glPushMatrix();
			drawInnerCubit(mi, 1, 0);
			glPopMatrix();
		} else if (y1 < 0 && z1 > 0) {
			/* LEFT_FACE, Yellow, 1 (white blue)*/
			glPushMatrix();
			glRotatef(-90.0, 0, 1, 0);
			glRotatef(-90.0, 0, 0, 1);
			drawInnerStickerlessCubit();
			glPopMatrix();
			glPushMatrix();
			drawInnerCubit(mi, 1, 1);
			glPopMatrix();
		} else if (y1 < 0 && z1 < 0) {
			/* LEFT_FACE, Yellow, 1 (white green)*/
			glPushMatrix();
			glRotatef(-90.0, 0, 1, 0);
			glRotatef(180.0, 0, 0, 1);
			drawInnerStickerlessCubit();
			glPopMatrix();
			glPushMatrix();
			drawInnerCubit(mi, 1, 2);
			glPopMatrix();
		} else if (y1 > 0 && z1 < 0) {
			/* LEFT_FACE, Yellow, 1 (red green)*/
			glPushMatrix();
			glRotatef(-90.0, 0, 1, 0);
			glRotatef(90.0, 0, 0, 1);
			drawInnerStickerlessCubit();
			glPopMatrix();
			glPushMatrix();
			drawInnerCubit(mi, 1, 3);
			glPopMatrix();
		}
	} else if (x > 0) {
		if (y1 > 0 && z1 < 0) {
			 /* RIGHT_FACE, Orange, 3 (green red)*/
			glPushMatrix();
			glRotatef(90.0, 0, 1, 0);
			drawInnerStickerlessCubit();
			glPopMatrix();
			glPushMatrix();
			drawInnerCubit(mi, 3, 0);
			glPopMatrix();
		} else if (y1 < 0 && z1 < 0) {
			 /* RIGHT_FACE, Orange, 3 (green white)*/
			glPushMatrix();
			glRotatef(90.0, 0, 1, 0);
			glRotatef(-90.0, 0, 0, 1);
			drawInnerStickerlessCubit();
			glPopMatrix();
			glPushMatrix();
			drawInnerCubit(mi, 3, 1);
			glPopMatrix();
		} else if (y1 < 0 && z1 > 0) {
			 /* RIGHT_FACE, Orange, 3 (blue white)*/
			glPushMatrix();
			glRotatef(90.0, 0, 1, 0);
			glRotatef(180.0, 0, 0, 1);
			drawInnerStickerlessCubit();
			glPopMatrix();
			glPushMatrix();
			drawInnerCubit(mi, 3, 2);
			glPopMatrix();
		} else if (y1 > 0 && z1 > 0) {
			 /* RIGHT_FACE, Orange, 3 (blue red)*/
			glPushMatrix();
			glRotatef(90.0, 0, 1, 0);
			glRotatef(90.0, 0, 0, 1);
			drawInnerStickerlessCubit();
			glPopMatrix();
			glPushMatrix();
			drawInnerCubit(mi, 3, 3);
			glPopMatrix();
		}
	} else if (y < 0) {
		if (x1 > 0 && z1 > 0) {
			/* BOTTOM_FACE, White 4 (blue orange)*/
			glPushMatrix();
			glRotatef(90.0, 1, 0, 0);
			drawInnerStickerlessCubit();
			glPopMatrix();
			glPushMatrix();
			drawInnerCubit(mi, 4, 0);
			glPopMatrix();
		} else if (x1 > 0 && z1 < 0) {
			/* BOTTOM_FACE, White 4 (green orange)*/
			glPushMatrix();
			glRotatef(90.0, 1, 0, 0);
			glRotatef(-90.0, 0, 0, 1);
			drawInnerStickerlessCubit();
			glPopMatrix();
			glPushMatrix();
			drawInnerCubit(mi, 4, 1);
			glPopMatrix();
		} else if (x1 < 0 && z1 < 0) {
			/* BOTTOM_FACE, White 4 (green yellow)*/
			glPushMatrix();
			glRotatef(90.0, 1, 0, 0);
			glRotatef(180.0, 0, 0, 1);
			drawInnerStickerlessCubit();
			glPopMatrix();
			glPushMatrix();
			drawInnerCubit(mi, 4, 2);
			glPopMatrix();
		} else if (x1 < 0 && z1 > 0) {
			/* BOTTOM_FACE, White 4 (blue yellow)*/
			glPushMatrix();
			glRotatef(90.0, 1, 0, 0);
			glRotatef(90.0, 0, 0, 1);
			drawInnerStickerlessCubit();
			glPopMatrix();
			glPushMatrix();
			drawInnerCubit(mi, 4, 3);
			glPopMatrix();
		}
	} else if (y > 0) {
		if (x1 > 0 && z1 < 0) {
			/* TOP_FACE, Red 0 (green orange)*/
			glPushMatrix();
			glRotatef(-90.0, 1, 0, 0);
			drawInnerStickerlessCubit();
			glPopMatrix();
			glPushMatrix();
			drawInnerCubit(mi, 0, 0);
			glPopMatrix();
		} else if (x1 > 0 && z1 > 0) {
			/* TOP_FACE, Red 0 (blue orange)*/
			glPushMatrix();
			glRotatef(-90.0, 1, 0, 0);
			glRotatef(-90.0, 0, 0, 1);
			drawInnerStickerlessCubit();
			glPopMatrix();
			glPushMatrix();
			drawInnerCubit(mi, 0, 1);
			glPopMatrix();
		} else if (x1 < 0 && z1 > 0) {
			/* TOP_FACE, Red 0 (blue yellow)*/
			glPushMatrix();
			glRotatef(-90.0, 1, 0, 0);
			glRotatef(180.0, 0, 0, 1);
			drawInnerStickerlessCubit();
			glPopMatrix();
			glPushMatrix();
			drawInnerCubit(mi, 0, 2);
			glPopMatrix();
		} else if (x1 < 0 && z1 < 0) {
			/* TOP_FACE, Red 0 (green yellow)*/
			glPushMatrix();
			glRotatef(-90.0, 1, 0, 0);
			glRotatef(90.0, 0, 0, 1);
			drawInnerStickerlessCubit();
			glPopMatrix();
			glPushMatrix();
			drawInnerCubit(mi, 0, 3);
			glPopMatrix();
		}
	} else if (z < 0) {
		if (x1 > 0 && y1 < 0) {
			/* BACK_FACE, Green 5 (white orange)*/
			glPushMatrix();
			glRotatef(180.0, 0, 1, 0);
			glRotatef(180.0, 0, 0, 1);
			drawInnerStickerlessCubit();
			glPopMatrix();
			glPushMatrix();
			drawInnerCubit(mi, 5, 0);
			glPopMatrix();
		} else if (x1 > 0 && y1 > 0) {
			/* BACK_FACE, Green 5 (red orange)*/
			glPushMatrix();
			glRotatef(180.0, 0, 1, 0);
			glRotatef(90.0, 0, 0, 1);
			drawInnerStickerlessCubit();
			glPopMatrix();
			glPushMatrix();
			drawInnerCubit(mi, 5, 1);
			glPopMatrix();
		} else if (x1 < 0 && y1 > 0) {
			/* BACK_FACE, Green 5 (red yellow)*/
			glPushMatrix();
			glRotatef(180.0, 0, 2, 0);
			drawInnerStickerlessCubit();
			glPopMatrix();
			glPushMatrix();
			drawInnerCubit(mi, 5, 2);
			glPopMatrix();
		} else if (x1 < 0 && y1 < 0) {
			/* BACK_FACE, Green 5 (white yellow)*/
			glPushMatrix();
			glRotatef(180.0, 0, 1, 0);
			glRotatef(-90.0, 0, 0, 1);
			drawInnerStickerlessCubit();
			glPopMatrix();
			glPushMatrix();
			drawInnerCubit(mi, 5, 3);
			glPopMatrix();
		}
	} else if (z > 0) {
		if (x1 > 0 && y1 > 0) {
			/* FRONT_FACE, Blue, 2 (orange white)*/
			glPushMatrix();
			drawInnerStickerlessCubit();
			glPopMatrix();
			glPushMatrix();
			drawInnerCubit(mi, 2, 0);
			glPopMatrix();
		} else if (x1 > 0 && y1 < 0) {
			/* FRONT_FACE, Blue, 2 (orange red)*/
			glPushMatrix();
			glRotatef(-90.0, 0, 0, 1);
			drawInnerStickerlessCubit();
			glPopMatrix();
			glPushMatrix();
			drawInnerCubit(mi, 2, 1);
			glPopMatrix();
		} else if (x1 < 0 && y1 < 0) {
			/* FRONT_FACE, Blue, 2 (yellow red)*/
			glPushMatrix();
			glRotatef(180.0, 0, 0, 1);
			drawInnerStickerlessCubit();
			glPopMatrix();
			glPushMatrix();
			drawInnerCubit(mi, 2, 2);
			glPopMatrix();
		} else if (x1 < 0 && y1 > 0) {
			/* FRONT_FACE, Blue, 2 (yellow white)*/
			glPushMatrix();
			glRotatef(90.0, 0, 0, 1);
			drawInnerStickerlessCubit();
			glPopMatrix();
			glPushMatrix();
			drawInnerCubit(mi, 2, 3);
			glPopMatrix();
		}
	}
}

static void
drawWholeOuterCubit(ModeInfo * mi, int x, int y, int z)
{
	bevelstruct *bp = &bevel[MI_SCREEN(mi)];
	const float h = 0.5;

	if (x < 0) {
		if (y < 0) {
			if (z < 0) {
				glPushMatrix();
				glTranslatef(-h, -h, -h);
				glPushMatrix();
				glRotatef(90.0, 0, 1, 0);
				glRotatef(180.0, 1, 0, 0);
				drawOuterStickerlessCubit();
				glPopMatrix();
				drawOuterCubit(mi, /* BLL */
					bp->cubeLoc[BACK_FACE][3].face,
					NO_FACE,
					bp->cubeLoc[LEFT_FACE][2].face,
					NO_FACE,
					bp->cubeLoc[BOTTOM_FACE][2].face,
					NO_FACE);
				glPopMatrix();
			} else if (z > 0) {
				glPushMatrix();
				glTranslatef(-h, -h, h);
				glPushMatrix();
				glRotatef(180.0, 0, 0, 1);
				drawOuterStickerlessCubit();
				glPopMatrix();
				drawOuterCubit(mi, /* FLL */
					NO_FACE,
					bp->cubeLoc[FRONT_FACE][2].face,
					bp->cubeLoc[LEFT_FACE][1].face,
					NO_FACE,
					bp->cubeLoc[BOTTOM_FACE][3].face,
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
				drawOuterStickerlessCubit();
				glPopMatrix();
				drawOuterCubit(mi, /* BUL */
					bp->cubeLoc[BACK_FACE][2].face,
					NO_FACE,
					bp->cubeLoc[LEFT_FACE][3].face,
					NO_FACE,
					NO_FACE,
					bp->cubeLoc[TOP_FACE][3].face);
				glPopMatrix();
			} else if (z > 0) {
				glPushMatrix();
				glTranslatef(-h, h, h);
				glPushMatrix();
				glRotatef(90.0, 0, 0, 1);
				drawOuterStickerlessCubit();
				glPopMatrix();
				drawOuterCubit(mi, /* FUL */
					NO_FACE,
					bp->cubeLoc[FRONT_FACE][3].face,
					bp->cubeLoc[LEFT_FACE][0].face,
					NO_FACE,
					NO_FACE,
					bp->cubeLoc[TOP_FACE][2].face);
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
				drawOuterStickerlessCubit();
				glPopMatrix();
				drawOuterCubit(mi, /* BLR */
					bp->cubeLoc[BACK_FACE][0].face,
					NO_FACE,
					NO_FACE,
					bp->cubeLoc[RIGHT_FACE][1].face,
					bp->cubeLoc[BOTTOM_FACE][1].face,
					NO_FACE);
				glPopMatrix();
			} else if (z > 0) {
				glPushMatrix();
				glTranslatef(h, -h, h);
				glPushMatrix();
				glRotatef(90.0, 1, 0, 0);
				drawOuterStickerlessCubit();
				glPopMatrix();
				drawOuterCubit(mi, /* FLR */
					NO_FACE,
					bp->cubeLoc[FRONT_FACE][1].face,
					NO_FACE,
					bp->cubeLoc[RIGHT_FACE][2].face,
					bp->cubeLoc[BOTTOM_FACE][0].face,
					NO_FACE);
				glPopMatrix();
			}
		} else if (y > 0) {
			if (z < 0) {
				glPushMatrix();
				glTranslatef(h, h, -h);
				glPushMatrix();
				glRotatef(90.0, 0, 1, 0);
				drawOuterStickerlessCubit();
				glPopMatrix();
				drawOuterCubit(mi, /* BUR */
					bp->cubeLoc[BACK_FACE][1].face,
					NO_FACE,
					NO_FACE,
					bp->cubeLoc[RIGHT_FACE][0].face,
					NO_FACE,
					bp->cubeLoc[TOP_FACE][0].face);
				glPopMatrix();
			} else if (z > 0) {
				glPushMatrix();
				glTranslatef(h, h, h);
				glPushMatrix();
				drawOuterStickerlessCubit();
				glPopMatrix();
				drawOuterCubit(mi, /* FUR */
					NO_FACE,
					bp->cubeLoc[FRONT_FACE][0].face,
					NO_FACE,
					bp->cubeLoc[RIGHT_FACE][3].face,
					NO_FACE,
					bp->cubeLoc[TOP_FACE][1].face);
				glPopMatrix();
			}
		}
	}
}

static void
convertMove(BevelMove move, BevelSlice * slice)
{
	int position = move.position;

	switch (move.face) {
	case 0:
		if (move.direction % 2 == 0) {
			if (position == 0 || position == 1) {
				(*slice).edge = 11;
				(*slice).rotation = (move.direction == 2) ?
					CCW : CW;
			} else if (position == 2 || position == 3) {
				(*slice).edge = 9;
				(*slice).rotation = (move.direction == 0) ?
					CCW : CW;
			}
		} else {
			if (position == 0 || position == 3) {
				(*slice).edge = 2;
				(*slice).rotation = (move.direction == 1) ?
					CCW : CW;
			} else if (position == 1 || position == 2) {
				(*slice).edge = 3;
				(*slice).rotation = (move.direction == 3) ?
					CCW : CW;
			}
		}
		break;
	case 1:
		if (move.direction % 2 == 0) {
			if (position == 0 || position == 1) {
				(*slice).edge = 5;
				(*slice).rotation = (move.direction == 2) ?
					CCW : CW;
			} else if (position == 2 || position == 3) {
				(*slice).edge = 4;
				(*slice).rotation = (move.direction == 0) ?
					CCW : CW;
			}
		} else {
			if (position == 0 || position == 3) {
				(*slice).edge = 9;
				(*slice).rotation = (move.direction == 1) ?
					CCW : CW;
			} else if (position == 1 || position == 2) {
				(*slice).edge = 8;
				(*slice).rotation = (move.direction == 3) ?
					CCW : CW;
			}
		}
		break;
	case 2:
		if (move.direction % 2 == 0) {
			if (position == 0 || position == 1) {
				(*slice).edge = 7;
				(*slice).rotation = (move.direction == 2) ?
					CCW : CW;
			} else if (position == 2 || position == 3) {
				(*slice).edge = 5;
				(*slice).rotation = (move.direction == 0) ?
					CCW : CW;
			}
		} else {
			if (position == 0 || position == 3) {
				(*slice).edge = 3;
				(*slice).rotation = (move.direction == 1) ?
					CCW : CW;
			} else if (position == 1 || position == 2) {
				(*slice).edge = 1;
				(*slice).rotation = (move.direction == 3) ?
					CCW : CW;
			}
		}
		break;
	case 3:
		if (move.direction % 2 == 0) {
			if (position == 0 || position == 1) {
				(*slice).edge = 6;
				(*slice).rotation = (move.direction == 2) ?
					CCW : CW;
			} else if (position == 2 || position == 3) {
				(*slice).edge = 7;
				(*slice).rotation = (move.direction == 0) ?
					CCW : CW;
			}
		} else {
			if (position == 0 || position == 3) {
				(*slice).edge = 11;
				(*slice).rotation = (move.direction == 1) ?
					CCW : CW;
			} else if (position == 1 || position == 2) {
				(*slice).edge = 10;
				(*slice).rotation = (move.direction == 3) ?
					CCW : CW;
			}
		}
		break;
	case 4:
		if (move.direction % 2 == 0) {
			if (position == 0 || position == 1) {
				(*slice).edge = 10;
				(*slice).rotation = (move.direction == 2) ?
					CCW : CW;
			} else if (position == 2 || position == 3) {
				(*slice).edge = 8;
				(*slice).rotation = (move.direction == 0) ?
					CCW : CW;
			}
		} else {
			if (position == 0 || position == 3) {
				(*slice).edge = 1;
				(*slice).rotation = (move.direction == 1) ?
					CCW : CW;
			} else if (position == 1 || position == 2) {
				(*slice).edge = 0;
				(*slice).rotation = (move.direction == 3) ?
					CCW : CW;
			}
		}
		break;
	case 5:
		if (move.direction % 2 == 0) {
			if (position == 0 || position == 1) {
				(*slice).edge = 6;
				(*slice).rotation = (move.direction == 2) ?
					CCW : CW;
			} else if (position == 2 || position == 3) {
				(*slice).edge = 4;
				(*slice).rotation = (move.direction == 0) ?
					CCW : CW;
			}
		} else {
			if (position == 0 || position == 3) {
				(*slice).edge = 0;
				(*slice).rotation = (move.direction == 1) ?
					CCW : CW;
			} else if (position == 1 || position == 2) {
				(*slice).edge = 2;
				(*slice).rotation = (move.direction == 3) ?
					CCW : CW;
			}
		}
		break;
	}
}

/*-
 * If you want to rotate a slice, you can draw that at the angleStep,
  then the without angleStep.
 * There is a sequence for drawing cubies for each axis being moved...
 */
static void
edgeRotate(ModeInfo * mi, int edge, GLfloat rotateStep)
{ /* simiar to Dino period 2 */
	int Xf = 0, Yf = 0, Zf = 0;
	int X1 = 0, Y1 = 0, Z1 = 0;
	int Xn = 0, Yn = 0, Zn = 0;

	if (edge < 4) {
		Yf = (edge / 2 == 1) ? 1.0 : -1.0;
		Zf = (edge % 2 == 1) ? 1.0 : -1.0;
		X1 = -1;
		Xn = 1;
	} else if (edge >= 8) {
		Xf = ((edge % 4) / 2 == 1) ? 1.0 : -1.0;
		Yf = (edge % 2 == 1) ? 1.0 : -1.0;
		Z1 = -1;
		Zn = 1;
	} else {
		Xf = ((edge % 4) / 2 == 1) ? 1.0 : -1.0;
		Zf = (edge % 2 == 1) ? 1.0 : -1.0;
		Y1 = -1;
		Yn = 1;
	}
#ifdef DEBUG
	(void) printf("edge=%d  Xf=%d Yf=%d Zf=%d X1=%d Xn=%d Y1=%d Yn=%d Z1=%d Zn=%d\n",
		edge, Xf, Yf, Zf, X1, Xn, Y1, Yn, Z1, Zn);
#endif
	drawCenter();
	glPushMatrix();
	glRotatef(rotateStep, (float) Xf, (float) Yf, (float) Zf);
	/*drawEdge(mi, x, y, z);*/
	if (edge < 4) {
		drawWholeOuterCubit(mi, X1, Yf, Zf);
		drawWholeOuterCubit(mi, Xn, Yf, Zf);
		drawWholeInnerCubit(mi, 0, 0, Zf, X1, Yf, 0);
		drawWholeInnerCubit(mi, 0, 0, Zf, Xn, Yf, 0);
		drawWholeInnerCubit(mi, 0, Yf, 0, X1, 0, Zf);
		drawWholeInnerCubit(mi, 0, Yf, 0, Xn, 0, Zf);
	} else if (edge >= 8) {
		drawWholeOuterCubit(mi, Xf, Yf, Z1);
		drawWholeOuterCubit(mi, Xf, Yf, Zn);
		drawWholeInnerCubit(mi, Xf, 0, 0, 0, Yf, Z1);
		drawWholeInnerCubit(mi, Xf, 0, 0, 0, Yf, Zn);
		drawWholeInnerCubit(mi, 0, Yf, 0, Xf, 0, Z1);
		drawWholeInnerCubit(mi, 0, Yf, 0, Xf, 0, Zn);
	} else {
		drawWholeOuterCubit(mi, Xf, Y1, Zf);
		drawWholeOuterCubit(mi, Xf, Yn, Zf);
		drawWholeInnerCubit(mi, Xf, 0, 0, 0, Y1, Zf);
		drawWholeInnerCubit(mi, Xf, 0, 0, 0, Yn, Zf);
		drawWholeInnerCubit(mi, 0, 0, Zf, Xf, Y1, 0);
		drawWholeInnerCubit(mi, 0, 0, Zf, Xf, Yn, 0);
	}
	glPopMatrix();
	if (edge < 4) {
		drawWholeOuterCubit(mi, X1, -Yf, -Zf);
		drawWholeOuterCubit(mi, Xn, -Yf, -Zf);
		drawWholeOuterCubit(mi, X1, Yf, -Zf);
		drawWholeOuterCubit(mi, Xn, Yf, -Zf);
		drawWholeOuterCubit(mi, X1, -Yf, Zf);
		drawWholeOuterCubit(mi, Xn, -Yf, Zf);
		drawWholeInnerCubit(mi, 0, 0, -Zf, X1, -Yf, 0);
		drawWholeInnerCubit(mi, 0, 0, -Zf, Xn, -Yf, 0);
		drawWholeInnerCubit(mi, 0, -Yf, 0, X1, 0, -Zf);
		drawWholeInnerCubit(mi, 0, -Yf, 0, Xn, 0, -Zf);
		drawWholeInnerCubit(mi, 0, 0, -Zf, X1, Yf, 0);
		drawWholeInnerCubit(mi, 0, 0, -Zf, Xn, Yf, 0);
		drawWholeInnerCubit(mi, 0, Yf, 0, X1, 0, -Zf);
		drawWholeInnerCubit(mi, 0, Yf, 0, Xn, 0, -Zf);
		drawWholeInnerCubit(mi, 0, 0, Zf, X1, -Yf, 0);
		drawWholeInnerCubit(mi, 0, 0, Zf, Xn, -Yf, 0);
		drawWholeInnerCubit(mi, 0, -Yf, 0, X1, 0, Zf);
		drawWholeInnerCubit(mi, 0, -Yf, 0, Xn, 0, Zf);
		drawWholeInnerCubit(mi, X1, 0, 0, 0, Yf, Zf);
		drawWholeInnerCubit(mi, X1, 0, 0, 0, -Yf, Zf);
		drawWholeInnerCubit(mi, X1, 0, 0, 0, Yf, -Zf);
		drawWholeInnerCubit(mi, X1, 0, 0, 0, -Yf, -Zf);
		drawWholeInnerCubit(mi, Xn, 0, 0, 0, Yf, Zf);
		drawWholeInnerCubit(mi, Xn, 0, 0, 0, -Yf, Zf);
		drawWholeInnerCubit(mi, Xn, 0, 0, 0, Yf, -Zf);
		drawWholeInnerCubit(mi, Xn, 0, 0, 0, -Yf, -Zf);
	} else if (edge >= 8) {
		drawWholeOuterCubit(mi, -Xf, -Yf, Z1);
		drawWholeOuterCubit(mi, -Xf, -Yf, Zn);
		drawWholeOuterCubit(mi, Xf, -Yf, Z1);
		drawWholeOuterCubit(mi, Xf, -Yf, Zn);
		drawWholeOuterCubit(mi, -Xf, Yf, Z1);
		drawWholeOuterCubit(mi, -Xf, Yf, Zn);
		drawWholeInnerCubit(mi, -Xf, 0, 0, 0, -Yf, Z1);
		drawWholeInnerCubit(mi, -Xf, 0, 0, 0, -Yf, Zn);
		drawWholeInnerCubit(mi, 0, -Yf, 0, -Xf, 0, Z1);
		drawWholeInnerCubit(mi, 0, -Yf, 0, -Xf, 0, Zn);
		drawWholeInnerCubit(mi, Xf, 0, 0, 0, -Yf, Z1);
		drawWholeInnerCubit(mi, Xf, 0, 0, 0, -Yf, Zn);
		drawWholeInnerCubit(mi, 0, -Yf, 0, Xf, 0, Z1);
		drawWholeInnerCubit(mi, 0, -Yf, 0, Xf, 0, Zn);
		drawWholeInnerCubit(mi, -Xf, 0, 0, 0, Yf, Z1);
		drawWholeInnerCubit(mi, -Xf, 0, 0, 0, Yf, Zn);
		drawWholeInnerCubit(mi, 0, Yf, 0, -Xf, 0, Z1);
		drawWholeInnerCubit(mi, 0, Yf, 0, -Xf, 0, Zn);
		drawWholeInnerCubit(mi, 0, 0, Z1, Xf, Yf, 0);
		drawWholeInnerCubit(mi, 0, 0, Z1, -Xf, Yf, 0);
		drawWholeInnerCubit(mi, 0, 0, Z1, Xf, -Yf, 0);
		drawWholeInnerCubit(mi, 0, 0, Z1, -Xf, -Yf, 0);
		drawWholeInnerCubit(mi, 0, 0, Zn, Xf, Yf, 0);
		drawWholeInnerCubit(mi, 0, 0, Zn, Xf, -Yf, 0);
		drawWholeInnerCubit(mi, 0, 0, Zn, -Xf, Yf, 0);
		drawWholeInnerCubit(mi, 0, 0, Zn, -Xf, -Yf, 0);
	} else {
		drawWholeOuterCubit(mi, -Xf, Y1, -Zf);
		drawWholeOuterCubit(mi, -Xf, Yn, -Zf);
		drawWholeOuterCubit(mi, Xf, Y1, -Zf);
		drawWholeOuterCubit(mi, Xf, Yn, -Zf);
		drawWholeOuterCubit(mi, -Xf, Y1, Zf);
		drawWholeOuterCubit(mi, -Xf, Yn, Zf);
		drawWholeInnerCubit(mi, -Xf, 0, 0, 0, Y1, -Zf);
		drawWholeInnerCubit(mi, -Xf, 0, 0, 0, Yn, -Zf);
		drawWholeInnerCubit(mi, 0, 0, -Zf, -Xf, Y1, 0);
		drawWholeInnerCubit(mi, 0, 0, -Zf, -Xf, Yn, 0);
		drawWholeInnerCubit(mi, Xf, 0, 0, 0, Y1, -Zf);
		drawWholeInnerCubit(mi, Xf, 0, 0, 0, Yn, -Zf);
		drawWholeInnerCubit(mi, 0, 0, -Zf, Xf, Y1, 0);
		drawWholeInnerCubit(mi, 0, 0, -Zf, Xf, Yn, 0);
		drawWholeInnerCubit(mi, -Xf, 0, 0, 0, Y1, Zf);
		drawWholeInnerCubit(mi, -Xf, 0, 0, 0, Yn, Zf);
		drawWholeInnerCubit(mi, 0, 0, Zf, -Xf, Y1, 0);
		drawWholeInnerCubit(mi, 0, 0, Zf, -Xf, Yn, 0);
		drawWholeInnerCubit(mi, 0, Y1, 0, Xf, 0, Zf);
		drawWholeInnerCubit(mi, 0, Y1, 0, -Xf, 0, Zf);
		drawWholeInnerCubit(mi, 0, Y1, 0, Xf, 0, -Zf);
		drawWholeInnerCubit(mi, 0, Y1, 0, -Xf, 0, -Zf);
		drawWholeInnerCubit(mi, 0, Yn, 0, Xf, 0, Zf);
		drawWholeInnerCubit(mi, 0, Yn, 0, -Xf, 0, Zf);
		drawWholeInnerCubit(mi, 0, Yn, 0, Xf, 0, -Zf);
		drawWholeInnerCubit(mi, 0, Yn, 0, -Xf, 0, -Zf);
	}
}

/*-
 * This rotates whole cube.
 */
static void
faceRotate(ModeInfo * mi, int face, GLfloat rotateStep)
{
#define X1 (((corner & 4) != 0) ? 1 : -1)
#define Y1 (((corner & 2) != 0) ? 1 : -1)
#define Z1 (((corner & 1) != 0) ? 1 : -1)
#define Xn (((corner & 4) != 0) ? -1 : 1)
#define Yn (((corner & 2) != 0) ? -1 : 1)
#define Zn (((corner & 1) != 0) ? -1 : 1)
#define XP ((face == 4) ? 1 : ((face == -4) ? -1 : 0))
#define YP ((face == 2) ? 1 : ((face == -2) ? -1 : 0))
#define ZP ((face == 1) ? 1 : ((face == -1) ? -1 : 0))
	int corner = 0;

	drawCenter();
	glPushMatrix();
	glRotatef(rotateStep, XP, YP, ZP);
	drawWholeInnerCubit(mi, X1, 0, 0, 0, Y1, Z1);
	drawWholeInnerCubit(mi, X1, 0, 0, 0, Yn, Z1);
	drawWholeInnerCubit(mi, X1, 0, 0, 0, Y1, Zn);
	drawWholeInnerCubit(mi, X1, 0, 0, 0, Yn, Zn);
	drawWholeInnerCubit(mi, 0, Y1, 0, X1, 0, Z1);
	drawWholeInnerCubit(mi, 0, Y1, 0, Xn, 0, Z1);
	drawWholeInnerCubit(mi, 0, Y1, 0, X1, 0, Zn);
	drawWholeInnerCubit(mi, 0, Y1, 0, Xn, 0, Zn);
	drawWholeInnerCubit(mi, 0, 0, Z1, X1, Y1, 0);
	drawWholeInnerCubit(mi, 0, 0, Z1, Xn, Y1, 0);
	drawWholeInnerCubit(mi, 0, 0, Z1, X1, Yn, 0);
	drawWholeInnerCubit(mi, 0, 0, Z1, Xn, Yn, 0);

	drawWholeOuterCubit(mi, X1, Y1, Z1);
	drawWholeOuterCubit(mi, Xn, Y1, Z1);
	drawWholeOuterCubit(mi, X1, Yn, Z1);
	drawWholeOuterCubit(mi, X1, Y1, Zn);

	drawWholeInnerCubit(mi, Xn, 0, 0, 0, Y1, Z1);
	drawWholeInnerCubit(mi, Xn, 0, 0, 0, Yn, Z1);
	drawWholeInnerCubit(mi, Xn, 0, 0, 0, Y1, Zn);
	drawWholeInnerCubit(mi, Xn, 0, 0, 0, Yn, Zn);
	drawWholeInnerCubit(mi, 0, Yn, 0, X1, 0, Z1);
	drawWholeInnerCubit(mi, 0, Yn, 0, Xn, 0, Z1);
	drawWholeInnerCubit(mi, 0, Yn, 0, X1, 0, Zn);
	drawWholeInnerCubit(mi, 0, Yn, 0, Xn, 0, Zn);
	drawWholeInnerCubit(mi, 0, 0, Zn, X1, Y1, 0);
	drawWholeInnerCubit(mi, 0, 0, Zn, Xn, Y1, 0);
	drawWholeInnerCubit(mi, 0, 0, Zn, X1, Yn, 0);
	drawWholeInnerCubit(mi, 0, 0, Zn, Xn, Yn, 0);

	drawWholeOuterCubit(mi, Xn, Yn, Zn);
	drawWholeOuterCubit(mi, X1, Yn, Zn);
	drawWholeOuterCubit(mi, Xn, Y1, Zn);
	drawWholeOuterCubit(mi, Xn, Yn, Z1);
	glPopMatrix();
}

static void
drawCube(ModeInfo * mi)
{
	bevelstruct *bp = &bevel[MI_SCREEN(mi)];
	BevelSlice slice;
	GLfloat rotateStep = 0.0;

	slice.edge = NO_EDGE;
	slice.rotation = CW;
	if (bp->movement.face == NO_FACE) {
		slice.rotation = NO_ROTATION;
		edgeRotate(mi, slice.edge, rotateStep);
	} else if (bp->movement.direction < MAX_ORIENT) {
		convertMove(bp->movement, &slice);
		if (slice.edge != NO_EDGE) {
			rotateStep = (float) ((slice.rotation == CCW) ?
				bp->rotateStep : -bp->rotateStep);
		}
		edgeRotate(mi, slice.edge, rotateStep);
	} else {
		int face = nextFace[bp->movement.face][bp->movement.direction % MAX_ORIENT];
		rotateStep = (float) bp->rotateStep;
		faceRotate(mi, face, rotateStep);
	}
}

/*-
 * If you want to rotate a slice, you can draw that at the angleStep,
  then the without angleStep.
 * There is a sequence for drawing cubies for each axis being moved...
 */

/* From David Bagley's xbevel.  Used by permission. ;)  */
static void
rotateFace(bevelstruct * bp, int face, int direction)
{
	BevelLoc faceLoc[MAX_CUBES];
	int corner;

	/* Read Face */
	for (corner = 0; corner < MAX_CUBES; corner++)
		faceLoc[corner] = bp->cubeLoc[face][corner];
	/* Write Face */
	for (corner = 0; corner < MAX_ORIENT; corner++) {
		bp->cubeLoc[face][corner] = (direction == CW) ?
			faceLoc[(corner + MAX_ORIENT - 1) % MAX_ORIENT] :
			faceLoc[(corner + 1) % MAX_ORIENT];
		bp->cubeLoc[face][corner].rotation =
			(bp->cubeLoc[face][corner].rotation + direction) %
			MAX_ORIENT;
		/* drawOuterTriangle(w, face, corner, FALSE); */
		bp->cubeLoc[face][corner + MAX_ORIENT] = (direction == CW) ?
			faceLoc[(corner + MAX_ORIENT - 1) % MAX_ORIENT + MAX_ORIENT] :
			faceLoc[(corner + 1) % MAX_ORIENT + MAX_ORIENT];
		bp->cubeLoc[face][corner + MAX_ORIENT].rotation =
			(bp->cubeLoc[face][corner + MAX_ORIENT].rotation + direction) %
			MAX_ORIENT;
		/* drawInnerTriangle(w, face, corner, FALSE); */
	}
}

static void
readFace(bevelstruct * bp, int face, int h)
{
	int position;

	for (position = 0; position < MAX_CUBES; position++)
		bp->rowLoc[h][position] = bp->cubeLoc[face][position];
}

static void
writeFace(bevelstruct * bp, int face, int rotate, int h)
{
	int corner, newCorner;

	for (corner = 0; corner < MAX_ORIENT; corner++) {
		newCorner = (corner + rotate) % MAX_ORIENT;
		bp->cubeLoc[face][newCorner] = bp->rowLoc[h][corner];
		bp->cubeLoc[face][newCorner].rotation =
			(bp->cubeLoc[face][newCorner].rotation + rotate) %
			MAX_ORIENT;
		/* drawOuterTriangle(face, (corner + rotate) % MAX_ORIENT, FALSE); */
		bp->cubeLoc[face][newCorner + MAX_ORIENT] = bp->rowLoc[h][corner + MAX_ORIENT];
		bp->cubeLoc[face][newCorner + MAX_ORIENT].rotation =
			(bp->cubeLoc[face][newCorner + MAX_ORIENT].rotation + rotate) %
			MAX_ORIENT;
		/* drawInnerTriangle(face, (corner + rotate) % MAX_ORIENT, FALSE); */
	}
}

static void
readRow(bevelstruct * bp, int face, int side, int rotate, int size)
{
#ifdef DEBUG
printf("read face %d side %d, rotate %d size %d\n", face, side, rotate, size);
#endif
	if (size == MINOR) {
		int minorFace, corner;
		minorFace = majToMin[face][side].face;
		corner = majToMin[face][side].position;
		bp->minorLoc[rotate] = bp->cubeLoc[minorFace][corner];
#ifdef DEBUG
printf(" min %d %d\n", bp->minorLoc[rotate].face,
		bp->minorLoc[rotate].rotation);
#endif
	} else
	{
		int g;
		for (g = 0; g < 2; g++) {
			bp->majorLoc[rotate][g] =
				bp->cubeLoc[face][(side + g + MAX_ORIENT - 1) %
				MAX_ORIENT];
			bp->majorLoc[rotate][g + MAX_ROTATE] =
				bp->cubeLoc[face][(side + g + MAX_ORIENT - 1) %
				MAX_ORIENT + MAX_ORIENT];
#ifdef DEBUG
printf("maj %d %d, %d %d\n", bp->majorLoc[rotate][g].face,
		bp->majorLoc[rotate][g].rotation,
		bp->majorLoc[rotate][g + MAX_ROTATE].face,
		bp->majorLoc[rotate][g + MAX_ROTATE].rotation);
#endif
		}
	}
}

#if 0
static void
rotateRow(bevelstruct * bp, int rotate, int orient, int size)
{
	int g;

	if (size == MINOR)
		bp->minorLoc[orient].rotation =
			(bp->minorLoc[orient].rotation + rotate) % MAX_ORIENT;
	else		    /* size == MAJOR */
		for (g = 0; g < 2; g++) {
			bp->majorLoc[orient][g].rotation =
				(bp->majorLoc[orient][g].rotation + rotate) %
				MAX_ORIENT;
			bp->majorLoc[orient][g].rotation =
				(bp->majorLoc[orient][g + MAX_ROTATE].rotation + rotate) %
				MAX_ORIENT + MAX_ORIENT;
		}
}
#endif

static void
writeRow(bevelstruct *bp, int face, int side, int rotate, int size)
{
	int g, h;

	if (size == MINOR) {
		int minorFace, corner;
		minorFace = majToMin[face][side].face;
		corner = majToMin[face][side].position;
#ifdef DEBUG
printf("min write face %d, corner %d, side %d\n", minorFace, corner, side);
#endif
		bp->cubeLoc[minorFace][corner] = bp->minorLoc[rotate];
		/* drawOuterTriangle(w, minorFace, corner, FALSE); */
	} else {	/* size == MAJOR */
		for (g = 0; g < 2; g++) {
			h = (side + g + MAX_ORIENT - 1) % MAX_ORIENT;
#ifdef DEBUG
printf("maj write face %d, corner %d, g %d, h %d\n", face, side, g, h);
#endif
			bp->cubeLoc[face][h] = bp->majorLoc[rotate][g];
			bp->cubeLoc[face][h + MAX_ORIENT] = bp->majorLoc[rotate][g + MAX_ROTATE];
			/* drawOuterTriangle(face, h, FALSE); */
			/* drawInnerTriangle(face, h, FALSE); */
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
moveBevel(bevelstruct * bp, int face, int position, int direction)
{
	int oldFace = face, pos = position, dir = direction;
	int newFace, newDirection, newSide, k, rotate;

	if (direction < 2 * MAX_ORIENT) {
		if ((direction - pos + MAX_ORIENT) % MAX_ORIENT == 2)
			pos = (pos + 1) % MAX_ORIENT;
		readRow(bp, oldFace, pos, 0, MAJOR);
		readRow(bp, oldFace, pos, 0, MINOR);
		for (k = 1; k <= MAX_ROTATE; k++) {
			newFace = matchFaceSide[oldFace][pos].face;
			newSide = matchFaceSide[oldFace][pos].position;
			rotate = matchFaceSide[oldFace][pos].direction %
				MAX_ORIENT;
			newDirection = (rotate + dir) % MAX_ORIENT;
#ifdef DEBUG
printf("newFace %d newSide %d, rotate %d newDirection %d\n",
		newFace, newSide, rotate, newDirection);
#endif
			if (k != MAX_ROTATE) {
				readRow(bp, newFace, newSide, k, MAJOR);
				readRow(bp, newFace, newSide, k, MINOR);
			}
#if 0
			rotateRow(bp, rotate, k - 1, MAJOR);
			rotateRow(bp, rotate, k - 1, MINOR);
#endif
			writeRow(bp, newFace, newSide, k - 1, MAJOR);
			writeRow(bp, newFace, newSide, k - 1, MINOR);
			oldFace = newFace;
			pos = newSide;
			dir = newDirection;
		}
	} else {
		rotateFace(bp, faceToRotate[face][direction % MAX_ORIENT], CW);
		rotateFace(bp, faceToRotate[face][(direction + 2) % MAX_ORIENT], CCW);
		readFace(bp, face, 0);
		for (k = 1; k <= MAX_ORIENT; k++) {
			newFace = slideNextFace[face][direction % MAX_ORIENT].face;
			rotate = slideNextFace[face][direction % MAX_ORIENT].rotation;
			newDirection = (rotate + direction) % MAX_ORIENT;
			if (k != MAX_ORIENT)
				readFace(bp, newFace, k);
			writeFace(bp, newFace, rotate, k - 1);
			face = newFace;
			direction = newDirection;
		}
	}
}

#ifdef DEBUG
static void
printCube(bevelstruct * bp)
{
	int	 face, position;

	for (face = 0; face < MAX_FACES; face++) {
		for (position = 0; position < MAX_CUBES; position++) {
			(void) printf("%d %d  ", bp->cubeLoc[face][position].face,
			bp->cubeLoc[face][position].rotation);
		}
		(void) printf("\n");
	}
	(void) printf("\n");
}

#endif

static void
evalmovement(ModeInfo * mi, BevelMove movement)
{
	bevelstruct *bp = &bevel[MI_SCREEN(mi)];

#ifdef DEBUG
	printCube(bp);
#endif
	if (movement.face < 0 || movement.face >= MAX_FACES)
		return;

	moveBevel(bp, movement.face, movement.position, movement.direction);

}

static Bool
compareMoves(BevelMove move1, BevelMove move2)
{
	BevelSlice slice1, slice2;

	convertMove(move1, &slice1);
	convertMove(move2, &slice2);
	return (slice1.edge == slice2.edge);
}

static Bool
shuffle(ModeInfo * mi)
{
	bevelstruct *bp = &bevel[MI_SCREEN(mi)];
	int	 i, face, position;
	BevelMove   move;

	for (face = 0; face < MAX_FACES; face++) {
		for (position = 0; position < MAX_CUBES; position++) {
			bp->cubeLoc[face][position].face = face;
			bp->cubeLoc[face][position].rotation = TOP;
		}
	}
	bp->storedmoves = MI_COUNT(mi);
	if (bp->storedmoves < 0) {
		if (bp->moves != NULL)
			free(bp->moves);
		bp->moves = (BevelMove *) NULL;
		bp->storedmoves = NRAND(-bp->storedmoves) + 1;
	}
	if ((bp->storedmoves) && (bp->moves == NULL))
		if ((bp->moves = (BevelMove *) calloc(bp->storedmoves + 1,
				sizeof (BevelMove))) == NULL) {
			return False;
		}
	if (MI_CYCLES(mi) <= 1) {
		bp->angleStep = 120.0;
	} else {
		bp->angleStep = 120.0 / (GLfloat) (MI_CYCLES(mi));
	}

	for (i = 0; i < bp->storedmoves; i++) {
		Bool condition;

		do {
			move.face = NRAND(MAX_FACES);
			move.position = NRAND(MAX_ORIENT);
			move.direction = ((move.position +
				((NRAND(2) == 0) ? 1 : 3)) % MAX_ORIENT);
			condition = True;
			if (i > 0 && compareMoves(move, bp->moves[i - 1])) {
				condition = False;
			}
		} while (!condition);
		if (hideshuffling)
			evalmovement(mi, move);
		bp->moves[i] = move;
	}
	bp->VX = 0.05;
	if (NRAND(100) < 50)
		bp->VX *= -1;
	bp->VY = 0.05;
	if (NRAND(100) < 50)
		bp->VY *= -1;
	bp->movement.face = NO_FACE;
	bp->rotateStep = 0;
	bp->action = hideshuffling ? ACTION_SOLVE : ACTION_SHUFFLE;
	bp->shufflingmoves = 0;
	bp->done = 0;
	return True;
}

static void
reshape_bevel(ModeInfo * mi, int width, int height)
{
	bevelstruct *bp = &bevel[MI_SCREEN(mi)];

	glViewport(0, 0, bp->WindW = (GLint) width, bp->WindH = (GLint) height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-1.0, 1.0, -1.0, 1.0, 5.0, 15.0);
	glMatrixMode(GL_MODELVIEW);

	bp->AreObjectsDefined[ObjFacit] = False;
	bp->AreObjectsDefined[ObjCubit] = False;
}

#ifdef STANDALONE
ENTRYPOINT Bool
bevel_handle_event (ModeInfo *mi, XEvent *event)
{
	bevelstruct *bp = &bevel[MI_SCREEN(mi)];

	if (gltrackball_event_handler (event, bp->trackball,
			MI_WIDTH (mi), MI_HEIGHT (mi),
			&bp->button_down_p))
		return True;
	else if (screenhack_event_helper (MI_DISPLAY(mi), MI_WINDOW(mi), event)) {
		bp->done = 1;
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
free_bevel_screen(bevelstruct *bp)
{
	if (bp == NULL) {
		return;
	}
#ifdef STANDALONE
	if (bp->trackball)
		gltrackball_free (bp->trackball);
#endif
	if (bp->moves != NULL) {
		free(bp->moves);
		bp->moves = (BevelMove *) NULL;
	}
	bp = NULL;
}

ENTRYPOINT void
release_bevel(ModeInfo * mi)
{
	if (bevel != NULL) {
		int	 screen;

		for (screen = 0; screen < MI_NUM_SCREENS(mi); screen++)
			free_bevel_screen(&bevel[screen]);
		free(bevel);
		bevel = (bevelstruct *) NULL;
	}
	FreeAllGL(mi);
}

ENTRYPOINT void
init_bevel(ModeInfo * mi)
{
	bevelstruct *bp;

	MI_INIT(mi, bevel);
	bp = &bevel[MI_SCREEN(mi)];

	bp->step = NRAND(180);
	bp->PX = ((float) LRAND() / (float) MAXRAND) * 2.0 - 1.0;
	bp->PY = ((float) LRAND() / (float) MAXRAND) * 2.0 - 1.0;

#ifdef STANDALONE
	bp->trackball = gltrackball_init (True);
#endif

	if ((bp->glx_context = init_GL(mi)) != NULL) {
		reshape_bevel(mi, MI_WIDTH(mi), MI_HEIGHT(mi));
		glDrawBuffer(GL_BACK);
		if (!pinit(mi)) {
			free_bevel_screen(bp);
			if (MI_IS_VERBOSE(mi)) {
				 (void) fprintf(stderr,
					"Could not allocate memory for bevel\n");
			}
		}
	} else {
		MI_CLEARWINDOW(mi);
	}
}

ENTRYPOINT void
draw_bevel(ModeInfo * mi)
{
	Bool bounced = False;
	Display    *display = MI_DISPLAY(mi);
	Window      window = MI_WINDOW(mi);
	bevelstruct *bp;

	if (bevel == NULL)
		return;
	bp = &bevel[MI_SCREEN(mi)];
	if ((bp->storedmoves) && (bp->moves == NULL))
		return;
	if (!bp->glx_context)
		return;
	MI_IS_DRAWN(mi) = True;
#ifdef WIN32
	wglMakeCurrent(hdc, bp->glx_context);
#else
	glXMakeCurrent(display, window, *(bp->glx_context));
#endif
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	glTranslatef(0.0, 0.0, -10.0);

	bp->PX += bp->VX;
	bp->PY += bp->VY;

	if (bp->PY < -1) {
		bp->PY += (-1) - (bp->PY);
		bp->VY = -bp->VY;
		bounced = True;
	}
	if (bp->PY > 1) {
		bp->PY -= (bp->PY) - 1;
		bp->VY = -bp->VY;
		bounced = True;
	}
	if (bp->PX < -1) {
		bp->PX += (-1) - (bp->PX);
		bp->VX = -bp->VX;
		bounced = True;
	}
	if (bp->PX > 1) {
		bp->PX -= (bp->PX) - 1;
		bp->VX = -bp->VX;
		bounced = True;
	}
	if (bounced) {
		bp->VX += ((float) LRAND() / (float) MAXRAND) * 0.02 - 0.01;
		bp->VX += ((float) LRAND() / (float) MAXRAND) * 0.02 - 0.01;
		if (bp->VX > 0.06)
			bp->VX = 0.06;
		if (bp->VY > 0.06)
			bp->VY = 0.06;
		if (bp->VX < -0.06)
			bp->VX = -0.06;
		if (bp->VY < -0.06)
			bp->VY = -0.06;
	}
	if (!MI_IS_ICONIC(mi)) {
		glTranslatef(bp->PX, bp->PY, 0);
		glScalef(Scale4Window * bp->WindH / bp->WindW,
			Scale4Window, Scale4Window);
	} else {
		glScalef(Scale4Iconic * bp->WindH / bp->WindW,
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
	gltrackball_rotate (bp->trackball);
#endif

	glRotatef(bp->step * 100, 1, 0, 0);
	glRotatef(bp->step * 95, 0, 1, 0);
	glRotatef(bp->step * 90, 0, 0, 1);
	drawCube(mi);
	if (MI_IS_FPS(mi)) {
		do_fps (mi);
	}
	glXSwapBuffers(display, window);
	if (bp->action == ACTION_SHUFFLE) {
		if (bp->done) {
			if (++bp->rotateStep > DELAY_AFTER_SHUFFLING) {
				bp->movement.face = NO_FACE;
				bp->rotateStep = 0;
				bp->action = ACTION_SOLVE;
				bp->done = 0;
			}
		} else {
			if (bp->movement.face == NO_FACE) {
				if (bp->shufflingmoves < bp->storedmoves) {
					bp->rotateStep = 0;
					bp->movement = bp->moves[bp->shufflingmoves];
				} else {
					bp->rotateStep = 0;
					bp->done = 1;
				}
			} else {
				if (bp->rotateStep == 0) {
					;
				}
				bp->rotateStep += bp->angleStep;
				if (bp->rotateStep > 180) {
					evalmovement(mi, bp->movement);
					bp->shufflingmoves++;
					bp->movement.face = NO_FACE;
				}
			}
		}
	} else {
		if (bp->done) {
			if (++bp->rotateStep > DELAY_AFTER_SOLVING)
				if (!shuffle(mi)) {
					free_bevel_screen(bp);
					if (MI_IS_VERBOSE(mi)) {
						 (void) fprintf(stderr,
							"Could not allocate memory for bevel\n");
					}
				}
		} else {
			if (bp->movement.face == NO_FACE) {
				if (bp->storedmoves > 0) {
					bp->rotateStep = 0;
					bp->movement = bp->moves[bp->storedmoves - 1];
					bp->movement.direction = (bp->movement.direction +
						(MAX_ORIENT / 2)) % MAX_ORIENT;
				} else {
					bp->rotateStep = 0;
					bp->done = 1;
				}
			} else {
				if (bp->rotateStep == 0) {
					;
				}
				bp->rotateStep += bp->angleStep;
				if (bp->rotateStep > 180) {
					evalmovement(mi, bp->movement);
					bp->storedmoves--;
					bp->movement.face = NO_FACE;
				}
			}
		}
	}

	glPopMatrix();

	glFlush();

	bp->step += 0.05;
}

#ifndef STANDALONE
ENTRYPOINT void
change_bevel(ModeInfo * mi)
{
	bevelstruct *bp;

	if (bevel == NULL)
		return;
	bp = &bevel[MI_SCREEN(mi)];

	if (!bp->glx_context)
		return;
	if (!pinit(mi)) {
		free_bevel_screen(bp);
		if (MI_IS_VERBOSE(mi)) {
			 (void) fprintf(stderr,
				"Could not allocate memory for bevel\n");
		}
	}
}
#endif

XSCREENSAVER_MODULE ("Bevel", bevel)

#endif /* MODE_bevel */
