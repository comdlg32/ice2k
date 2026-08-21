#include <fx.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
// #include <ice2k/comctl32.h>

#include "res/foxres.h"
char welpath[PATH_MAX] = {0};

FXIcon* ico_mainicon;
FXImage* img_connect;
FXImage* img_discover;
FXImage* img_logo;
FXImage* img_register;
FXImage* img_welcome;

FXFont* titlefont;
FXFont* boldfont;
FXFont* normalfont;
FXFont* smallfont;

int runwelcome = 0;

#define BACKCOLOR FXRGB(255,255,255)
#define SIDECOLOR FXRGB(166,202,240)
#define BTNCOLOR FXRGB(128,128,128)


#define TEXTCOLOR FXRGB(0,0,0)
#define SIDETEXTCOLOR FXRGB(0,0,0)

#define SEPCOLOR FXRGB(51,102,153)
#define FOCUSCOLOR FXRGB(43,47,55)

#define TITLECOLOR SEPCOLOR

#define AMOUNTBARS 3

#define CHECKRECT(ev, rx, ry, rw, rh) \
	( ((rx) < (ev)->rect.x + (ev)->rect.w) && \
	  ((rx) +         (rw) > (ev)->rect.x) && \
	  ((ry) < (ev)->rect.y + (ev)->rect.h) && \
	  ((ry) +         (rh) > (ev)->rect.y) )



int hoveredbar = 0;

class WelcomeWindow : public FXMainWindow {
	FXDECLARE(WelcomeWindow);

protected:
	WelcomeWindow() {}

private:
	FXPacker *cont;
	FXCanvas *leftcanvas;
	FXCanvas *rightcanvas;
	FXCanvas *btncanvas;

	FXCheckButton* chk;

	int btntextwidth;
	int btntextheight;
	
	int btnhover;

public:
	long onMotionLeft(FXObject*, FXSelector, void*);
	long onLeaveLeft(FXObject*, FXSelector, void*);

	long onPaintLeft(FXObject*, FXSelector, void*);

	long onPaintRight(FXObject*, FXSelector, void*);
	long onKeypressBtn(FXObject*, FXSelector, void*);
	long onClick(FXObject*, FXSelector, void*);
	long onEnterBtn(FXObject*, FXSelector, void*);


	long onLeaveBtn(FXObject*, FXSelector, void*);

	long onPaintBtn(FXObject*, FXSelector, void*);




public:
	enum {
		ID_LEFTCANVAS = FXMainWindow::ID_LAST,
		ID_RIGHTCANVAS,
		ID_BTNCANVAS,
		ID_LAST
	};

public:
	WelcomeWindow(FXApp* a);

	virtual void create();
	void setFocus() {};
	virtual ~WelcomeWindow();
};

FXDEFMAP(WelcomeWindow) WelcomeWindowMap[] = {
	FXMAPFUNC(SEL_MOTION,          WelcomeWindow::ID_LEFTCANVAS,  WelcomeWindow::onMotionLeft),
	FXMAPFUNC(SEL_LEAVE,           WelcomeWindow::ID_LEFTCANVAS,  WelcomeWindow::onLeaveLeft),

	FXMAPFUNC(SEL_PAINT,           WelcomeWindow::ID_LEFTCANVAS,  WelcomeWindow::onPaintLeft),
	FXMAPFUNC(SEL_PAINT,           WelcomeWindow::ID_BTNCANVAS,  WelcomeWindow::onPaintBtn),

	FXMAPFUNC(SEL_KEYPRESS,        WelcomeWindow::ID_HOTKEY,  WelcomeWindow::onKeypressBtn),
	FXMAPFUNC(SEL_ENTER,           WelcomeWindow::ID_BTNCANVAS,  WelcomeWindow::onEnterBtn),

	FXMAPFUNC(SEL_LEAVE,           WelcomeWindow::ID_BTNCANVAS,  WelcomeWindow::onLeaveBtn),
	FXMAPFUNC(SEL_LEFTBUTTONPRESS,           WelcomeWindow::ID_LEFTCANVAS,  WelcomeWindow::onClick),
	FXMAPFUNC(SEL_LEFTBUTTONPRESS,           WelcomeWindow::ID_BTNCANVAS,  WelcomeWindow::onClick),






	FXMAPFUNC(SEL_PAINT,           WelcomeWindow::ID_RIGHTCANVAS, WelcomeWindow::onPaintRight),

};

FXIMPLEMENT(WelcomeWindow, FXMainWindow, WelcomeWindowMap, ARRAYNUMBER(WelcomeWindowMap));

