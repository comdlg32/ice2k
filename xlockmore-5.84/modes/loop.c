/* -*- Mode: C; tab-width: 4 -*- */
/* loop --- Chris Langton's self-producing loops */

#if 0
static const char sccsid[] = "@(#)loop.c	5.24 2007/01/18 xlockmore";

#endif

/*-
 * Copyright (c) 1996 - 2011 by David Bagley.
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
 * 02-Mar-2011: Byl and Chou, Reggia Loops
 * 18-Jan-2007: Added vertical option.
 * 10-Apr-2004: EvoLoopS - Evolving SDSR Loop Simulator idea stolen
 *              from Hiroki Sayama <sayama AT is.s.u-tokyo.ac.jp>.
 *              -evolve option
 * 09-Apr-2004: LoopS - Structurally Dissolvable Self-Reproducing Loop
 *              Simulator idea stolen from Hiroki Sayama
 *              <sayama AT is.s.u-tokyo.ac.jp>.
 *              -sheath and -dissolve options
 * 12-Mar-2004: Added wrap-
ping.
 * 15-Mar-2001: Added some flaws, random blue wall spots, to liven it up.
 *              This mod seems to expose a bug where hexagons are erased
 *              for no apparent reason.
 * 01-Nov-2000: Allocation checks
 * 16-Jun-2000: Fully coded the hexagonal rules.  (Rules that end up in
 *              state zero were not bothered with since a calloc was used
 *              to set non-explicit rules to zero.  This allows the
 *              compile-time option RAND_RULES to work here (at least to
 *              generation 540).)
 *              Also added compile-time option DELAYDEBUGLOOP for debugging
 *              life form.  This also turns off the random initial direction
 *              of the loop.  Set DELAYDEBUGLOOP to be 10 and use with
 *              something like this:
 *       xlock -mode loop -neighbors 6 -size 5 -delay 1 -count 540 -nolock
 * 18-Oct-1998: Started creating a hexagon version.
 *              It proved not that difficult because I used Langton's Loop
 *              as a guide, hexagons have more neighbors so there is more
 *              freedom to program, and the loop will has six sides to
 *              store its genes (data).
 *              (Soon after this a triangular version with neighbors = 6
 *              was attempted but was unsuccessful).
 * 10-May-1997: Compatible with xscreensaver
 * 15-Nov-1995: Coded from Chris Langton's Self-Reproduction in Cellular
 *              Automata Physica 10D 135-144 1984, also used wire.c as a
 *              guide.
 */

/*-
  Grid     Number of Neighbors
  ----     ------------------
  Square   4
  Hexagon  6
*/

/*-
 * From Steven Levy's Artificial Life
 * Chris Langton's cellular automata "loops" reproduce in the spirit of life.
 * Beginning from a single organism, the loops from a colony.  As the loops
 * on the outer fringes reproduce, the inner loops -- blocked by their
 * daughters -- can no longer produce offspring.  These dead progenitors
 * provide a base for future generations' expansion, much like the formation
 * of a coral reef.  This self-organizing behavior emerges spontaneously,
 * from the bottom up -- a key characteristic of artificial life.
 */

/*-
   Don't Panic  --  When the artificial life tries to leave its petri
   dish (ie. the screen) it will (usually) die...
   The loops are short of "real" life because a general purpose Turing
   machine is not contained in the loop.  This is a simplification of
   von Neumann and Codd's self-producing Turing machine.
   The data spinning around could be viewed as both its DNA and its internal
   clock.  The program can be initialized to have the loop spin both ways...
   but only one way around will work for a given rule.  An open question (at
   least to me): Is handedness a requirement for (artificial) life?  Here
   there is handedness at both the initial condition and the transition rule.
 */

#ifdef STANDALONE
#define MODE_loop
#define DEFAULTS "*delay: 100000 \n" \
	"*count: -5 \n" \
	"*cycles: 1600 \n" \
	"*size: -12 \n" \
	"*ncolors: 15 \n" \

# define free_loop 0
# define reshape_loop 0
# define loop_handle_event 0
#define UNIFORM_COLORS
#include "xlockmore.h"		/* in xscreensaver distribution */
#else /* STANDALONE */
#include "xlock.h"		/* in xlockmore distribution */
#define DO_STIPPLE
#endif /* STANDALONE */
#include "automata.h"

#ifdef MODE_loop

/*-
 * neighbors of 0 randomizes between 4 and 6.
 */
#define DEF_NEIGHBORS  "0"      /* choose random value */
#define DEF_LANGTON  "False"    /* neighbors 4 or 6 */
#define DEF_BYL  "False"        /* neighbors 4 only */
#define DEF_CHOU1  "False"      /* neighbors 4 only */
#define DEF_CHOU2  "False"      /* neighbors 4 only */
#define DEF_EVOLVE  "False"     /* neighbors 4 only */
#define DEF_DISSOLVE  "False"   /* neighbors 4 only and no evolve */
#define DEF_SHEATH  "False"     /* neighbors 4 only and no evolve */
#define DEF_WRAP  "True"
#define DEF_VERTICAL  "False"

static int  neighbors;
static Bool langton;
static Bool byl;
static Bool chou1;
static Bool chou2;
static Bool evolve;
static Bool dissolve;
static Bool sheath;
static Bool wrap;
static Bool vertical;

