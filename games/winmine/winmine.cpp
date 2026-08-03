// #define DEBUG

#include <fx.h>
#include <FXPNGIcon.h>
#include <time.h>
#include <limits.h>
#include <ice2k/comctl32.h>
#include <ice2k/branding.h>

#include "res/foxres.h"

FXIcon* ico_main_16;
FXIcon* ico_main_32;
FXImage* img_seg;
FXImage* img_smil;
FXImage* img_mine;

FXImage* img_coolseg;
FXImage* img_coolsmil;
FXImage* img_coolmine;

FXImage* img_monoseg;
FXImage* img_monosmil;
FXImage* img_monomine;

FXMainWindow* mainwin;



//FXIcon* ico_two;

#define MINEBTN_NORMAL          (0*16)
#define MINEBTN_FLAG            (1*16)
#define MINEBTN_UNKNOWN         (2*16)
#define MINEBTN_MINE_PRESSED    (3*16)
#define MINEBTN_NOTMINE         (4*16)
#define MINEBTN_MINE            (5*16)
#define MINEBTN_UNKNOWN_PRESSED (6*16)
#define MINEBTN_EIGHT           (7*16)
#define MINEBTN_SEVEN           (8*16)
#define MINEBTN_SIX             (9*16)
#define MINEBTN_FIVE           (10*16)
#define MINEBTN_FOUR           (11*16)
#define MINEBTN_THREE          (12*16)
#define MINEBTN_TWO            (13*16)
#define MINEBTN_ONE            (14*16)
#define MINEBTN_NORMAL_PRESSED (15*16)

#define SMILEBTN_PRESSED        (0*24)
#define SMILEBTN_WIN            (1*24)
#define SMILEBTN_LOST           (2*24)
#define SMILEBTN_WATCHOUT       (3*24)
#define SMILEBTN_NORMAL         (4*24)

int smiley = SMILEBTN_NORMAL;

//typedef unsigned char CELL;
typedef struct {
	unsigned char todo:1;
	unsigned char shown:1;
	unsigned char state:2;
	unsigned char neigh:4;
} CELL;

#define NOFLAG 0
#define FLAG 1
#define UNKNOWN 2
//#define PRESSEDMINE 3
#define INVALIDFLAG 3

#define ISMINE 15


#define MAXWIDTH  30
#define MAXHEIGHT 24
CELL board[MAXWIDTH*MAXHEIGHT];

#define BOARDX 12
#define BOARDY 55

#define DIFF_BEGINNER 0
#define DIFF_INTERMEDIATE 1
#define DIFF_EXPERT 2
#define DIFF_CUSTOM 3


#define ROWS_BEGINNER 9
#define COLS_BEGINNER 9
#define MINES_BEGINNER 10

#define ROWS_INTERMEDIATE 16
#define COLS_INTERMEDIATE 16
#define MINES_INTERMEDIATE 40

#define ROWS_EXPERT 16
#define COLS_EXPERT 30
#define MINES_EXPERT 99


int boardw;
int boardh;
int boardm = MINES_BEGINNER;
int minesover = boardm;
int notmines = 0;

int seconds = 0;
int customaccept = 0;

struct GAMEFLAGS {
	unsigned char minesplaced:1;
	unsigned char color:1;
	unsigned char marks:1;
	unsigned char disfield:1;
	unsigned char lost:1;
	unsigned char won:1;
	unsigned char difficulty:2;
};

//struct GAMEFLAGS gf = {FALSE, TRUE, TRUE, FALSE, FALSE, FALSE};
struct GAMEFLAGS gf = {FALSE, TRUE, TRUE, FALSE, FALSE, FALSE};


class AboutBox: public FXDialogBox {
	FXDECLARE(AboutBox);
protected:
	AboutBox() {}
public:
	AboutBox(FXWindow* owner);
	void setFocus() {};
	//virtual void create();
	virtual ~AboutBox() {};
};

FXIMPLEMENT(AboutBox, FXDialogBox, NULL, 0);
// about box code comes from uhexe
// hexeditor i used to work on
AboutBox::AboutBox(FXWindow* owner): FXDialogBox(owner,"About Minesweeper",DECOR_TITLE|DECOR_BORDER|DECOR_CLOSE|DECOR_MENU, 0,0,0,0,10,10,10,10,8,12) {
	FXHorizontalFrame* cont = new FXHorizontalFrame(this, LAYOUT_SIDE_TOP, 0,0,0,0, 4,4,4,4, 10,10);

	new FXLabel(cont, "", ico_main_32);

	new FXLabel(cont,
	"Minesweeper 2.0.0\n"
	"Clear the board, avoid mines and win.\n"
	"\n"
	"Written by xcomposite\n"
	"\n"
	"Thanks to Curt Johnson and Robert Donner\n"
	"for the original game!",
	NULL, JUSTIFY_LEFT|LAYOUT_FILL_X);

	FXButton* okbtn = new FXButton(this, "OK", NULL, this, ID_ACCEPT,
	BUTTON_DEFAULT|BUTTON_INITIAL|LAYOUT_RIGHT|FRAME_THICK|FRAME_RAISED|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT,
	0,0,75,23, 3,3,2,3);

	okbtn->setFocus();
}


//FXbool minesplaced = FALSE;
//FXbool color = TRUE;

//FXbool disfield = FALSE;
//FXbool lost = FALSE;

/*int checkRect(FXEvent* ev, int x, int y, int w, int h) {
	if ( (x < ev->rect.x + ev->rect.w) &&
	     (x +          w > ev->rect.x) &&
		 (y < ev->rect.y + ev->rect.h) &&
		 (y +          h > ev->rect.y) ) {
		return 1;
	}

	return 0;
}*/

#define CHECKRECT(ev, rx, ry, rw, rh) \
	( ((rx) < (ev)->rect.x + (ev)->rect.w) && \
	  ((rx) +         (rw) > (ev)->rect.x) && \
	  ((ry) < (ev)->rect.y + (ev)->rect.h) && \
	  ((ry) +         (rh) > (ev)->rect.y) )

#define CHECKMOUSE(ev, rx, ry, rw, rh) \
	( ((rx) <= (ev)->win_x) && \
	  ((ry) <= (ev)->win_y) && \
	  ((ev)->win_x < (rx) + (rw)) && \
	   (ev)->win_y < (ry)+(rh) )

void initBoard(int w, int h) {
	boardw = w;
	boardh = h;

	if (boardw > MAXWIDTH)  boardw = MAXWIDTH;
	if (boardh > MAXHEIGHT) boardh = MAXHEIGHT;
	
	memset(board, 0x00, (boardw*boardh)*sizeof(board[0]));

	srand(time(NULL));

	gf.minesplaced = 0;
	gf.disfield = 0;
	gf.won = 0;
	gf.lost = 0;
	seconds = 0;

	minesover = boardm;	
}



void placeMines(int m, int mx, int my) {
	notmines = boardw * boardh;

	boardm = m;

	for (int i = 0; i < boardm;) {
		int x = rand() % boardw;
		int y = rand() % boardh;
#ifdef DEBUG
		printf("x: %d, y: %d\n", x, y);
#endif
		int cell = y*boardw+x;
		int mcell = my*boardw+mx;
		//printf("cell: %d, mcell: %d\n", cell, mcell);
		
		if (cell != mcell && board[cell].neigh != ISMINE) {
			board[cell].neigh = ISMINE;
			--notmines;
			//printf("notmines: %d\n", notmines);
			++i;
		}
	}

	for (int y = 0; y < boardh; ++y) {
		for (int x = 0; x < boardw; ++x) {
			int cell = y*boardw+x;
			if (board[cell].neigh == ISMINE) continue;

			for (int dy = -1; dy <= 1; ++dy) {
				for (int dx = -1; dx <= 1; ++dx) {
					int ny = y + dy;
					int nx = x + dx;

					if (ny >= 0 && ny < boardh && nx >= 0 && nx < boardw) {
						if (board[ny*boardw+nx].neigh == ISMINE) board[cell].neigh++;
					}
				}
			}
		}
	}

	gf.minesplaced = TRUE;
}


