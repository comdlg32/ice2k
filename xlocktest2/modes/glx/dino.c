/* -*- Mode: C; tab-width: 4 -*- */
/* dino --- Shows an auto-solving Dino */

#if 0
static const char sccsid[] = "@(#)dino.c	5.72 2023/08/19 xlockmore";
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
 * This mode shows an auto-solving Dinosaur Cube.
 *
 * Thanks goes also to Brian Paul for making it possible and inexpensive
 * to use OpenGL at home.
 *
 * Based on rubik.c by Marcelo F. Vianna
 *
 * Revision History:
 * 19-Aug-2023: Started writing, using xlock skewb.c and DinoGL.c from
		xpuzzles.  The physical puzzle originally had dinosaurs
		printed on the puzzle.  There is also a difficult (at
		least for me) period 2 turning in original DinoGL.c.
 */

/*-
 * Color labels mapping:
 * =====================
 *
 *	+------+
 *	|  0   |
 *	|      |
 *	| TOP  |
 *	|3(0) 1|
 *	| RED  |
 *	|  2   |
 * +------+------+------+
 * |  0   |  0   |  0   |
 * |      |      |      |
 * | LEFT |FRONT |RIGHT |
 * |3(1) 1|3(2) 1|3(3) 1|
 * | BLUE |YELLOW|GREEN |
 * |  2   |  2   |  2   |
 * +------+------+------+
 *	|  0   |
 *	|      |
 *	|BOTTOM|
 *	|3(4) 1|
 *	|WHITE |
 *	|  2   |
 *	+------+	 +------+
 *	|  0   |	 |\ 0  /|
 *	|      |	 | \  / |
 *	| BACK |	 | xxxx |
 *	|3(5) 1|	 |3(N) 1|
 *	|ORANGE|	 | /  \ |
 *	|  2   |	 |/ 2  \|
 *	+------+	 +------+
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
# define MODE_dino
# define DEFAULTS	"*delay: 100000 \n" \
			"*count: -30 \n" \
			"*cycles: 5 \n" \

# define free_dino 0
# include "xlockmore.h"		/* from the xscreensaver distribution */
# include "gltrackball.h"

#else /* !STANDALONE */
#include "xlock.h"		/* from the xlockmore distribution */
#include "visgl.h"
#endif /* !STANDALONE */

#ifdef MODE_dino

#include <GL/gl.h>
#define DEF_HIDESHUFFLING     "False"
#define DEF_PERIOD2     "False"

static Bool hideshuffling;
static Bool period2;

static XrmOptionDescRec opts[] =
{
	{(char *) "-hideshuffling", (char *) ".dino.hideshuffling", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+hideshuffling", (char *) ".dino.hideshuffling", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-period2", (char *) ".dino.period2", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+period2", (char *) ".dino.period2", XrmoptionNoArg, (caddr_t) "off"}
};

static argtype vars[] =
{
	{(void *) & hideshuffling, (char *) "hideshuffling", (char *) "Hideshuffling", (char *) DEF_HIDESHUFFLING, t_Bool},
	{(void *) & period2, (char *) "period2", (char *) "Period2", (char *) DEF_PERIOD2, t_Bool}
};

static OptionStruct desc[] =
{
	{(char *) "-/+hideshuffling", (char *) "turn on/off hidden shuffle phase"},
	{(char *) "-/+period2", (char *) "turn on/off period 2 turns"}
};

ENTRYPOINT ModeSpecOpt dino_opts =
{sizeof opts / sizeof opts[0], opts, sizeof vars / sizeof vars[0], vars, desc};

#ifdef USE_MODULES
ModStruct   dino_description =
{"dino", "init_dino", "draw_dino", "release_dino",
 "draw_dino", "change_dino", "(char *) NULL", &dino_opts,
 100000, -30, 5, 1, 64, 1.0, "",
 "Shows an auto-solving Dinosaur Cube", 0, NULL};

#endif

#define VectMul(X1,Y1,Z1,X2,Y2,Z2) (Y1)*(Z2)-(Z1)*(Y2),(Z1)*(X2)-(X1)*(Z2),(X1)*(Y2)-(Y1)*(X2)
#define sqr(A)		     ((A)*(A))

#ifndef Pi
#define Pi			 M_PI
#endif


#define ACTION_SOLVE    1
#define ACTION_SHUFFLE  0

#define DELAY_AFTER_SHUFFLING  5
#define DELAY_AFTER_SOLVING   20

/*************************************************************************/

#define Scale4Window	       (0.9/3.0)
#define Scale4Iconic	       (2.1/3.0)

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
#define CORNER 0
#define MIDDLE 1
#define EDGE 2
#define FACE 3
#define MAX_CORNERS 8
#define MAX_EDGES 12

#define TOP_FACE 0
#define LEFT_FACE 1
#define FRONT_FACE 2
#define RIGHT_FACE 3
#define BOTTOM_FACE 4
#define BACK_FACE 5
#define NO_FACE (MAX_FACES+1)
#define NO_CORNER 8
#define NO_EDGE 12
#define NO_POSITION (IGNORE_DIR)
#define NO_ROTATION (2*MAX_ORIENT)

#define CUBE_LENGTH 0.5
#define CUBE_ROUND (CUBE_LENGTH-0.02)
#define CUBEFULLROUND (2.0*CUBE_ROUND)
#define STICKER_LONG (CUBE_ROUND-0.05)
#define STICKERFULLLONG (2.0*STICKER_LONG)
#define STICKER_SHORT (STICKER_LONG-0.05)
#define STICKERFULLSHORT (2.0*STICKER_SHORT)
#define STICKER_DEPTH (CUBE_LENGTH+0.01)
#define STICKER_OFFSET (float) 0.1

#define ObjCubit	0
#define MaxObj	  1

typedef struct _DinoLoc {
	int	 face;
	int	 rotation;  /* Not used yet */
} DinoLoc;

typedef struct _DinoLocPos {
	int	 face, position, direction;
} DinoLocPos;

typedef struct _RowNext {
	int	 face, direction, sideFace;
} RowNext;

typedef struct _DinoMove {
	int	 face, direction;
	int	 position;
	int	 style;
} DinoMove;

typedef struct _DinoSlice {
	int	 ce; /* corner or edge */
	int	 rotation;
} DinoSlice;

static DinoLoc slideNextFace[MAX_FACES][MAX_ORIENT] =
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

static int faceToRotate2[MAX_FACES][MAX_ORIENT][2] =
{
	{
		{3, 5},
		{2, 3},
		{1, 2},
		{1, 5}
	},
	{
		{0, 2},
		{2, 4},
		{4, 5},
		{0, 5}
	},
	{
		{3, 0},
		{4, 3},
		{1, 4},
		{0, 1}
	},
	{
		{0, 5},
		{4, 5},
		{2, 4},
		{0, 2}
	},
	{
		{2, 3},
		{3, 5},
		{1, 5},
		{1, 2}
	},
	{
		{4, 3},
		{3, 0},
		{0, 1},
		{1, 4}
	}
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
	DinoMove  *moves;
	int	 storedmoves;
	int      degreeTurn;
	int	 shufflingmoves;
	int	 action;
	int	 done;
	GLfloat     angleStep;
	DinoLoc    cubeLoc[MAX_FACES][MAX_ORIENT];
	DinoLoc faceLoc[MAX_ORIENT];
	DinoLoc rowLoc[MAX_ORIENT][MAX_ORIENT];
	DinoLoc spindleLoc[MAX_ROTATE][2];
	DinoMove   movement;
	GLfloat     rotateStep;
	GLfloat     PX, PY, VX, VY;
	Bool	AreObjectsDefined[2];
	Bool    period2;
#ifdef STANDALONE
	Bool button_down_p;
	trackball_state *trackball;
#endif
} dinostruct;

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
static float MaterialYellow[] =
{0.7, 0.7, 0.0, 1.0};
static float MaterialOrange[] =
{0.9, 0.45, 0.0, 1.0};

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

static dinostruct *dino = (dinostruct *) NULL;

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
		if (mono)
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray4);
		else
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialYellow);
		break;
	case RIGHT_FACE:
		if (mono)
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray7);
		else
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGreen);
		break;
	case BOTTOM_FACE:
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialWhite);
		break;
	case BACK_FACE:
		if (mono)
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray6);
		else
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialOrange);
		break;
	}
}

static void
drawStickerlessHalfCubit(int val)
{
	glBegin(GL_TRIANGLES);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray);
	if (val == 1) {
		glNormal3f(1.0, 0.0, 1.0);
		glVertex3f(CUBE_LENGTH, CUBE_ROUND, -CUBEFULLROUND - CUBE_LENGTH);
		glVertex3f(CUBE_LENGTH, -CUBE_ROUND, -CUBE_LENGTH);
		glVertex3f(CUBE_LENGTH - CUBE_ROUND, 0, -CUBE_LENGTH);
		glNormal3f(1.0, 0.0, 1.0);
		glVertex3f(CUBE_LENGTH - CUBE_ROUND, 0, -CUBE_LENGTH);
		glVertex3f(CUBE_LENGTH, -CUBE_ROUND, -CUBE_LENGTH);
		glVertex3f(CUBE_LENGTH, CUBE_ROUND, CUBEFULLROUND - CUBE_LENGTH);
		glNormal3f(1.0, 1.0, 0.0);
		glVertex3f(CUBE_LENGTH, CUBE_ROUND, CUBEFULLROUND - CUBE_LENGTH);
		glVertex3f(CUBE_LENGTH, CUBE_ROUND, -CUBEFULLROUND - CUBE_LENGTH);
		glVertex3f(CUBE_LENGTH - CUBE_ROUND, 0, -CUBE_LENGTH);
		/* Put sticker here */
		glNormal3f(1.0, 0.0, 0.0);
		glVertex3f(CUBE_LENGTH, CUBE_ROUND, -CUBEFULLROUND - CUBE_LENGTH);
		glVertex3f(CUBE_LENGTH, CUBE_ROUND, CUBEFULLROUND - CUBE_LENGTH);
		glVertex3f(CUBE_LENGTH, -CUBE_ROUND, -CUBE_LENGTH);
	} else {
		glNormal3f(0.0, 1.0, 1.0);
		glVertex3f(-CUBE_ROUND, CUBE_LENGTH, -CUBE_LENGTH);
		glVertex3f(CUBE_ROUND, CUBE_LENGTH, -CUBEFULLROUND - CUBE_LENGTH);
		glVertex3f(0, CUBE_LENGTH - CUBE_ROUND, -CUBE_LENGTH);
		glNormal3f(0.0, 1.0, 1.0);
		glVertex3f(-CUBE_ROUND, CUBE_LENGTH, -CUBE_LENGTH);
		glVertex3f(0, CUBE_LENGTH - CUBE_ROUND, -CUBE_LENGTH);
		glVertex3f(CUBE_ROUND, CUBE_LENGTH, CUBEFULLROUND - CUBE_LENGTH);
		glNormal3f(1.0, 1.0, 0.0);
		glVertex3f(CUBE_ROUND, CUBE_LENGTH, -CUBEFULLROUND - CUBE_LENGTH);
		glVertex3f(CUBE_ROUND, CUBE_LENGTH, CUBEFULLROUND - CUBE_LENGTH);
		glVertex3f(0, CUBE_LENGTH - CUBE_ROUND, -CUBE_LENGTH);
		/* Put sticker here */
		glNormal3f(0.0, 1.0, 0.0);
		glVertex3f(-CUBE_ROUND, CUBE_LENGTH, -CUBE_LENGTH);
		glVertex3f(CUBE_ROUND, CUBE_LENGTH, CUBEFULLROUND - CUBE_LENGTH);
		glVertex3f(CUBE_ROUND, CUBE_LENGTH, -CUBEFULLROUND - CUBE_LENGTH);
	}
	glEnd();
}