static XrmOptionDescRec opts[] =
{
	{(char *) "-neighbors", (char *) ".loop.neighbors", XrmoptionSepArg, (caddr_t) NULL},
	{(char *) "-langton", (char *) ".loop.langton", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+langton", (char *) ".loop.langton", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-byl", (char *) ".loop.byl", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+byl", (char *) ".loop.byl", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-chou1", (char *) ".loop.chou1", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+chou1", (char *) ".loop.chou1", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-chou2", (char *) ".loop.chou2", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+chou2", (char *) ".loop.chou2", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-evolve", (char *) ".loop.evolve", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+evolve", (char *) ".loop.evolve", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-dissolve", (char *) ".loop.dissolve", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+dissolve", (char *) ".loop.dissolve", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-sheath", (char *) ".loop.sheath", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+sheath", (char *) ".loop.sheath", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-wrap", (char *) ".loop.wrap", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+wrap", (char *) ".loop.wrap", XrmoptionNoArg, (caddr_t) "off"},
	{(char *) "-vertical", (char *) ".loop.vertical", XrmoptionNoArg, (caddr_t) "on"},
	{(char *) "+vertical", (char *) ".loop.vertical", XrmoptionNoArg, (caddr_t) "off"}
};

static argtype vars[] =
{
	{(void *) & neighbors, (char *) "neighbors", (char *) "Neighbors", (char *) DEF_NEIGHBORS, t_Int},
	{(void *) & langton, (char *) "langton", (char *) "Langton", (char *) DEF_LANGTON, t_Bool},
	{(void *) & byl, (char *) "byl", (char *) "Byl", (char *) DEF_BYL, t_Bool},
	{(void *) & chou1, (char *) "chou1", (char *) "Chou1", (char *) DEF_CHOU1, t_Bool},
	{(void *) & chou2, (char *) "chou2", (char *) "Chou2", (char *) DEF_CHOU2, t_Bool},
	{(void *) & evolve, (char *) "evolve", (char *) "Evolve", (char *) DEF_EVOLVE, t_Bool},
	{(void *) & dissolve, (char *) "dissolve", (char *) "Dissolve", (char *) DEF_DISSOLVE, t_Bool},
	{(void *) & sheath, (char *) "sheath", (char *) "Extension", (char *) DEF_SHEATH, t_Bool},
	{(void *) & wrap, (char *) "wrap", (char *) "Wrap", (char *) DEF_WRAP, t_Bool},
	{(void *) & vertical, (char *) "vertical", (char *) "Vertical", (char *) DEF_VERTICAL, t_Bool}
};

static OptionStruct desc[] =
{
	{(char *) "-neighbors num", (char *) "squares 4 or hexagons 6"},
	{(char *) "-/+langton", (char *) "turn on/off Langton's Loops"},
	{(char *) "-/+byl", (char *) "turn on/off Byl's Loops"},
	{(char *) "-/+chou1", (char *) "turn on/off Chou-Reggia 1 Loops"},
	{(char *) "-/+chou2", (char *) "turn on/off Chou-Reggia 2 Loops"},
	{(char *) "-/+evolve", (char *) "turn on/off Evolving Loops"},
	{(char *) "-/+dissolve", (char *) "turn on/off structurally dissolvable loops"},
	{(char *) "-/+sheath", (char *) "turn on/off sheath extension"},
	{(char *) "-/+wrap", (char *) "turn on/off wrapping"},
	{(char *) "-/+vertical", (char *) "change orientation for hexagons"}
};

ENTRYPOINT ModeSpecOpt loop_opts =
{sizeof opts / sizeof opts[0], opts, sizeof vars / sizeof vars[0], vars, desc};


#ifdef USE_MODULES
ModStruct   loop_description =
{"loop", "init_loop", "draw_loop", "release_loop",
 "refresh_loop", "init_loop", (char *) NULL, &loop_opts,
 100000, 5, 1600, -12, 64, 1.0, "",
 "Shows Langton's self-producing loops", 0, NULL};

#endif

#define LOOPBITS(n,w,h)\
  if ((lp->pixmaps[lp->init_bits]=\
  XCreatePixmapFromBitmapData(display,window,(char *)n,w,h,1,0,1))==None){\
  free_loop_screen(display,lp); return;} else {lp->init_bits++;}

static int  local_neighbors = 0;

#if 0
/* Used to fast forward to troubled generations, mainly used for debugging.
   -delay 1 -count 170 -neighbors 6  # divisions starts
   540 first cell collision
   1111 mutant being born from 2 parents
   1156 mutant born
 */
#define DELAYDEBUGLOOP 10
#endif

#define COLORS 8
#define REALCOLORS (COLORS-2)
#define STATES COLORS
#define EXT_STATES (COLORS+1)
#define MINLOOPS 1
#define REDRAWSTEP 2000		/* How many cells to draw per cycle */
#define ADAM_SIZE 8 /* MIN 5 */
#define BYL_ADAM_SIZE 4
#define CHOU1_ADAM_SIZE 4
#define CHOU2_ADAM_SIZE 3
#define EVO_ADAM_SIZE 15
#if 1
#define ADAM_LOOPX  (ADAM_SIZE+2)
#define ADAM_LOOPY  (ADAM_SIZE+2)
#define BYL_ADAM_LOOPX  (BYL_ADAM_SIZE+2)
#define BYL_ADAM_LOOPY  (BYL_ADAM_SIZE+2)
#define CHOU1_ADAM_LOOPX  (CHOU1_ADAM_SIZE+2)
#define CHOU1_ADAM_LOOPY  (CHOU1_ADAM_SIZE+2)
#define CHOU2_ADAM_LOOPX  (CHOU2_ADAM_SIZE+2)
#define CHOU2_ADAM_LOOPY  (CHOU2_ADAM_SIZE+2)
#define EVO_ADAM_LOOPX  (EVO_ADAM_SIZE+2)
#define EVO_ADAM_LOOPY  (EVO_ADAM_SIZE+2)
#define SQ_ADAM_LOOPX(evolve) ((evolve)?EVO_ADAM_LOOPX:ADAM_LOOPX)
#define SQ_ADAM_LOOPY(evolve) ((evolve)?EVO_ADAM_LOOPY:ADAM_LOOPY)
#else
#define ADAM_LOOPX 16
#define ADAM_LOOPY 10
#endif
#define MINGRIDSIZE (4*ADAM_LOOPX)
#if 0
/* TRIA stuff was an attempt to make a triangular lifeform on a
   hexagonal grid but I got bored.  You may need an additional 7th
   state for a coherent step by step process of cell separation and
   initial stem development.
 */
#define TRIA 1
#endif
#ifdef TRIA
#define HEX_ADAM_SIZE 3 /* MIN 3 */
#else
#define HEX_ADAM_SIZE 5 /* MIN 3 */
#endif
#if 1
#define HEX_ADAM_LOOPX (2*HEX_ADAM_SIZE+1)
#define HEX_ADAM_LOOPY (2*HEX_ADAM_SIZE+1)
#else
#define HEX_ADAM_LOOPX 3
#define HEX_ADAM_LOOPY 7
#endif
#define HEX_MINGRIDSIZE (6*HEX_ADAM_LOOPX)
#define MINSIZE ((MI_NPIXELS(mi)>=COLORS)?1:(2+(local_neighbors==6)))
#define NEIGHBORKINDS 2
#define ANGLES 360
#define MAXNEIGHBORS 6

/* Singly linked list */
typedef struct _CellList {
	XPoint      pt;
	struct _CellList *next;
} CellList;

typedef struct {
	int         init_bits;
	int         generation;
	int         xs, ys;
	int         xb, yb;
	int         nrows, ncols;
	int         bx, by, bnrows, bncols;
	int         mincol, minrow, maxcol, maxrow;
	int         width, height;
	int         redrawing, redrawpos;
	Bool        dead, clockwise, wrap, vertical;
	unsigned char *newcells, *oldcells;
	int         ncells[EXT_STATES];
	CellList   *cellList[EXT_STATES];
	unsigned long colors[COLORS];
	GC          stippledGC;
	Pixmap      pixmaps[EXT_STATES];
	union {
		XPoint      hexagon[6];
	} shape;
} loopstruct;

static loopstruct *loops = (loopstruct *) NULL;

/*#define INEF2 1*/
#ifndef INEF2
#define TRANSITION(TT,V) V=TT&7;TT>>=3
#define FINALTRANSITION(TT,V) V=TT&7
#define TABLE(R,T,L,B) (table[((B)<<9)|((L)<<6)|((T)<<3)|(R)])
#define TABLE_OUT(C,R,T,L,B) ((TABLE(R,T,L,B)>>((C)*3))&7)
#else
static const int M8[]={1, 8, 64, 512, 4096, 32768, 262144, 2097152, 16777216};
#define TRANSITION(TT,V) V=TT%8;TT/=8
#define FINALTRANSITION(TT,V) V=TT%8
#define TABLE(R,T,L,B) (table[((B)*512)+((L)*64)+((T)*8)+(R)])
#define TABLE_OUT(C,R,T,L,B) ((TABLE(R,T,L,B)/(M8[C]))%8)
#endif
#if 0
static const int M9[]={1, 9, 81, 729, 6561, 59049, 531441, 4782969, 43046721,
	387420489};
#define EXT_TRANSITION(TT,V) V=TT%9;TT/=9
#define EXT_FINALTRANSITION(TT,V) V=TT%9
#define EXT_TABLE(R,T,L,B) (table[((B)*729)+((L)*81)+((T)*9)+(R)])
#define EXT_TABLE_OUT(C,R,T,L,B) ((EXT_TABLE(R,T,L,B)/(M9[C]))%9)
#endif
#define INEF_TABLE_OUT(C,R,T,L,B) table[(B)*6561+(L)*729+(T)*81+(R)*9+(C)]
#define HEX_TABLE(R,T,t,l,b,B) (table[((B)<<15)|((b)<<12)|((l)<<9)|((t)<<6)|((T)<<3)|(R)])
#define HEX_TABLE_OUT(C,R,T,t,l,b,B) ((HEX_TABLE(R,T,t,l,b,B)>>((C)*3))&7)

#if 0
/* Instead of setting "unused" state rules to zero it randomizes them.
   These rules take over when something unexpected happens... like when a
   cell hits a wall (the end of the screen).
 */
#define RAND_RULES
#endif

#ifdef RAND_RULES
#define TABLE_IN(C,R,T,L,B,I) (TABLE(R,T,L,B)&=~(7<<((C)*3)));\
(TABLE(R,T,L,B)|=((I)<<((C)*3)))
#if 0
#define EXT_TABLE_IN(C,R,T,L,B,I) (EXT_TABLE(R,T,L,B)-=(9*M9[C]-1));\
(EXT_TABLE(R,T,L,B)+=((I)*(M9[C])))
#endif
#define HEX_TABLE_IN(C,R,T,t,l,b,B,I) (HEX_TABLE(R,T,t,l,b,B)&=~(7<<((C)*3)));\
(HEX_TABLE(R,T,t,l,b,B)|=((I)<<((C)*3)))
#else
#ifndef INEF2
#define TABLE_IN(C,R,T,L,B,I) (TABLE(R,T,L,B)|=((I)<<((C)*3)))
#else
#define TABLE_IN(C,R,T,L,B,I) if(TABLE_OUT(C,R,T,L,B)==0) (TABLE(R,T,L,B)+=((I)*(M8[C])))
#endif
#if 0
#define EXT_TABLE_IN(C,R,T,L,B,I) if (EXT_TABLE_OUT(C,R,T,L,B)==0) (EXT_TABLE(R,T,L,B)+=((I)*(M9[C])))
#endif
#define HEX_TABLE_IN(C,R,T,t,l,b,B,I) (HEX_TABLE(R,T,t,l,b,B)|=((I)<<((C)*3)))
#endif
#define INEF_TABLE_IN(C,R,T,L,B,I) INEF_TABLE_OUT(C,R,T,L,B)=(I)

static unsigned int *table = (unsigned int *) NULL;
  /* square: 8*8*8*8      = 2^12 = 2^3^4 = 4096 */
  /* extsquare: 9*9*9*9   = 3^8 = 3^2^4 = 6561 */
  /* inefsquare: 9*9*9*9*9   = 3^10 = 3^2^5 = 59049 */
  /* hexagon: 8*8*8*8*8*8 = 2^18 = 2^3^6 = 262144 = too big? */

static char plots[NEIGHBORKINDS] =
{
  4, 6 /* Neighborhoods */
};

/* C.G.Langton. "Self-reproduction in cellular automata." */
/* Physica D, Vol. 10, pages 135-144, 1984. */
/* transition rules from: http://necsi.org/postdocs/sayama/sdsr/java/loops.java */
/* credits: "Self-Replicating Loops & Ant, Programmed by Eli Bachmutsky, Copyleft Feb.1999" */
/* states: 8, rules: 219, format: CNESWC' */
static unsigned int transitionTable[] =
{	/* Octal  CNESW->I */
      /* CNESWI   CNESWI   CNESWI   CNESWI   CNESWI */
	0000000, 0000012, 0000020, 0000030, 0000050,
	0000063, 0000071, 0000112, 0000122, 0000132,
	0000212, 0000220, 0000230, 0000262, 0000272,
	0000320, 0000525, 0000622, 0000722, 0001022,
	0001120, 0002020, 0002030, 0002050, 0002125,
	0002220, 0002322, 0005222, 0012321, 0012421,
	0012525, 0012621, 0012721, 0012751, 0014221,
	0014321, 0014421, 0014721, 0016251, 0017221,
	0017255, 0017521, 0017621, 0017721, 0025271,
	0100011, 0100061, 0100077, 0100111, 0100121,
	0100211, 0100244, 0100277, 0100511, 0101011,
	0101111, 0101244, 0101277, 0102026, 0102121,
	0102211, 0102244, 0102263, 0102277, 0102327,
	0102424, 0102626, 0102644, 0102677, 0102710,
	0102727, 0105427, 0111121, 0111221, 0111244,
	0111251, 0111261, 0111277, 0111522, 0112121,
	0112221, 0112244, 0112251, 0112277, 0112321,
	0112424, 0112621, 0112727, 0113221, 0122244,
	0122277, 0122434, 0122547, 0123244, 0123277,
	0124255, 0124267, 0125275, 0200012, 0200022,
	0200042, 0200071, 0200122, 0200152, 0200212,
	0200222, 0200232, 0200242, 0200250, 0200262,
	0200272, 0200326, 0200423, 0200517, 0200522,
	0200575, 0200722, 0201022, 0201122, 0201222,
	0201422, 0201722, 0202022, 0202032, 0202052,
	0202073, 0202122, 0202152, 0202212, 0202222,
	0202272, 0202321, 0202422, 0202452, 0202520,
	0202552, 0202622, 0202722, 0203122, 0203216,
	0203226, 0203422, 0204222, 0205122, 0205212,
	0205222, 0205521, 0205725, 0206222, 0206722,
	0207122, 0207222, 0207422, 0207722, 0211222,
	0211261, 0212222, 0212242, 0212262, 0212272,
	0214222, 0215222, 0216222, 0217222, 0222272,
	0222442, 0222462, 0222762, 0222772, 0300013,
	0300022, 0300041, 0300076, 0300123, 0300421,
	0300622, 0301021, 0301220, 0302511, 0401120,
	0401220, 0401250, 0402120, 0402221, 0402326,
	0402520, 0403221, 0500022, 0500215, 0500225,
	0500232, 0500272, 0500520, 0502022, 0502122,
	0502152, 0502220, 0502244, 0502722, 0512122,
	0512220, 0512422, 0512722, 0600011, 0600021,
	0602120, 0612125, 0612131, 0612225, 0700077,
	0701120, 0701220, 0701250, 0702120, 0702221,
	0702251, 0702321, 0702525, 0702720
};

/* J. Byl. "Self-Reproduction in small cellular automata." */
/* Physica D, Vol. 34, pages 295-299, 1989. */
/* transition rules from: http://necsi.org/postdocs/sayama/sdsr/java/loops.java */
/* credits: "Self-Replicating Loops & Ant, Programmed by Eli Bachmutsky, Copyleft Feb.1999" */
/* states: 6, rules: 146, format: CNESWC' */
static unsigned int bylTransitionTable[] =
{
	0000000, 0000010, 0000031, 0000420, 0000110,
	0000120, 0000311, 0000330, 0000320, 0000200,
	0000242, 0000233, 0004330, 0005120, 0001400,
	0001120, 0001305, 0001310, 0001320, 0003020,
	0003400, 0003420, 0003120, 0002040, 0002050,
	0002010, 0002020, 0002420, 0002500, 0002520,
	0002200, 0002250, 0002220, 0040000, 0050000,
	0050500, 0010052, 0010022, 0010400, 0010100,
	0020055, 0400233, 0405235, 0401233, 0442024,
	0452020, 0415233, 0411233, 0412024, 0412533,
	0432024, 0421433, 0422313, 0501302, 0502230,
	0540022, 0542002, 0512024, 0530025, 0520025,
	0520442, 0523242, 0522020, 0100000, 0100010,
	0100033, 0100330, 0101233, 0103401, 0103244,
	0111244, 0113244, 0112404, 0133244, 0121351,
	0123444, 0123543, 0122414, 0122434, 0300100,
	0300300, 0300211, 0300233, 0304233, 0301100,
	0301211, 0303211, 0303233, 0302233, 0344233,
	0343233, 0351202, 0353215, 0314233, 0311233,
	0313251, 0313211, 0312211, 0335223, 0333211,
	0320533, 0324433, 0325415, 0321433, 0321511,
	0321321, 0323411, 0200000, 0200042, 0200032,
	0200022, 0200442, 0200515, 0200112, 0200122,
	0200342, 0200332, 0200242, 0200212, 0201502,
	0203202, 0202302, 0244022, 0245022, 0242042,
	0242022, 0254202, 0255042, 0252025, 0210042,
	0214022, 0215022, /*0212052,*/ 0212055, 0212022,
	0230052, 0234022, 0235002, 0235022, 0232042,
	0232022, 0220042, 0220020, 0220533, 0221552,
	0322343, 0204122,
 /* last 2 are not listed but implied by initializing rules to be stable */
};

/* J. A. Reggia, S.L.Armentrout, H.-H. Chou, and Y. Peng. */
/* Simple systems that exhibit self-directed replication. */
/* Science, Vol. 259, pages 1282-1287, February 1993. */
/* transition rules from: http://necsi.org/postdocs/sayama/sdsr/java/loops.java */
/* credits: "Self-Replicating Loops & Ant, Programmed by Eli Bachmutsky, Copyleft Feb.1999" */
/* rules: 174, states: 8, format: CNESWC' */
static unsigned int chou1TransitionTable[] =
{
	0000000, 0000200, 0000110, 0004420, 0006301,
	0001447, 0003030, 0046000, 0060400, 0062102,
	0051060, 0010420, 0010360, 0012152, 0032000,
	0401033, 0410011, 0431206, 0640406, 0660630,
	0616600, 0636000, 0000440, 0000260, 0000330,
	0004512, 0005103, 0001540, 0003100, 0045002,
	0060600, 0061110, 0051710, 0010506, 0010320,
	0030400, 0076010, 0460313, 0410633, 0600000,
	0640106, 0610006, 0616100, 0636100, 0000420,
	0000231, 0000705, 0006030, 0002020, 0001210,
	0040000, 0045600, 0060100, 0050006, 0020320,
	0010140, 0016000, 0036000, 0400035, 0466633,
	0410100, 0606100, 0660060, 0610106, 0613001,
	0676600, 0000600, 0000100, 0004040, 0006440,
	0001010, 0001110, 0040160, 0041040, 0060166,
	0050306, 0024200, 0010154, 0016100, 0036606,
	0400313, 0423106, 0411033, 0606313, 0660460,
	0614004, 0630160, 0500000, 0000540, 0000152,
	0004010, 0006110, 0001030, 0003000, 0044400,
	0060066, 0062030, 0054040, 0023010, 0010340,
	0012000, 0036300, 0405033, 0410061, 0431033,
	0640006, 0660615, 0616000, 0630360, 0500044,
	0500011, 0230000, 0100111, 0101044, 0146004,
	0143321, 0166611, 0121121, 0117044, 0300111,
	0340066, 0366611, 0374017, 0200100, 0100044,
	0104414, 0101601, 0141403, 0160414, 0161101,
	0112244, 0130414, 0304011, 0340611, 0314001,
	0703010, 0203010, 0100011, 0104674, 0101301,
	0141424, 0160111, 0161301, 0111044, 0133001,
	0301471, 0340261, 0314676, 0703330, 0240400,
	0100441, 0106101, 0103011, 0141141, 0164104,
	0120414, 0113011, 0133011, 0301303, 0340211,
	0313023, 0733630, 0210102, 0100414, 0105011,
	0103103, 0141121, 0166644, 0124104, 0113601,
	0300101, 0307141, 0344011, 0331001,
};

/* J. A. Reggia, S.L.Armentrout, H.-H. Chou, and Y. Peng. */
/* Simple systems that exhibit self-directed replication. */
/* Science, Vol. 259, pages 1282-1287, February 1993. */
/* transition rules from: http://necsi.org/postdocs/sayama/sdsr/java/loops.java */
/* credits: "Self-Replicating Loops & Ant, Programmed by Eli Bachmutsky, Copyleft Feb.1999" */
/* rules: 65, states: 8, format: CNESWC' */
static unsigned int chou2TransitionTable[] =
{
	0000000, 0000440, 0000547, 0000100, 0000110,
	0000330, 0004040, 0004445, 0004100, 0001040,
	0001010, 0001740, 0003000, 0003010, 0003030,
	0007040, 0007030, 0007104, 0007110, 0040000,
	0040070, 0047100, 0050007, 0017000, 0070000,
	0070077, 0071010, 0400101, 0400313, 0401033,
	0407103, 0411033, 0431033, 0500033, 0503330,
	0100045, 0100011, 0100414, 0101044, 0101301,
	0103011, 0107131, 0140414, 0141044, 0111044,
	0133011, 0177711, 0304011, 0305011, 0305141,
	0307141, 0344011, 0345011, 0354010, 0314001,
	0377711, 0700000, 0700337, 0707100, 0707141,
	0707110, 0717000, 0730007, 0770070, 0770711,
};

/* From Hiroki Sayama */
static unsigned int evoTransitionTable[] =
{
	0000012, 0000043, 0000122, 0000152, 0000212,
	0000242, 0000422, 0000452, 0000752, 0001022,
	0002141, 0002171, 0002322, 0011221, 0012121,
	0012321, 0012421, 0012451, 0012526, 0012626,
	0012721, 0012751, 0013421, 0013721, 0014221,
	0014251, 0014321, 0014351, 0014421, 0014621,
	0017221, 0017251, 0017561, 0017621, 0017721,
	0100011, 0100121, 0100211, 0100244, 0100277,
	0101211, 0101244, 0101277, 0102021, 0102111,
	0102121, 0102131, 0102211, 0102244, 0102277,
	0102324, 0102414, 0102424, 0102434, 0102511,
	0102527, 0102543, 0102577, 0102717, 0102727,
	0102735, 0105121, 0105424, 0105727, 0106211,
	0106244, 0106277, 0111121, 0111221, 0111244,
	0111251, 0111277, 0111621, 0112121, 0112131,
	0112151, 0112221, 0112244, 0112277, 0112321,
	0112424, 0112434, 0112527, 0112543, 0112577,
	0112626, 0112727, 0112735, 0113221, 0113321,
	0115424, 0115727, 0116244, 0116277, 0122244,
	0122277, 0122434, 0122737, 0123244, 0123277,
	0124266, 0124333, 0126276, 0200012, 0200022,
	0200042, 0200052, 0200060, 0200071, 0200122,
	0200152, 0200212, 0200222, 0200232, 0200242,
	0200260, 0200272, 0200324, 0200423, 0200452,
	0200545, 0200575, 0200620, 0200722, 0200752,
	0201022, 0201122, 0201222, 0201422, 0201722,
	0202022, 0202032, 0202052, 0202065, 0202073,
	0202122, 0202152, 0202212, 0202222, 0202232,
	0202323, 0202422, 0202452, 0202525, 0202620,
	0202650, 0202722, 0202752, 0203122, 0203222,
	0203422, 0203452, 0203722, 0204122, 0204222,
	0204422, 0205122, 0205425, 0205725, 0206125,
	0206212, 0206425, 0206725, 0207122, 0207222,
	0207722, 0211222, 0212222, 0212232, 0212242,
	0212272, 0212323, 0213222, 0214222, 0216222,
	0217222, 0222242, 0222272, 0222342, 0222372,
	0222432, 0222442, 0222732, 0222772, 0223243,
	0223273, 0300013, 0300022, 0300032, 0300043,
	0300074, 0300123, 0300322, 0300421, 0301021,
	0301250, 0302123, 0302423, 0302521, 0302723,
	0303321, 0312123, 0312423, 0312521, 0312723,
	0324243, 0324251, 0324273, 0325271, 0327273,
	0400001, 0400021, 0401020, 0401120, 0401220,
	0401250, 0401620, 0402120, 0402150, 0402221,
	0402321, 0402626, 0403120, 0403221, 0500025,
	0500125, 0500215, 0500232, 0500245, 0500275,
	0500425, 0500725, 0502022, 0502052, 0502125,
	0502152, 0502425, 0502725, 0503120, 0602022,
	0602122, 0602220, 0602422, 0602722, 0612220,
	0622240, 0622270, 0701020, 0701120, 0701220,
	0701250, 0701620, 0702120, 0702150, 0702221,
	0702320, 0702626, 0703120
};

/* From David Bagley */
static unsigned int hexTransitionTable[] =
{				/* Octal CNESswn->I */
  /* CNESswnI   CNESswnI   CNESswnI   CNESswnI   CNESswnI */

#ifdef TRIA
    000000000, 000000020, 000000220, 000002220, 000022220,
    011122121, 011121221, 011122221, 011221221,
    011222221, 011112121, 011112221,
    020021122, 020002122, 020211222, 021111222,
    020221122, 020027122, 020020722, 020021022,
    001127221,
    011122727, 011227227, 010122121, 010222211,
    021117222, 020112272,
    070221220,
    001227221,
    010221121, 011721221, 011222277,
    020111222, 020221172,
    070211220,
    001217221,
    010212277, 010221221,
    020122112,
    070122220,
    001722221,
    010221271,
    020002022, 021122172,
    070121220,
    011122277, 011172121,
    010212177, 011212277,
    070112220,
    001772221,
    021221772,
    070121270, 070721220,
    000112721, 000272211,
    010022211, 012222277,
    020072272, 020227122, 020217222,
    010211121,
    020002727,
    070222220,
    001727721,
    020021072, 020070722,
    070002072, 070007022,
    001772721,
    070002022,
    000000070, 000000770, 000072220, 000000270,
    020110222, 020220272, 020220722,
    070007071, 070002072, 070007022,
    000000012, 000000122, 000000212, 001277721,
    020122072, 020202212,
    010002121,
    020001122, 020002112,
    020021722,
    020122022, 020027022, 020070122, 020020122,
    010227027,
    020101222,
    010227227, 010227277,
    021722172,
    001727221,
    010222277,
    020702272,
    070122020,
    000172721,
    010022277, 010202177, 010227127,

    001214221,
    010202244,
    020024122, 020020422,
    040122220,
    001422221,
    010221241, 010224224,
    021122142,
    040121220,
    001124221,
    010224274,
    020112242, 021422172,
    040221220,
    001224221, 001427221,
    010222244,
    020227042,
    040122020,
    000142721,
    010022244, 010202144, 010224124,
    040112220,
    001442221,
    021221442,
    040121240, 040421220,
    000242211, 000112421,
    020042242, 020214222, 020021422, 020220242, 020024022,
    011224224,
    020224122,
    020220422,
    012222244,
    020002424,
    040222220,
    001244421, 000000420, 000000440, 000000240, 000000040,
    020040121, 020021042,
    040004022, 040004042, 040002042,
    010021121,
    020011122, 020002112,
    001424421,
    020040422,
    001442421,
    040002022,
    001724221,
    010227247,
    020224072, 021417222,
    000172421,
    010021721,
    020017022,
    020120212,
    020271727,
    070207072, 070701220,
    000001222,
    020110122,
    001277221,
    001777721,
    020021222, 020202272, 020120222, 020221722,
    020027227,
    070070222,
    000007220,
    020101272, 020272172, 020721422, 020721722,
    020011222, 020202242,
#if 0
              {2,2,0,0,2,7,0},
             {2,0,2,0,2,0,2},
            {2,4,1,2,2,1,2},
           {2,1,2,1,2,1,2},
          {2,0,2,2,1,1,2},
         {2,7,1,1,1,1,2},
        {0,2,2,2,2,2,2},
              {2,2,0,0,7,7,0},
             {2,1,2,0,2,0,7},
            {2,0,1,2,2,1,2},
           {2,4,2,1,2,1,2},
          {2,1,2,2,1,1,2},
         {2,0,7,1,1,1,2},
        {0,2,2,2,2,2,2},
#endif
#else
    000000000, 000000020, 000000220, 000002220,
    011212121, 011212221, 011221221, 011222221,
    020002122, 020021122, 020211122,

    010221221, 010222121,
    020002022, 020021022, 020020122, 020112022,

    010202121,
    020102022, 020202112,

    000000012, 000000122, 000000212,
    010002121,
    020001122, 020002112, 020011122,


    001227221, 001272221, 001272721,
    012212277, 011222727, 011212727,
    020021722, 020027122, 020020722, 020027022,
    020211722, 020202172, 020120272,
    020271122, 020202172, 020207122, 020217122,
    020120272, 020210722, 020270722,
    070212220, 070221220, 070212120,


    012222277,
    020002727,
    070222220,

    001277721, 000000070, 000000270, 000000720, 000000770,
    020070122, 020021072,
    070002072, 070007022, 070007071,

    020070722,
    070002022,

    010227227, 010222727, 010202727,
    020172022, 020202712,

    001224221, 001242221, 001242421,
    012212244, 011222424, 011212424,
    020021422, 020024122, 020020422, 020024022,
    020211422, 020202142, 020120242,
    020241122, 020202142, 020204122, 020214122,
    020120242, 020210422, 020240422,
    040212220, 040221220, 040212120,


    012222244,
    020002424,
    040222220,

    001244421, 000000040, 000000240, 000000420, 000000440,
    020040122, 020021042,
    040002042,
    040004021, 040004042,

    020040422,
    040002022,

    010224224, 010222424, 010202424,
    020142022, 020202412,
    020011722, 020112072, 020172072, 020142072,



    000210225, 000022015, 000022522,
    011225521,
    020120525, 020020152, 020005122, 020214255, 020021152,
    020255242,
    050215222, 050225121,

    000225220, 001254222,
    010221250, 011221251, 011225221,
    020025122, 020152152, 020211252, 020214522, 020511125,
    050212241, 05221120,
    040521225,

    000000250, 000000520, 000150220, 000220520, 000222210,
    001224251,
    010022152, 010251221, 010522121, 011212151, 011221251,
    011215221,
    020000220, 020002152, 020020220, 020022152,
    020021422, 020022152, 020022522, 020025425, 020050422,
    020051022, 020051122, 020211122, 020211222, 020215222,
    020245122,
    050021125, 050021025, 050011125, 051242221,
    041225220,

    000220250, 000220520, 001227521, 001275221,
    011257227, 011522727,
    020002052, 020002752, 020021052, 020057125,
    050020722, 050027125,
    070215220,

    070212255,
    071225220,
    020275122,
    051272521,
    020055725,
    020021552,
    012252277,
    050002521,
    020005725,

    050011022,
    000000155,
    020050722,
    001227250,
    010512727,
    010002151,
    020027112,
    001227251,
    012227257,
    050002125,
    020517122,
    050002025,
    020050102,
    050002725,
    020570722,
    001252721,
    020007051,
    020102052,
    020271072,
    050001122,
    010002151,
    011227257,
    020051722,
    020057022,
    020050122,


    020051422,
    011224254,
    012224254,

    020054022,
    050002425,
    040252220,
    020002454,


    000000540,
    001254425,
    050004024,
    040004051,

    000000142,
    040001522,
    010002547,
    020045122,
    051221240,
    020002512,
    020021522,


    020020022,
    021125522,
    020521122,
    020025022,
    020025522,
    020020522,

    020202222,
    020212222,
    021212222,
    021222722,
    021222422,
    020002222,
    020021222,
    020022122,
    020212122,
    020027222,
    020024222,
    020212722,
    020212422,
    020202122,
    001222221,
    020002522,

    020017125,
    010022722,
    020212052,

    020205052,
    070221250,

    000000050, 000005220, 000002270, 070252220,
    000000450, 000007220,
    000220220, 000202220, 000022020, 000020220,

    000222040,
    000220440,
    000022040,
    000040220,

    000252220,
    050221120, 010221520,
    002222220,

    000070220, 000220720,
    000020520, 000070250, 000222070, 000027020,
    000022070, 000202270, 000024020, 000220420,
    000220270, 000220240, 000072020, 000042020,
    000002020, 000002070, 000020270, 000020250,
    000270270, 000007020, 000040270,

    /* Collision starts (gen 540), not sure to have rules to save it
       or depend on calloc to initialize remaining rules to 0 so that
       the mutant will be born
     */
    000050220,
#endif
};


/*-
Neighborhoods are read as follows (rotations are not listed):
    N
  W C E  ==>  I
    S

   n N
  w C E  ==>  I
   s S
 */

static unsigned char selfReproducingLoop[ADAM_LOOPY][ADAM_LOOPX] =
{
/* 10x10 */
	{0, 2, 2, 2, 2, 2, 2, 2, 2, 0},
	{2, 4, 0, 1, 4, 0, 1, 1, 1, 2},
	{2, 1, 2, 2, 2, 2, 2, 2, 1, 2},
	{2, 0, 2, 0, 0, 0, 0, 2, 1, 2},
	{2, 7, 2, 0, 0, 0, 0, 2, 7, 2},
	{2, 1, 2, 0, 0, 0, 0, 2, 0, 2},
	{2, 0, 2, 0, 0, 0, 0, 2, 1, 2},
	{2, 7, 2, 2, 2, 2, 2, 2, 7, 2},
	{2, 1, 0, 6, 1, 0, 7, 1, 0, 2},
	{0, 2, 2, 2, 2, 2, 2, 2, 2, 0}
};

static unsigned char bylReproducingLoop[BYL_ADAM_LOOPY][BYL_ADAM_LOOPX] =
{
	{0, 2, 2, 0},
	{2, 3, 1, 2},
	{2, 3, 4, 2},
	{0, 2, 5, 0}
};

static unsigned char chou1ReproducingLoop[CHOU1_ADAM_LOOPY][CHOU1_ADAM_LOOPX] =
{
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{1, 1, 0, 0},
	{3, 4, 1, 1}
};

static unsigned char chou2ReproducingLoop[CHOU2_ADAM_LOOPY][CHOU2_ADAM_LOOPX] =
{
	{0, 0, 0},
	{1, 1, 0},
	{3, 4, 1}
};

static unsigned char evoReproducingLoop[EVO_ADAM_LOOPY][EVO_ADAM_LOOPX] =
{
/* 17x17 */
	{0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0},
	{2, 7, 0, 1, 7, 0, 1, 7, 0, 1, 7, 0, 1, 1, 1, 1, 2},
	{2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2},
	{2, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2},
	{2, 7, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2},
	{2, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2},
	{2, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2},
	{2, 7, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2},
	{2, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2},
	{2, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2},
	{2, 7, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 7, 2},
	{2, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 2},
	{2, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2},
	{2, 7, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 7, 2},
	{2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 2},
	{2, 0, 7, 1, 0, 7, 1, 0, 7, 1, 0, 4, 1, 0, 4, 1, 2},
	{0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 5, 0}
};
static unsigned char hexReproducingLoop[HEX_ADAM_LOOPY][HEX_ADAM_LOOPX] =
{
#if 0
/* Experimental TRIA5:7x7 */
	      {2,2,0,0,0,0,0},
	     {2,1,2,0,2,2,0},
	    {2,0,4,2,2,0,2},
	   {2,7,2,0,2,0,2},
	  {2,1,2,2,1,1,2},
	 {2,0,7,1,0,7,2},
	{0,2,2,2,2,2,2},
  /* Stem cells, only "5" will fully reproduce itself */
/* 3:12x7 */
	      {2,2,2,2,0,0,0,0,0,0,0,0},
	     {2,1,1,1,2,0,0,0,0,0,0,0},
	    {2,1,2,2,1,2,2,2,2,2,2,0},
	   {2,1,2,0,2,7,1,1,1,1,1,2},
	  {0,2,1,2,2,0,2,2,2,2,2,2},
	 {0,0,2,0,4,1,2,0,0,0,0,0},
	{0,0,0,2,2,2,2,0,0,0,0,0}
/* 4:14x9 */
	        {2,2,2,2,2,0,0,0,0,0,0,0,0,0},
	       {2,1,1,1,1,2,0,0,0,0,0,0,0,0},
	      {2,1,2,2,2,1,2,0,0,0,0,0,0,0},
	     {2,1,2,0,0,2,1,2,2,2,2,2,2,0},
	    {2,1,2,0,0,0,2,7,1,1,1,1,1,2},
	   {0,2,1,2,0,0,2,0,2,2,2,2,2,2},
	  {0,0,2,0,2,2,2,1,2,0,0,0,0,0},
	 {0,0,0,2,4,1,0,7,2,0,0,0,0,0},
	{0,0,0,0,2,2,2,2,2,0,0,0,0,0}
/* 5:16x11 */
	          {2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0},
	         {2,1,1,1,1,1,2,0,0,0,0,0,0,0,0,0},
	        {2,1,2,2,2,2,1,2,0,0,0,0,0,0,0,0},
	       {2,1,2,0,0,0,2,1,2,0,0,0,0,0,0,0},
	      {2,1,2,0,0,0,0,2,1,2,2,2,2,2,2,0},
	     {2,1,2,0,0,0,0,0,2,7,1,1,1,1,1,2},
	    {0,2,1,2,0,0,0,0,2,0,2,2,2,2,2,2},
	   {0,0,2,0,2,0,0,0,2,1,2,0,0,0,0,0},
	  {0,0,0,2,4,2,2,2,2,7,2,0,0,0,0,0},
	 {0,0,0,0,2,1,0,7,1,0,2,0,0,0,0,0},
	{0,0,0,0,0,2,2,2,2,2,2,0,0,0,0,0}
/* test:3x7  (0,4) is blank  ... very strange.
          init_adam seems ok something after that I guess */
	             {2,2,0},
	            {2,0,2},
	           {0,2,2},
	          {0,0,0},
	         {2,2,0},
	        {2,1,2},
	       {0,2,2},
#else /* this might be better for hexagons, space-wise efficient... */
#ifdef TRIA
/* Experimental TRIA5:7x7 */
	      {2,2,0,0,2,2,0},
	     {2,4,2,0,2,7,2},
	    {2,1,0,2,2,0,2},
	   {2,0,2,1,2,1,2},
	  {2,7,2,2,7,7,2},
	 {2,1,0,7,1,0,2},
	{0,2,2,2,2,2,2},
#else
/* 5:11x11 */
	          {2,2,2,2,2,2,0,0,0,0,0},
	         {2,1,1,7,0,1,2,0,0,0,0},
	        {2,1,2,2,2,2,7,2,0,0,0},
	       {2,1,2,0,0,0,2,0,2,0,0},
	      {2,1,2,0,0,0,0,2,1,2,0},
	     {2,1,2,0,0,0,0,0,2,7,2},
	    {0,2,1,2,0,0,0,0,2,0,2},
	   {0,0,2,1,2,0,0,0,2,1,2},
	  {0,0,0,2,1,2,2,2,2,4,2},
	 {0,0,0,0,2,1,1,1,1,5,2},
	{0,0,0,0,0,2,2,2,2,2,2}
#endif
#endif
};

static void
positionOfNeighbor(loopstruct * lp, int dir, int *pcol, int *prow)
{
	int         col = *pcol, row = *prow;

	{
		switch (dir) {
			case 0:
				col = (lp->wrap && (col + 1 == lp->ncols)) ?
					0 : col + 1;
				break;
			case 45:
				col = (lp->wrap && (col + 1 == lp->ncols)) ?
					0 : col + 1;
				row = (lp->wrap && !row) ?
					lp->bnrows - 1 : row - 1;
				break;
			case 60:
				if (lp->wrap) {
					if (!(row & 1))
						col = (col + 1 == lp->ncols) ?
							0 : col + 1;
					row = (!row) ? lp->nrows - 1 : row - 1;
				} else {
					col += (row & 1);
					row--;
				}
				break;
			case 90:
				row = (lp->wrap && !row) ?
					lp->bnrows - 1 : row - 1;
				break;
			case 120:
				if (lp->wrap) {
					if (row & 1)
						col = (!col) ? lp->ncols - 1 :
							col - 1;
					row = (!row) ? lp->nrows - 1 : row - 1;
				} else {
					col -= !(row & 1);
					row--;
				}
				break;
			case 135:
				col = (lp->wrap && !col) ?
					lp->ncols - 1 : col - 1;
				row = (lp->wrap && !row) ?
					lp->bnrows - 1 : row - 1;
				break;
			case 180:
				col = (lp->wrap && !col) ?
					lp->ncols - 1 : col - 1;
				break;
			case 225:
				col = (lp->wrap && !col) ?
					lp->ncols - 1 : col - 1;
				row = (lp->wrap && (row + 1 == lp->bnrows)) ?
					0 : row + 1;
				break;
			case 240:
				if (lp->wrap) {
					if (row & 1)
						col = (!col) ? lp->ncols - 1 :
							col - 1;
					row = (row + 1 == lp->nrows) ?
						0 : row + 1;
				} else {
					col -= !(row & 1);
					row++;
				}
				break;
			case 270:
				row = (lp->wrap && (row + 1 == lp->bnrows)) ?
					0 : row + 1;
				break;
			case 300:
				if (lp->wrap) {
					if (!(row & 1))
						col = (col + 1 == lp->ncols) ?
							0 : col + 1;
					row = (row + 1 == lp->nrows) ?
						0 : row + 1;
				} else {
					col += (row & 1);
					row++;
				}
				break;
			case 315:
				col = (lp->wrap && (col + 1 == lp->ncols)) ?
					0 : col + 1;
				row = (lp->wrap && (row + 1 == lp->bnrows)) ?
					0 : row + 1;
				break;
			default:
				(void) fprintf(stderr, "wrong direction %d\n", dir);
		}
	}
	*pcol = col;
	*prow = row;
}

static      Bool
withinBounds(loopstruct * lp, int col, int row)
{
	return (row >= !lp->wrap && row <= lp->bnrows - 1 - !lp->wrap &&
		col >= !lp->wrap && col <= lp->bncols - 1 - (!lp->wrap) *
		((local_neighbors == 6 && (row % 2)) - 1));
}

static void
fillcell(ModeInfo * mi, GC gc, int col, int row)
{
	loopstruct *lp = &loops[MI_SCREEN(mi)];

	if (local_neighbors == 6) {
		int         ccol = 2 * col + !(row & 1), crow = 2 * row;

		if (lp->vertical) {
			lp->shape.hexagon[0].x = lp->xb + ccol * lp->xs;
			lp->shape.hexagon[0].y = lp->yb + crow * lp->ys;
		} else {
			lp->shape.hexagon[0].y = lp->xb + ccol * lp->xs;
			lp->shape.hexagon[0].x = lp->yb + crow * lp->ys;
		}
		if (lp->xs == 1 && lp->ys == 1)
			XDrawPoint(MI_DISPLAY(mi), MI_WINDOW(mi), gc,
				lp->shape.hexagon[0].x, lp->shape.hexagon[0].y);
		else
			XFillPolygon(MI_DISPLAY(mi), MI_WINDOW(mi), gc,
				lp->shape.hexagon, 6, Convex, CoordModePrevious);
	} else {
		XFillRectangle(MI_DISPLAY(mi), MI_WINDOW(mi), gc,
			lp->xb + lp->xs * col, lp->yb + lp->ys * row,
			lp->xs - (lp->xs > 3), lp->ys - (lp->ys > 3));
	}
}

static void
drawcell(ModeInfo * mi, int col, int row, int state)
{
	loopstruct *lp = &loops[MI_SCREEN(mi)];
	XGCValues   gcv;
	GC          gc;

	if (MI_NPIXELS(mi) >= COLORS && state < COLORS) {
		gc = MI_GC(mi);
		XSetForeground(MI_DISPLAY(mi), gc, lp->colors[state]);
	} else {
		gcv.stipple = lp->pixmaps[state];
		gcv.foreground = MI_WHITE_PIXEL(mi);
		gcv.background = MI_BLACK_PIXEL(mi);
		XChangeGC(MI_DISPLAY(mi), lp->stippledGC,
			  GCStipple | GCForeground | GCBackground, &gcv);
		gc = lp->stippledGC;
	}
	fillcell(mi, gc, col, row);
}

#ifdef DEBUG
static void
print_state(ModeInfo * mi, int state)
{
	loopstruct *lp = &loops[MI_SCREEN(mi)];
	CellList   *locallist = lp->cellList[state];
	int         i = 0;

	(void) printf("state %d\n", state);
	while (locallist) {
		(void) printf("%d x %d, y %d\n", i,
			      locallist->pt.x, locallist->pt.y);
		locallist = locallist->next;
		i++;
	}
}

#endif

static void
free_state(loopstruct * lp, int state)
{
	CellList   *current;

	while (lp->cellList[state]) {
		current = lp->cellList[state];
		lp->cellList[state] = lp->cellList[state]->next;
		free(current);
	}
	lp->ncells[state] = 0;
}

static void
free_list(loopstruct * lp)
{
	int         state;

	for (state = 0; state < EXT_STATES; state++)
		free_state(lp, state);
}


static void
free_cells(loopstruct * lp)
{
	if (lp->oldcells != NULL) {
		free(lp->oldcells);
		lp->oldcells = (unsigned char *) NULL;
	}
	if (lp->newcells != NULL) {
		free(lp->newcells);
		lp->newcells = (unsigned char *) NULL;
	}
	free_list(lp);
}

static void
free_loop_screen(Display *display, loopstruct * lp)
{
	int         shade;

	if (lp == NULL) {
		return;
	}
	for (shade = 0; shade < lp->init_bits; shade++)
		if (lp->pixmaps[shade] != None) {
			XFreePixmap(display, lp->pixmaps[shade]);
			lp->pixmaps[shade] = None;
		}
	lp->init_bits = 0;
	if (lp->stippledGC != None) {
		XFreeGC(display, lp->stippledGC);
		lp->stippledGC = None;
	}
	free_cells(lp);
	lp = NULL;
}

static Bool
addtolist(ModeInfo * mi, int col, int row, unsigned char state)
{
	loopstruct *lp = &loops[MI_SCREEN(mi)];
	CellList   *current = lp->cellList[state];

	if ((lp->cellList[state] = (CellList *) malloc(sizeof (CellList))) ==
			NULL) {
		lp->cellList[state] = current;
		free_loop_screen(MI_DISPLAY(mi), lp);
		return False;
	}
	lp->cellList[state]->pt.x = col;
	lp->cellList[state]->pt.y = row;
	lp->cellList[state]->next = current;
	lp->ncells[state]++;
	return True;
}

static Bool
draw_state(ModeInfo * mi, int state)
{
	loopstruct *lp = &loops[MI_SCREEN(mi)];
	Display    *display = MI_DISPLAY(mi);
	GC          gc;
	XGCValues   gcv;
	CellList   *current = lp->cellList[state];

	if (MI_NPIXELS(mi) >= COLORS && state < COLORS) {
		gc = MI_GC(mi);
		XSetForeground(display, gc, lp->colors[state]);
	} else {
		gcv.stipple = lp->pixmaps[state];
		gcv.foreground = MI_WHITE_PIXEL(mi);
		gcv.background = MI_BLACK_PIXEL(mi);
		XChangeGC(display, lp->stippledGC,
			  GCStipple | GCForeground | GCBackground, &gcv);
		gc = lp->stippledGC;
	}

	if (local_neighbors == 6) {       /* Draw right away, slow */
		while (current) {
			int	 col, row, ccol, crow;

			col = current->pt.x;
			row = current->pt.y;
			ccol = 2 * col + !(row & 1), crow = 2 * row;
			if (lp->vertical) {
				lp->shape.hexagon[0].x = lp->xb + ccol * lp->xs;
				lp->shape.hexagon[0].y = lp->yb + crow * lp->ys;
			} else {
				lp->shape.hexagon[0].y = lp->xb + ccol * lp->xs;
				lp->shape.hexagon[0].x = lp->yb + crow * lp->ys;
			}
			if (lp->xs == 1 && lp->ys == 1)
				XDrawPoint(display, MI_WINDOW(mi), gc,
					lp->shape.hexagon[0].x, lp->shape.hexagon[0].y);
			else
				XFillPolygon(display, MI_WINDOW(mi), gc,
				  	lp->shape.hexagon, 6, Convex, CoordModePrevious);
			current = current->next;
		}
	} else {
		/* Take advantage of XFillRectangles */
		XRectangle *rects;
		int         nrects = 0;

		/* Create Rectangle list from part of the cellList */
		if ((rects = (XRectangle *) malloc(lp->ncells[state] *
				sizeof (XRectangle))) == NULL) {
			return False;
		}

		while (current) {
			rects[nrects].x = lp->xb + current->pt.x * lp->xs;
			rects[nrects].y = lp->yb + current->pt.y * lp->ys;
			rects[nrects].width = lp->xs - (lp->xs > 3);
			rects[nrects].height = lp->ys - (lp->ys > 3);
			current = current->next;
			nrects++;
		}
		/* Finally get to draw */
		XFillRectangles(display, MI_WINDOW(mi), gc, rects, nrects);
		/* Free up rects list and the appropriate part of the cellList */
		free(rects);
	}
	free_state(lp, state);
	XFlush(display);
	return True;
}

#define SHEATHCOUNT(a) ((a==1)||(a==2)||(a==4)||(a==6)||(a==7))
#define INSHEATH(b, c, d, e) ((SHEATHCOUNT(b) + SHEATHCOUNT(c) + \
	SHEATHCOUNT(d) + SHEATHCOUNT(e)) >= 2)