void printBoard() {
	for (int y = 0; y < boardh; ++y) {
		for (int x = 0; x < boardw; ++x) {
			if (board[y*boardw+x].neigh == ISMINE) putchar('*');
			else putchar('.');
		}

		putchar('\n');
	}
}

class HighScoreBox: public FXDialogBox {
	FXDECLARE(HighScoreBox);

	FXHorizontalFrame* cont;
	FXLabel* begScoreLbl;
	FXLabel* intScoreLbl;
	FXLabel* expScoreLbl;

	FXLabel* begNameLbl;
	FXLabel* intNameLbl;
	FXLabel* expNameLbl;

protected:
	HighScoreBox() {}

public:
	enum {
		ID_DIALOG = FXDialogBox::ID_LAST,
		ID_CLEAR,
	};

	HighScoreBox(FXWindow* owner);
	long clearScores(FXObject*, FXSelector, void*);

	virtual void create();
	void setFocus() {};
	virtual ~HighScoreBox();
};


FXDEFMAP(HighScoreBox) HighScoreBoxMap[] = {
	FXMAPFUNC(SEL_COMMAND, HighScoreBox::ID_CLEAR, HighScoreBox::clearScores),
};
FXIMPLEMENT(HighScoreBox, FXDialogBox, HighScoreBoxMap, ARRAYNUMBER(HighScoreBoxMap));

HighScoreBox::HighScoreBox(FXWindow* owner): FXDialogBox(owner, "Fastest Mine Sweepers", DECOR_TITLE|DECOR_BORDER|DECOR_CLOSE|DECOR_MENU, 0,0,0,0, 16,0,23,13, 10,18) {
	char timestr[24];

	int   beginnerScore = getApp()->reg().readIntEntry   ("Scores", "Beginner", 999);
	const char* beginnerName  = getApp()->reg().readStringEntry("Names",  "Beginner", "Anonymous");

	int   intermediateScore = getApp()->reg().readIntEntry   ("Scores", "Intermediate", 999);
	const char* intermediateName  = getApp()->reg().readStringEntry("Names",  "Intermediate", "Anonymous");

	int   expertScore = getApp()->reg().readIntEntry   ("Scores", "Expert", 999);
	const char* expertName  = getApp()->reg().readStringEntry("Names",  "Expert", "Anonymous");

	FXMatrix* scoregrid = new FXMatrix(this, 3, PACK_UNIFORM_HEIGHT, 0,0,0,0, 0,0,0,0, 0,3);
	new FXLabel(scoregrid, "Beginner:", NULL, JUSTIFY_LEFT, 0,0,0,0, 0,0,00,0);
	new FXLabel(scoregrid, "Intermediate:", NULL, JUSTIFY_LEFT, 0,0,0,0, 0,0,0,0);
	new FXLabel(scoregrid, "Expert:", NULL, JUSTIFY_LEFT|LAYOUT_FIX_WIDTH, 0,0,75,0, 00,0,0,0);

	snprintf(timestr, sizeof(timestr), "%d seconds", beginnerScore);
	begScoreLbl = new FXLabel(scoregrid, timestr, NULL, JUSTIFY_LEFT|LAYOUT_FIX_WIDTH, 0,0,82,0, 0,0,0,0);

	snprintf(timestr, sizeof(timestr), "%d seconds", intermediateScore);
	intScoreLbl = new FXLabel(scoregrid, timestr, NULL, JUSTIFY_LEFT, 0,0,0,0, 0,0,0,0);

	snprintf(timestr, sizeof(timestr), "%d seconds", expertScore);
	expScoreLbl = new FXLabel(scoregrid, timestr, NULL, JUSTIFY_LEFT, 0,0,0,0, 0,0,0,0);

	begNameLbl = new FXLabel(scoregrid, beginnerName, NULL, JUSTIFY_LEFT|LAYOUT_FIX_WIDTH, 0,0,82,0, 0,0,0,0);
	intNameLbl = new FXLabel(scoregrid, intermediateName, NULL, JUSTIFY_LEFT, 0,0,0,0, 0,0,0,0);
	expNameLbl = new FXLabel(scoregrid, expertName, NULL, JUSTIFY_LEFT, 0,0,0,0, 0,0,0,0);

	FXHorizontalFrame* btncont = new FXHorizontalFrame(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X, 0,0,0,0, 22,37,0,0, 0,0);

	new FXButton(btncont, "&Reset Scores", NULL, this, ID_CLEAR, LAYOUT_LEFT|BUTTON_NORMAL|BUTTON_INITIAL|BUTTON_DEFAULT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0,0,75,20, 0,0,0,0);
	FXButton* okbtn = new FXButton(btncont, "OK", NULL, this, ID_ACCEPT, LAYOUT_RIGHT|BUTTON_NORMAL|BUTTON_INITIAL|BUTTON_DEFAULT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0,0,45,20, 0,0,0,0);
	this->show();
	this->setFocus();
	okbtn->setFocus();
}

long HighScoreBox::clearScores(FXObject* sender, FXSelector sel, void* ptr) {
	getApp()->reg().writeStringEntry("Names", "Beginner", "Anonymous");
	getApp()->reg().writeIntEntry("Scores", "Beginner", 999);
	
	getApp()->reg().writeStringEntry("Names", "Intermediate", "Anonymous");
	getApp()->reg().writeIntEntry("Scores", "Intermediate", 999);

	getApp()->reg().writeStringEntry("Names", "Expert", "Anonymous");
	getApp()->reg().writeIntEntry("Scores", "Expert", 999);

	begNameLbl->setText("Anonymous");
	intNameLbl->setText("Anonymous");
	expNameLbl->setText("Anonymous");

	begScoreLbl->setText("999 seconds");
	intScoreLbl->setText("999 seconds");
	expScoreLbl->setText("999 seconds");

	return 1;
}


HighScoreBox::~HighScoreBox() {}
void HighScoreBox::create() { FXDialogBox::create(); }

class CustomGameBox: public FXDialogBox {
	FXDECLARE(CustomGameBox);

	FXHorizontalFrame* cont;
	FXLabel* begScoreLbl;
	FXLabel* intScoreLbl;
	FXLabel* expScoreLbl;

	FXLabel* begNameLbl;
	FXLabel* intNameLbl;
	FXLabel* expNameLbl;

	FXTextField* heighttfield;
	FXTextField* widthtfield;
	FXTextField* minestfield;


protected:
	CustomGameBox() {}

public:
	enum {
		ID_DIALOG = FXDialogBox::ID_LAST,
		ID_CLEAR,
	};

	CustomGameBox(FXWindow* owner);
	long onCmdAccept(FXObject*, FXSelector, void*);

	virtual void create();
	void setFocus() {};
	virtual ~CustomGameBox();
};


FXDEFMAP(CustomGameBox) CustomGameBoxMap[] = {
	FXMAPFUNC(SEL_COMMAND, CustomGameBox::ID_ACCEPT, CustomGameBox::onCmdAccept),
};


long CustomGameBox::onCmdAccept(FXObject* obj, FXSelector sel, void* ptr) {
	gf.difficulty = DIFF_CUSTOM;

	int nrows = atoi(heighttfield->getText().text());
	if (nrows < 9) nrows = 9;
	else if (nrows > 24) nrows = 24;

	int ncols = atoi(widthtfield->getText().text());
	if (ncols < 9) ncols = 9;
	else if (ncols > 30) ncols = 30;

	int totsize = ncols * nrows;

	int nmines = atoi(minestfield->getText().text());
	if (nmines < 1) nmines = 1;
	else if ( (nmines > totsize-1) ) nmines = totsize-1;

	boardw = ncols;
	boardh = nrows;
	boardm = nmines;

	customaccept = 1;

	FXDialogBox::onCmdAccept(obj, sel, ptr);

	return 1;
}


FXIMPLEMENT(CustomGameBox, FXDialogBox, CustomGameBoxMap, ARRAYNUMBER(CustomGameBoxMap));

