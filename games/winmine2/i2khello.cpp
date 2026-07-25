#include <fx.h>
#include <time.h>
#include <limits.h>
// #include <ice2k/comctl32.h>

#include "res/foxres.h"

FXIcon* ico_main_16;
FXImage* img_segment;
FXImage* img_coolsmil;
FXImage* img_coolmine;

FXIcon* ico_two;

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

//typedef unsigned char CELL;
typedef struct {
	unsigned char mine:1;
	unsigned char shown:1;
	unsigned char state:2;
	unsigned char neigh:4;
} CELL;

#define NOFLAG 0
#define FLAG 1
#define UNKNOWN 2
//#define PRESSEDMINE 3
#define INVALIDFLAG 3



#define MAXWIDTH  30
#define MAXHEIGHT 24
CELL board[MAXWIDTH*MAXHEIGHT];

int boardx;
int boardy;

int boardw;
int boardh;
int boardm = 10;

FXbool minesplaced = FALSE;
FXbool color = TRUE;

FXbool disfield = FALSE;

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
	( (rx < ev->rect.x + ev->rect.w) && \
	  (rx +         rw > ev->rect.x) && \
	  (ry < ev->rect.y + ev->rect.h) && \
	  (ry +         rh > ev->rect.y) )

void initBoard(int w, int h) {
	boardw = w;
	boardh = h;

	if (boardw > MAXWIDTH)  boardw = MAXWIDTH;
	if (boardh > MAXHEIGHT) boardh = MAXHEIGHT;
	
	memset(board, 0x00, (boardw*boardh)*sizeof(board[0]));
}

void placeMines(int m, int mx, int my) {
	boardm = m;

	for (int i = 0; i < boardm;) {
		int x = rand() % boardw;
		int y = rand() % boardh;
		printf("x: %d, y: %d\n", x, y);
		int cell = y*boardw+x;
		int mcell = my*boardw+mx;
		
		if (cell != mcell && board[cell].mine == 0) {
			board[cell].mine = 1;
			++i;
		}
	}

	for (int y = 0; y < boardh; ++y) {
		for (int x = 0; x < boardw; ++x) {
			int cell = y*boardw+x;
			if (board[cell].mine) continue;

			for (int dy = -1; dy <= 1; ++dy) {
				for (int dx = -1; dx <= 1; ++dx) {
					int ny = y + dy;
					int nx = x + dx;

					if (ny >= 0 && ny < boardh && nx >= 0 && nx < boardw) {
						if (board[ny*boardw+nx].mine) board[cell].neigh++;
					}
				}
			}
		}
	}

	minesplaced = TRUE;
}


void printBoard() {
	for (int y = 0; y < boardh; ++y) {
		for (int x = 0; x < boardw; ++x) {
			if (board[y*boardw+x].mine) putchar('*');
			else putchar('.');
		}

		putchar('\n');
	}
}

class Minesweeper : public FXMainWindow {
	FXDECLARE(Minesweeper);

protected:
	Minesweeper() {}

private:
	FXMenuBar* menubar;
	FXHorizontalFrame *contents;
	FXCanvas* mscanvas;

public:
	long onPaint(FXObject*, FXSelector, void*);
	long onLeftButtonPress(FXObject*, FXSelector, void*);
	long onLeftButtonRelease(FXObject*, FXSelector, void*);

	long onMotion(FXObject*, FXSelector, void*);

	long onRightButtonPress(FXObject*, FXSelector, void*);

	void revealCell(int, int, int);

	int paintBoard(FXWindow*, FXEvent*);
	int paintBoardBorders(FXWindow*, FXEvent*);
	int paintBorders(FXWindow* win, FXEvent* ev);
	int drawNumber(FXWindow* win, FXEvent* ev, int x, int y, int num2);

public:
	enum {
		ID_CANVAS = FXMainWindow::ID_LAST,
		ID_LAST
	};

public:
	Minesweeper(FXApp* a);

	virtual void create();
	virtual ~Minesweeper();
};

FXDEFMAP(Minesweeper) MinesweeperMap[] = {
	FXMAPFUNC(SEL_PAINT,             Minesweeper::ID_CANVAS,  Minesweeper::onPaint),
	FXMAPFUNC(SEL_LEFTBUTTONPRESS,   Minesweeper::ID_CANVAS,  Minesweeper::onLeftButtonPress),
	FXMAPFUNC(SEL_RIGHTBUTTONPRESS,  Minesweeper::ID_CANVAS,  Minesweeper::onRightButtonPress),

	FXMAPFUNC(SEL_LEFTBUTTONRELEASE, Minesweeper::ID_CANVAS,  Minesweeper::onLeftButtonRelease),

	FXMAPFUNC(SEL_MOTION,          Minesweeper::ID_CANVAS,  Minesweeper::onMotion),

};