static void
setUndefinedRule(void)
{
	int a, b, c, d, e;

	if (sheath) {
	  for (a = 0; a < STATES; a++)
	    for (b = 0; b < STATES; b++)
	      for (c = 0; c < STATES; c++)
		for (d = 0; d < STATES; d++)
	 	  for (e = 0; e < STATES; e++)
		    if (INEF_TABLE_OUT(a, b, c, d, e) == EXT_STATES)
		      switch(a) {
		      case 0:
			if (((b == 1) || (c == 1) || (d == 1) || (e == 1)) &&
			    (INSHEATH(b, c, d, e))) {
			  INEF_TABLE_IN(a, b, c, d, e, 1);
			} else {
			  INEF_TABLE_IN(a, b, c, d, e, 0);
			}
			break;
		      case 1:
			if (((b == 7) || (c == 7) || (d == 7) || (e == 7)) &&
			    (INSHEATH(b, c, d, e))) {
			  INEF_TABLE_IN(a, b, c, d, e, 7);
			} else if (((b == 6) || (c == 6) || (d == 6) ||
			    (e == 6)) && (INSHEATH(b, c, d, e))) {
			  INEF_TABLE_IN(a, b, c, d, e, 6);
			} else if (((b == 4) || (c == 4) || (d == 4) ||
			    (e == 4)) && (INSHEATH(b, c, d, e))) {
			  INEF_TABLE_IN(a, b, c, d, e, 4);
			}
			break;
		      case 2:
			if (((b == 3) || (c == 3) || (d == 3) || (e == 3))) {
			  INEF_TABLE_IN(a, b, c, d, e, 1);
			} else if (((b == 2) || (c == 2) || (d == 2) ||
			    (e == 2))) {
			  INEF_TABLE_IN(a, b, c, d, e, 2);
			}
			break;
		      case 4:
		      case 6:
		      case 7:
			if (((b == 0) || (c == 0) || (d == 0) || (e == 0)) &&
			    (INSHEATH(b, c, d, e))) {
			  INEF_TABLE_IN(a, b, c, d, e, 0);
			}
			break;
		      }
	}
	if (dissolve) {
	  for (a = 0; a < EXT_STATES; a++)
	    for (b = 0; b < EXT_STATES; b++)
	      for (c = 0; c < EXT_STATES; c++)
		for (d = 0; d < EXT_STATES; d++)
		  for (e = 0; e < EXT_STATES; e++)
		    if (INEF_TABLE_OUT(a, b, c, d, e) == EXT_STATES) {
		      if (a == 8) {
			INEF_TABLE_IN(a, b, c, d, e, 0);
		      } else if ((b == 8) || (c == 8) || (d == 8) || (e == 8))
		        switch(a) {
		        case 0:
		        case 1:
			  if (((b > 1) && (b < 8)) || ((c > 1) && (c < 8)) ||
			      ((d > 1) && (d < 8)) || ((e > 1) && (e < 8))) {
			    INEF_TABLE_IN(a, b, c, d, e, 8);
			  } else {
			    INEF_TABLE_IN(a, b, c, d, e, a);
			  }
		 	  break;
		        case 2:
		        case 3:
		        case 5:
		          INEF_TABLE_IN(a, b, c, d, e, 0);
			  break;
		        case 4:
		        case 6:
		        case 7:
		          INEF_TABLE_IN(a, b, c, d, e, 1);
			  break;
		        }
		  }
	  INEF_TABLE_IN(1, 1, 1, 5, 2, 8);
	  INEF_TABLE_IN(1, 1, 5, 2, 1, 8);
	  INEF_TABLE_IN(1, 5, 2, 1, 1, 8);
	  INEF_TABLE_IN(1, 2, 1, 1, 5, 8);
	}
	/* Clearance of undefined rules */
	for (a = 0; a < EXT_STATES; a++)
	  for (b = 0; b < EXT_STATES; b++)
	    for (c = 0; c < EXT_STATES; c++)
	      for (d = 0; d < EXT_STATES; d++)
		for (e = 0; e < EXT_STATES; e++)
		  if (INEF_TABLE_OUT(a, b, c, d, e) == EXT_STATES) {
		    if (dissolve) {
		      INEF_TABLE_IN(a, b, c, d, e, 8);
		    } else {
		      INEF_TABLE_IN(a, b, c, d, e, a);
		    }
		  }
}