WelcomeWindow::WelcomeWindow(FXApp *a) : FXMainWindow(a, "Getting Started with Windows 2000", ico_mainicon, NULL, DECOR_MENU|DECOR_BORDER|DECOR_CLOSE|DECOR_TITLE|DECOR_MINIMIZE, 0,0,0,0) {
	btnhover = 0;
	titlefont = new FXFont(getApp(), "Tahoma", 12, FXFont::Bold);
	boldfont = new FXFont(getApp(), "Tahoma", 8, FXFont::Bold);
	normalfont = new FXFont(getApp(), "Tahoma", 8, FXFont::Normal);
	smallfont = new FXFont(getApp(), "Tahoma", 7, FXFont::Normal);

	new FXImageFrame(this, img_logo, FRAME_NONE);

	cont = new FXHorizontalFrame(this, LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0,0, 478,229, 0,0,0,0, 0,0);
	FXVerticalFrame* cont2 = new FXVerticalFrame(cont, LAYOUT_FILL_Y, 0,0, 0,0, 0,0,0,0, 0,0);

	leftcanvas = new FXCanvas(cont2, this, ID_LEFTCANVAS, LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0,0,179,0);
	chk = new FXCheckButton(cont2, "&Show this screen at startup.", NULL, 0,
		CHECKBUTTON_NORMAL|LAYOUT_FILL_X|JUSTIFY_LEFT|LAYOUT_BOTTOM, 0,0,0,0, 8,8,4,6);
	cont2->setBackColor(SIDECOLOR);
	chk->setBackColor(SIDECOLOR);
	chk->setFont(smallfont);
	chk->setCheck(runwelcome);

	chk->setTextColor(TEXTCOLOR);
	rightcanvas = new FXCanvas(cont, this, ID_RIGHTCANVAS, LAYOUT_FIX_WIDTH|LAYOUT_FILL_Y, 0,0,299,0);
	btncanvas = new FXCanvas(this, this, ID_BTNCANVAS, LAYOUT_EXPLICIT, 0,0,0,0);
	btncanvas->setDefaultCursor(getApp()->getDefaultCursor(DEF_HAND_CURSOR));

	addHotKey(parseAccel("Alt+E"));
	addHotKey(parseAccel("Alt+R"));
	addHotKey(parseAccel("Alt+D"));
	addHotKey(parseAccel("Alt+C"));

	//addHotKey(parseAccel("Alt+Shift+E"));


}

WelcomeWindow::~WelcomeWindow() {
	FILE* fp;
	fp = fopen(welpath, "w");

	if (fp != NULL) {
		fprintf(fp, "%d\n", !!(chk->getCheck()));
		fclose(fp);
	}
}

void WelcomeWindow::create() {
	titlefont->create();
	boldfont->create();
	normalfont->create();
	smallfont->create();

	FXMainWindow::create();

	btntextwidth = boldfont->getTextWidth("Exit");
	btncanvas->setWidth(btntextwidth+(11*2));
	btntextheight = boldfont->getTextHeight("Exit");
	btncanvas->setHeight(btntextheight+4+3);

	leftcanvas->setHeight((10+boldfont->getFontHeight()+1)*(AMOUNTBARS));
	leftcanvas->setDefaultCursor(getApp()->getDefaultCursor(DEF_HAND_CURSOR));

	btncanvas->setX(getWidth()-(btncanvas->getWidth())-8);
	btncanvas->setY(getHeight()-(btncanvas->getHeight())-8);

	/*printf("width: %d, height: %d, x: %d, y: %d\n", 
			btncanvas->getWidth(), btncanvas->getHeight(),
			btncanvas->getX(), btncanvas->getY());*/

	show(PLACEMENT_SCREEN);
}

void drawText(FXDC *dc, FXint x, FXint y, FXFont* fn, int uline, const FXString &str) {
	dc->setFont(fn);
	dc->drawText(x, y+fn->getFontAscent(), str);

	if (uline) {
		dc->fillRectangle(x, y+fn->getFontAscent()+1, fn->getCharWidth(str.text()[0]), 1);
	}

	return;
}

void drawTextWrap(FXDC *dc, FXint x, FXint y, FXFont* fn, int width, const FXString &str) {
	const char* start;
	const char* ptr;
	const char* lastspace;

	int lines = 0;

	if (str.empty()) return;

	dc->setFont(fn);
	//strcpy(text, str.text());
	start = str.text();

	ptr = start;
	lastspace = NULL;

	while (1) {
		int lw = fn->getTextWidth(start, ptr-start);

		if (lw > width && lastspace != NULL && lastspace > start) {
			int len = lastspace - start;
			dc->drawText(x, y+fn->getFontAscent()+(fn->getFontHeight()*lines), start, len);
			

			++lines;

			start = lastspace + 1;
			lastspace = NULL;
			ptr = start;
			continue;
		}

		if (*ptr == '\n' || *ptr == '\0') {
			int len = ptr - start;
			dc->drawText(x, y+fn->getFontAscent()+(fn->getFontHeight()*lines), start, len);

			++lines;

			if (*ptr == '\0') break;

			start = ptr+1;
			lastspace = NULL;
			ptr = start;
			continue;
		}

		if (*ptr == ' ') lastspace = ptr;
		++ptr;

	}

	//dc->drawText(x, y+fn->getFontAscent(), str);

	return;
}