FXIMPLEMENT(Minesweeper, FXMainWindow, MinesweeperMap, ARRAYNUMBER(MinesweeperMap));

Minesweeper::Minesweeper(FXApp *a) : FXMainWindow(a, "Minesweeper", ico_main_16, NULL, DECOR_ALL, 0,0,0,0) {
	menubar = new FXMenuBar(this, this, LAYOUT_SIDE_TOP|LAYOUT_FILL_X, 0,0,0,0, 0,0,0,1, 0,0);
	FXMenuPane* gamemenu = new FXMenuPane(this);
	FXMenuPane* helpmenu = new FXMenuPane(this);
	new FXMenuTitle(menubar,"&Game", NULL, gamemenu);
	new FXMenuTitle(menubar,"&Help", NULL, helpmenu);

	new FXMenuRadio(gamemenu, "&Beginner");
	mscanvas = new FXCanvas(this, this, ID_CANVAS, LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT);
	mscanvas->setWidth(512);
	mscanvas->setHeight(512);
	//new FXLabel(this, "test", ico_two);
}

Minesweeper::~Minesweeper() {
}

void Minesweeper::create() {
	FXMainWindow::create();

	show(PLACEMENT_SCREEN);
}

unsigned pressedx = UINT_MAX;
unsigned pressedy = UINT_MAX;

int pressed = 0;