static void
setUndefinedEvoRule(void)
{
	int a, b, c, d, e;

	for (a = 0; a < EXT_STATES; a++)
	  for (b = 0; b < EXT_STATES; b++)
	    for (c = 0; c < EXT_STATES; c++)
	      for (d = 0; d < EXT_STATES; d++)
		for (e = 0; e < EXT_STATES; e++)
		  if (INEF_TABLE_OUT(a, b, c, d, e) == EXT_STATES) {
		    if (a == 8) {
		      INEF_TABLE_IN(a, b, c, d, e, 0);
		    } else if ((b == 8) || (c == 8) || (d == 8) || (e == 8))
		      switch(a) {
		      case 0:
		      case 1:
		        if (((b > 1) && (b < 8)) || ((c > 1) && (c < 8)) ||
			    ((d > 1) && (d < 8)) || ((e > 1) && (e < 8))) {
			  INEF_TABLE_IN(a, b, c, d, e, 8);
			} else {
			  INEF_TABLE_IN(a, b, c, d, e, a);
			}
		 	break;
		      case 2:
		      case 3:
		      case 5:
		        INEF_TABLE_IN(a, b, c, d, e, 0);
			break;
		      case 4:
		      case 6:
		      case 7:
		        INEF_TABLE_IN(a, b, c, d, e, 1);
			break;
		      }
		  }
	/* Clearance of undefined rules */
	for (a = 0; a < EXT_STATES; a++)
	  for (b = 0; b < EXT_STATES; b++)
	    for (c = 0; c < EXT_STATES; c++)
	      for (d = 0; d < EXT_STATES; d++)
		for (e = 0; e < EXT_STATES; e++)
		  if (INEF_TABLE_OUT(a, b, c, d, e) == EXT_STATES) {
		    if (a == 0) {
		      INEF_TABLE_IN(a, b, c, d, e, 0);
		    } else {
		      INEF_TABLE_IN(a, b, c, d, e, 8);
		    }
		  }
}