static void
drawStickerlessCubit(Bool sep)
{
	if (sep) {
		drawStickerlessHalfCubit(1);
		drawStickerlessHalfCubit(0);
		return;
	}
	glBegin(GL_QUADS);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray);
	/* Edge of cubit */
	glNormal3f(1.0, 1.0, 0.0);
	glVertex3f(CUBE_ROUND, CUBE_LENGTH, -CUBEFULLROUND - CUBE_LENGTH);
	glVertex3f(CUBE_ROUND, CUBE_LENGTH, CUBEFULLROUND - CUBE_LENGTH);
	glVertex3f(CUBE_LENGTH, CUBE_ROUND, CUBEFULLROUND - CUBE_LENGTH);
	glVertex3f(CUBE_LENGTH, CUBE_ROUND, -CUBEFULLROUND - CUBE_LENGTH);
	/* Internal to cubit */
	glNormal3f(0.0, 1.0, 1.0);
	glVertex3f(CUBE_ROUND, CUBE_LENGTH, -CUBEFULLROUND - CUBE_LENGTH);
	glVertex3f(CUBE_LENGTH, CUBE_ROUND, -CUBEFULLROUND - CUBE_LENGTH);
	glVertex3f(CUBE_LENGTH, -CUBE_ROUND, -CUBE_LENGTH);
	glVertex3f(-CUBE_ROUND, CUBE_LENGTH, -CUBE_LENGTH);
	glNormal3f(0.0, 1.0, 1.0);
	glVertex3f(CUBE_ROUND, CUBE_LENGTH, CUBEFULLROUND - CUBE_LENGTH);
	glVertex3f(CUBE_LENGTH, CUBE_ROUND, CUBEFULLROUND - CUBE_LENGTH);
	glVertex3f(-CUBE_ROUND, CUBE_LENGTH, -CUBE_LENGTH);
	glVertex3f(CUBE_LENGTH, -CUBE_ROUND, -CUBE_LENGTH);
	glEnd();
	glBegin(GL_TRIANGLES);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray);
	/* Put sticker here */
	glNormal3f(1.0, 0.0, 0.0);
	glVertex3f(CUBE_LENGTH, CUBE_ROUND, -CUBEFULLROUND - CUBE_LENGTH);
	glVertex3f(CUBE_LENGTH, CUBE_ROUND, CUBEFULLROUND - CUBE_LENGTH);
	glVertex3f(CUBE_LENGTH, -CUBE_ROUND, -CUBE_LENGTH);
	glNormal3f(0.0, 1.0, 0.0);
	glVertex3f(-CUBE_ROUND, CUBE_LENGTH, -CUBE_LENGTH);
	glVertex3f(CUBE_ROUND, CUBE_LENGTH, CUBEFULLROUND - CUBE_LENGTH);
	glVertex3f(CUBE_ROUND, CUBE_LENGTH, -CUBEFULLROUND - CUBE_LENGTH);
	glEnd();
}

static void
drawCenter(void)
{
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray);
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);
	glVertex3f(-CUBE_ROUND, -CUBE_ROUND, CUBE_ROUND);
	glVertex3f(CUBE_ROUND, -CUBE_ROUND, CUBE_ROUND);
	glVertex3f(CUBE_ROUND, CUBE_ROUND, CUBE_ROUND);
	glVertex3f(-CUBE_ROUND, CUBE_ROUND, CUBE_ROUND);
	glNormal3f(0.0, 0.0, -1.0);
	glVertex3f(-CUBE_ROUND, CUBE_ROUND, -CUBE_ROUND);
	glVertex3f(CUBE_ROUND, CUBE_ROUND, -CUBE_ROUND);
	glVertex3f(CUBE_ROUND, -CUBE_ROUND, -CUBE_ROUND);
	glVertex3f(-CUBE_ROUND, -CUBE_ROUND, -CUBE_ROUND);
	glNormal3f(-1.0, 0.0, 0.0);
	glVertex3f(-CUBE_ROUND, -CUBE_ROUND, CUBE_ROUND);
	glVertex3f(-CUBE_ROUND, CUBE_ROUND, CUBE_ROUND);
	glVertex3f(-CUBE_ROUND, CUBE_ROUND, -CUBE_ROUND);
	glVertex3f(-CUBE_ROUND, -CUBE_ROUND, -CUBE_ROUND);
	glNormal3f(1.0, 0.0, 0.0);
	glVertex3f(CUBE_ROUND, -CUBE_ROUND, -CUBE_ROUND);
	glVertex3f(CUBE_ROUND, CUBE_ROUND, -CUBE_ROUND);
	glVertex3f(CUBE_ROUND, CUBE_ROUND, CUBE_ROUND);
	glVertex3f(CUBE_ROUND, -CUBE_ROUND, CUBE_ROUND);
	glNormal3f(0.0, -1.0, 0.0);
	glVertex3f(CUBE_ROUND, -CUBE_ROUND, -CUBE_ROUND);
	glVertex3f(CUBE_ROUND, -CUBE_ROUND, CUBE_ROUND);
	glVertex3f(-CUBE_ROUND, -CUBE_ROUND, CUBE_ROUND);
	glVertex3f(-CUBE_ROUND, -CUBE_ROUND, -CUBE_ROUND);
	glNormal3f(0.0, 1.0, 0.0);
	glVertex3f(-CUBE_ROUND, CUBE_ROUND, -CUBE_ROUND);
	glVertex3f(-CUBE_ROUND, CUBE_ROUND, CUBE_ROUND);
	glVertex3f(CUBE_ROUND, CUBE_ROUND, CUBE_ROUND);
	glVertex3f(CUBE_ROUND, CUBE_ROUND, -CUBE_ROUND);
	glEnd();
}