CustomGameBox::CustomGameBox(FXWindow* owner): FXDialogBox(owner, "Custom Field", DECOR_TITLE|DECOR_BORDER|DECOR_CLOSE|DECOR_MENU, 0,0,0,0, 10,10,10,10, 20,10) {
	char fieldstr[4];

	FXMatrix* scoregrid = new FXMatrix(this, 3, LAYOUT_SIDE_LEFT|PACK_UNIFORM_HEIGHT, 0,0,0,0, 0,0,0,0, 0,2);
	new FXLabel(scoregrid, "Height:", NULL, JUSTIFY_LEFT, 0,0,0,0, 0,16,0,0);
	new FXLabel(scoregrid, "Width:", NULL, JUSTIFY_LEFT, 0,0,0,0,   0,16,0,0);
	new FXLabel(scoregrid, "Mines:", NULL, JUSTIFY_LEFT, 0,0,0,0,   0,16,0,0);


	heighttfield = new FXTextField(scoregrid, 5,this,ID_ACCEPT,TEXTFIELD_INTEGER|TEXTFIELD_LIMITED|TEXTFIELD_ENTER_ONLY|FRAME_SUNKEN|FRAME_THICK, 0,0,0,0, 1,1,1,1);
	snprintf(fieldstr, sizeof(fieldstr), "%d", boardw);
	heighttfield->setText(fieldstr);

	widthtfield = new FXTextField(scoregrid, 5,this,ID_ACCEPT,TEXTFIELD_INTEGER|TEXTFIELD_LIMITED|TEXTFIELD_ENTER_ONLY|FRAME_SUNKEN|FRAME_THICK, 0,0,0,0, 1,1,1,1);
	snprintf(fieldstr, sizeof(fieldstr), "%d", boardh);
	widthtfield->setText(fieldstr);

	minestfield = new FXTextField(scoregrid, 5,this,ID_ACCEPT,TEXTFIELD_INTEGER|TEXTFIELD_LIMITED|TEXTFIELD_ENTER_ONLY|FRAME_SUNKEN|FRAME_THICK, 0,0,0,0, 1,1,1,1);
	snprintf(fieldstr, sizeof(fieldstr), "%d", boardm);
	minestfield->setText(fieldstr);

	FXVerticalFrame* btncont = new FXVerticalFrame(this, LAYOUT_SIDE_RIGHT|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0, 0,0);

	new FXButton(btncont, "OK", NULL, this, ID_ACCEPT, LAYOUT_TOP|BUTTON_NORMAL|BUTTON_INITIAL|BUTTON_DEFAULT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0,0,60,23, 0,0,0,0);
	new FXButton(btncont, "Cancel", NULL, this, ID_CANCEL, LAYOUT_BOTTOM|BUTTON_NORMAL|BUTTON_DEFAULT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0,0,60,23, 0,0,0,0);
	this->show();
	//this->setFocus();
}

CustomGameBox::~CustomGameBox() {}
void CustomGameBox::create() { FXDialogBox::create(); heighttfield->setFocus(); }


class NewScoreBox: public FXDialogBox {
	FXDECLARE(NewScoreBox);

	FXHorizontalFrame* cont;
	FXTextField* textfield;
	char* name;

protected:
	NewScoreBox() {}
	

public:
	NewScoreBox(FXWindow* owner);
	long onCmdAccept(FXObject*, FXSelector, void*);

	virtual void create();
	void setFocus() {};
	virtual ~NewScoreBox();
};

long NewScoreBox::onCmdAccept(FXObject* obj, FXSelector sel, void* ptr) {
	name = strndup(textfield->getText().text(), 12);

	if (gf.difficulty == DIFF_BEGINNER) {
		getApp()->reg().writeStringEntry("Names", "Beginner", name);
		getApp()->reg().writeIntEntry("Scores", "Beginner", seconds);
	} else if (gf.difficulty == DIFF_INTERMEDIATE) {
		getApp()->reg().writeStringEntry("Names", "Intermediate", name);
		getApp()->reg().writeIntEntry("Scores", "Intermediate", seconds);
	} else {
		getApp()->reg().writeStringEntry("Names", "Expert", name);
		getApp()->reg().writeIntEntry("Scores", "Expert", seconds);
	}

	HighScoreBox highscorebox(mainwin);
	FXDialogBox::onCmdAccept(obj, sel, ptr);

	highscorebox.execute(PLACEMENT_OWNER);
	return 1;
}

FXDEFMAP(NewScoreBox) NewScoreBoxMap[] = {
	FXMAPFUNC(SEL_COMMAND, NewScoreBox::ID_ACCEPT, NewScoreBox::onCmdAccept),
};

FXIMPLEMENT(NewScoreBox, FXDialogBox, NewScoreBoxMap, ARRAYNUMBER(NewScoreBoxMap));

NewScoreBox::NewScoreBox(FXWindow* owner): FXDialogBox(owner, "Fastest Time", DECOR_BORDER, 0,0,0,0, 9,9,6,22, 0,0) {
	new FXLabel(this, "You have the fastest time", NULL, LAYOUT_CENTER_X, 0,0,0,0, 0,0,0,-1);
	if (gf.difficulty == DIFF_BEGINNER) {
		name = strdup(getApp()->reg().readStringEntry("Names", "Beginner", "Anonymous"));
		new FXLabel(this, "for beginner level.", NULL, LAYOUT_CENTER_X, 0,0,0,0, 0,0,0,-1);
	} else if (gf.difficulty == DIFF_INTERMEDIATE) {
		name = strdup(getApp()->reg().readStringEntry("Names", "Intermediate", "Anonymous"));
		new FXLabel(this, "for intermediate level.", NULL, LAYOUT_CENTER_X, 0,0,0,0, 0,0,0,-1);
	} else {
		name = strdup(getApp()->reg().readStringEntry("Names", "Expert", "Anonymous"));
		new FXLabel(this, "for expert level.", NULL, LAYOUT_CENTER_X, 0,0,0,0, 0,0,0,-1);
	}

	new FXLabel(this, "Please enter your name.", NULL, LAYOUT_CENTER_X, 0,0,0,0, 0,0,0,0);

	new FXSeparator(this, SEPARATOR_NONE|LAYOUT_FIX_HEIGHT|LAYOUT_FIX_WIDTH, 0,0,132,37);	
	textfield = new FXTextField(this, 18,this,ID_ACCEPT,TEXTFIELD_ENTER_ONLY|LAYOUT_CENTER_X|FRAME_SUNKEN|FRAME_THICK, 0,0,0,0, 1,1,1,1);
	textfield->setText(name);
	textfield->selectAll();


	new FXSeparator(this, SEPARATOR_NONE|LAYOUT_FIX_HEIGHT|LAYOUT_FIX_WIDTH, 0,0,132,12);
	new FXButton(this, "OK", NULL, this, ID_ACCEPT, LAYOUT_CENTER_X|BUTTON_NORMAL|BUTTON_DEFAULT|BUTTON_INITIAL|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0,0,54,26, 0,0,0,0);
}


NewScoreBox::~NewScoreBox() {}
void NewScoreBox::create() { FXDialogBox::create(); textfield->setFocus(); }





class Minesweeper : public FXMainWindow {
	FXDECLARE(Minesweeper);

protected:
	Minesweeper() {}

private:
	//int seconds;

	FXMenuBar* menubar;
	FXHorizontalFrame *contents;
	FXCanvas* mscanvas;

	FXMenuRadio* begradio;
	FXMenuRadio* intradio;
	FXMenuRadio* expradio;
	FXMenuRadio* cusradio;

	FXMenuPane* gamemenu;
	FXMenuPane* helpmenu;

public:
	long onPaint(FXObject*, FXSelector, void*);
	
	long onLeftButtonPress(FXObject*, FXSelector, void*);
	long onLeftButtonPressSmiley(FXObject*, FXSelector, void*);
	long onLeftButtonPressField(FXObject*, FXSelector, void*);

	long onLeftButtonRelease(FXObject*, FXSelector, void*);
	long onLeftButtonReleaseSmiley(FXObject*, FXSelector, void*);
	long onLeftButtonReleaseField(FXObject*, FXSelector, void*);

	long onMotion(FXObject*, FXSelector, void*);
	long onMotionField(FXObject*, FXSelector, void*);
	long onMotionSmiley(FXObject*, FXSelector, void*);

	long onRightButtonPress(FXObject*, FXSelector, void*);
	long onRightButtonPressField(FXObject*, FXSelector, void*);
	long onTimeout(FXObject*, FXSelector, void*);