static Bool
init_table(ModeInfo *mi)
{
	if (table == NULL) {
		int mult = 1;
		unsigned int tt, c, n[MAXNEIGHBORS], i;
		int         j, k;
		int  sizeTransitionTable = sizeof (transitionTable) /
			sizeof (unsigned int);
		int  sizeBylTransitionTable = sizeof (bylTransitionTable) /
			sizeof (unsigned int);
		int  sizeChou1TransitionTable = sizeof (chou1TransitionTable) /
			sizeof (unsigned int);
		int  sizeChou2TransitionTable = sizeof (chou2TransitionTable) /
			sizeof (unsigned int);
		int  sizeEvoTransitionTable = sizeof (evoTransitionTable) /
			sizeof (unsigned int);
		int  sizeHexTransitionTable = sizeof (hexTransitionTable) /
			sizeof (unsigned int);

		if (local_neighbors == 4) {
			if (MI_IS_FULLRANDOM(mi) ||
			    !(langton || byl || chou1 || chou2 || dissolve || evolve || sheath)) {
				int i = NRAND(5);

				if (i == 1) /* Make 1-3 less likely */
					i += NRAND(3);
				else if (i > 1)
					i += 2;
				byl = (i == 1);
				chou1 = (i == 2);
				chou2 = (i == 3);
				evolve = (i == 4);
				dissolve = (i == 5);
				if (i == 5)
					sheath = (LRAND() & 1);
				sheath = (i == 6);
			} else {
				evolve = (evolve && !langton && !byl && !chou1 && !chou2);
				dissolve = (dissolve && !evolve && !langton && !byl && !chou1 && !chou2);
				sheath = (sheath && !evolve && !langton && !byl && !chou1 && !chou2);
			}
			langton = (!(dissolve || evolve || sheath || byl || chou1 || chou2));
		} else {
			langton = True;
			byl = False;
			chou1 = False;
			chou2 = False;
			evolve = False;
			dissolve = False;
			sheath = False;
		}
		if (MI_IS_VERBOSE(mi)) {
			if (langton)
				(void) fprintf(stdout, "Langton's Loops\n");
			else if (byl)
				(void) fprintf(stdout, "Byl's Loops\n");
			else if (chou1)
				(void) fprintf(stdout, "Chou-Reggia 1 Loops\n");
			else if (chou2)
				(void) fprintf(stdout, "Chou-Reggia 2 Loops\n");
			else if (evolve)
				(void) fprintf(stdout, "Evolving Loops\n");
			else if (dissolve && !sheath)
				(void) fprintf(stdout, "Disolving Loops\n");
			else if (dissolve && sheath)
				(void) fprintf(stdout, "Disolving Sheath Loops\n");
			else if (sheath)
				(void) fprintf(stdout, "Sheath Loops\n");
			else
				(void) fprintf(stderr, "Unknown Loop\n");
		}
		for (j = 0; j < local_neighbors; j++)
			mult *= (8 + !(langton || byl || chou1 || chou2));
#if 1
/* using space-wise inefficient method,
   uhhh I'll try to think harder on how to do it later %-) */
/* also (!dissolve && sheath) should only be multiplied by 8... here and
   above, but lets keep it simple for now */
/* Because of this will be using INEF_TABLE instead of EXT_TABLE */
		if (!(langton || byl || chou1 || chou2))
			mult *= 9;
#endif

		if ((table = (unsigned int *) calloc(mult, sizeof (unsigned int))) == NULL) {
			return False;
		}
		if ((!(langton || byl || chou1 || chou2)) && (dissolve || evolve || sheath)) {
	int a, b, c, d, e;

	for (a = 0; a < EXT_STATES; a++)
	  for (b = 0; b < EXT_STATES; b++)
	    for (c = 0; c < EXT_STATES; c++)
	      for (d = 0; d < EXT_STATES; d++)
		for (e = 0; e < EXT_STATES; e++) {
		  INEF_TABLE_IN(a, b, c, d, e, EXT_STATES);
		}
		}
#ifdef RAND_RULES
		/* Here I was interested to see what happens when it hits a
		   wall....  Rules not normally used take over... takes too
		   much time though.  Not for dissolve or evolve. */
		/* Each state = 3 bits */
		if (MAXRAND < 16777216.0) {
			for (j = 0; j < mult; j++) {
				table[j] = (unsigned int) ((NRAND(4096) << 12) & NRAND(4096));
			}
		} else {
			for  (j = 0; j < mult; j++) {
				table[j] = (unsigned int) (NRAND(16777216));
			}
		}
#endif
		if (local_neighbors == 6) {
			for (j = 0; j < sizeHexTransitionTable; j++) {
				tt = hexTransitionTable[j];
				TRANSITION(tt, i);
				for (k = 0; k < local_neighbors; k++) {
					TRANSITION(tt, n[k]);
				}
				FINALTRANSITION(tt, c);
				HEX_TABLE_IN(c, n[0], n[1], n[2], n[3], n[4], n[5], i);
				HEX_TABLE_IN(c, n[1], n[2], n[3], n[4], n[5], n[0], i);
				HEX_TABLE_IN(c, n[2], n[3], n[4], n[5], n[0], n[1], i);
				HEX_TABLE_IN(c, n[3], n[4], n[5], n[0], n[1], n[2], i);
				HEX_TABLE_IN(c, n[4], n[5], n[0], n[1], n[2], n[3], i);
				HEX_TABLE_IN(c, n[5], n[0], n[1], n[2], n[3], n[4], i);
			}
		} else if (byl) {
			for (j = 0; j < sizeBylTransitionTable; j++) {
				tt = bylTransitionTable[j];
				TRANSITION(tt, i);
				for (k = 0; k < local_neighbors; k++) {
					TRANSITION(tt, n[k]);
				}
				FINALTRANSITION(tt, c);
				TABLE_IN(c, n[0], n[1], n[2], n[3], i);
				TABLE_IN(c, n[1], n[2], n[3], n[0], i);
				TABLE_IN(c, n[2], n[3], n[0], n[1], i);
				TABLE_IN(c, n[3], n[0], n[1], n[2], i);
			}
		} else if (chou1) {
			for (j = 0; j < sizeChou1TransitionTable; j++) {
				tt = chou1TransitionTable[j];
				TRANSITION(tt, i);
				for (k = 0; k < local_neighbors; k++) {
					TRANSITION(tt, n[k]);
				}
				FINALTRANSITION(tt, c);
				TABLE_IN(c, n[0], n[1], n[2], n[3], i);
				TABLE_IN(c, n[1], n[2], n[3], n[0], i);
				TABLE_IN(c, n[2], n[3], n[0], n[1], i);
				TABLE_IN(c, n[3], n[0], n[1], n[2], i);
			}
		} else if (chou2) {
			for (j = 0; j < sizeChou2TransitionTable; j++) {
				tt = chou2TransitionTable[j];
				TRANSITION(tt, i);
				for (k = 0; k < local_neighbors; k++) {
					TRANSITION(tt, n[k]);
				}
				FINALTRANSITION(tt, c);
				TABLE_IN(c, n[0], n[1], n[2], n[3], i);
				TABLE_IN(c, n[1], n[2], n[3], n[0], i);
				TABLE_IN(c, n[2], n[3], n[0], n[1], i);
				TABLE_IN(c, n[3], n[0], n[1], n[2], i);
			}
		} else if (langton) {
			for (j = 0; j < sizeTransitionTable; j++) {
				tt = transitionTable[j];
				TRANSITION(tt, i);
				for (k = 0; k < local_neighbors; k++) {
					TRANSITION(tt, n[k]);
				}
				FINALTRANSITION(tt, c);
				TABLE_IN(c, n[0], n[1], n[2], n[3], i);
				TABLE_IN(c, n[1], n[2], n[3], n[0], i);
				TABLE_IN(c, n[2], n[3], n[0], n[1], i);
				TABLE_IN(c, n[3], n[0], n[1], n[2], i);
			}
		} else if (dissolve || evolve || sheath) {
			for (j = 0; j < ((evolve) ? sizeEvoTransitionTable :
					sizeTransitionTable);  j++) {
				tt = (evolve) ? evoTransitionTable[j] :
					transitionTable[j];
				TRANSITION(tt, i);
				for (k = 0; k < local_neighbors; k++) {
					TRANSITION(tt, n[k]);
				}
				FINALTRANSITION(tt, c);
				INEF_TABLE_IN(c, n[0], n[1], n[2], n[3], i);
				INEF_TABLE_IN(c, n[1], n[2], n[3], n[0], i);
				INEF_TABLE_IN(c, n[2], n[3], n[0], n[1], i);
				INEF_TABLE_IN(c, n[3], n[0], n[1], n[2], i);
			}
			if (evolve)
				setUndefinedEvoRule();
			else
				setUndefinedRule();
		}
	}
	return True;
}