static void
drawCubit(ModeInfo * mi,
	   int back, int front, int left, int right, int bottom, int top)
{
	/* dinostruct *dp = &dino[MI_SCREEN(mi)]; */
	int	 mono = MI_IS_MONO(mi);

	if (back < NO_FACE) { /* Orange */
		glPushMatrix();
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray);
		glNormal3f(0.0, 0.0, -1.0);
		if (top != NO_FACE) {
			glPushMatrix();
			glTranslatef(-CUBE_LENGTH, -CUBE_ROUND, 0.0);
			glBegin(GL_TRIANGLES);
			pickColor(back, mono);
			glVertex3f(0.0, STICKER_OFFSET, -STICKER_DEPTH);
			glVertex3f(-STICKERFULLSHORT, STICKERFULLLONG, -STICKER_DEPTH);
			glVertex3f(STICKERFULLSHORT, STICKERFULLLONG, -STICKER_DEPTH);
			glEnd();
			glPopMatrix();
		}
		if (left != NO_FACE) {
			glPushMatrix();
			glTranslatef(CUBE_ROUND, CUBE_LENGTH, 0.0);
			glBegin(GL_TRIANGLES);
			pickColor(back, mono);
			glVertex3f(-STICKER_OFFSET, 0.0, -STICKER_DEPTH);
			glVertex3f(-STICKERFULLLONG, -STICKERFULLSHORT, -STICKER_DEPTH);
			glVertex3f(-STICKERFULLLONG, STICKERFULLSHORT, -STICKER_DEPTH);
			glEnd();
			glPopMatrix();
		}
		if (bottom != NO_FACE) {
			glPushMatrix();
			glTranslatef(CUBE_LENGTH, CUBE_ROUND, 0.0);
			glBegin(GL_TRIANGLES);
			pickColor(back, mono);
			glVertex3f(0.0, -STICKER_OFFSET, -STICKER_DEPTH);
			glVertex3f(STICKERFULLSHORT, -STICKERFULLLONG, -STICKER_DEPTH);
			glVertex3f(-STICKERFULLSHORT, -STICKERFULLLONG, -STICKER_DEPTH);
			glEnd();
			glPopMatrix();
		}
		if (right != NO_FACE) {
			glPushMatrix();
			glTranslatef(-CUBE_ROUND, -CUBE_LENGTH, 0.0);
			glBegin(GL_TRIANGLES);
			pickColor(back, mono);
			glVertex3f(STICKER_OFFSET, 0.0, -STICKER_DEPTH);
			glVertex3f(STICKERFULLLONG, STICKERFULLSHORT, -STICKER_DEPTH);
			glVertex3f(STICKERFULLLONG, -STICKERFULLSHORT, -STICKER_DEPTH);
			glEnd();
			glPopMatrix();
		}
		glPopMatrix();
	}
	if (front < NO_FACE) { /* Yellow */
		glPushMatrix();
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray);
		glNormal3f(0.0, 0.0, 1.0);
		if (bottom != NO_FACE) {
			glPushMatrix();
			glTranslatef(-CUBE_LENGTH, CUBE_ROUND, 0.0);
			glBegin(GL_TRIANGLES);
			pickColor(front, mono);
			glVertex3f(0.0, -STICKER_OFFSET, STICKER_DEPTH);
			glVertex3f(-STICKERFULLSHORT, -STICKERFULLLONG, STICKER_DEPTH);
			glVertex3f(STICKERFULLSHORT, -STICKERFULLLONG, STICKER_DEPTH);
			glEnd();
			glPopMatrix();
		}
		if (left != NO_FACE) {
			glPushMatrix();
			glTranslatef(CUBE_ROUND, -CUBE_LENGTH, 0.0);
			glBegin(GL_TRIANGLES);
			pickColor(front, mono);
			glVertex3f(-STICKER_OFFSET, 0.0, STICKER_DEPTH);
			glVertex3f(-STICKERFULLLONG, STICKERFULLSHORT, STICKER_DEPTH);
			glVertex3f(-STICKERFULLLONG, -STICKERFULLSHORT, STICKER_DEPTH);
			glEnd();
			glPopMatrix();
		}
		if (top != NO_FACE) {
			glPushMatrix();
			glTranslatef(CUBE_LENGTH, -CUBE_ROUND, 0.0);
			glBegin(GL_TRIANGLES);
			pickColor(front, mono);
			glVertex3f(0.0, STICKER_OFFSET, STICKER_DEPTH);
			glVertex3f(STICKERFULLSHORT, STICKERFULLLONG, STICKER_DEPTH);
			glVertex3f(-STICKERFULLSHORT, STICKERFULLLONG, STICKER_DEPTH);
			glEnd();
			glPopMatrix();
		}
		if (right != NO_FACE) {
			glPushMatrix();
			glTranslatef(-CUBE_ROUND, CUBE_LENGTH, 0.0);
			glBegin(GL_TRIANGLES);
			pickColor(front, mono);
			glVertex3f(STICKER_OFFSET, 0.0, STICKER_DEPTH);
			glVertex3f(STICKERFULLLONG, -STICKERFULLSHORT, STICKER_DEPTH);
			glVertex3f(STICKERFULLLONG, STICKERFULLSHORT, STICKER_DEPTH);
			glEnd();
			glPopMatrix();
		}
		glPopMatrix();
	}
	if (left < NO_FACE) { /* Blue */
		glPushMatrix();
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray);
		glNormal3f(-1.0, 0.0, 0.0);
		if (top != NO_FACE) {
			glPushMatrix();
			glTranslatef(0.0, -CUBE_ROUND, CUBE_LENGTH);
			glBegin(GL_TRIANGLES);
			pickColor(left, mono);
			glVertex3f(-STICKER_DEPTH, STICKER_OFFSET, 0.0);
			glVertex3f(-STICKER_DEPTH, STICKERFULLLONG, STICKERFULLSHORT);
			glVertex3f(-STICKER_DEPTH, STICKERFULLLONG, -STICKERFULLSHORT);
			glEnd();
			glPopMatrix();
		}
		if (back != NO_FACE) {
			glPushMatrix();
			glTranslatef(0.0, CUBE_LENGTH, CUBE_ROUND);
			glBegin(GL_TRIANGLES);
			pickColor(left, mono);
			glVertex3f(-STICKER_DEPTH, 0.0, -STICKER_OFFSET);
			glVertex3f(-STICKER_DEPTH, STICKERFULLSHORT, -STICKERFULLLONG);
			glVertex3f(-STICKER_DEPTH, -STICKERFULLSHORT, -STICKERFULLLONG);
			glEnd();
			glPopMatrix();
		}
		if (bottom != NO_FACE) {
			glPushMatrix();
			glTranslatef(0.0, CUBE_ROUND, -CUBE_LENGTH);
			glBegin(GL_TRIANGLES);
			pickColor(left, mono);
			glVertex3f(-STICKER_DEPTH, -STICKER_OFFSET, 0.0);
			glVertex3f(-STICKER_DEPTH, -STICKERFULLLONG, -STICKERFULLSHORT);
			glVertex3f(-STICKER_DEPTH, -STICKERFULLLONG, STICKERFULLSHORT);
			glEnd();
			glPopMatrix();
		}
		if (front != NO_FACE) {
			glPushMatrix();
			glTranslatef(0.0, -CUBE_LENGTH, -CUBE_ROUND);
			glBegin(GL_TRIANGLES);
			pickColor(left, mono);
			glVertex3f(-STICKER_DEPTH, 0.0, STICKER_OFFSET);
			glVertex3f(-STICKER_DEPTH, -STICKERFULLSHORT, STICKERFULLLONG);
			glVertex3f(-STICKER_DEPTH, STICKERFULLSHORT, STICKERFULLLONG);
			glEnd();
			glPopMatrix();
		}
		glPopMatrix();
	}
	if (right < NO_FACE) { /* Green */
		glPushMatrix();
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray);
		glNormal3f(1.0, 0.0, 0.0);
		if (top != NO_FACE) {
			glPushMatrix();
			glTranslatef(0.0, -CUBE_ROUND, -CUBE_LENGTH);
			glBegin(GL_TRIANGLES);
			pickColor(right, mono);
			glVertex3f(STICKER_DEPTH, STICKER_OFFSET, 0.0);
			glVertex3f(STICKER_DEPTH, STICKERFULLLONG, -STICKERFULLSHORT);
			glVertex3f(STICKER_DEPTH, STICKERFULLLONG, STICKERFULLSHORT);
			glEnd();
			glPopMatrix();
		}
		if (front != NO_FACE) {
			glPushMatrix();
			glTranslatef(0.0, CUBE_LENGTH, -CUBE_ROUND);
			glBegin(GL_TRIANGLES);
			pickColor(right, mono);
			glVertex3f(STICKER_DEPTH, 0.0, STICKER_OFFSET);
			glVertex3f(STICKER_DEPTH, STICKERFULLSHORT, STICKERFULLLONG);
			glVertex3f(STICKER_DEPTH, -STICKERFULLSHORT, STICKERFULLLONG);
			glEnd();
			glPopMatrix();
		}
		if (bottom != NO_FACE) {
			glPushMatrix();
			glTranslatef(0.0, CUBE_ROUND, CUBE_LENGTH);
			glBegin(GL_TRIANGLES);
			pickColor(right, mono);
			glVertex3f(STICKER_DEPTH, -STICKER_OFFSET, 0.0);
			glVertex3f(STICKER_DEPTH, -STICKERFULLLONG, STICKERFULLSHORT);
			glVertex3f(STICKER_DEPTH, -STICKERFULLLONG, -STICKERFULLSHORT);
			glEnd();
			glPopMatrix();
		}
		if (back != NO_FACE) {
			glPushMatrix();
			glTranslatef(0.0, -CUBE_LENGTH, CUBE_ROUND);
			glBegin(GL_TRIANGLES);
			pickColor(right, mono);
			glVertex3f(STICKER_DEPTH, 0.0, -STICKER_OFFSET);
			glVertex3f(STICKER_DEPTH, -STICKERFULLSHORT, -STICKERFULLLONG);
			glVertex3f(STICKER_DEPTH, STICKERFULLSHORT, -STICKERFULLLONG);
			glEnd();
			glPopMatrix();
		}
		glPopMatrix();
	}
	if (bottom < NO_FACE) { /* White */
		glPushMatrix();
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray);
		glNormal3f(0.0, -1.0, 0.0);
		if (front != NO_FACE) {
			glPushMatrix();
			glTranslatef(-CUBE_LENGTH, 0.0, -CUBE_ROUND);
			glBegin(GL_TRIANGLES);
			pickColor(bottom, mono);
			glVertex3f(0.0, -STICKER_DEPTH, STICKER_OFFSET);
			glVertex3f(STICKERFULLSHORT, -STICKER_DEPTH, STICKERFULLLONG);
			glVertex3f(-STICKERFULLSHORT, -STICKER_DEPTH, STICKERFULLLONG);
			glEnd();
			glPopMatrix();
		}
		if (right != NO_FACE) {
			glPushMatrix();
			glTranslatef(-CUBE_ROUND, 0.0, CUBE_LENGTH);
			glBegin(GL_TRIANGLES);
			pickColor(bottom, mono);
			glVertex3f(STICKER_OFFSET, -STICKER_DEPTH, 0.0);
			glVertex3f(STICKERFULLLONG, -STICKER_DEPTH, -STICKERFULLSHORT);
			glVertex3f(STICKERFULLLONG, -STICKER_DEPTH, STICKERFULLSHORT);
			glEnd();
			glPopMatrix();
		}
		if (back != NO_FACE) {
			glPushMatrix();
			glTranslatef(CUBE_LENGTH, 0.0, CUBE_ROUND);
			glBegin(GL_TRIANGLES);
			pickColor(bottom, mono);
			glVertex3f(0.0, -STICKER_DEPTH, -STICKER_OFFSET);
			glVertex3f(-STICKERFULLSHORT, -STICKER_DEPTH, -STICKERFULLLONG);
			glVertex3f(STICKERFULLSHORT, -STICKER_DEPTH, -STICKERFULLLONG);
			glEnd();
			glPopMatrix();
		}
		if (left != NO_FACE) {
			glPushMatrix();
			glTranslatef(CUBE_ROUND, 0.0, -CUBE_LENGTH);
			glBegin(GL_TRIANGLES);
			pickColor(bottom, mono);
			glVertex3f(-STICKER_OFFSET, -STICKER_DEPTH, 0.0);
			glVertex3f(-STICKERFULLLONG, -STICKER_DEPTH, STICKERFULLSHORT);
			glVertex3f(-STICKERFULLLONG, -STICKER_DEPTH, -STICKERFULLSHORT);
			glEnd();
			glPopMatrix();
		}
		glPopMatrix();
	}
	if (top < NO_FACE) { /* Red */
		glPushMatrix();
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialGray);
		glNormal3f(0.0, 1.0, 0.0);
		if (back != NO_FACE) {
			glPushMatrix();
			glTranslatef(-CUBE_LENGTH, 0, CUBE_ROUND);
			glBegin(GL_TRIANGLES);
			pickColor(top, mono);
			glVertex3f(0.0, STICKER_DEPTH, -STICKER_OFFSET);
			glVertex3f(STICKERFULLSHORT, STICKER_DEPTH, -STICKERFULLLONG);
			glVertex3f(-STICKERFULLSHORT, STICKER_DEPTH, -STICKERFULLLONG);
			glEnd();
			glPopMatrix();
		}
		if (right != NO_FACE) {
			glPushMatrix();
			glTranslatef(-CUBE_ROUND, 0.0, -CUBE_LENGTH);
			glBegin(GL_TRIANGLES);
			pickColor(top, mono);
			glVertex3f(STICKER_OFFSET, STICKER_DEPTH, 0.0);
			glVertex3f(STICKERFULLLONG, STICKER_DEPTH, STICKERFULLSHORT);
			glVertex3f(STICKERFULLLONG, STICKER_DEPTH, -STICKERFULLSHORT);
			glEnd();
			glPopMatrix();
		}
		if (front != NO_FACE) {
			glPushMatrix();
			glTranslatef(CUBE_LENGTH, 0, -CUBE_ROUND);
			glBegin(GL_TRIANGLES);
			pickColor(top, mono);
			glVertex3f(0.0, STICKER_DEPTH, STICKER_OFFSET);
			glVertex3f(-STICKERFULLSHORT, STICKER_DEPTH, STICKERFULLLONG);
			glVertex3f(STICKERFULLSHORT, STICKER_DEPTH, STICKERFULLLONG);
			glEnd();
			glPopMatrix();
		}
		if (left != NO_FACE) {
			glPushMatrix();
			glTranslatef(CUBE_ROUND, 0.0, CUBE_LENGTH);
			glBegin(GL_TRIANGLES);
			pickColor(top, mono);
			glVertex3f(-STICKER_OFFSET, STICKER_DEPTH, 0.0);
			glVertex3f(-STICKERFULLLONG, STICKER_DEPTH, -STICKERFULLSHORT);
			glVertex3f(-STICKERFULLLONG, STICKER_DEPTH, STICKERFULLSHORT);
			glEnd();
			glPopMatrix();
		}
		glPopMatrix();
	}
}