	long onCmdColor(FXObject*, FXSelector, void*);
	long onUpdColor(FXObject*, FXSelector, void*);
	long onCmdMarks(FXObject*, FXSelector, void*);
	long onUpdMarks(FXObject*, FXSelector, void*);
	long onCmdAbout(FXObject*, FXSelector, void*);

	long displayBestScores(FXObject*, FXSelector, void*);
	long resetBoard(FXObject*, FXSelector, void*);

	void revealCell(int, int, int);

	void paintBoard(FXWindow*, FXEvent*);

	void paintBoardBorders(FXWindow*, FXEvent*);
	void paintBorders(FXWindow* win, FXEvent* ev);
	void drawNumber(FXWindow* win, FXEvent* ev, int x, int y, int num2);
	void drawSmile(FXWindow* win, FXEvent* ev);
	void updateSmiley();

	void changeDifficulty(int w, int h, int m);
	long changeDifficultyBeginner(FXObject*, FXSelector, void*);
	long changeDifficultyIntermediate(FXObject*, FXSelector, void*);
	long changeDifficultyExpert(FXObject*, FXSelector, void*);
	long changeDifficultyCustom(FXObject*, FXSelector, void*);
	//void setColorMode(int);

	void paintTopBorders(FXWindow* win, FXEvent* ev);

public:
	enum {
		ID_CANVAS = FXMainWindow::ID_LAST,
		ID_TIMEOUT,
		ID_COLOR,
		ID_MARKS,
		ID_BESTSCORE,
		ID_ABOUT,
		ID_NEW,

		ID_DIFFICULTY_BEGINNER,
		ID_DIFFICULTY_INTERMEDIATE,
		ID_DIFFICULTY_EXPERT,
		ID_DIFFICULTY_CUSTOM,

		ID_LAST
	};

public:
	Minesweeper(FXApp* a);

	virtual void create();
	void setFocus() {};
	virtual ~Minesweeper();
};

FXDEFMAP(Minesweeper) MinesweeperMap[] = {
	FXMAPFUNC(SEL_PAINT,             Minesweeper::ID_CANVAS,  Minesweeper::onPaint),
	FXMAPFUNC(SEL_LEFTBUTTONPRESS,   Minesweeper::ID_CANVAS,  Minesweeper::onLeftButtonPress),
	FXMAPFUNC(SEL_RIGHTBUTTONPRESS,  Minesweeper::ID_CANVAS,  Minesweeper::onRightButtonPress),

	FXMAPFUNC(SEL_LEFTBUTTONRELEASE, Minesweeper::ID_CANVAS,  Minesweeper::onLeftButtonRelease),

	FXMAPFUNC(SEL_MOTION,          Minesweeper::ID_CANVAS,  Minesweeper::onMotion),
	
	FXMAPFUNC(SEL_TIMEOUT,         Minesweeper::ID_TIMEOUT, Minesweeper::onTimeout),

	FXMAPFUNC(SEL_COMMAND, Minesweeper::ID_BESTSCORE, Minesweeper::displayBestScores),
	FXMAPFUNC(SEL_COMMAND, Minesweeper::ID_NEW, Minesweeper::resetBoard),

	FXMAPFUNC(SEL_COMMAND,         Minesweeper::ID_COLOR, Minesweeper::onCmdColor),
	FXMAPFUNC(SEL_UPDATE,          Minesweeper::ID_COLOR, Minesweeper::onUpdColor),

	FXMAPFUNC(SEL_COMMAND, Minesweeper::ID_DIFFICULTY_BEGINNER, Minesweeper::changeDifficultyBeginner),
	FXMAPFUNC(SEL_COMMAND, Minesweeper::ID_DIFFICULTY_INTERMEDIATE, Minesweeper::changeDifficultyIntermediate),
	FXMAPFUNC(SEL_COMMAND, Minesweeper::ID_DIFFICULTY_EXPERT, Minesweeper::changeDifficultyExpert),
	FXMAPFUNC(SEL_COMMAND, Minesweeper::ID_DIFFICULTY_CUSTOM, Minesweeper::changeDifficultyCustom),

	FXMAPFUNC(SEL_COMMAND,         Minesweeper::ID_MARKS, Minesweeper::onCmdMarks),
	FXMAPFUNC(SEL_UPDATE,          Minesweeper::ID_MARKS, Minesweeper::onUpdMarks),

	FXMAPFUNC(SEL_COMMAND, Minesweeper::ID_ABOUT, Minesweeper::onCmdAbout),
};


FXIMPLEMENT(Minesweeper, FXMainWindow, MinesweeperMap, ARRAYNUMBER(MinesweeperMap));

void Minesweeper::changeDifficulty(int w, int h, int m) {
	boardw = w;
	boardh = h;
	boardm = m;
	
	resetBoard(NULL, 0, NULL);

	mscanvas->setHeight((boardh<<4)+63);
	mscanvas->setWidth((boardw<<4)+20);
	mscanvas->update();

	mainwin->recalc();
	mainwin->resize(mainwin->getDefaultWidth(), mainwin->getDefaultHeight());
}





long Minesweeper::changeDifficultyBeginner(FXObject* obj, FXSelector sel, void* ptr) {
	changeDifficulty(COLS_BEGINNER, ROWS_BEGINNER, MINES_BEGINNER);
	gf.difficulty = DIFF_BEGINNER;

	begradio->setCheck(TRUE);
	intradio->setCheck(FALSE);
	expradio->setCheck(FALSE);
	cusradio->setCheck(FALSE);

	return 1;
}

long Minesweeper::changeDifficultyIntermediate(FXObject* obj, FXSelector sel, void* ptr) {
	changeDifficulty(COLS_INTERMEDIATE, ROWS_INTERMEDIATE, MINES_INTERMEDIATE);
	gf.difficulty = DIFF_INTERMEDIATE;

	begradio->setCheck(FALSE);
	intradio->setCheck(TRUE);
	expradio->setCheck(FALSE);
	cusradio->setCheck(FALSE);

	return 1;
}


long Minesweeper::changeDifficultyExpert(FXObject* obj, FXSelector sel, void* ptr) {
	changeDifficulty(COLS_EXPERT, ROWS_EXPERT, MINES_EXPERT);
	gf.difficulty = DIFF_EXPERT;

	begradio->setCheck(FALSE);
	intradio->setCheck(FALSE);
	expradio->setCheck(TRUE);
	cusradio->setCheck(FALSE);

	return 1;
}

long Minesweeper::changeDifficultyCustom(FXObject* obj, FXSelector sel, void* ptr) {
	cusradio->setCheck(FALSE);

	CustomGameBox customgamebox(mainwin);
	customgamebox.execute(PLACEMENT_OWNER);

	if (customaccept) {
		customaccept = 0;
		
		changeDifficulty(boardh, boardw, boardm);

		begradio->setCheck(FALSE);
		intradio->setCheck(FALSE);
		expradio->setCheck(FALSE);
		cusradio->setCheck(TRUE);
	}

	return 1;
}




long Minesweeper::onCmdColor(FXObject* obj, FXSelector sel, void* ptr) {
	if (!gf.color) {
		gf.color = 1;
		
		img_seg  = img_coolseg;
		img_smil = img_coolsmil;
		img_mine = img_coolmine;
	} else {
		gf.color = 0;
		
		img_seg  = img_monoseg;
		img_smil = img_monosmil;
		img_mine = img_monomine;
	}

	mscanvas->update();

	return 1;
}


long Minesweeper::onUpdColor(FXObject* obj, FXSelector sel, void* ptr) {
	obj->handle(this, FXSEL(SEL_COMMAND,ID_ENABLE), NULL);
	obj->handle(this, FXSEL(SEL_COMMAND,ID_SHOW), NULL);
	obj->handle(this, FXSEL(SEL_COMMAND,ID_SETVALUE), (void*)(FXuval)gf.color);
	//puts("Hi");

	return 1;
}


long Minesweeper::onCmdMarks(FXObject* obj, FXSelector sel, void* ptr) {
	gf.marks = !gf.marks;
	return 1;
}


long Minesweeper::onUpdMarks(FXObject* obj, FXSelector sel, void* ptr) {
	obj->handle(this, FXSEL(SEL_COMMAND,ID_ENABLE), NULL);
	obj->handle(this, FXSEL(SEL_COMMAND,ID_SHOW), NULL);
	obj->handle(this, FXSEL(SEL_COMMAND,ID_SETVALUE), (void*)(FXuval)gf.marks);
	
	return 1;
}