#define	BLUE 1
#define RED 2
static void
init_flaw(ModeInfo * mi)
{
	loopstruct *lp = &loops[MI_SCREEN(mi)];
	int a, b;
	int color = RED;

	if (lp->bncols <= 6 || lp->bnrows <= 6)
		return;
	a = MIN(lp->bncols - 6, 2 * ((local_neighbors == 6) ?
		 HEX_MINGRIDSIZE : MINGRIDSIZE));
	a = NRAND(a) + (lp->bncols - a) / 2;
	b = MIN(lp->bnrows - 6, 2 * ((local_neighbors == 6) ?
		 HEX_MINGRIDSIZE : MINGRIDSIZE));
	b = NRAND(b) + (lp->bnrows - b) / 2;
	if (lp->mincol > a) {
		lp->mincol = a;
		if (lp->mincol < !lp->wrap)
			lp->mincol = !lp->wrap;
	}
	if (lp->minrow > b) {
		lp->minrow = b;
		if (lp->minrow < !lp->wrap)
			lp->minrow = !lp->wrap;
	}
	if (lp->maxcol < a + 2) {
		lp->maxcol = a + 2;
		if (lp->maxcol > lp->ncols - 1 - !lp->wrap)
			lp->maxcol = lp->ncols - 1 - !lp->wrap;
	}
	if (lp->maxrow < b + 2) {
		lp->maxrow = b + 2;
		if (lp->maxrow > lp->nrows - 1 - !lp->wrap)
			lp->maxrow = lp->nrows - 1 - !lp->wrap;
	}
	if (local_neighbors == 6) {
		lp->newcells[b * lp->bncols + a + (!lp->wrap + b) % 2 ] = color;
		lp->newcells[b * lp->bncols + a + 1 +
			(!lp->wrap + b) % 2] = color;
		lp->newcells[(b + 1) * lp->bncols + a] = color;
		lp->newcells[(b + 1) * lp->bncols + a + 2] = color;
		lp->newcells[(b + 2) * lp->bncols + a +
			(!lp->wrap + b) % 2] = color;
		lp->newcells[(b + 2) * lp->bncols + a + 1 +
			(!lp->wrap + b) % 2] = color;
	} else if (byl || chou1 || chou2) {
		int orient = NRAND(2);
		if (chou1 || chou2)
			color = BLUE;
		lp->newcells[lp->bncols * (b + 1) + a + 1] = color;
		if (orient == 0)
			lp->newcells[lp->bncols * b + a + 1] = color;
		else
			lp->newcells[lp->bncols * (b + 1) + a] = color;
	} else {
		int orient = NRAND(4);
		lp->newcells[lp->bncols * (b + 1) + a + 1] = color;
		if (orient == 0 || orient == 1) {
			lp->newcells[lp->bncols * b + a + 1] = color;
		}
		if (orient == 1 || orient == 2) {
			lp->newcells[lp->bncols * (b + 1) + a + 2] = color;
		}
		if (orient == 2 || orient == 3) {
			lp->newcells[lp->bncols * (b + 2) + a + 1] = color;
		}
		if (orient == 3 || orient == 0) {
			lp->newcells[lp->bncols * (b + 1) + a] = color;
		}
	}
}

static int
sqAdamLoopX(void)
{
	if (byl) {
		return BYL_ADAM_LOOPX;
	} else if (chou1) {
		return CHOU1_ADAM_LOOPX;
	} else if (chou2) {
		return CHOU2_ADAM_LOOPX;
	} else {
		return SQ_ADAM_LOOPX(evolve);
	}
}

static int
sqAdamLoopY(void)
{
	if (byl) {
		return BYL_ADAM_LOOPY;
	} else if (chou1) {
		return CHOU1_ADAM_LOOPY;
	} else if (chou2) {
		return CHOU2_ADAM_LOOPY;
	} else {
		return SQ_ADAM_LOOPY(evolve);
	}
}

static int loopCell(int i, int j, Bool clockwise)
{
	int loopX = sqAdamLoopX();
	if (byl) {
		return ((clockwise) ?
			bylReproducingLoop[j][loopX - i - 1] :
			bylReproducingLoop[j][i]);
	} else if (chou1) {
		return ((clockwise) ?
			chou1ReproducingLoop[j][loopX - i - 1] :
			chou1ReproducingLoop[j][i]);
	} else if (chou2) {
		return ((clockwise) ?
			chou2ReproducingLoop[j][loopX - i - 1] :
			chou2ReproducingLoop[j][i]);
	}
	return ((evolve) ? ((clockwise) ?
		evoReproducingLoop[j][loopX - i - 1] :
		evoReproducingLoop[j][i]) :
		((clockwise) ?
		selfReproducingLoop[j][loopX - i - 1] :
		selfReproducingLoop[j][i]));
}