void Minesweeper::revealCell(int col, int row, int click) {
	if (row < 0 || row >= boardh || col < 0 || col >= boardw) return;

	CELL* cell = &board[row*boardw+col];
	if (cell->shown || cell->state == FLAG) return;

	if (cell->mine) {
		if (click) {
			for (int r = 0; r < boardh; ++r) {
				for (int c = 0; c < boardw; ++c) {
					if (board[r*boardw+c].mine && board[r*boardw+c].state != FLAG) {
						board[r*boardw+c].shown = TRUE;
						mscanvas->update(boardx+(c<<4), boardy+(r<<4), 16, 16);
					}

					if (!board[r*boardw+c].mine && board[r*boardw+c].state == FLAG) {
						board[r*boardw+c].state = INVALIDFLAG;
						mscanvas->update(boardx+(c<<4), boardy+(r<<4), 16, 16);
					}
				}
			}

			disfield = TRUE;
			//board[row*boardw+col].state = PRESSEDMINE;
		}

		return;
	}
	cell->shown = TRUE;
	mscanvas->update(boardx+(col*16), boardy+(row*16), 16, 16);

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


long Minesweeper::onLeftButtonPress(FXObject* obj, FXSelector sel, void* ptr) {
	if (disfield) return 1;
	FXEvent* ev = (FXEvent*)ptr;
	unsigned mousex = ev->win_x - boardx;
	unsigned mousey = ev->win_y - boardy;

	unsigned oldpressedx = pressedx;
	unsigned oldpressedy = pressedy;

	unsigned minex = mousex >> 4;
	unsigned miney = mousey >> 4;

	if (miney < 0 || (int)miney >= boardh || minex < 0 || (int)minex >= boardw) return 1;
	if (board[miney*boardw+minex].state == FLAG) return 1;

	pressedx = minex;
	pressedy = miney;


	//unsigned pressedx16 = pressedx << 4;
	//unsigned pressedy16 = pressedy << 4;

	//mscanvas->update(boardx+(oldpressedx<<4), boardy+(oldpressedy<<4), 16, 16);
	mscanvas->update(boardx+(pressedx<<4), boardy+(pressedy<<4), 16, 16);

	pressed = 1;

	//mscanvas->repaint(boardx+(pressedx<<4), boardy+(pressedy<<4), 16, 16);

	return 1;
}

long Minesweeper::onRightButtonPress(FXObject* obj, FXSelector sel, void* ptr) {
	if (disfield) return 1;
	FXEvent* ev = (FXEvent*)ptr;
	unsigned mousex = ev->win_x - boardx;
	unsigned mousey = ev->win_y - boardy;
	
	unsigned rightx = mousex >> 4;
	unsigned righty = mousey >> 4;

	if (righty < 0 || (int)righty >= boardh || rightx < 0 || (int)rightx >= boardw) return 1;
	if (board[righty*boardw+rightx].state == FLAG) return 1;


	if (++(board[righty*boardw+rightx].state) == INVALIDFLAG) {
		board[righty*boardw+rightx].state = NOFLAG;
	}

	mscanvas->update(boardx+(rightx<<4), boardy+(righty<<4), 16, 16);

	return 1;
}

long Minesweeper::onLeftButtonRelease(FXObject* obj, FXSelector sel, void* ptr) {
	if (disfield) return 1;
	pressed = 0;
	if (0 > pressedx || 0 > pressedy || (int)pressedy > boardh || (int)pressedx > boardw) return 0;
	//if (board[pressedx*boardw+(mousex>>4)].state == FLAG) return 1;

	mscanvas->update(boardx+(pressedx<<4), boardy+(pressedy<<4), 16, 16);
	if (!minesplaced) {
		minesplaced = TRUE;
		//initBoard(boardw,boardy);
		placeMines(boardm, pressedx, pressedy);
		printBoard();
	}
	if (!board[pressedy*boardw+pressedx].shown) revealCell(pressedx, pressedy, 1);
	if (!board[pressedy*boardw+pressedx].mine) {
		pressedx = UINT_MAX;
		pressedy = UINT_MAX;
	}

	return 1;
}

long Minesweeper::onMotion(FXObject* obj, FXSelector sel, void* ptr) {
	if (disfield) return 1;
	FXEvent* ev = (FXEvent*)ptr;
	unsigned mousex = ev->win_x - boardx;
	unsigned mousey = ev->win_y - boardy;

	unsigned oldpressedx = pressedx;
	unsigned oldpressedy = pressedy;

	if (oldpressedx == (mousex>>4) && oldpressedy == (mousey>>4)) return 1;


	if (pressed) {
		pressedx = mousex >> 4;
		pressedy = mousey >> 4;

		mscanvas->update(boardx+(oldpressedx<<4), boardy+(oldpressedy<<4), 16, 16);
		mscanvas->update(boardx+(pressedx<<4), boardy+(pressedy<<4), 16, 16);
	}

	//mscanvas->repaint(boardx+(pressedx<<4), boardy+(pressedy<<4), 16, 16);

	return 1;
}

int paintamount = 0;

int Minesweeper::paintBoard(FXWindow* win, FXEvent* ev) {
	boardx = 12;
	boardy = 55;

	FXDCWindow dc(win, ev);


	//((ev->rect.x>>4)-boardx, (r>>4)-boardy, 16, 16)
	//printf("x: %d\n", ev->rect.y>>4);
	for (int cy = 0; cy < boardh; ++cy) {
		for (int cx = 0; cx < boardw; ++cx) {
			int rx = boardx+cx*16;
			int ry = boardy+cy*16;

			if (CHECKRECT(ev, boardx+cx*16, boardy+cy*16, 16, 16)) {
				//printf("painted: %d\n", ++paintamount);
				int cell = cy*boardw+cx;
				int pressedcell = pressedy*boardw+pressedx;

				if (board[cell].shown) {
					if (board[cell].mine) {
						if ((int)pressedx == cx && (int)pressedy == cy) {
							dc.drawArea(img_coolmine, 0, MINEBTN_MINE_PRESSED, 16,16, rx,ry);
						} else {
							dc.drawArea(img_coolmine, 0, MINEBTN_MINE, 16,16, rx,ry);
						}
					} else {
						int img;
						if (board[cell].neigh) img = MINEBTN_ONE-(board[cell].neigh-1)*16;
						else img = MINEBTN_NORMAL_PRESSED;
						dc.drawArea(img_coolmine, 0, img, 16,16, rx,ry);
					}
				} else {
					switch(board[cell].state) {
					case FLAG:
						dc.drawArea(img_coolmine, 0, MINEBTN_FLAG, 16,16, rx,ry);
						break;
					case UNKNOWN:
						if (cell == pressedcell) dc.drawArea(img_coolmine, 0, MINEBTN_UNKNOWN_PRESSED, 16,16, rx,ry);
						else dc.drawArea(img_coolmine, 0, MINEBTN_UNKNOWN, 16,16, rx,ry);
						break;
					case INVALIDFLAG:
						dc.drawArea(img_coolmine, 0, MINEBTN_NOTMINE, 16,16, rx,ry);
						break;
					default:
						if (cell == pressedcell) dc.drawArea(img_coolmine, 0, MINEBTN_NORMAL_PRESSED, 16,16, rx,ry);
						else dc.drawArea(img_coolmine, 0, MINEBTN_NORMAL, 16,16, rx,ry);
					}
				}
			}
		}

		//putchar('\n');
	}

	dc.end();

	return 1;
}

int Minesweeper::paintBorders(FXWindow* win, FXEvent* ev) {
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

	return 1;
}

int Minesweeper::paintBoardBorders(FXWindow* win, FXEvent* ev) {
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

	return 1;
}

int Minesweeper::drawNumber(FXWindow* win, FXEvent* ev, int x, int y, int num2) {
	int num;
	char numstr[4];
	int segbmph = img_segment->getHeight();

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
		dc.setForeground(FXRGB(128,128,128));
		dc.fillRectangle(x, y, (13*3)+1, 1);
	}

	if (CHECKRECT(ev, x, y, 1, 23+1)) {
		dc.setForeground(FXRGB(128,128,128));
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

	int c = 0;
	while (c < 3) {
		if (CHECKRECT(ev, x+(13*c)+1, y+1, 13, 23)) {
			switch(numstr[c]) {
				case '0': dc.drawArea(img_segment, 0, 23*11, 13,23, 1+x+(13*c),1+y); break;
				case '1': dc.drawArea(img_segment, 0, 23*10, 13,23, 1+x+(13*c),1+y); break;
				case '2': dc.drawArea(img_segment, 0, 23* 9, 13,23, 1+x+(13*c),1+y); break;
				case '3': dc.drawArea(img_segment, 0, 23* 8, 13,23, 1+x+(13*c),1+y); break;
				case '4': dc.drawArea(img_segment, 0, 23* 7, 13,23, 1+x+(13*c),1+y); break;
				case '5': dc.drawArea(img_segment, 0, 23* 6, 13,23, 1+x+(13*c),1+y); break;
				case '6': dc.drawArea(img_segment, 0, 23* 5, 13,23, 1+x+(13*c),1+y); break;
				case '7': dc.drawArea(img_segment, 0, 23* 4, 13,23, 1+x+(13*c),1+y); break;
				case '8': dc.drawArea(img_segment, 0, 23* 3, 13,23, 1+x+(13*c),1+y); break;
				case '9': dc.drawArea(img_segment, 0, 23* 2, 13,23, 1+x+(13*c),1+y); break;
				case '-': dc.drawArea(img_segment, 0,     0, 13,23, 1+x+(13*c),1+y); break;
				default:  dc.drawArea(img_segment, 0, 23* 1, 13,23, 1+x+(13*c),1+y);
			}
		}
		++c;
	}

	dc.end();

	return 1;
}


long Minesweeper::onPaint(FXObject* obj, FXSelector sel, void* ptr) {
	FXEvent* ev = (FXEvent*)ptr;
	FXWindow* win = (FXWindow*)obj;

	FXDCWindow dc(win, ev);

	dc.setForeground(FXRGB(192,192,192));
	dc.fillRectangle(ev->rect.x, ev->rect.y, ev->rect.w, ev->rect.h);

	dc.drawArea(img_coolsmil, 0, SMILEBTN_NORMAL, 24,24, 16,16);

	dc.end();

	paintBorders     (win, ev);
	paintBoardBorders(win, ev);
	paintBoard       (win, ev);

	drawNumber(win, ev, 16, 15, boardm);

	return 1;
}

int main(int argc, char *argv[]) {
	FXApp application("Minesweeper", "I2KProject");
	application.init(argc, argv);
	ico_main_16 = new FXGIFIcon(&application, res_ico_main_16, 0, IMAGE_OPAQUE);
	img_segment = new FXGIFImage(&application, res_img_segment);
	img_segment->create();
	img_coolsmil = new FXGIFImage(&application, res_img_coolsmil);
	img_coolsmil->create();
	img_coolmine = new FXGIFImage(&application, res_img_coolmine, IMAGE_KEEP);
	img_coolmine->create();

	/*ico_two = new FXIcon(&application, img_coolmine->getData()+(MINEBTN_SIX*16), 0, IMAGE_OPAQUE, 16, 16);
	ico_two->create();
	delete img_coolmine;*/
	
	initBoard(10,10);

	new Minesweeper(&application);

	printBoard();


	application.create();
	return application.run();
}