Minesweeper::Minesweeper(FXApp *a) : FXMainWindow(a, "Minesweeper", ico_main_16, NULL, DECOR_MENU|DECOR_BORDER|DECOR_MINIMIZE|DECOR_CLOSE|DECOR_TITLE, 0,0,0,0) {
	menubar = new FXMenuBar(this, this, LAYOUT_SIDE_TOP|LAYOUT_FILL_X, 0,0,0,0, 0,0,0,1, 0,0);
	gamemenu = new FXMenuPane(this);
	helpmenu = new FXMenuPane(this);
	new FXMenuTitle(menubar,"&Game", NULL, gamemenu);
	new FXMenuTitle(menubar,"&Help", NULL, helpmenu);

	//new FXMenuRadio(gamemenu, "&Beginner");

	new FXMenuCommand(gamemenu, "&New\tF2",NULL,this,ID_NEW);
	new FXMenuSeparator(gamemenu);
	begradio = new FXMenuRadio(gamemenu, "&Beginner",this,ID_DIFFICULTY_BEGINNER);
	begradio->setCheck(TRUE);

	intradio = new FXMenuRadio(gamemenu, "&Intermediate",this,ID_DIFFICULTY_INTERMEDIATE);
	expradio = new FXMenuRadio(gamemenu, "&Expert",this,ID_DIFFICULTY_EXPERT);
	cusradio = new FXMenuRadio(gamemenu, "&Custom...",this,ID_DIFFICULTY_CUSTOM);
	new FXMenuSeparator(gamemenu);
	
	new FXMenuCheck(gamemenu, "&Color", this, ID_COLOR);
	new FXMenuCheck(gamemenu, "&Marks (?)", this, ID_MARKS);
	new FXMenuSeparator(gamemenu);
	new FXMenuCommand(gamemenu, "Best &Times", NULL, this, ID_BESTSCORE);
	new FXMenuSeparator(gamemenu);
	new FXMenuCommand(gamemenu,"E&xit",NULL,getApp(),FXApp::ID_QUIT);

	new FXMenuCommand(helpmenu, "&About Minesweeper...",NULL,this,ID_ABOUT);


	mscanvas = new FXCanvas(this, this, ID_CANVAS, LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT);
	/*mscanvas->setWidth(512);
	mscanvas->setHeight(512);*/
	mscanvas->setHeight((boardh<<4)+63);
	mscanvas->setWidth((boardw<<4)+20);
	
	//new FXLabel(this, "test", ico_two);
}

Minesweeper::~Minesweeper() {
	delete gamemenu;
	delete helpmenu;
}


void Minesweeper::create() {
	FXMainWindow::create();

	show(PLACEMENT_SCREEN);
}

unsigned pressedx = UINT_MAX;
unsigned pressedy = UINT_MAX;

int pressed = 0;
#if 0
void Minesweeper::revealCell(int col, int row, int click) {
	if (row < 0 || row >= boardh || col < 0 || col >= boardw) return;

	CELL* cell = &board[row*boardw+col];
	if (cell->shown || cell->state == FLAG) return;

	if (cell->neigh == ISMINE) {
		if (click) {
			for (int r = 0; r < boardh; ++r) {
				for (int c = 0; c < boardw; ++c) {
					if (board[r*boardw+c].neigh == ISMINE && board[r*boardw+c].state != FLAG) {
						board[r*boardw+c].shown = TRUE;
						mscanvas->update(BOARDX+(c<<4), BOARDY+(r<<4), 16, 16);
					}

					if (board[r*boardw+c].neigh != ISMINE && board[r*boardw+c].state == FLAG) {
						board[r*boardw+c].state = INVALIDFLAG;
						mscanvas->update(BOARDX+(c<<4), BOARDY+(r<<4), 16, 16);
					}
				}
			}

			disfield = TRUE;
			lost = TRUE;
			//board[row*boardw+col].state = PRESSEDMINE;
		}

		return;
	}
	cell->shown = TRUE;
	mscanvas->update(BOARDX+(col*16), BOARDY+(row*16), 16, 16);

	if (cell->neigh == 0) {
		for (int dr = -1; dr <= 1; ++dr) {
			for (int dc = -1; dc <= 1; ++dc) {
				if (dr != 0 || dc != 0) {
					revealCell(col+dc, row + dr, FALSE);
				}
			}
		}
	}

	//mscanvas->update();
}
#endif

long Minesweeper::displayBestScores(FXObject* obj, FXSelector sel, void* ptr) {
	HighScoreBox highscorebox(this);
	return highscorebox.execute(PLACEMENT_OWNER);
}


void Minesweeper::revealCell(int col, int row, int click) {
	int dr, dc, r, c;
	int col2, row2;

	CELL* cell1;
	CELL* cell2;
	CELL* celln;

	col2 = col;
	row2 = row;

	cell1 = &board[row*boardw+col];

	if (row2 < 0 || row2 >= boardh || col2 < 0 || col2 >= boardw) return;
	if (cell1->shown || cell1->state == FLAG) return;

	if (cell1->neigh == ISMINE) {
		getApp()->removeTimeout(this, ID_TIMEOUT);

		for (int r = 0; r < boardh; ++r) {
			for (int c = 0; c < boardw; ++c) {
				if (board[r*boardw+c].neigh == ISMINE && board[r*boardw+c].state != FLAG) {
					board[r*boardw+c].shown = TRUE;
					mscanvas->update(BOARDX+(c<<4), BOARDY+(r<<4), 16, 16);
				}

				if (board[r*boardw+c].neigh != ISMINE && board[r*boardw+c].state == FLAG) {
					board[r*boardw+c].state = INVALIDFLAG;
					mscanvas->update(BOARDX+(c<<4), BOARDY+(r<<4), 16, 16);
				}
			}
		}

		gf.disfield = TRUE;
		gf.lost = TRUE;

		return;
	}

	cell1->todo = 1;

	int check = 1;
	while (check) {
		check = 0;

		for (r = 0; r < boardh; ++r) {
			for (c = 0; c < boardw; ++c) {
				cell2 = &board[r*boardw+c];
				if (!cell2->todo) continue;
				
				cell2->todo = 0;

				if (!cell2->shown) {
					cell2->shown = 1;
					--notmines;
			//printf("notmines: %d\n", notmines);
					mscanvas->update(BOARDX+(c<<4), BOARDY+(r<<4), 16, 16);
				}

				if (cell2->neigh != 0) continue;

				for (dr = -1; dr <= 1; ++dr) {
					for (dc = -1; dc <= 1; ++dc) {
						int nr = r + dr;
						int nc = c+dc;

						if (nr >= 0 && nr < boardh && nc >= 0 && nc < boardw) {
							celln = &board[nr*boardw+nc];

							if (!celln->shown && celln->state != FLAG && celln->neigh != ISMINE) {
								celln->todo = 1;
								check = 1;
							}
						}
					}
				}

			}
		}
	}

	if (!gf.won && notmines == 0) {
		getApp()->removeTimeout(this, ID_TIMEOUT);

		for (int r = 0; r < boardh; ++r) {
			for (int c = 0; c < boardw; ++c) {
				if (board[r*boardw+c].neigh == ISMINE) {
					board[r*boardw+c].state = FLAG;
					mscanvas->update(BOARDX+(c<<4), BOARDY+(r<<4), 16, 16);
				}
			}
		}


		smiley = SMILEBTN_WIN;
		updateSmiley();

		gf.won = 1;
		gf.disfield = 1;

		if (gf.difficulty != DIFF_CUSTOM) {
			//newscore = timedisp->getValue();
			int oldscore = 999;

			if (gf.difficulty == DIFF_BEGINNER) {
				oldscore = getApp()->reg().readIntEntry("Scores", "Beginner", 999);
			} else if (gf.difficulty == DIFF_INTERMEDIATE) {
				oldscore = getApp()->reg().readIntEntry("Scores", "Intermediate", 999);
			} else if (gf.difficulty == DIFF_EXPERT) {
				oldscore = getApp()->reg().readIntEntry("Scores", "Expert", 999);
			}

			if (oldscore > seconds) {
				NewScoreBox newscorebox(this);
				newscorebox.create();

				newscorebox.execute(PLACEMENT_OWNER);
			}
		}
	}

}