static void
init_adam(ModeInfo * mi)
{
	loopstruct *lp = &loops[MI_SCREEN(mi)];
	XPoint      start;
	int         i, j, dir;

#ifdef DELAYDEBUGLOOP
	lp->clockwise = 0;
	if (!MI_COUNT(mi)) /* Probably doing testing so do not confuse */
#endif
	lp->clockwise = (Bool) (LRAND() & 1);
#ifdef DELAYDEBUGLOOP
	dir = 0;
	if (!MI_COUNT(mi)) /* Probably doing testing so do not confuse */
#endif
	dir = NRAND(local_neighbors);
	start.x = 0;
	start.y = 0;
	if (local_neighbors == 6) {
		int k;

		switch (dir) {
			case 0:
				start.x = (lp->bncols - HEX_ADAM_LOOPX / 2) / 2;
				start.y = (lp->bnrows - HEX_ADAM_LOOPY) / 2;
				if (lp->mincol > start.x - 2)
					lp->mincol = start.x - 2;
				if (lp->minrow > start.y - 1)
					lp->minrow = start.y - 1;
				if (lp->maxcol < start.x + HEX_ADAM_LOOPX + 1)
					lp->maxcol = start.x + HEX_ADAM_LOOPX + 1;
				if (lp->maxrow < start.y + HEX_ADAM_LOOPY + 1)
					lp->maxrow = start.y + HEX_ADAM_LOOPY + 1;
				for (j = 0; j < HEX_ADAM_LOOPY; j++) {
					for (i = 0; i < HEX_ADAM_LOOPX; i++) {
						k = (((lp->bnrows / 2 + HEX_ADAM_LOOPY / 2) % 2) ? -j / 2 : -(j + 1) / 2);
						lp->newcells[(start.y + j) * lp->bncols + start.x + i + k] =
		 					(lp->clockwise) ?
					hexReproducingLoop[i][j] :
					hexReproducingLoop[j][i];
					}
				}
				break;
			case 1:
				start.x = (lp->bncols - (HEX_ADAM_LOOPX + HEX_ADAM_LOOPY) / 2) / 2;
				start.y = (lp->bnrows - HEX_ADAM_LOOPX + HEX_ADAM_LOOPY) / 2;
				if (lp->mincol > start.x - 1)
					lp->mincol = start.x - 1;
				if (lp->minrow > start.y - HEX_ADAM_LOOPX)
					lp->minrow = start.y - HEX_ADAM_LOOPX;
				if (lp->maxcol < start.x + (HEX_ADAM_LOOPX + HEX_ADAM_LOOPY) / 2 + 1)
					lp->maxcol = start.x + (HEX_ADAM_LOOPX + HEX_ADAM_LOOPY) / 2 + 1;
				if (lp->maxrow < start.y + HEX_ADAM_LOOPY + 1)
					lp->maxrow = start.y + HEX_ADAM_LOOPY + 1;
				for (j = 0; j < HEX_ADAM_LOOPY; j++) {
					for (i = 0; i < HEX_ADAM_LOOPX; i++) {
						k = (((lp->bnrows / 2 + (HEX_ADAM_LOOPX + HEX_ADAM_LOOPY) / 2) % 2)
              ? -(i + j + 1) / 2 : -(i + j) / 2);
						lp->newcells[(start.y + j - i - lp->wrap) * lp->bncols + lp->wrap * (lp->bncols - (start.y + j - i + 1) % 2) + start.x + i + j + k] =
		 					(lp->clockwise) ?
		 					hexReproducingLoop[i][j] :
		 					hexReproducingLoop[j][i];
					}
				}
				break;
			case 2:
				start.x = (lp->bncols - HEX_ADAM_LOOPY / 2) / 2;
				start.y = (lp->bnrows - HEX_ADAM_LOOPX) / 2;
				if (lp->mincol > start.x - 2)
					lp->mincol = start.x - 2;
				if (lp->minrow > start.y - 1)
					lp->minrow = start.y - 1;
				if (lp->maxcol < start.x + HEX_ADAM_LOOPX + 1)
					lp->maxcol = start.x + HEX_ADAM_LOOPX + 1;
				if (lp->maxrow < start.y + HEX_ADAM_LOOPY + 1)
					lp->maxrow = start.y + HEX_ADAM_LOOPY + 1;
				for (j = 0; j < HEX_ADAM_LOOPX; j++) {
					for (i = 0; i < HEX_ADAM_LOOPY; i++) {
						k = (((lp->bnrows / 2 + HEX_ADAM_LOOPX / 2) % 2) ? -(HEX_ADAM_LOOPX - j - 1) / 2 : -(HEX_ADAM_LOOPX - j) / 2);
						lp->newcells[(start.y + j) * lp->bncols + start.x + i + k] =
		 					(lp->clockwise) ?
		 					hexReproducingLoop[j][HEX_ADAM_LOOPX - i - 1] :
		 					hexReproducingLoop[i][HEX_ADAM_LOOPY - j - 1];
					}
				}
				break;
			case 3:
				start.x = (lp->bncols - HEX_ADAM_LOOPX / 2) / 2;
				start.y = (lp->bnrows - HEX_ADAM_LOOPY) / 2;
				if (lp->mincol > start.x - 1)
					lp->mincol = start.x - 1;
				if (lp->minrow > start.y - 1)
					lp->minrow = start.y - 1;
				if (lp->maxcol < start.x + HEX_ADAM_LOOPX + 1)
					lp->maxcol = start.x + HEX_ADAM_LOOPX + 1;
				if (lp->maxrow < start.y + HEX_ADAM_LOOPY + 1)
					lp->maxrow = start.y + HEX_ADAM_LOOPY + 1;
				for (j = 0; j < HEX_ADAM_LOOPY; j++) {
					for (i = 0; i < HEX_ADAM_LOOPX; i++) {
						k = (((lp->bnrows / 2 + HEX_ADAM_LOOPY / 2) % 2) ? -j / 2 : -(j + 1) / 2);
						lp->newcells[(start.y + j) * lp->bncols + start.x + i + k] =
		 					(lp->clockwise) ?
		 					hexReproducingLoop[HEX_ADAM_LOOPX - i - 1][HEX_ADAM_LOOPY - j - 1] :
		 					hexReproducingLoop[HEX_ADAM_LOOPY - j - 1][HEX_ADAM_LOOPX - i - 1];
					}
				}
				break;
			case 4:
				start.x = (lp->bncols - (HEX_ADAM_LOOPX + HEX_ADAM_LOOPY) / 2) / 2;
				start.y = (lp->bnrows - HEX_ADAM_LOOPX + HEX_ADAM_LOOPY) / 2;
				if (lp->mincol > start.x - 1)
					lp->mincol = start.x - 1;
				if (lp->minrow > start.y - HEX_ADAM_LOOPX)
					lp->minrow = start.y - HEX_ADAM_LOOPX;
				if (lp->maxcol < start.x + (HEX_ADAM_LOOPX + HEX_ADAM_LOOPY) / 2 + 1)
					lp->maxcol = start.x + (HEX_ADAM_LOOPX + HEX_ADAM_LOOPY) / 2 + 1;
				if (lp->maxrow < start.y + HEX_ADAM_LOOPY + 1)
					lp->maxrow = start.y + HEX_ADAM_LOOPY + 1;
				for (j = 0; j < HEX_ADAM_LOOPY; j++) {
					for (i = 0; i < HEX_ADAM_LOOPX; i++) {
						k = (((lp->bnrows / 2 + (HEX_ADAM_LOOPX + HEX_ADAM_LOOPY) / 2) % 2)
              ? -(i + j + 1) / 2 : -(i + j) / 2);
						lp->newcells[(start.y + j - i) * lp->bncols - lp->wrap * ((start.y + j - i + 1) % 2) + start.x + i + j + k] =
		 					(lp->clockwise) ?
		 					hexReproducingLoop[HEX_ADAM_LOOPX - i - 1][HEX_ADAM_LOOPY - j - 1] :
		 					hexReproducingLoop[HEX_ADAM_LOOPY - j - 1][HEX_ADAM_LOOPX - i - 1];
					}
				}
				break;
			case 5:
				start.x = (lp->bncols - HEX_ADAM_LOOPY / 2) / 2;
				start.y = (lp->bnrows - HEX_ADAM_LOOPX) / 2;
				if (lp->mincol > start.x - 2)
					lp->mincol = start.x - 2;
				if (lp->minrow > start.y - 1)
					lp->minrow = start.y - 1;
				if (lp->maxcol < start.x + HEX_ADAM_LOOPY + 1)
					lp->maxcol = start.x + HEX_ADAM_LOOPY + 1;
				if (lp->maxrow < start.y + HEX_ADAM_LOOPX + 1)
					lp->maxrow = start.y + HEX_ADAM_LOOPX + 1;
				for (j = 0; j < HEX_ADAM_LOOPX; j++) {
					for (i = 0; i < HEX_ADAM_LOOPY; i++) {
						k = (((lp->bnrows / 2 + HEX_ADAM_LOOPX / 2) % 2) ? -(HEX_ADAM_LOOPX - j - 1) / 2 : -(HEX_ADAM_LOOPX - j) / 2);
						lp->newcells[(start.y + j) * lp->bncols + start.x + i + k] =
		 					(lp->clockwise) ?
		 					hexReproducingLoop[HEX_ADAM_LOOPY - j - 1][i] :
		 					hexReproducingLoop[HEX_ADAM_LOOPX - i - 1][j];
					}
				}
				break;
		}
#ifdef DEBUGTEST
				/* (void) printf ("s %d  s %d \n", start.x, start.y); */
		(void) printf ("%d %d %d %d %d\n",
     start.x + i + ((lp->bnrows / 2 % 2) ? -j / 2 : -(j + 1) / 2) - lp->bx,
		 start.y + j - lp->by, i, j, hexReproducingLoop[j][i]);
		/* Draw right away */
		drawcell(mi, start.x + i + ((lp->bnrows / 2 % 2) ? -j / 2 : -(j + 1) / 2) - lp->bx,
		 start.y + j - lp->by,
		 hexReproducingLoop[j][i]);
#endif
	} else {
		XPoint dirx, diry;
		dirx.x = 0, dirx.y = 0;
		diry.x = 0, diry.y = 0;
		switch (dir) {
			case 0:
				start.x = (lp->bncols - sqAdamLoopX()) / 2;
				start.y = (lp->bnrows - sqAdamLoopY()) / 2;
				dirx.x = 1, dirx.y = 0;
				diry.x = 0, diry.y = 1;
				if (lp->mincol > start.x)
					lp->mincol = start.x;
				if (lp->minrow > start.y)
					lp->minrow = start.y;
				if (lp->maxcol < start.x + sqAdamLoopX())
					lp->maxcol = start.x + sqAdamLoopX();
				if (lp->maxrow < start.y + sqAdamLoopY())
					lp->maxrow = start.y + sqAdamLoopY();
				break;
			case 1:
				start.x = (lp->bncols + sqAdamLoopY()) / 2;
				start.y = (lp->bnrows - sqAdamLoopX()) / 2;
				dirx.x = 0, dirx.y = 1;
				diry.x = -1, diry.y = 0;
				if (lp->mincol > start.x - sqAdamLoopY())
					lp->mincol = start.x - sqAdamLoopY();
				if (lp->minrow > start.y)
					lp->minrow = start.y;
				if (lp->maxcol < start.x)
					lp->maxcol = start.x;
				if (lp->maxrow < start.y + sqAdamLoopX())
					lp->maxrow = start.y + sqAdamLoopX();
				break;
			case 2:
				start.x = (lp->bncols + sqAdamLoopX()) / 2;
				start.y = (lp->bnrows + sqAdamLoopY()) / 2;
				dirx.x = -1, dirx.y = 0;
				diry.x = 0, diry.y = -1;
				if (lp->mincol > start.x - sqAdamLoopX())
					lp->mincol = start.x - sqAdamLoopX();
				if (lp->minrow > start.y - sqAdamLoopY())
					lp->minrow = start.y - sqAdamLoopY();
				if (lp->maxcol < start.x)
					lp->maxcol = start.x;
				if (lp->maxrow < start.y)
					lp->maxrow = start.y;
				break;
			case 3:
				start.x = (lp->bncols - sqAdamLoopY()) / 2;
				start.y = (lp->bnrows + sqAdamLoopX()) / 2;
				dirx.x = 0, dirx.y = -1;
				diry.x = 1, diry.y = 0;
				if (lp->mincol > start.x)
					lp->mincol = start.x;
				if (lp->minrow > start.y - sqAdamLoopX())
					lp->minrow = start.y - sqAdamLoopX();
				if (lp->maxcol < start.x + sqAdamLoopX())
					lp->maxcol = start.x + sqAdamLoopX();
				if (lp->maxrow < start.y)
					lp->maxrow = start.y;
				break;
		}
		for (j = 0; j < sqAdamLoopY(); j++)
			for (i = 0; i < sqAdamLoopX(); i++) {
			  int value = loopCell(i, j, lp->clockwise);

			  lp->newcells[lp->bncols * (start.y + dirx.y * i + diry.y * j) +
			    start.x + dirx.x * i + diry.x * j] = value;
#ifdef DEBUG
			  /* Draw right away */
			  drawcell(mi, start.x + dirx.x * i + diry.x * j - lp->bx,
			   start.y + dirx.y * i + diry.y * j - lp->by,
			   value);
#endif
			}
	}
}