void drawBar(FXEvent* ev, FXDC *dc, FXint width, FXint which, FXint hover, const FXString &str) {
	int fontheight = boldfont->getFontHeight();
	if (!CHECKRECT(ev, 0, (10+fontheight+1)*which, width, 10+fontheight+1)) return;
	if (hover) {
		dc->setForeground(BACKCOLOR);
		dc->fillRectangle(0, (10+fontheight+1)*which, width, 10+fontheight+1);
	} else {
		dc->setForeground(SIDECOLOR);
		dc->fillRectangle(0, (10+fontheight+1)*which, width, 10+fontheight+1);
	}

	dc->setForeground(SIDETEXTCOLOR);
	drawText(dc, 10, (10+fontheight+1)*which+5, boldfont, 1, str);

	dc->setForeground(SEPCOLOR);
	dc->fillRectangle(0,(10+fontheight+1)*(which+1)-1, width,1);

	//puts("hi");
}
long WelcomeWindow::onClick(FXObject* obj, FXSelector, void* ptr) {
	switch (hoveredbar) {
		case 1:
		case 2:
			*(int*)0 = 0;
			//FXMessageBox::error(this, MBOX_OK, "Error", "Not implemented!");
			break;
		case 3:
			system("ncpa.cpi &");
			break;
		case 255:
			getApp()->exit(0);
			break;
	}

	return 1;
}


long WelcomeWindow::onKeypressBtn(FXObject* obj, FXSelector, void* ptr) {
	FXEvent* ev = (FXEvent*)ptr;
	switch (ev->code) {
		case 'e':
			hoveredbar = 255;
			break;
		case 'r':
			hoveredbar = 1;
			break;
		case 'd':
			hoveredbar = 2;
			break;
		case 'c':
			hoveredbar = 3;
			break;
	}

	if (hoveredbar != 255) {
		leftcanvas->setFocus();
		leftcanvas->update();
		rightcanvas->update();
	}

	onClick(NULL, 0, NULL);

	return 1;
}

long WelcomeWindow::onPaintLeft(FXObject* obj, FXSelector, void* ptr) {
	//puts("hi");
	FXWindow* win = (FXWindow*)obj;
	FXEvent* ev = (FXEvent*)ptr;

	FXDCWindow dc(win, ev);

	dc.setForeground(SIDECOLOR);
	dc.fillRectangle(0, 0, width, height);

	drawBar(ev, &dc, width, 0, (hoveredbar == (0+1)), "Register Now");
	drawBar(ev, &dc, width, 1, (hoveredbar == (1+1)), "Discover Windows");
	drawBar(ev, &dc, width, 2, (hoveredbar == (2+1)), "Connect to the Internet");

	//puts("drawing");

	dc.end();
	return 1;
}

long WelcomeWindow::onPaintBtn(FXObject* obj, FXSelector, void* ptr) {
	//puts("button");
	FXWindow* win = (FXWindow*)obj;
	FXEvent* ev = (FXEvent*)ptr;

	int winwidth = win->getWidth();
	int winheight = win->getHeight();

	FXDCWindow dc(win, ev);

	dc.setForeground(BACKCOLOR);
	dc.fillRectangle(0, 0, winwidth, winheight);
	dc.setForeground(BTNCOLOR);
	dc.drawRectangle(0, 0, winwidth-1, winheight-1);
	drawText(&dc, (winwidth-btntextwidth)/2+1, (winheight-btntextheight)/2+1, boldfont, 1, "Exit");

	if (btnhover) {
		dc.setStipple(STIPPLE_GRAY, 0, 1);
		dc.setFillStyle(FILL_STIPPLED);
		dc.setForeground(FOCUSCOLOR);
		dc.drawRectangle(3, 3, winwidth-7, winheight-7);
		dc.setFillStyle(FILL_SOLID);
	}



	dc.end();
	return 1;
}


long WelcomeWindow::onEnterBtn(FXObject* obj, FXSelector, void* ptr) {
	btnhover = 1;
	hoveredbar = 255;
	btncanvas->setFocus();
	btncanvas->update();
	return 1;
}