#if 0
void Minesweeper::revealCell(int col, int row, int click) {
	int dr, dc, r, c;
	int col2, row2;

	CELL* cell;

	row2 = row;
	col2 = col;
	
start:
	if (row2 < 0 || row2 >= boardh || col2 < 0 || col2 >= boardw) return;

	cell = &board[row2*boardw+col2];
	if (cell->neigh != 0) return;

	for (dr = -1; dr <= 1; ++dr) {
		for (dc = -1; dc <= 1; ++dc) {
			if (!(dr != 0 || dc != 0)) return;
			if (cell->shown || cell->state == FLAG) return;

			if (!cell->mine) {
				cell->shown = TRUE;
				mscanvas->update(BOARDX+(col2*16), BOARDY+(row2*16), 16, 16);
			}
			
			if (!click) return;

			if (cell->neigh != 0) return;

			for (r = 0; r < boardh; ++r) {
				for (c = 0; c < boardw; ++c) {
					if (board[r*boardw+c].mine && board[r*boardw+c].state != FLAG) {
						board[r*boardw+c].shown = TRUE;
						mscanvas->update(BOARDX+(c<<4), BOARDY+(r<<4), 16, 16);
					}

					if (!board[r*boardw+c].mine && board[r*boardw+c].state == FLAG) {
						board[r*boardw+c].state = INVALIDFLAG;
						mscanvas->update(BOARDX+(c<<4), BOARDY+(r<<4), 16, 16);
					}
				}
			}
			disfield = TRUE;
			lost = TRUE;
			
		}
	}
}

#endif

long Minesweeper::onLeftButtonPressSmiley(FXObject* obj, FXSelector sel, void* ptr) {
	FXEvent* ev = (FXEvent*)ptr;

	if (CHECKMOUSE(ev, BOARDX+(boardw<<3)-12,15, 26, 26)) {
		smiley = SMILEBTN_PRESSED;
		updateSmiley();
	}

	return 1;
}


long Minesweeper::resetBoard(FXObject* obj, FXSelector sel, void* ptr) {
	initBoard(boardw, boardh);
	smiley = SMILEBTN_NORMAL;
	mscanvas->update();
	gf.minesplaced = 0;
	gf.lost = 0;
	gf.disfield = 0;

	pressedx = UINT_MAX;
	pressedy = UINT_MAX;

	getApp()->removeTimeout(this, ID_TIMEOUT);
	seconds = 0;

	return 1;
}


long Minesweeper::onLeftButtonPressField(FXObject* obj, FXSelector sel, void* ptr) {
	if (gf.disfield) return 1;
	FXEvent* ev = (FXEvent*)ptr;
	unsigned mousex = ev->win_x - BOARDX;
	unsigned mousey = ev->win_y - BOARDY;

	//unsigned oldpressedx = pressedx;
	//unsigned oldpressedy = pressedy;

	unsigned minex = mousex >> 4;
	unsigned miney = mousey >> 4;
	if (gf.lost) smiley = SMILEBTN_LOST;
	else if (gf.won) smiley = SMILEBTN_WIN;
	else smiley = SMILEBTN_WATCHOUT;
	updateSmiley();

	if (miney < 0 || (int)miney >= boardh || minex < 0 || (int)minex >= boardw) return 1;
	if (board[miney*boardw+minex].state == FLAG) return 1;

	pressedx = minex;
	pressedy = miney;

	mscanvas->update(BOARDX+(pressedx<<4), BOARDY+(pressedy<<4), 16, 16);

	return 1;
}


long Minesweeper::onLeftButtonPress(FXObject* obj, FXSelector sel, void* ptr) {
	onLeftButtonPressField(obj, sel, ptr);
	onLeftButtonPressSmiley(obj, sel, ptr);
	pressed = 1;

	return 1;
}

long Minesweeper::onRightButtonPressField(FXObject* obj, FXSelector sel, void* ptr) {
	if (gf.disfield) return 1;
	FXEvent* ev = (FXEvent*)ptr;
	unsigned mousex = ev->win_x - BOARDX;
	unsigned mousey = ev->win_y - BOARDY;
	
	unsigned rightx = mousex >> 4;
	unsigned righty = mousey >> 4;

	if (righty < 0 || (int)righty >= boardh || rightx < 0 || (int)rightx >= boardw) return 1;
	if (board[righty*boardw+rightx].shown) return 1;
	//if (board[righty*boardw+rightx].state == FLAG) return 1;


	switch (++(board[righty*boardw+rightx].state)) {
		case FLAG:
			--minesover;
			mscanvas->update(16+1, 15+1, 13*3, 23);
			break;
		case UNKNOWN:
			++minesover;
			mscanvas->update(16+1, 15+1, 13*3, 23);
			if (!gf.marks) board[righty*boardw+rightx].state = NOFLAG;
			break;
		default:
			board[righty*boardw+rightx].state = NOFLAG;
	}

	mscanvas->update(BOARDX+(rightx<<4), BOARDY+(righty<<4), 16, 16);
	return 1;
}

long Minesweeper::onRightButtonPress(FXObject* obj, FXSelector sel, void* ptr) {
	onRightButtonPressField(obj, sel, ptr);
	return 1;
}

long Minesweeper::onLeftButtonReleaseField(FXObject* obj, FXSelector sel, void* ptr) {
	if (gf.disfield) return 1;
	//puts("RELEASEFIELD");
	pressed = 0;
	//printf("x: %d, y: %d\n", pressedx, pressedy);
	if (pressedy > (unsigned)boardh || pressedx > (unsigned)boardw) return 0;
	//puts("YEAH");
	//if (board[pressedx*boardw+(mousex>>4)].state == FLAG) return 1;

	mscanvas->update(BOARDX+(pressedx<<4), BOARDY+(pressedy<<4), 16, 16);
	if (!gf.minesplaced) {
		gf.minesplaced = TRUE;
		//initBoard(boardw,BOARDY);
		placeMines(boardm, pressedx, pressedy);
		getApp()->addTimeout(this, ID_TIMEOUT, 1000);
#ifdef DEBUG
		printBoard();
#endif
	}
	if (!board[pressedy*boardw+pressedx].shown) revealCell(pressedx, pressedy, 1);
	if (board[pressedy*boardw+pressedx].neigh != ISMINE) {
		pressedx = UINT_MAX;
		pressedy = UINT_MAX;
	}
	if (gf.lost) smiley = SMILEBTN_LOST;
	else if (gf.won) smiley = SMILEBTN_WIN;
	else smiley = SMILEBTN_NORMAL;
	updateSmiley();

	return 1;
}

long Minesweeper::onLeftButtonReleaseSmiley(FXObject* obj, FXSelector sel, void* ptr) {
	FXEvent* ev = (FXEvent*)ptr;

	if (CHECKMOUSE(ev, BOARDX+(boardw<<3)-12,15, 26, 26)) {
		resetBoard(NULL, 0, NULL);
	}
	
	return 1;
}


long Minesweeper::onLeftButtonRelease(FXObject* obj, FXSelector sel, void* ptr) {
	onLeftButtonReleaseField(obj, sel, ptr);
	onLeftButtonReleaseSmiley(obj, sel, ptr);

	pressed = 0;

	if (gf.lost) smiley = SMILEBTN_LOST;
	else if (gf.won) smiley = SMILEBTN_WIN;
	else smiley = SMILEBTN_NORMAL;
	updateSmiley();
	return 1;
}

long Minesweeper::onMotionField(FXObject* obj, FXSelector sel, void* ptr) {
	if (gf.disfield) return 1;
	FXEvent* ev = (FXEvent*)ptr;
	unsigned mousex = ev->win_x - BOARDX;
	unsigned mousey = ev->win_y - BOARDY;

	unsigned oldpressedx = pressedx;
	unsigned oldpressedy = pressedy;

	if (oldpressedx == (mousex>>4) && oldpressedy == (mousey>>4)) return 1;


	if (pressed) {
		pressedx = mousex >> 4;
		pressedy = mousey >> 4;

		mscanvas->update(BOARDX+(oldpressedx<<4), BOARDY+(oldpressedy<<4), 16, 16);
		mscanvas->update(BOARDX+(pressedx<<4), BOARDY+(pressedy<<4), 16, 16);
	}

	//mscanvas->repaint(BOARDX+(pressedx<<4), BOARDY+(pressedy<<4), 16, 16);

	return 1;
}