static void
drawHalfCubit(ModeInfo * mi, int face, int x, int y, int z)
{
	dinostruct *dp = &dino[MI_SCREEN(mi)];
	const float h = 0.5;
	Bool s = dp->period2;
	int f1 = NO_FACE + 1;
	int f2 = NO_FACE + 1;

	if (x < 0) {
		if (y < 0) {
			if (z == 0) {
				glPushMatrix();
				glTranslatef(-h, -h, h);
				glPushMatrix();
				glRotatef(180.0, 0, 0, 1);
				s = (face == LEFT_FACE) ? 1 : 0;
				drawStickerlessHalfCubit(s); /* White Blue */
				glPopMatrix();
				if (face == LEFT_FACE)
					f1 = dp->cubeLoc[LEFT_FACE][2].face;
				else if (face == BOTTOM_FACE)
					f2 = dp->cubeLoc[BOTTOM_FACE][3].face;
				drawCubit(mi, /* LB */
					NO_FACE,
					NO_FACE,
					f1,
					NO_FACE,
					f2,
					NO_FACE);
				glPopMatrix();
			}
		} else if (y == 0) {
			if (z < 0) {
				glPushMatrix();
				glTranslatef(-h, -h, -h);
				glPushMatrix();
				glRotatef(90.0, 1, 0, 0);
				glRotatef(180.0, 0, 0, 1);
				s = (face == LEFT_FACE) ? 1 : 0;
				drawStickerlessHalfCubit(s); /* Blue Orange */
				glPopMatrix();
				if (face == BACK_FACE)
					f1 = dp->cubeLoc[BACK_FACE][3].face;
				else if (face == LEFT_FACE)
					f2 = dp->cubeLoc[LEFT_FACE][3].face;
				drawCubit(mi, /* BL */
					f1,
					NO_FACE,
					f2,
					NO_FACE,
					NO_FACE,
					NO_FACE);
				glPopMatrix();
			} else if (z > 0) {
				glPushMatrix();
				glTranslatef(-h, h, h);
				glPushMatrix();
				glRotatef(90.0, -1, 0, 0);
				glRotatef(180.0, 0, 0, 1);
				s = (face == LEFT_FACE) ? 1 : 0;
				drawStickerlessHalfCubit(s); /* Blue Yellow */
				glPopMatrix();
				if (face == FRONT_FACE)
					f1 = dp->cubeLoc[FRONT_FACE][3].face;
				else if (face == LEFT_FACE)
					f2 = dp->cubeLoc[LEFT_FACE][1].face;
				drawCubit(mi, /* FL */
					NO_FACE,
					f1,
					f2,
					NO_FACE,
					NO_FACE,
					NO_FACE);
				glPopMatrix();
			}
		} else /* if (y > 0) */ {
			if (z == 0) {
				glPushMatrix();
				glTranslatef(-h, h, -h);
				glPushMatrix();
				glRotatef(180.0, 0, 1, 0);
				s = (face == LEFT_FACE) ? 1 : 0;
				drawStickerlessHalfCubit(s); /* Red Blue */
				glPopMatrix();
				if (face == LEFT_FACE)
					f1 = dp->cubeLoc[LEFT_FACE][0].face;
				else if (face == TOP_FACE)
					f2 = dp->cubeLoc[TOP_FACE][3].face;
				drawCubit(mi, /* LT */
					NO_FACE,
					NO_FACE,
					f1,
					NO_FACE,
					NO_FACE,
					f2);
				glPopMatrix();
			}
		}
	} else if (x == 0) {
		if (y < 0) {
			if (z < 0) {
				glPushMatrix();
				glTranslatef(-h, -h, -h);
				glPushMatrix();
				glRotatef(90.0, 0, -1, 0);
				glRotatef(180.0, 0, 0, 1);
				s = (face == BACK_FACE) ? 1 : 0;
				drawStickerlessHalfCubit(s); /* White Orange */
				glPopMatrix();
				if (face == BACK_FACE)
					f1 = dp->cubeLoc[BACK_FACE][0].face;
				else if (face == BOTTOM_FACE)
					f2 = dp->cubeLoc[BOTTOM_FACE][2].face;
				drawCubit(mi, /* BB */
					f1,
					NO_FACE,
					NO_FACE,
					NO_FACE,
					f2,
					NO_FACE);
				glPopMatrix();
			} else if (z > 0) {
				glPushMatrix();
				glTranslatef(h, -h, h);
				glPushMatrix();
				glRotatef(90.0, 0, 1, 0);
				glRotatef(180.0, 0, 0, 1);
				s = (face == FRONT_FACE) ? 1 : 0;
				drawStickerlessHalfCubit(s); /* White Yellow */
				glPopMatrix();
				if (face == FRONT_FACE)
					f1 = dp->cubeLoc[FRONT_FACE][2].face;
				else if (face == BOTTOM_FACE)
					f2 = dp->cubeLoc[BOTTOM_FACE][0].face;
				drawCubit(mi, /* FB */
					NO_FACE,
					f1,
					NO_FACE,
					NO_FACE,
					f2,
					NO_FACE);
				glPopMatrix();
			}
		} else if (y > 0) {
			if (z < 0) {
				glPushMatrix();
				glTranslatef(h, h, -h);
				glPushMatrix();
				glRotatef(90.0, 0, 1, 0);
				s = (face == TOP_FACE) ? 1 : 0;
				drawStickerlessHalfCubit(s); /* Red Orange */
				glPopMatrix();
				if (face == BACK_FACE)
					f1 = dp->cubeLoc[BACK_FACE][2].face;
				else if (face == TOP_FACE)
					f2 = dp->cubeLoc[TOP_FACE][0].face;
				drawCubit(mi, /* BT */
					f1,
					NO_FACE,
					NO_FACE,
					NO_FACE,
					NO_FACE,
					f2);
				glPopMatrix();
			} else if (z > 0) {
				glPushMatrix();
				glTranslatef(-h, h, h);
				glPushMatrix();
				glRotatef(-90.0, 0, 1, 0);
				s = (face == FRONT_FACE) ? 1 : 0;
				drawStickerlessHalfCubit(s); /* Red Yellow */
				glPopMatrix();
				if (face == FRONT_FACE)
					f1 = dp->cubeLoc[FRONT_FACE][0].face;
				else if (face == TOP_FACE)
					f2 = dp->cubeLoc[TOP_FACE][2].face;
				drawCubit(mi, /* FT */
					NO_FACE,
					f1,
					NO_FACE,
					NO_FACE,
					NO_FACE,
					f2);
				glPopMatrix();
			}
		}
	} else /* if (x > 0) */ {
		if (y < 0) {
			if (z == 0) {
				glPushMatrix();
				glTranslatef(h, -h, -h);
				glPushMatrix();
				glRotatef(180.0, 0, 1, 0);
				glRotatef(180.0, 0, 0, 1);
				s = (face == RIGHT_FACE) ? 1 : 0;
				drawStickerlessHalfCubit(s); /* White Green */
				glPopMatrix();
				if (face == RIGHT_FACE)
					f1 = dp->cubeLoc[RIGHT_FACE][2].face;
				else if (face == BOTTOM_FACE)
					f2 = dp->cubeLoc[BOTTOM_FACE][1].face;
				drawCubit(mi, /* RB */
					NO_FACE,
					NO_FACE,
					NO_FACE,
					f1,
					f2,
					NO_FACE);
				glPopMatrix();
			}
		} else if (y == 0) {
			if (z < 0) {
				glPushMatrix();
				glTranslatef(h, h, -h);
				glPushMatrix();
				glRotatef(90.0, -1, 0, 0);
				s = (face == RIGHT_FACE) ? 1 : 0;
				drawStickerlessHalfCubit(s); /* Green Orange */
				glPopMatrix();
				if (face == BACK_FACE)
					f1 = dp->cubeLoc[BACK_FACE][1].face;
				else if (face == RIGHT_FACE)
					f2 = dp->cubeLoc[RIGHT_FACE][1].face;
				drawCubit(mi, /* BR */
					f1,
					NO_FACE,
					NO_FACE,
					f2,
					NO_FACE,
					NO_FACE);
				glPopMatrix();
			} else if (z > 0) {
				glPushMatrix();
				glTranslatef(h, -h, h);
				glPushMatrix();
				glRotatef(90.0, 1, 0, 0);
				s = (face == RIGHT_FACE) ? 1 : 0;
				drawStickerlessHalfCubit(s); /* Green Yellow */
				glPopMatrix();
				if (face == FRONT_FACE)
					f1 = dp->cubeLoc[FRONT_FACE][1].face;
				else if (face == RIGHT_FACE)
					f2 = dp->cubeLoc[RIGHT_FACE][3].face;
				drawCubit(mi, /* FR */
					NO_FACE,
					f1,
					NO_FACE,
					f2,
					NO_FACE,
					NO_FACE);
				glPopMatrix();
			}
		} else /* if (y > 0) */ {
			if (z == 0) {
				glPushMatrix();
				glTranslatef(h, h, h);
				glPushMatrix();
				s = (face == RIGHT_FACE) ? 1 : 0;
				drawStickerlessHalfCubit(s); /* Red Green */
				glPopMatrix();
				if (face == RIGHT_FACE)
					f1 = dp->cubeLoc[RIGHT_FACE][0].face;
				else if (face == TOP_FACE)
					f2 = dp->cubeLoc[TOP_FACE][1].face;
				drawCubit(mi, /* RT */
					NO_FACE,
					NO_FACE,
					NO_FACE,
					f1,
					NO_FACE,
					f2);
				glPopMatrix();
			}
		}
	}
}

static void
drawWholeFace(ModeInfo * mi, int face)
{
	int s, v = 1;

	switch (face) {
	case 0:
		s = 1;
		drawHalfCubit(mi, face, 0, s, -v);
		drawHalfCubit(mi, face, v, s, 0);
		drawHalfCubit(mi, face, 0, s, v);
		drawHalfCubit(mi, face, -v, s, 0);
		break;
	case 1:
		s = -1;
		drawHalfCubit(mi, face, s, v, 0);
		drawHalfCubit(mi, face, s, 0, v);
		drawHalfCubit(mi, face, s, -v, 0);
		drawHalfCubit(mi, face, s, 0, -v);
		break;
	case 2:
		s = 1;
		drawHalfCubit(mi, face, 0, v, s);
		drawHalfCubit(mi, face, v, 0, s);
		drawHalfCubit(mi, face, 0, -v, s);
		drawHalfCubit(mi, face, -v, 0, s);
		break;
	case 3:
		s = 1;
		drawHalfCubit(mi, face, s, v, 0);
		drawHalfCubit(mi, face, s, 0, -v);
		drawHalfCubit(mi, face, s, -v, 0);
		drawHalfCubit(mi, face, s, 0, v);
		break;
	case 4:
		s = -1;
		drawHalfCubit(mi, face, 0, s, v);
		drawHalfCubit(mi, face, v, s, 0);
		drawHalfCubit(mi, face, 0, s, -v);
		drawHalfCubit(mi, face, -v, s, 0);
		break;
	case 5:
		s = -1;
		drawHalfCubit(mi, face, 0, -v, s);
		drawHalfCubit(mi, face, v, 0, s);
		drawHalfCubit(mi, face, 0, v, s);
		drawHalfCubit(mi, face, -v, 0, s);
		break;
	}
}

static void
drawHalfFace(ModeInfo * mi, int face, int rad)
{
	int s, v = 1;

	switch (face) {
	case 0:
		s = 1;
		if (rad == 0 || rad == 3)
			drawHalfCubit(mi, face, 0, s, -v);
		if (rad == 0 || rad == 1)
			drawHalfCubit(mi, face, v, s, 0);
		if (rad == 1 || rad == 2)
			drawHalfCubit(mi, face, 0, s, v);
		if (rad == 2 || rad == 3)
			drawHalfCubit(mi, face, -v, s, 0);
		break;
	case 1:
		s = -1;
		if (rad == 0 || rad == 3)
			drawHalfCubit(mi, face, s, v, 0);
		if (rad == 0 || rad == 1)
			drawHalfCubit(mi, face, s, 0, v);
		if (rad == 1 || rad == 2)
			drawHalfCubit(mi, face, s, -v, 0);
		if (rad == 2 || rad == 3)
			drawHalfCubit(mi, face, s, 0, -v);
		break;
	case 2:
		s = 1;
		if (rad == 0 || rad == 3)
			drawHalfCubit(mi, face, 0, v, s);
		if (rad == 0 || rad == 1)
			drawHalfCubit(mi, face, v, 0, s);
		if (rad == 1 || rad == 2)
			drawHalfCubit(mi, face, 0, -v, s);
		if (rad == 2 || rad == 3)
			drawHalfCubit(mi, face, -v, 0, s);
		break;
	case 3:
		s = 1;
		if (rad == 0 || rad == 3)
			drawHalfCubit(mi, face, s, v, 0);
		if (rad == 0 || rad == 1)
			drawHalfCubit(mi, face, s, 0, -v);
		if (rad == 1 || rad == 2)
			drawHalfCubit(mi, face, s, -v, 0);
		if (rad == 2 || rad == 3)
			drawHalfCubit(mi, face, s, 0, v);
		break;
	case 4:
		s = -1;
		if (rad == 0 || rad == 3)
			drawHalfCubit(mi, face, 0, s, v);
		if (rad == 0 || rad == 1)
			drawHalfCubit(mi, face, v, s, 0);
		if (rad == 1 || rad == 2)
			drawHalfCubit(mi, face, 0, s, -v);
		if (rad == 2 || rad == 3)
			drawHalfCubit(mi, face, -v, s, 0);
		break;
	case 5:
		s = -1;
		if (rad == 0 || rad == 3)
			drawHalfCubit(mi, face, 0, -v, s);
		if (rad == 0 || rad == 1)
			drawHalfCubit(mi, face, v, 0, s);
		if (rad == 1 || rad == 2)
			drawHalfCubit(mi, face, 0, v, s);
		if (rad == 2 || rad == 3)
			drawHalfCubit(mi, face, -v, 0, s);
		break;
	}
}