static void
do_gen(loopstruct * lp)
{
	int         i, j, k;
	unsigned char *z;
	unsigned int n[MAXNEIGHBORS];
	unsigned int c;

#if 0
#define COLBOUND(X) ((!lp->wrap || ((X != 0) && (X != lp->bncols - 1))) ? X : ((X == 0) ? lp->bncols - 1 : 0))
#define ROWBOUND(Y) ((!lp->wrap || ((Y != 0) && (Y != lp->bnrows - 1))) ? Y : ((Y == 0) ? lp->bnrows - 1 : 0))
#define LOC(X, Y) (*(lp->oldcells + COLBOUND(X) + (ROWBOUND(Y) * lp->bncols)))
#else
#define LOC(X, Y) (*(lp->oldcells + X + Y * lp->bncols))
#endif

	for (j = lp->minrow; j <= lp->maxrow; j++) {
		for (i = lp->mincol; i <= lp->maxcol; i++) {
			z = lp->newcells + i + j * lp->bncols;
			c = LOC(i, j);
			for (k = 0; k < local_neighbors; k++) {
				int         newi = i, newj = j;

				positionOfNeighbor(lp,
					k * ANGLES / local_neighbors,
					&newi, &newj);
				n[k] = 0;
		    if (withinBounds(lp, newi, newj)) {
					n[k] = LOC(newi, newj);
				}
#if 0
		    else {
			(void) printf("newi %d newj %d, %d %d\n",
				newi, newj, lp->maxcol, lp->maxrow);
			}
#endif
			}
			if (local_neighbors == 6) {
				*z = (lp->clockwise) ?
					HEX_TABLE_OUT(c, n[5], n[4], n[3], n[2], n[1], n[0]) :
					HEX_TABLE_OUT(c, n[0], n[1], n[2], n[3], n[4], n[5]);
			} else if (langton || byl || chou1 || chou2) {
				*z = (lp->clockwise) ?
					TABLE_OUT(c, n[3], n[2], n[1], n[0]) :
					TABLE_OUT(c, n[0], n[1], n[2], n[3]);
			} else if (dissolve || evolve || sheath) {
				*z = (lp->clockwise) ?
					INEF_TABLE_OUT(c, n[3], n[2], n[1], n[0]) :
					INEF_TABLE_OUT(c, n[0], n[1], n[2], n[3]);
			}
		}
	}
}

ENTRYPOINT void
release_loop(ModeInfo * mi)
{
	if (loops != NULL) {
		int         screen;

		for (screen = 0; screen < MI_NUM_SCREENS(mi); screen++)
			free_loop_screen(MI_DISPLAY(mi), &loops[screen]);
		free(loops);
		loops = (loopstruct *) NULL;
	}
	if (table != NULL) {
		free(table);
		table = (unsigned int *) NULL;
	}
	local_neighbors = 0;
}

ENTRYPOINT void
init_loop(ModeInfo * mi)
{
	Display    *display = MI_DISPLAY(mi);
	Window      window = MI_WINDOW(mi);
	int         i, size = MI_SIZE(mi);
	loopstruct *lp;
	XGCValues   gcv;

	MI_INIT(mi, loops);
	lp = &loops[MI_SCREEN(mi)];

	lp->redrawing = 0;

	free_cells(lp);
	/* Need stipple for Evoloop */
	if (lp->init_bits == 0) {
		if (lp->stippledGC == None) {
			gcv.fill_style = FillOpaqueStippled;
			if ((lp->stippledGC = XCreateGC(display, window,
				 GCFillStyle, &gcv)) == None) {
				free_loop_screen(display, lp);
				return;
			}
		}
		LOOPBITS(stipples[0], STIPPLESIZE, STIPPLESIZE);
		LOOPBITS(stipples[2], STIPPLESIZE, STIPPLESIZE);
		LOOPBITS(stipples[3], STIPPLESIZE, STIPPLESIZE);
		LOOPBITS(stipples[4], STIPPLESIZE, STIPPLESIZE);
		LOOPBITS(stipples[6], STIPPLESIZE, STIPPLESIZE);
		LOOPBITS(stipples[7], STIPPLESIZE, STIPPLESIZE);
		LOOPBITS(stipples[8], STIPPLESIZE, STIPPLESIZE);
		LOOPBITS(stipples[10], STIPPLESIZE, STIPPLESIZE);
		LOOPBITS(stipples[5], STIPPLESIZE, STIPPLESIZE);
	}
	if (MI_NPIXELS(mi) >= COLORS) {
		/* Maybe these colors should be randomized */
		lp->colors[0] = MI_BLACK_PIXEL(mi);
		lp->colors[2] = MI_PIXEL(mi, 0);	/* RED */
		lp->colors[4] = MI_PIXEL(mi, MI_NPIXELS(mi) / REALCOLORS);	/* YELLOW */
		lp->colors[3] = MI_PIXEL(mi, 2 * MI_NPIXELS(mi) / REALCOLORS);	/* GREEN */
		lp->colors[7] = MI_PIXEL(mi, 3 * MI_NPIXELS(mi) / REALCOLORS);	/* CYAN */
		lp->colors[1] = MI_PIXEL(mi, 4 * MI_NPIXELS(mi) / REALCOLORS);	/* BLUE */
		lp->colors[5] = MI_PIXEL(mi, 5 * MI_NPIXELS(mi) / REALCOLORS);	/* MAGENTA */
		lp->colors[6] = MI_WHITE_PIXEL(mi);
	}
	lp->generation = 0;
	lp->width = MI_WIDTH(mi);
	lp->height = MI_HEIGHT(mi);

	if (!local_neighbors) {
		for (i = 0; i < NEIGHBORKINDS; i++) {
			if (neighbors == plots[i]) {
				local_neighbors = neighbors;
				break;
			}
			if (i == NEIGHBORKINDS - 1) {
				if (NRAND(7))
					local_neighbors = 4;
				else
					local_neighbors = 6;
				break;
			}
		}
	}
	if (MI_IS_FULLRANDOM(mi)) {

		lp->wrap = (Bool) (LRAND() & 1);
		lp->vertical = (Bool) (LRAND() & 1);
	} else {
		lp->wrap = wrap;
		lp->vertical= vertical;
	}

  if (local_neighbors == 6) {
    int         nccols, ncrows;

    if (!lp->vertical) {
       lp->height = MI_WIDTH(mi);
       lp->width = MI_HEIGHT(mi);
    }
    if (lp->width < 8)
      lp->width = 8;
    if (lp->height < 8)
      lp->height = 8;
    if (size < -MINSIZE) {
      lp->ys = NRAND(MIN(-size, MAX(MINSIZE, MIN(lp->width, lp->height) /
              HEX_MINGRIDSIZE)) - MINSIZE + 1) + MINSIZE;
    } else if (size < MINSIZE) {
      if (!size)
        lp->ys = MAX(MINSIZE, MIN(lp->width, lp->height) / HEX_MINGRIDSIZE);
      else
        lp->ys = MINSIZE;
    } else
      lp->ys = MIN(size, MAX(MINSIZE, MIN(lp->width, lp->height) /
                 HEX_MINGRIDSIZE));
    lp->xs = lp->ys;
    nccols = MAX(lp->width / lp->xs - 2, HEX_MINGRIDSIZE);
    ncrows = MAX(lp->height / lp->ys - 1, HEX_MINGRIDSIZE);
    lp->ncols = nccols / 2;
    if (lp->wrap) {
        lp->nrows = 2 * (ncrows / 4);
    } else {
        lp->nrows = ncrows / 2;
	lp->nrows -= !(lp->nrows & 1);  /* Must be odd */
    }
    lp->xb = (lp->width - lp->xs * nccols) / 2 +
         ((lp->wrap) ? lp->xs / 2 : lp->xs);
    lp->yb = (lp->height - lp->ys * ((lp->wrap) ? (ncrows / 2) * 2 : ncrows)) /
         2 + lp->ys - 2 * lp->wrap;
    for (i = 0; i < 6; i++) {
      if (lp->vertical) {
        lp->shape.hexagon[i].x = (lp->xs - 1) * hexagonUnit[i].x;
        lp->shape.hexagon[i].y = ((lp->ys - 1) * hexagonUnit[i].y / 2) * 4 / 3;
      } else {
        lp->shape.hexagon[i].y = (lp->xs - 1) * hexagonUnit[i].x;
        lp->shape.hexagon[i].x = ((lp->ys - 1) * hexagonUnit[i].y / 2) * 4 / 3;
      }
    }
  } else {
		if (size < -MINSIZE)
			lp->ys = NRAND(MIN(-size, MAX(MINSIZE, MIN(lp->width, lp->height) /
					      MINGRIDSIZE)) - MINSIZE + 1) + MINSIZE;
		else if (size < MINSIZE) {
			if (!size)
				lp->ys = MAX(MINSIZE, MIN(lp->width, lp->height) / MINGRIDSIZE);
			else
				lp->ys = MINSIZE;
		} else
			lp->ys = MIN(size, MAX(MINSIZE, MIN(lp->width, lp->height) /
					       MINGRIDSIZE));
		lp->xs = lp->ys;
		lp->ncols = MAX(lp->width / lp->xs, sqAdamLoopX() + 1);
		lp->nrows = MAX(lp->height / lp->ys, sqAdamLoopY() + 1);
		lp->xb = (lp->width - lp->xs * lp->ncols) / 2;
		lp->yb = (lp->height - lp->ys * lp->nrows) / 2;
	}
	lp->bx = !lp->wrap;
	lp->by = !lp->wrap;
	lp->bncols = lp->ncols + 2 * lp->bx;
	lp->bnrows = lp->nrows + 2 * lp->by;

	MI_CLEARWINDOW(mi);

	if ((lp->oldcells = (unsigned char *) calloc(lp->bncols * lp->bnrows,
			sizeof (unsigned char))) == NULL) {
		free_loop_screen(display, lp);
		return;
	}
	if ((lp->newcells = (unsigned char *) calloc(lp->bncols * lp->bnrows,
			sizeof (unsigned char))) == NULL) {
		free_loop_screen(display, lp);
		return;
	}
	if (!init_table(mi)) {
		release_loop(mi);
		return;
	}
	lp->mincol = lp->bncols - 1;
	lp->minrow = lp->bnrows - 1;
	lp->maxcol = 0;
	lp->maxrow = 0;
#ifndef DELAYDEBUGLOOP
	{
		int flaws = MI_COUNT(mi);

		if (flaws < 0)
			flaws = NRAND(-MI_COUNT(mi) + 1);
		for (i = 0; i < flaws; i++) {
			init_flaw(mi);
		}
		/* actual flaws might be less since the adam loop done next */
	}
#endif
	init_adam(mi);
}

ENTRYPOINT void
draw_loop(ModeInfo * mi)
{
	int         offset, i, j, factor = 1;
	unsigned char *z, *znew;
	loopstruct *lp;

	if (loops == NULL)
		return;
	lp = &loops[MI_SCREEN(mi)];
	if (lp->newcells == NULL)
		return;

	MI_IS_DRAWN(mi) = True;
	lp->dead = True;
#ifdef DELAYDEBUGLOOP
	if (MI_COUNT(mi) && lp->generation > MI_COUNT(mi)) {
		(void) sleep(DELAYDEBUGLOOP);
	}
#endif
	for (j = lp->minrow; j <= lp->maxrow; j++) {
		for (i = lp->mincol; i <= lp->maxcol; i++) {
			offset = j * lp->bncols + i;
			z = lp->oldcells + offset;
			znew = lp->newcells + offset;
			if (*z != *znew) {
				lp->dead = False;
				*z = *znew;
				if (!addtolist(mi, i - lp->bx, j - lp->by, *znew))
					return;
				if (i == lp->mincol && i > lp->bx)
					lp->mincol--;
				if (j == lp->minrow && j > lp->by)
					lp->minrow--;
				if (i == lp->maxcol && i < lp->bncols - 2 * lp->bx)
					lp->maxcol++;
				if (j == lp->maxrow && j < lp->bnrows - 2 * lp->by)
					lp->maxrow++;
				if (lp->wrap) {
					if (i <= 1 || i >= lp->bncols - 2) {
						lp->maxcol = lp->bncols - 1;
						lp->mincol = 0;
					}
					if (j <= 1 || j >= lp->bnrows - 2) {
						lp->maxrow = lp->bnrows - 1;
						lp->minrow = 0;
					}
				}
			}
		}
	}
	for (i = 0; i < EXT_STATES; i++)
		if (!draw_state(mi, i)) {
			free_loop_screen(MI_DISPLAY(mi), lp);
			return;
		}
	if (byl | chou1 | chou2)
		factor = 2; /* kill earlier */
	if (++lp->generation * factor > MI_CYCLES(mi) || lp->dead) {
		init_loop(mi);
		return;
	} else
		do_gen(lp);

	if (lp->redrawing) {
		for (i = 0; i < REDRAWSTEP; i++) {
			if ((*(lp->oldcells + lp->redrawpos))) {
				drawcell(mi, lp->redrawpos % lp->bncols - lp->bx,
					 lp->redrawpos / lp->bncols - lp->by,
					 *(lp->oldcells + lp->redrawpos));
			}
			if (++(lp->redrawpos) >= lp->bncols * (lp->bnrows - lp->bx)) {
				lp->redrawing = 0;
				break;
			}
		}
	}
}

#ifndef STANDALONE
ENTRYPOINT void
refresh_loop(ModeInfo * mi)
{
	loopstruct *lp;

	if (loops == NULL)
		return;
	lp = &loops[MI_SCREEN(mi)];

	MI_CLEARWINDOW(mi);
	lp->redrawing = 1;
	lp->redrawpos = lp->by * lp->ncols + lp->bx;
}
#endif

XSCREENSAVER_MODULE ("Loop", loop)

#endif /* MODE_loop */