long Minesweeper::onMotionSmiley(FXObject* obj, FXSelector sel, void* ptr) {
	FXEvent* ev = (FXEvent*)ptr;

#ifdef DEBUG
	printf("pressed: %d\n", pressed);
#endif

	if (pressed) {
#ifdef DEBUG
		printf("x: %d, y: %d\n", ev->win_x, ev->win_y);
#endif
		if (CHECKMOUSE(ev, BOARDX+(boardw<<3)-12,15, 26, 26)) {
#ifdef DEBUG
			puts("UPDATE!!");
#endif
			if (smiley != SMILEBTN_PRESSED) {
				smiley = SMILEBTN_PRESSED;
				updateSmiley();
			}
		} else {
#ifdef DEBUG
			puts("WATCHOUT");
#endif
			if (gf.lost) {
				if (smiley != SMILEBTN_LOST) {
					smiley = SMILEBTN_LOST;
					updateSmiley();
				}
			} else if (gf.won) {
				if (smiley != SMILEBTN_WIN) {
					smiley = SMILEBTN_WIN;
					updateSmiley();
				}
			} else {
				if (smiley != SMILEBTN_WATCHOUT) {
					smiley = SMILEBTN_WATCHOUT;
					updateSmiley();
				}
			}
		}
	}
	return 1;
}

long Minesweeper::onMotion(FXObject* obj, FXSelector sel, void* ptr) {
	onMotionField(obj, sel, ptr);
	onMotionSmiley(obj, sel, ptr);

	return 1;
}

int paintamount = 0;

void Minesweeper::paintBoard(FXWindow* win, FXEvent* ev) {
	if (!CHECKRECT(ev, BOARDX, BOARDY, boardw<<4, boardh<<4)) return;

	FXDCWindow dc(win, ev);

	//((ev->rect.x>>4)-BOARDX, (r>>4)-BOARDY, 16, 16)
	//printf("x: %d\n", ev->rect.y>>4);
	for (int cy = 0; cy < boardh; ++cy) {
		for (int cx = 0; cx < boardw; ++cx) {
			int rx = BOARDX+cx*16;
			int ry = BOARDY+cy*16;

			if (CHECKRECT(ev, BOARDX+cx*16, BOARDY+cy*16, 16, 16)) {
				//printf("painted: %d\n", ++paintamount);
				int cell = cy*boardw+cx;
				int pressedcell = pressedy*boardw+pressedx;

				if (board[cell].shown) {
					if (board[cell].neigh == ISMINE) {
						if ((int)pressedx == cx && (int)pressedy == cy) {
							dc.drawArea(img_mine, 0, MINEBTN_MINE_PRESSED, 16,16, rx,ry);
						} else {
							dc.drawArea(img_mine, 0, MINEBTN_MINE, 16,16, rx,ry);
						}
					} else {
						int img;
						if (board[cell].neigh) img = MINEBTN_ONE-(board[cell].neigh-1)*16;
						else img = MINEBTN_NORMAL_PRESSED;
						dc.drawArea(img_mine, 0, img, 16,16, rx,ry);
					}
				} else {
					switch(board[cell].state) {
					case FLAG:
						dc.drawArea(img_mine, 0, MINEBTN_FLAG, 16,16, rx,ry);
						break;
					case UNKNOWN:
						if (cell == pressedcell) dc.drawArea(img_mine, 0, MINEBTN_UNKNOWN_PRESSED, 16,16, rx,ry);
						else dc.drawArea(img_mine, 0, MINEBTN_UNKNOWN, 16,16, rx,ry);
						break;
					case INVALIDFLAG:
						dc.drawArea(img_mine, 0, MINEBTN_NOTMINE, 16,16, rx,ry);
						break;
					default:
						if (cell == pressedcell) dc.drawArea(img_mine, 0, MINEBTN_NORMAL_PRESSED, 16,16, rx,ry);
						else dc.drawArea(img_mine, 0, MINEBTN_NORMAL, 16,16, rx,ry);
					}
				}
			}
		}

		//putchar('\n');
	}

	dc.end();

	return;
}

void Minesweeper::paintBorders(FXWindow* win, FXEvent* ev) {
	FXDCWindow dc(win, ev);

	if (CHECKRECT(ev, 0, 0, width, 3)) {
		dc.setForeground(FXRGB(255,255,255));
		dc.fillRectangle(0, 0, width, 3);
	}


	if (CHECKRECT(ev, 0, 3, 3, height)) {
		dc.setForeground(FXRGB(255,255,255));
		dc.fillRectangle(0, 3, 3, height);
	}

	dc.end();

	return;
}

void Minesweeper::paintBoardBorders(FXWindow* win, FXEvent* ev) {
	FXDCWindow dc(win, ev);

	if (CHECKRECT(ev, BOARDX-3, BOARDY-3, (boardw<<4)+6, 3)) {
		dc.setForeground( (gf.color ? FXRGB(128,128,128) : FXRGB(0,0,0)) );
		dc.fillRectangle(BOARDX-3, BOARDY-3, (boardw<<4)+3, 3);
		dc.drawPoint( (BOARDX-3) + ((boardw<<4)+3)+1, (BOARDY-3));
		dc.drawPoint( (BOARDX-3) + ((boardw<<4)+3),   (BOARDY-3));
		dc.drawPoint( (BOARDX-3) + ((boardw<<4)+3),   (BOARDY-2));
	
		dc.setForeground(FXRGB(255,255,255));

		dc.drawPoint( (BOARDX-2) + ((boardw<<4)+3)+1, (BOARDY-2));
		dc.drawPoint( (BOARDX-2) + ((boardw<<4)+3)+1,   (BOARDY-1));
		dc.drawPoint( (BOARDX-2) + ((boardw<<4)+3),   (BOARDY-1));
	}

	if (CHECKRECT(ev, BOARDX-3, BOARDY-3, 3, (boardh<<4)+4)) {
		dc.setForeground( (gf.color ? FXRGB(128,128,128) : FXRGB(0,0,0)) );
		dc.fillRectangle(BOARDX-3, BOARDY-3, 3, (boardh<<4)+3);

		dc.drawPoint( (BOARDX-3) + 1, (BOARDY-3) + ((boardh<<4)+3));
		dc.drawPoint( (BOARDX-3),     (BOARDY-3) + ((boardh<<4)+3));
		dc.drawPoint( (BOARDX-3),     (BOARDY-2) + ((boardh<<4)+3));

		dc.setForeground(FXRGB(255,255,255));

		dc.drawPoint( (BOARDX-2) + 1, (BOARDY-2) + ((boardh<<4)+4));
		dc.drawPoint( (BOARDX-2) + 1, (BOARDY-2) + ((boardh<<4)+3));
		dc.drawPoint( (BOARDX-2),     (BOARDY-2) + ((boardh<<4)+4));
	}

	if (CHECKRECT(ev, BOARDX, BOARDY+(boardh<<4), (boardw<<4)+3, 3)) {
		dc.setForeground(FXRGB(255,255,255));
		dc.fillRectangle(BOARDX, BOARDY+(boardh<<4), (boardw<<4)+3, 3);
	}

	dc.setForeground(FXRGB(255,255,255));
	dc.fillRectangle(BOARDX+(boardw<<4), BOARDY, 3, (boardh<<4));

	dc.end();

	return;
}