static void
drawWholeCubit(ModeInfo * mi, int x, int y, int z)
{
	dinostruct *dp = &dino[MI_SCREEN(mi)];
	const float h = 0.5;
	Bool s = dp->period2;

	if (x < 0) {
		if (y < 0) {
			if (z == 0) {
				glPushMatrix();
				glTranslatef(-h, -h, h);
				glPushMatrix();
				glRotatef(180.0, 0, 0, 1);
				drawStickerlessCubit(s); /* White Blue */
				glPopMatrix();
				drawCubit(mi, /* LB */
					NO_FACE,
					NO_FACE,
					dp->cubeLoc[LEFT_FACE][2].face,
					NO_FACE,
					dp->cubeLoc[BOTTOM_FACE][3].face,
					NO_FACE);
				glPopMatrix();
			}
		} else if (y == 0) {
			if (z < 0) {
				glPushMatrix();
				glTranslatef(-h, -h, -h);
				glPushMatrix();
				glRotatef(90.0, 1, 0, 0);
				glRotatef(180.0, 0, 0, 1);
				drawStickerlessCubit(s); /* Blue Orange */
				glPopMatrix();
				drawCubit(mi, /* BL */
					dp->cubeLoc[BACK_FACE][3].face,
					NO_FACE,
					dp->cubeLoc[LEFT_FACE][3].face,
					NO_FACE,
					NO_FACE,
					NO_FACE);
				glPopMatrix();
			} else if (z > 0) {
				glPushMatrix();
				glTranslatef(-h, h, h);
				glPushMatrix();
				glRotatef(90.0, -1, 0, 0);
				glRotatef(180.0, 0, 0, 1);
				drawStickerlessCubit(s); /* Blue Yellow */
				glPopMatrix();
				drawCubit(mi, /* FL */
					NO_FACE,
					dp->cubeLoc[FRONT_FACE][3].face,
					dp->cubeLoc[LEFT_FACE][1].face,
					NO_FACE,
					NO_FACE,
					NO_FACE);
				glPopMatrix();
			}
		} else /* if (y > 0) */ {
			if (z == 0) {
				glPushMatrix();
				glTranslatef(-h, h, -h);
				glPushMatrix();
				glRotatef(180.0, 0, 1, 0);
				drawStickerlessCubit(s); /* Red Blue */
				glPopMatrix();
				drawCubit(mi, /* LT */
					NO_FACE,
					NO_FACE,
					dp->cubeLoc[LEFT_FACE][0].face,
					NO_FACE,
					NO_FACE,
					dp->cubeLoc[TOP_FACE][3].face);
				glPopMatrix();
			}
		}
	} else if (x == 0) {
		if (y < 0) {
			if (z < 0) {
				glPushMatrix();
				glTranslatef(-h, -h, -h);
				glPushMatrix();
				glRotatef(90.0, 0, -1, 0);
				glRotatef(180.0, 0, 0, 1);
				drawStickerlessCubit(s); /* White Orange */
				glPopMatrix();
				drawCubit(mi, /* BB */
					dp->cubeLoc[BACK_FACE][0].face,
					NO_FACE,
					NO_FACE,
					NO_FACE,
					dp->cubeLoc[BOTTOM_FACE][2].face,
					NO_FACE);
				glPopMatrix();
			} else if (z > 0) {
				glPushMatrix();
				glTranslatef(h, -h, h);
				glPushMatrix();
				glRotatef(90.0, 0, 1, 0);
				glRotatef(180.0, 0, 0, 1);
				drawStickerlessCubit(s); /* White Yellow */
				glPopMatrix();
				drawCubit(mi, /* FB */
					NO_FACE,
					dp->cubeLoc[FRONT_FACE][2].face,
					NO_FACE,
					NO_FACE,
					dp->cubeLoc[BOTTOM_FACE][0].face,
					NO_FACE);
				glPopMatrix();
			}
		} else if (y > 0) {
			if (z < 0) {
				glPushMatrix();
				glTranslatef(h, h, -h);
				glPushMatrix();
				glRotatef(90.0, 0, 1, 0);
				drawStickerlessCubit(s); /* Red Orange */
				glPopMatrix();
				drawCubit(mi, /* BT */
					dp->cubeLoc[BACK_FACE][2].face,
					NO_FACE,
					NO_FACE,
					NO_FACE,
					NO_FACE,
					dp->cubeLoc[TOP_FACE][0].face);
				glPopMatrix();
			} else if (z > 0) {
				glPushMatrix();
				glTranslatef(-h, h, h);
				glPushMatrix();
				glRotatef(-90.0, 0, 1, 0);
				drawStickerlessCubit(s); /* Red Yellow */
				glPopMatrix();
				drawCubit(mi, /* FT */
					NO_FACE,
					dp->cubeLoc[FRONT_FACE][0].face,
					NO_FACE,
					NO_FACE,
					NO_FACE,
					dp->cubeLoc[TOP_FACE][2].face);
				glPopMatrix();
			}
		}
	} else /* if (x > 0) */ {
		if (y < 0) {
			if (z == 0) {
				glPushMatrix();
				glTranslatef(h, -h, -h);
				glPushMatrix();
				glRotatef(180.0, 0, 1, 0);
				glRotatef(180.0, 0, 0, 1);
				drawStickerlessCubit(s); /* White Green */
				glPopMatrix();
				drawCubit(mi, /* RB */
					NO_FACE,
					NO_FACE,
					NO_FACE,
					dp->cubeLoc[RIGHT_FACE][2].face,
					dp->cubeLoc[BOTTOM_FACE][1].face,
					NO_FACE);
				glPopMatrix();
			}
		} else if (y == 0) {
			if (z < 0) {
				glPushMatrix();
				glTranslatef(h, h, -h);
				glPushMatrix();
				glRotatef(90.0, -1, 0, 0);
				drawStickerlessCubit(s); /* Green Orange */
				glPopMatrix();
				drawCubit(mi, /* BR */
					dp->cubeLoc[BACK_FACE][1].face,
					NO_FACE,
					NO_FACE,
					dp->cubeLoc[RIGHT_FACE][1].face,
					NO_FACE,
					NO_FACE);
				glPopMatrix();
			} else if (z > 0) {
				glPushMatrix();
				glTranslatef(h, -h, h);
				glPushMatrix();
				glRotatef(90.0, 1, 0, 0);
				drawStickerlessCubit(s); /* Green Yellow */
				glPopMatrix();
				drawCubit(mi, /* FR */
					NO_FACE,
					dp->cubeLoc[FRONT_FACE][1].face,
					NO_FACE,
					dp->cubeLoc[RIGHT_FACE][3].face,
					NO_FACE,
					NO_FACE);
				glPopMatrix();
			}
		} else /* if (y > 0) */ {
			if (z == 0) {
				glPushMatrix();
				glTranslatef(h, h, h);
				glPushMatrix();
				drawStickerlessCubit(s); /* Red Green */
				glPopMatrix();
				drawCubit(mi, /* RT */
					NO_FACE,
					NO_FACE,
					NO_FACE,
					dp->cubeLoc[RIGHT_FACE][0].face,
					NO_FACE,
					dp->cubeLoc[TOP_FACE][1].face);
				glPopMatrix();
			}
		}
	}
}

static DinoLocPos slideCorner[MAX_FACES][MAX_ORIENT][MAX_ORIENT / 2] =
{
	{
		{
			{3, TR, 0},
			{5, BR, 0}
		},
		{
			{3, TL, 1},
			{2, TR, 0}
		},
		{
			{2, TL, 1},
			{1, TR, 0}
		},
		{
			{5, BL, 1},
			{1, TL, 1}
		}
	},
	{
		{
			{2, TL, 0},
			{0, BL, 0}
		},
		{
			{2, BL, 0},
			{4, TL, 0}
		},
		{
			{4, BL, 0},
			{5, TL, 0}
		},
		{
			{0, TL, 0},
			{5, BL, 0}
		}
	},
	{
		{
			{3, TL, 0},
			{0, BR, 0}
		},
		{
			{3, BL, 0},
			{4, TR, 0}
		},
		{
			{4, TL, 1},
			{1, BR, 1}
		},
		{
			{0, BL, 1},
			{1, TR, 1}
		}
	},
	{
		{
			{5, BR, 1},
			{0, TR, 1}
		},
		{
			{5, TR, 1},
			{4, BR, 1}
		},
		{
			{4, TR, 1},
			{2, BR, 1}
		},
		{
			{0, BR, 1},
			{2, TR, 1}
		}
	},
	{
		{
			{3, BL, 1},
			{2, BR, 0}
		},
		{
			{3, BR, 0},
			{5, TR, 0}
		},
		{
			{5, TL, 1},
			{1, BL, 1}
		},
		{
			{2, BL, 1},
			{1, BR, 0}
		}
	},
	{
		{
			{3, BR, 1},
			{4, BR, 0}
		},
		{
			{3, TR, 1},
			{0, TR, 0}
		},
		{
			{0, TL, 1},
			{1, TL, 0}
		},
		{
			{4, BL, 1},
			{1, BL, 0}
		}
	}
};

static int oppFace[MAX_FACES] =
{4, 3, 5, 1, 0, 2};

static DinoLoc oppCorner[MAX_FACES][MAX_ORIENT] =
{
	{
		{4, 3},
		{4, 2},
		{4, 1},
		{4, 0}
	},
	{
		{3, 1},
		{3, 0},
		{3, 3},
		{3, 2}
	},
	{
		{5, 3},
		{5, 2},
		{5, 1},
		{5, 0}
	},
	{
		{1, 1},
		{1, 0},
		{1, 3},
		{1, 2}
	},
	{
		{0, 3},
		{0, 2},
		{0, 1},
		{0, 0}
	},
	{
		{2, 3},
		{2, 2},
		{2, 1},
		{2, 0}
	}
};