long WelcomeWindow::onLeaveBtn(FXObject* obj, FXSelector, void* ptr) {
	hoveredbar = 0;
	btnhover = 0;
	btncanvas->update();
	return 1;
}


long WelcomeWindow::onMotionLeft(FXObject* obj, FXSelector, void* ptr) {
	FXEvent* ev = (FXEvent*)ptr;
	int fontheight = boldfont->getFontHeight();

	int hoveredbar2 = (ev->win_y / (10+fontheight+1))+1;
	if (hoveredbar2 != hoveredbar && hoveredbar2 <= (AMOUNTBARS+1)) {
		hoveredbar = hoveredbar2;
		leftcanvas->update();
		rightcanvas->update();
	}

	return 1;
}

long WelcomeWindow::onLeaveLeft(FXObject* obj, FXSelector, void* ptr) {
	hoveredbar = 0;
	leftcanvas->update();
	rightcanvas->update();

	return 1;
}


long WelcomeWindow::onPaintRight(FXObject* obj, FXSelector, void* ptr) {
	FXWindow* win = (FXWindow*)obj;
	FXEvent* ev = (FXEvent*)ptr;

	FXDCWindow dc(win, ev);

	dc.setForeground(FXRGB(255, 255, 255));
	dc.fillRectangle(0, 0, width, height);

	dc.setForeground(TITLECOLOR);
	switch(hoveredbar) {
		case 1:
			dc.drawImage(img_register, 
					(rightcanvas->getWidth())-(img_register->getWidth()),
					(rightcanvas->getHeight())-(img_register->getHeight()));
			drawText(&dc, 16, 11, titlefont, 0, "Register Windows 2000");
			dc.setForeground(TEXTCOLOR);
			drawTextWrap(&dc, 16, 37, normalfont, 260,
					"Register your copy of Windows 2000 Professional and help Microsoft give you:\n"
					"\n"
					"• The best possible product support\n"
					"• The right software upgrades\n"
					"• The most relevant new product information");
			break;
		case 2:
			dc.drawImage(img_discover, 
					(rightcanvas->getWidth())-(img_discover->getWidth()),
					(rightcanvas->getHeight())-(img_discover->getHeight()));
			drawText(&dc, 16, 11, titlefont, 0, "Discover Windows 2000");
			dc.setForeground(TEXTCOLOR);
			drawTextWrap(&dc, 16, 37, normalfont, 260,
					"Windows 2000 Professional is easier to use, easier to manage, more compatible, "
					"and more powerful. The Discover Windows 2000 Professional tour highlights all "
					"the newest and coolest features.");
			break;
		case 3:
			dc.drawImage(img_connect, 
					(rightcanvas->getWidth())-(img_connect->getWidth()),
					(rightcanvas->getHeight())-(img_connect->getHeight()));
			drawText(&dc, 16, 11, titlefont, 0, "Connect to the Internet");
			dc.setForeground(TEXTCOLOR);
			drawTextWrap(&dc, 16, 37, normalfont, 260,
					"• Read the latest news and financial information\n"
					"• Visit Web sites around the world\n"
					"• Get e-mail\n"
					"\n"
					"We make it quick and simple for you to connect to the internet.");
			break;
		default:
			drawText(&dc, 16, 11, titlefont, 0, "Getting Started");
			dc.setForeground(TEXTCOLOR);
			drawTextWrap(&dc, 16, 37, normalfont, 260, "You can access all your documents, "
					"programs and settings from the start button.");

			dc.drawImage(img_welcome, 17, 71);
	}


	dc.end();
	return 1;
}

const char* getHomeDir() {
	const char *homedir;

	if ((homedir = getenv("HOME")) == NULL) {
		homedir = getpwuid(getuid())->pw_dir;
	}

	return homedir;
}

int main(int argc, char *argv[]) {
	FILE* fp;

	snprintf(welpath, sizeof(welpath), "%s/.icewm/cfg/ice2k/runwelcome", getHomeDir());

	fp = fopen(welpath, "r");

	if (fp != NULL) {
		runwelcome = (fgetc(fp) != '0');
		fclose(fp);
	}

	FXApp application("Welcome", "I2KProject");
	application.init(argc, argv);

	ico_mainicon = new FXGIFIcon(&application, res_ico_mainicon);
	img_connect = new FXGIFImage(&application, res_img_connect);
	img_connect->create();
	img_discover = new FXGIFImage(&application, res_img_discover);
	img_discover->create();
	img_logo = new FXGIFImage(&application, res_img_logo);
	img_register = new FXGIFImage(&application, res_img_register);
	img_register->create();
	img_welcome = new FXGIFImage(&application, res_img_welcome);
	img_welcome->create();

	new WelcomeWindow(&application);

	application.create();
	return application.run();
}