void Minesweeper::drawNumber(FXWindow* win, FXEvent* ev, int x, int y, int num2) {
	int num;
	char numstr[4];

	if (num2 > 999) {
		num = 999;
	} else if (num2 < -99) {
		num = -99;
	} else {
		num = num2;
	}

	sprintf(numstr, "%03d", num);

	FXDCWindow dc(win, ev);

	if (CHECKRECT(ev, x, y, (13*3)+1, 1)) {
		dc.setForeground( (gf.color ? FXRGB(128,128,128) : FXRGB(0,0,0)) );
		dc.fillRectangle(x, y, (13*3)+1, 1);
	}

	if (CHECKRECT(ev, x, y, 1, 23+1)) {
		dc.setForeground( (gf.color ? FXRGB(128,128,128) : FXRGB(0,0,0)) );
		dc.fillRectangle(x, y, 1, 23+1);
	}

	if (CHECKRECT(ev, (13*3)+x+1, y+1, 1, 23+1)) {
		dc.setForeground(FXRGB(255,255,255));
		dc.fillRectangle((13*3)+x+1, y+1, 1, 23+1);
	}

	if (CHECKRECT(ev, x+1, y+23+1, (13*3), 1)) {
		dc.setForeground(FXRGB(255,255,255));
		dc.fillRectangle(x+1, y+23+1, (13*3), 1);
	}

	//puts(numstr);
	int c = 0;
	while (c < 3) {
		if (CHECKRECT(ev, x+(13*c)+1, y+1, 13, 23)) {
			//putchar(numstr[c]);
			switch(numstr[c]) {
				case '-': dc.drawArea(img_seg, 0,     0, 13,23, 1+x+(13*c),1+y); break;
				case '0': dc.drawArea(img_seg, 0, 23*11, 13,23, 1+x+(13*c),1+y); break;
				case '1': dc.drawArea(img_seg, 0, 23*10, 13,23, 1+x+(13*c),1+y); break;
				case '2': dc.drawArea(img_seg, 0, 23* 9, 13,23, 1+x+(13*c),1+y); break;
				case '3': dc.drawArea(img_seg, 0, 23* 8, 13,23, 1+x+(13*c),1+y); break;
				case '4': dc.drawArea(img_seg, 0, 23* 7, 13,23, 1+x+(13*c),1+y); break;
				case '5': dc.drawArea(img_seg, 0, 23* 6, 13,23, 1+x+(13*c),1+y); break;
				case '6': dc.drawArea(img_seg, 0, 23* 5, 13,23, 1+x+(13*c),1+y); break;
				case '7': dc.drawArea(img_seg, 0, 23* 4, 13,23, 1+x+(13*c),1+y); break;
				case '8': dc.drawArea(img_seg, 0, 23* 3, 13,23, 1+x+(13*c),1+y); break;
				case '9': dc.drawArea(img_seg, 0, 23* 2, 13,23, 1+x+(13*c),1+y); break;
				default:  dc.drawArea(img_seg, 0, 23* 1, 13,23, 1+x+(13*c),1+y);
			}
		}
		++c;
	}
	//		putchar('\n');

	dc.end();

	return;
}

void Minesweeper::drawSmile(FXWindow* win, FXEvent* ev) {
	FXDCWindow dc(win, ev);

	if (CHECKRECT(ev, BOARDX+(boardw<<3)-12,15, 26, 26)) {
		dc.setForeground( (gf.color ? FXRGB(128,128,128) : FXRGB(0,0,0)) );
		dc.fillRectangle(BOARDX+(boardw<<3)-13, 15, 25,1);
		dc.fillRectangle(BOARDX+(boardw<<3)-13+1, 15+24+1, 25,1);

		dc.fillRectangle(BOARDX+(boardw<<3)-13, 16, 1,24);
		dc.fillRectangle(BOARDX+(boardw<<3)-13+25, 16, 1,24);

		dc.drawArea(img_smil, 0, smiley, 24,24, BOARDX+(boardw<<3)-12,16);
	}

	dc.end();
}

void Minesweeper::updateSmiley() {
	mscanvas->update(BOARDX+(boardw<<3)-12,15, 26, 26);
}


void Minesweeper::paintTopBorders(FXWindow* win, FXEvent* ev) {
	FXDCWindow dc(win, ev);

	if (CHECKRECT(ev, 9, 9, (boardw<<4)+5, 2)) {
		dc.setForeground( (gf.color ? FXRGB(128,128,128) : FXRGB(0,0,0)) );
		dc.fillRectangle(9, 9, (boardw<<4)+4, 2);
		dc.drawPoint(9+(boardw<<4)+4, 9);
	}

	if (CHECKRECT(ev, 9, 9+2, 2, 36)) {
		dc.setForeground( (gf.color ? FXRGB(128,128,128) : FXRGB(0,0,0)) );
		dc.fillRectangle(9, 9+2, 2, 35-2);
		dc.drawPoint(9, 9+35);
	}

	if (CHECKRECT(ev, 9, 9+36, (boardw<<4)+5, 2)) {
		dc.setForeground(FXRGB(255,255,255));
		dc.fillRectangle(9+2, 9+35, (boardw<<4)+4, 2);
		dc.drawPoint(9+1, 9+36);
	}

	if (CHECKRECT(ev, 9+(boardw<<4)+5, 9, 2, 36)) {
		dc.setForeground(FXRGB(255,255,255));
		dc.fillRectangle(9+(boardw<<4)+4, 9+2, 2, 33);
		dc.drawPoint(9+(boardw<<4)+5, 10);
	}


	dc.end();

	return;
}

long Minesweeper::onTimeout(FXObject* obj, FXSelector sel, void* ptr) {
	if (++seconds != 999) getApp()->addTimeout(this, ID_TIMEOUT, 1000);
	mscanvas->update(BOARDX+(boardw<<4)-(13*3)-8, 15, 13*3, 23);

	return 1;
}

long Minesweeper::onPaint(FXObject* obj, FXSelector sel, void* ptr) {
	FXEvent* ev = (FXEvent*)ptr;
	FXWindow* win = (FXWindow*)obj;

	FXDCWindow dc(win, ev);

	dc.setForeground(FXRGB(192,192,192));
	dc.fillRectangle(ev->rect.x, ev->rect.y, ev->rect.w, ev->rect.h);
	dc.end();

	paintBorders     (win, ev);
	paintBoard       (win, ev);
	paintBoardBorders(win, ev);
	paintTopBorders(win, ev);


	drawSmile(win, ev);
	drawNumber(win, ev, 16, 15, minesover);
	drawNumber(win, ev, BOARDX+(boardw<<4)-(13*3)-8, 15, seconds);

	return 1;
}


long Minesweeper::onCmdAbout(FXObject*, FXSelector, void* ptr) {
	AboutBox aboutbox(this);
	aboutbox.execute(PLACEMENT_OWNER);

	return 1;
}



int main(int argc, char *argv[]) {
	FXApp application("WinMine", "Ice2KProj");
	application.init(argc, argv);
	//ico_main_16 = new FXGIFIcon(&application, res_ico_main_16, 0, IMAGE_OPAQUE);


	if (i2kBGetWinVersionInt() != ICE2K_BRAND_WIN2K) {
		ico_main_16 = new FXPNGIcon(&application, res_ico_main_16_xp);
		//ico_main_16->blend(FXRGBA(0,0,0,0));
		ico_main_32 = new FXPNGIcon(&application, res_ico_main_32_xp);
		ico_main_32->blend(application.getBaseColor());
	} else {
		ico_main_16 = new FXPNGIcon(&application, res_ico_main_16_2k);
		ico_main_32 = new FXPNGIcon(&application, res_ico_main_32_2k);
	}


	img_coolseg = new FXGIFImage(&application, res_img_coolseg);
	img_coolseg->create();
	img_coolsmil = new FXGIFImage(&application, res_img_coolsmil);
	img_coolsmil->create();
	img_coolmine = new FXGIFImage(&application, res_img_coolmine);
	img_coolmine->create();

	img_monoseg = new FXGIFImage(&application, res_img_monoseg);
	img_monoseg->create();
	img_monosmil = new FXGIFImage(&application, res_img_monosmil);
	img_monosmil->create();
	img_monomine = new FXGIFImage(&application, res_img_monomine);
	img_monomine->create();

	if (gf.color) {
		img_seg  = img_coolseg;
		img_smil = img_coolsmil;
		img_mine = img_coolmine;
	} else {
		img_seg  = img_monoseg;
		img_smil = img_monosmil;
		img_mine = img_monomine;
	}


	initBoard(9,9);

	mainwin = new Minesweeper(&application);
#ifdef DEBUG
	printBoard();
#endif

	application.create();
	return application.run();
}