static int nextCorner[MAX_FACES][MAX_ORIENT][2] =
{
	{{2, 6}, {7, 6}, {7, 3}, {2, 3}},
	{{2, 3}, {1, 3}, {1, 0}, {2, 0}},
	{{3, 7}, {5, 7}, {5, 1}, {3, 1}},
	{{7, 6}, {4, 6}, {4, 5}, {7, 5}},
	{{1, 5}, {4, 5}, {4, 0}, {1, 0}},
	{{0, 4}, {6, 4}, {6, 2}, {0, 2}}
};

static int nextFace[MAX_FACES][MAX_ORIENT] =
{
	{-4, -1, 4, 1},
	{-1, 2, 1, -2},
	{-4, 2, 4, -2},
	{1, 2, -1, -2},
	{-4, 1, 4, -1},
	{-4, -2, 4, 2},
};

static int edgeToFaces[MAX_EDGES][MAX_FACES] =
{
	{4, 5, 1, 3, 0, 2},
	{2, 4, 1, 3, 0, 5},
	{0, 5, 1, 3, 2, 4},
	{0, 2, 1, 3, 4, 5},

	{1, 5, 0, 4, 2, 3},
	{1, 2, 0, 4, 3, 5},
	{3, 5, 0, 4, 1, 2},
	{2, 3, 0, 4, 1, 5},

	{1, 4, 2, 5, 0, 3},
	{0, 1, 2, 5, 3, 4},
	{3, 4, 2, 5, 0, 1},
	{0, 3, 2, 5, 1, 4}
};

static int edgeToRad[MAX_EDGES][2] =
{
	{2, 1}, {1, 2}, {3, 0}, {0, 3},
	{3, 2}, {2, 3}, {0, 1}, {1, 0},
	{2, 3}, {3, 2}, {1, 0}, {0, 1},
};

static void
convertMove3(DinoMove move, DinoSlice * slice, int style)
{
		int position = move.position;

	if (style == MIDDLE)
		position = (position + 2) % MAX_ORIENT;
	(*slice).ce = nextCorner[move.face][position][move.direction % 2];
	switch (move.direction % MAX_ORIENT) {
	case 0:
		(*slice).rotation = (position == 1 || position == 2) ?
			CW : CCW;
		break;
	case 1:
		(*slice).rotation = (position == 2 || position == 3) ?
			CW : CCW;
		break;
	case 2:
		(*slice).rotation = (position == 1 || position == 2) ?
			CCW : CW;
		break;
	case 3:
		(*slice).rotation = (position == 2 || position == 3) ?
			CCW : CW;
		break;
	}
}

static void
convertMove2(DinoMove move, DinoSlice * split)
{
	int position = move.position;

	switch (move.face) {
	case 0:
		if (move.direction % 2 == 0) {
			if (position == 0 || position  == 3) {
				(*split).ce = 4;
				(*split).rotation = (move.direction == 2) ?
					CW : CCW;
			} else if (position == 1 || position  == 2) {
				(*split).ce = 7;
				(*split).rotation = (move.direction == 0) ?
					CW : CCW;
			}
		} else {
			if (position == 0 || position  == 1) {
				(*split).ce = 6;
				(*split).rotation = (move.direction == 3) ?
					CW : CCW;
			} else if (position == 2 || position  == 3) {
				(*split).ce = 5;
				(*split).rotation = (move.direction == 1) ?
					CW : CCW;
			}
		}
		break;
	case 1:
		if (move.direction % 2 == 0) {
			if (position == 0 || position  == 3) {
				(*split).ce = 2;
				(*split).rotation = (move.direction == 2) ?
					CW : CCW;
			} else if (position == 1 || position  == 2) {
				(*split).ce = 1;
				(*split).rotation = (move.direction == 0) ?
					CW : CCW;
			}
		} else {
			if (position == 0 || position  == 1) {
				(*split).ce = 3;
				(*split).rotation = (move.direction == 3) ?
					CW : CCW;
			} else if (position == 2 || position  == 3) {
				(*split).ce = 0;
				(*split).rotation = (move.direction == 1) ?
					CW : CCW;
			}
		}
		break;
	case 2:
		if (move.direction % 2 == 0) {
			if (position == 0 || position  == 3) {
				(*split).ce = 9;
				(*split).rotation = (move.direction == 2) ?
					CW : CCW;
			} else if (position == 1 || position  == 2) {
				(*split).ce = 10;
				(*split).rotation = (move.direction == 0) ?
					CW : CCW;
			}
		} else {
			if (position == 0 || position  == 1) {
				(*split).ce = 11;
				(*split).rotation = (move.direction == 3) ?
					CW : CCW;
			} else if (position == 2 || position  == 3) {
				(*split).ce = 8;
				(*split).rotation = (move.direction == 1) ?
					CW : CCW;
			}
		}
		break;
	case 3:
		if (move.direction % 2 == 0) {
			if (position == 0 || position  == 3) {
				(*split).ce = 3;
				(*split).rotation = (move.direction == 2) ?
					CW : CCW;
			} else if (position == 1 || position  == 2) {
				(*split).ce = 0;
				(*split).rotation = (move.direction == 0) ?
					CW : CCW;
			}
		} else {
			if (position == 0 || position  == 1) {
				(*split).ce = 2;
				(*split).rotation = (move.direction == 3) ?
					CW : CCW;
			} else if (position == 2 || position  == 3) {
				(*split).ce = 1;
				(*split).rotation = (move.direction == 1) ?
					CW : CCW;
			}
		}
		break;
	case 4:
		if (move.direction % 2 == 0) {
			if (position == 0 || position  == 3) {
				(*split).ce = 5;
				(*split).rotation = (move.direction == 2) ?
					CW : CCW;
			} else if (position == 1 || position  == 2) {
				(*split).ce = 6;
				(*split).rotation = (move.direction == 0) ?
					CW : CCW;
			}
		} else {
			if (position == 0 || position  == 1) {
				(*split).ce = 7;
				(*split).rotation = (move.direction == 3) ?
					CW : CCW;
			} else if (position == 2 || position  == 3) {
				(*split).ce = 4;
				(*split).rotation = (move.direction == 1) ?
					CW : CCW;
			}
		}
		break;
	case 5:
		if (move.direction % 2 == 0) {
			if (position == 0 || position  == 3) {
				(*split).ce = 8;
				(*split).rotation = (move.direction == 2) ?
					CW : CCW;
			} else if (position == 1 || position  == 2) {
				(*split).ce = 11;
				(*split).rotation = (move.direction == 0) ?
					CW : CCW;
			}
		} else {
			if (position == 0 || position  == 1) {
				(*split).ce = 10;
				(*split).rotation = (move.direction == 3) ?
					CW : CCW;
			} else if (position == 2 || position  == 3) {
				(*split).ce = 9;
				(*split).rotation = (move.direction == 1) ?
					CW : CCW;
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
cornerRotate(ModeInfo *mi, int corner, GLfloat rotateStep)
{
#define X1 (((corner & 4) != 0) ? 1 : -1)
#define Y1 (((corner & 2) != 0) ? 1 : -1)
#define Z1 (((corner & 1) != 0) ? 1 : -1)
#define Xn (((corner & 4) != 0) ? -1 : 1)
#define Yn (((corner & 2) != 0) ? -1 : 1)
#define Zn (((corner & 1) != 0) ? -1 : 1)
	drawCenter();
	glPushMatrix();
	glRotatef(rotateStep, X1, Y1, Z1);
	drawWholeCubit(mi, 0, Y1, Z1);
	drawWholeCubit(mi, X1, 0, Z1);
	drawWholeCubit(mi, X1, Y1, 0);
	glPopMatrix();

	drawWholeCubit(mi, 0, Yn, Zn);
	drawWholeCubit(mi, Xn, 0, Zn);
	drawWholeCubit(mi, Xn, Yn, 0);
	drawWholeCubit(mi, 0, Y1, Zn);
	drawWholeCubit(mi, 0, Yn, Z1);
	drawWholeCubit(mi, X1, 0, Zn);
	drawWholeCubit(mi, Xn, 0, Z1);
	drawWholeCubit(mi, X1, Yn, 0);
	drawWholeCubit(mi, Xn, Y1, 0);
}

static void
middleRotate(ModeInfo *mi, int corner, GLfloat rotateStep)
{
	drawCenter();
	glPushMatrix();
	glRotatef(rotateStep, X1, Y1, Z1);
	drawWholeCubit(mi, 0, Y1, Zn);
	drawWholeCubit(mi, 0, Yn, Z1);
	drawWholeCubit(mi, X1, 0, Zn);
	drawWholeCubit(mi, Xn, 0, Z1);
	drawWholeCubit(mi, X1, Yn, 0);
	drawWholeCubit(mi, Xn, Y1, 0);
	glPopMatrix();

	drawWholeCubit(mi, 0, Y1, Z1);
	drawWholeCubit(mi, X1, 0, Z1);
	drawWholeCubit(mi, X1, Y1, 0);
	drawWholeCubit(mi, 0, Yn, Zn);
	drawWholeCubit(mi, Xn, 0, Zn);
	drawWholeCubit(mi, Xn, Yn, 0);
}

static void
edgeRotate(ModeInfo *mi, int edge, GLfloat rotateStep)
{
	float Xf = 0.0, Yf = 0.0, Zf = 0.0;
	int radian1, radian2;

	if (edge < 4) {
		Yf = (edge / 2 == 1) ? 1.0 : -1.0;
		Zf = (edge % 2 == 1) ? 1.0 : -1.0;
	} else if (edge >= 8) {
		Xf = ((edge % 4) / 2 == 1) ? 1.0 : -1.0;
		Yf = (edge % 2 == 1) ? 1.0 : -1.0;
	} else {
		Xf = ((edge % 4) / 2 == 1) ? 1.0 : -1.0;
		Zf = (edge % 2 == 1) ? 1.0 : -1.0;
	}
	radian1 = edgeToRad[edge][0];
	radian2 = edgeToRad[edge][1];
	drawCenter();
	glPushMatrix();
	glRotatef(rotateStep, Xf, Yf, Zf);
	drawWholeFace(mi, edgeToFaces[edge][0]);
	drawWholeFace(mi, edgeToFaces[edge][1]);
	drawHalfFace(mi, edgeToFaces[edge][2], radian1);
	drawHalfFace(mi, edgeToFaces[edge][3], radian2);
	glPopMatrix();

	drawWholeFace(mi, edgeToFaces[edge][4]);
	drawWholeFace(mi, edgeToFaces[edge][5]);
	drawHalfFace(mi, edgeToFaces[edge][2], (radian1 + 2) % MAX_ORIENT);
	drawHalfFace(mi, edgeToFaces[edge][3], (radian2 + 2) % MAX_ORIENT);
}

/*-
 * This rotates whole cube.
 */
static void
faceRotate(ModeInfo *mi, int face, GLfloat rotateStep)
{
#define XP ((face == 4) ? 1 : ((face == -4) ? -1 : 0))
#define YP ((face == 2) ? 1 : ((face == -2) ? -1 : 0))
#define ZP ((face == 1) ? 1 : ((face == -1) ? -1 : 0))
	int corner = 0;

	drawCenter();
	glPushMatrix();
	glRotatef(rotateStep, XP, YP, ZP);
	drawWholeCubit(mi, 0, Y1, Zn);
	drawWholeCubit(mi, 0, Yn, Z1);
	drawWholeCubit(mi, X1, 0, Zn);
	drawWholeCubit(mi, Xn, 0, Z1);
	drawWholeCubit(mi, X1, Yn, 0);
	drawWholeCubit(mi, Xn, Y1, 0);

	drawWholeCubit(mi, 0, Y1, Z1);
	drawWholeCubit(mi, X1, 0, Z1);
	drawWholeCubit(mi, X1, Y1, 0);
	drawWholeCubit(mi, 0, Yn, Zn);
	drawWholeCubit(mi, Xn, 0, Zn);
	drawWholeCubit(mi, Xn, Yn, 0);
	glPopMatrix();
}

static void
drawCube(ModeInfo *mi)
{
	dinostruct *dp = &dino[MI_SCREEN(mi)];
	DinoSlice  slice;
	GLfloat     rotateStep = 0.0;

	slice.ce = NO_CORNER;
	slice.rotation = CW;
	if (dp->movement.face == NO_FACE) {
		slice.rotation = NO_ROTATION;
		cornerRotate(mi, slice.ce, rotateStep);
	} else if (dp->movement.style == CORNER &&
			dp->movement.direction < MAX_ORIENT) {
		convertMove3(dp->movement, &slice, CORNER);
		if (slice.ce != NO_CORNER) {
			rotateStep = (float) ((slice.rotation == CCW) ?
				dp->rotateStep : -dp->rotateStep);
		}
		cornerRotate(mi, slice.ce, rotateStep);
	} else if (dp->movement.style == MIDDLE) {
		convertMove3(dp->movement, &slice, MIDDLE);
		if (slice.ce != NO_CORNER) {
			rotateStep = (float) ((slice.rotation == CCW) ?
				dp->rotateStep : -dp->rotateStep);
		}
		middleRotate(mi, slice.ce, rotateStep);
	} else if (dp->movement.style == EDGE) {
		DinoSlice split;

		split.ce = NO_EDGE;
		split.rotation = CW;
		convertMove2(dp->movement, &split);
		rotateStep = (float) ((split.rotation == CCW) ?
			dp->rotateStep : -dp->rotateStep);
		edgeRotate(mi, split.ce, rotateStep);
	} else {
		int face = nextFace[dp->movement.face][dp->movement.direction % MAX_ORIENT];

		rotateStep = (float) dp->rotateStep;
		faceRotate(mi, face, rotateStep);
	}
}

/* From David Bagley's xdino.  Used by permission. ;)  */
static void
rotateFace(dinostruct * dp, int face, int direction)
{
	int side;

	/* Read Face */
	for (side = 0; side < MAX_ORIENT; side++)
		dp->faceLoc[side] = dp->cubeLoc[face][side];
	/* Write Face */
	for (side = 0; side < MAX_ORIENT; side++) {
		dp->cubeLoc[face][side] = (direction == CW) ? 
			dp->faceLoc[(side + MAX_ORIENT - 1) % MAX_ORIENT] :
			dp->faceLoc[(side + 1) % MAX_ORIENT];
		dp->cubeLoc[face][side].rotation =
			(dp->cubeLoc[face][side].rotation + direction) %
			MAX_ORIENT;
		/* drawTriangle(face, side, False); */
	}
}

static void
readDiagonal(dinostruct * dp, int face, int corner, int h)
{
	dp->spindleLoc[h][0] = dp->cubeLoc[face][corner];
	dp->spindleLoc[h][1] = dp->cubeLoc[face][(corner + 1) % MAX_ORIENT];
}

static void
writeDiagonal(dinostruct * dp, int face, int corner, int rotate, int h)
{
	dp->spindleLoc[h][0].rotation =
		(dp->spindleLoc[h][0].rotation + rotate) % MAX_ORIENT;
	dp->spindleLoc[h][1].rotation =
		(dp->spindleLoc[h][1].rotation + rotate) % MAX_ORIENT;
	dp->cubeLoc[face][corner] = dp->spindleLoc[h][0];
	/* drawTriangle(face, corner, False); */
	dp->cubeLoc[face][(corner + 1) % MAX_ORIENT] = dp->spindleLoc[h][1];
	/* drawTriangle(face, (corner + 1) % MAX_ORIENT, False); */
}

static void
readFace(dinostruct * dp, int face, int h)
{
	int side;

	for (side = 0; side < MAX_ORIENT; side++)
		dp->rowLoc[h][side] = dp->cubeLoc[face][side];
}

static void
writeFace(dinostruct * dp, int face, int rotate, int h)
{
	int side, newSide;

	for (side = 0; side < MAX_ORIENT; side++) {
		newSide = (side + rotate) % MAX_ORIENT;
		dp->cubeLoc[face][newSide] = dp->rowLoc[h][side];
		dp->cubeLoc[face][newSide].rotation =
			(dp->cubeLoc[face][newSide].rotation + rotate) %
			MAX_ORIENT;
		/* drawTriangle(face, (side + rotate) % MAX_ORIENT, False); */
	}
}

static void
moveEdges(dinostruct * dp, int face, int corner, int direction)
{
	int oldFace = face, corn = corner, dir = direction;
	int k, newFace, rotate, newCorner, newDirection;

	readDiagonal(dp, oldFace, corn, 0);
	for (k = 1; k <= 2; k++) {
		newFace = oppFace[oldFace];
		/*rotate = (((oldFace == 1 || oldFace == 3) ? 1 : 3) + 3 * dir) %
		  MAX_ORIENT; */
		newCorner = ((((((oldFace == 1 || oldFace == 3) ? 1 : 0) +
			corn) & 1) == 1) ? (corn + 3) :
			(corn + 1)) % MAX_ORIENT;
		rotate = (newCorner - corn + MAX_ORIENT) % MAX_ORIENT;
		newDirection = (rotate + dir) % MAX_ORIENT;
		if (k != 2)
			readDiagonal(dp, newFace, newCorner, k);
		writeDiagonal(dp, newFace, newCorner, rotate, k - 1);
		oldFace = newFace;
		corn = newCorner;
		dir = newDirection;
	}
}

static void
moveFaces(dinostruct * dp, int f, int d, int rotate)
{
	int k, face, newFace, rot = rotate;

	face = faceToRotate2[f][d][0];
	readFace(dp, face, 0);
	for (k = 1; k <= 2; k++) {
		newFace = faceToRotate2[f][d][k & 1];
		rot = MAX_ORIENT - rot;
		if (k != 2)
			readFace(dp, newFace, k);
		writeFace(dp, newFace, rot, k - 1);
		face = newFace;
	}
}

static void
moveInsideCorners(dinostruct * dp, int face, int corner, int direction)
{
	int oldFace = face, corn = corner, oldDir = direction;
	int newFace, newCorner, newDirection, dir, k;

	readDiagonal(dp, oldFace, corn, 0);
	for (k = 1; k <= MAX_ROTATE; k++) {
		dir = oldDir / 2;
		newFace = slideCorner[oldFace][corn][dir].face;
		newCorner = slideCorner[oldFace][corn][dir].position;
		newDirection = 2 * slideCorner[oldFace][corn][dir].direction +
			(((newCorner & 1) == 0) ? 1 : 0);
		if (k != MAX_ROTATE)
			readDiagonal(dp, newFace, newCorner, k);
		writeDiagonal(dp, newFace, newCorner,
			(newDirection - oldDir + MAX_ORIENT) %
			MAX_ORIENT, k - 1);
		oldFace = newFace;
		corn = newCorner;
		oldDir = newDirection;
	}
}

static void
moveOutsideCorners(dinostruct * dp, int face, int corner, int direction)
{
	int oldFace = face, corn = corner, oldDir = direction;
	int newFace, newCorner, newDirection, dir, k;

	readDiagonal(dp, oldFace, corn, 0);
	for (k = 1; k <= MAX_ROTATE; k++) {
		corn = (corn + 2) % MAX_ORIENT;
		dir = oldDir / 2;
		newFace = slideCorner[oldFace][corn][dir].face;
		newCorner = (slideCorner[oldFace][corn][dir].position + 2) %
			MAX_ORIENT;
		newDirection = 2 * slideCorner[oldFace][corn][dir].direction +
			(((newCorner & 1) == 0) ? 1 : 0);
		if (k != MAX_ROTATE)
			readDiagonal(dp, newFace, newCorner, k);
		writeDiagonal(dp, newFace, newCorner,
			(newDirection - oldDir + MAX_ORIENT) %
			MAX_ORIENT, k - 1);
		oldFace = newFace;
		corn = newCorner;
		oldDir = newDirection;
	}
}

static void
moveDino(dinostruct * dp, int face, int position, int direction, int style)
{
	int oldFace = face, dir = direction;
	int corner, newCorner;

	corner = (position - ((((position + dir) & 1) == 0) ?
		1 : 0) + MAX_ORIENT) % MAX_ORIENT;
	if (style == CORNER) {
		moveInsideCorners(dp, oldFace, corner, dir);
	} else if (style == MIDDLE) {
		moveOutsideCorners(dp, oldFace, corner, dir);
		newCorner = oppCorner[oldFace][corner].rotation;
		oldFace = oppCorner[oldFace][corner].face;
		if (((((oldFace == 1 || oldFace == 3) ? 0 : 1) + corner) &
				1) == 1)
			dir = (dir + 1) % MAX_ORIENT;
		else
			dir = (dir + 3) % MAX_ORIENT;
		corner = newCorner;
		moveOutsideCorners(dp, oldFace, corner, dir);
	} else if (style == EDGE) {
		moveEdges(dp, oldFace, corner, dir);
		if (oldFace == 0 && corner == 1 && (dir & 1) == 1)
			moveFaces(dp, oldFace, corner, 0);
		else if (oldFace == 0 && corner == 3 && (dir & 1) == 0)
			moveFaces(dp, oldFace, corner, 0);
		else if (oldFace == 0 && corner == 0)
			moveFaces(dp, oldFace, corner, 0);
		else if (oldFace == 4 && corner == 1 && (dir & 1) == 0)
			moveFaces(dp, oldFace, corner, 0);
		else if (oldFace == 4 && corner == 3 && (dir & 1) == 1)
			moveFaces(dp, oldFace, corner, 0);
		else if (oldFace == 4 && corner == 2)
			moveFaces(dp, oldFace, corner, 0);
		else
			moveFaces(dp, oldFace, corner,
				((oldFace == 2 || oldFace == 5) ? CCW : HALF) %
				MAX_ORIENT);
	} else {
		int k, newFace, rotate, newDirection;

		rotateFace(dp, faceToRotate[oldFace][dir % MAX_ORIENT], CW);
		rotateFace(dp, faceToRotate[oldFace][(dir + 2) % MAX_ORIENT],
			CCW);
		readFace(dp, oldFace, 0);
		for (k = 1; k <= MAX_ORIENT; k++) {
			newFace = slideNextFace[oldFace][dir % MAX_ORIENT].face;
			rotate = slideNextFace[oldFace][dir % MAX_ORIENT].rotation;
			newDirection = (rotate + dir) % MAX_ORIENT;
			if (k != MAX_ORIENT)
				readFace(dp, newFace, k);
			writeFace(dp, newFace, rotate, k - 1);
			oldFace = newFace;
			dir = newDirection;
		}
	}
}

#ifdef DEBUG
static void
printCube(dinostruct * dp)
{
	int	 face, position;

	for (face = 0; face < MAX_FACES; face++) {
		for (position = 0; position < MAX_ORIENT; position++) {
			(void) printf("%d %d  ", dp->cubeLoc[face][position].face,
			dp->cubeLoc[face][position].rotation);
		}
		(void) printf("\n");
	}
	(void) printf("\n");
}

#endif

static void
evalmovement(ModeInfo * mi, DinoMove movement)
{
	dinostruct *dp = &dino[MI_SCREEN(mi)];

#ifdef DEBUG
	printCube(dp);
#endif
	if (movement.face < 0 || movement.face >= MAX_FACES)
		return;
	moveDino(dp, movement.face, movement.position, movement.direction,
		movement.style);

}

static Bool
compareMoves(DinoMove move1, DinoMove move2)
{
	DinoSlice slice1, slice2;

	if (move1.style == EDGE)
		convertMove2(move1, &slice1);
	else
		convertMove3(move1, &slice1, move1.style);
	if (move2.style == EDGE)
		convertMove2(move2, &slice2);
	else
		convertMove3(move2, &slice2, move2.style);
	if (move1.style != move2.style)
		return False;
	return (slice1.ce == slice2.ce);
}

static Bool
shuffle(ModeInfo * mi)
{
	dinostruct *dp = &dino[MI_SCREEN(mi)];
	int	 i, face, position;
	DinoMove   move;

	for (face = 0; face < MAX_FACES; face++) {
		for (position = 0; position < MAX_ORIENT; position++) {
			dp->cubeLoc[face][position].face = face;
			dp->cubeLoc[face][position].rotation = TOP;
		}
	}
	dp->storedmoves = MI_COUNT(mi);
	if (dp->storedmoves < 0) {
		if (dp->moves != NULL)
			free(dp->moves);
		dp->moves = (DinoMove *) NULL;
		dp->storedmoves = NRAND(-dp->storedmoves) + 1;
	}
	if ((dp->storedmoves) && (dp->moves == NULL))
		if ((dp->moves = (DinoMove *) calloc(dp->storedmoves + 1,
				sizeof (DinoMove))) == NULL) {
			return False;
		}
	if (MI_CYCLES(mi) <= 1) {
		dp->angleStep = dp->degreeTurn;
	} else {
		dp->angleStep = dp->degreeTurn / (GLfloat) (MI_CYCLES(mi));
	}

	for (i = 0; i < dp->storedmoves; i++) {
		Bool condition;

		do {
			move.face = NRAND(MAX_FACES);
			move.position = NRAND(MAX_ORIENT);
			move.direction = ((move.position +
				((NRAND(2) == 0) ? 1 : 3)) % MAX_ORIENT);
			move.style = ((dp->period2) ? EDGE : CORNER);
			condition = True;
			if (i > 0 && compareMoves(move, dp->moves[i - 1])) {
				condition = False;
			}
		} while (!condition);
		if (hideshuffling)
			evalmovement(mi, move);
		dp->moves[i] = move;
	}
	dp->VX = 0.05;
	if (NRAND(100) < 50)
		dp->VX *= -1;
	dp->VY = 0.05;
	if (NRAND(100) < 50)
		dp->VY *= -1;
	dp->movement.face = NO_FACE;
	dp->rotateStep = 0;
	dp->action = hideshuffling ? ACTION_SOLVE : ACTION_SHUFFLE;
	dp->shufflingmoves = 0;
	dp->done = 0;
	return True;
}

static void
reshape_dino(ModeInfo * mi, int width, int height)
{
	dinostruct *dp = &dino[MI_SCREEN(mi)];

	glViewport(0, 0, dp->WindW = (GLint) width, dp->WindH = (GLint) height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-1.0, 1.0, -1.0, 1.0, 5.0, 15.0);
	glMatrixMode(GL_MODELVIEW);

	dp->AreObjectsDefined[ObjCubit] = False;
}

#ifdef STANDALONE
ENTRYPOINT Bool
dino_handle_event (ModeInfo *mi, XEvent *event)
{
	dinostruct *dp = &dino[MI_SCREEN(mi)];

	if (gltrackball_event_handler (event, dp->trackball,
			MI_WIDTH (mi), MI_HEIGHT (mi),
			&dp->button_down_p))
		return True;
	else if (screenhack_event_helper (MI_DISPLAY(mi), MI_WINDOW(mi), event)) {
		dp->done = 1;
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
free_dino_screen(dinostruct *dp)
{
	if (dp == NULL) {
		return;
	}
#ifdef STANDALONE
	if (dp->trackball)
		gltrackball_free (dp->trackball);
#endif
	if (dp->moves != NULL) {
		free(dp->moves);
		dp->moves = (DinoMove *) NULL;
	}
	dp = NULL;
}

ENTRYPOINT void
release_dino(ModeInfo * mi)
{
	if (dino != NULL) {
		int	 screen;

		for (screen = 0; screen < MI_NUM_SCREENS(mi); screen++)
			free_dino_screen(&dino[screen]);
		free(dino);
		dino = (dinostruct *) NULL;
	}
	FreeAllGL(mi);
}

ENTRYPOINT void
init_dino(ModeInfo * mi)
{
	dinostruct *dp;

	MI_INIT(mi, dino);
	dp = &dino[MI_SCREEN(mi)];

	dp->PX = ((float) LRAND() / (float) MAXRAND) * 2.0 - 1.0;
	dp->PY = ((float) LRAND() / (float) MAXRAND) * 2.0 - 1.0;

	if (MI_IS_FULLRANDOM(mi)) {
		dp->period2 = (Bool) (LRAND() & 1);
	} else {
		dp->period2 = period2;
	}
	if (dp->period2) {
		dp->degreeTurn = 180;
		dp->movement.style = EDGE;
	} else {
		dp->degreeTurn = 120;
		dp->movement.style = CORNER;
	}
	dp->step = NRAND(dp->degreeTurn);

#ifdef STANDALONE
	dp->trackball = gltrackball_init (True);
#endif

	if ((dp->glx_context = init_GL(mi)) != NULL) {
		reshape_dino(mi, MI_WIDTH(mi), MI_HEIGHT(mi));
		glDrawBuffer(GL_BACK);
		if (!pinit(mi)) {
			free_dino_screen(dp);
			if (MI_IS_VERBOSE(mi)) {
				 (void) fprintf(stderr,
					"Could not allocate memory for dino\n");
			}
		}
	} else {
		MI_CLEARWINDOW(mi);
	}
}

ENTRYPOINT void
draw_dino(ModeInfo * mi)
{
	Bool bounced = False;
	Display    *display = MI_DISPLAY(mi);
	Window      window = MI_WINDOW(mi);
	dinostruct *dp;

	if (dino == NULL)
		return;
	dp = &dino[MI_SCREEN(mi)];
	if ((dp->storedmoves) && (dp->moves == NULL))
		return;
	if (!dp->glx_context)
		return;
	MI_IS_DRAWN(mi) = True;
#ifdef WIN32
	wglMakeCurrent(hdc, dp->glx_context);
#else
	glXMakeCurrent(display, window, *(dp->glx_context));
#endif
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	glTranslatef(0.0, 0.0, -10.0);

	dp->PX += dp->VX;
	dp->PY += dp->VY;

	if (dp->PY < -1) {
		dp->PY += (-1) - (dp->PY);
		dp->VY = -dp->VY;
		bounced = True;
	}
	if (dp->PY > 1) {
		dp->PY -= (dp->PY) - 1;
		dp->VY = -dp->VY;
		bounced = True;
	}
	if (dp->PX < -1) {
		dp->PX += (-1) - (dp->PX);
		dp->VX = -dp->VX;
		bounced = True;
	}
	if (dp->PX > 1) {
		dp->PX -= (dp->PX) - 1;
		dp->VX = -dp->VX;
		bounced = True;
	}
	if (bounced) {
		dp->VX += ((float) LRAND() / (float) MAXRAND) * 0.02 - 0.01;
		dp->VX += ((float) LRAND() / (float) MAXRAND) * 0.02 - 0.01;
		if (dp->VX > 0.06)
			dp->VX = 0.06;
		if (dp->VY > 0.06)
			dp->VY = 0.06;
		if (dp->VX < -0.06)
			dp->VX = -0.06;
		if (dp->VY < -0.06)
			dp->VY = -0.06;
	}
	if (!MI_IS_ICONIC(mi)) {
		glTranslatef(dp->PX, dp->PY, 0);
		glScalef(Scale4Window * dp->WindH / dp->WindW,
			Scale4Window, Scale4Window);
	} else {
		glScalef(Scale4Iconic * dp->WindH / dp->WindW,
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
	gltrackball_rotate (dp->trackball);
#endif

	glRotatef(dp->step * 100, 1, 0, 0);
	glRotatef(dp->step * 95, 0, 1, 0);
	glRotatef(dp->step * 90, 0, 0, 1);
	drawCube(mi);
	if (MI_IS_FPS(mi)) {
		do_fps (mi);
	}
	glXSwapBuffers(display, window);
	if (dp->action == ACTION_SHUFFLE) {
		if (dp->done) {
			if (++dp->rotateStep > DELAY_AFTER_SHUFFLING) {
				dp->movement.face = NO_FACE;
				dp->rotateStep = 0;
				dp->action = ACTION_SOLVE;
				dp->done = 0;
			}
		} else {
			if (dp->movement.face == NO_FACE) {
				if (dp->shufflingmoves < dp->storedmoves) {
					dp->rotateStep = 0;
					dp->movement = dp->moves[dp->shufflingmoves];
				} else {
					dp->rotateStep = 0;
					dp->done = 1;
				}
			} else {
				if (dp->rotateStep == 0) {
					;
				}
				dp->rotateStep += dp->angleStep;
				if (dp->rotateStep > dp->degreeTurn) {
					evalmovement(mi, dp->movement);
					dp->shufflingmoves++;
					dp->movement.face = NO_FACE;
				}
			}
		}
	} else {
		if (dp->done) {
			if (++dp->rotateStep > DELAY_AFTER_SOLVING)
				if (!shuffle(mi)) {
					free_dino_screen(dp);
					if (MI_IS_VERBOSE(mi)) {
						 (void) fprintf(stderr,
							"Could not allocate memory for dino\n");
					}
				}
		} else {
			if (dp->movement.face == NO_FACE) {
				if (dp->storedmoves > 0) {
					dp->rotateStep = 0;
					dp->movement = dp->moves[dp->storedmoves - 1];
					dp->movement.direction = (dp->movement.direction +
						(MAX_ORIENT / 2)) % MAX_ORIENT;
				} else {
					dp->rotateStep = 0;
					dp->done = 1;
				}
			} else {
				if (dp->rotateStep == 0) {
					;
				}
				dp->rotateStep += dp->angleStep;
				if (dp->rotateStep > dp->degreeTurn) {
					evalmovement(mi, dp->movement);
					dp->storedmoves--;
					dp->movement.face = NO_FACE;
				}
			}
		}
	}

	glPopMatrix();

	glFlush();

	dp->step += 0.05;
}

#ifndef STANDALONE
ENTRYPOINT void
change_dino(ModeInfo * mi)
{
	dinostruct *dp;

	if (dino == NULL)
		return;
	dp = &dino[MI_SCREEN(mi)];

	if (!dp->glx_context)
		return;
	if (!pinit(mi)) {
		free_dino_screen(dp);
		if (MI_IS_VERBOSE(mi)) {
			 (void) fprintf(stderr,
				"Could not allocate memory for dino\n");
		}
	}
}
#endif

XSCREENSAVER_MODULE ("Dino", dino)

#endif /* MODE_dino */
