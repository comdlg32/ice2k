#include <fx.h>
#include <ice2k/comctl32.h>

#include "res/foxres.h"

FXIcon*  ico_main32;
FXImage* img_bombspr_og;
FXImage* img_bombspr = NULL;

FXMainWindow* mainwin;
FXCanvas* canvas = NULL;


#define CRASH_SEGFAULT 0
#define CRASH_HANG 1

int imgscale = 1;
int crashmethod = CRASH_SEGFAULT;


#define BOMB_WIDTH 32
#define BOMB_HEIGHT 32

#define BOMB_HORFRAMES 11
#define BOMB_VERFRAMES 4
#define BOMB_DELAY 100

int explodecount = 0;

int bomb_verframe = 0;
int bomb_horframe = 0;

FXImage* resizeSprite(FXApp* app, FXImage* img, FXint scale) {
	FXImage* out;
	
	int sprwidth = img->getWidth();
	int sprheight = img->getHeight();

	out = new FXImage(app, NULL, IMAGE_OWNED,
			sprwidth, sprheight);

	memcpy(out->getData(), img->getData(),
			sizeof(FXColor)*(sprwidth*sprheight));

	out->scale(sprwidth*scale, sprheight*scale, 0);

	return out;
}

void resizeBombSprite(FXApp* app, FXint scale) {
	if (img_bombspr != NULL) delete img_bombspr;
	img_bombspr = resizeSprite(app, img_bombspr_og, scale);

	img_bombspr->create();

	imgscale = scale;
}

class AboutBox: public FXDialogBox {
	FXDECLARE(AboutBox);
protected:
	AboutBox() {};
public:
	AboutBox(FXWindow* owner): FXDialogBox(owner,"About Bad App",DECOR_TITLE|DECOR_BORDER|DECOR_CLOSE|DECOR_MENU, 0,0,0,0, 8,8,8,8, 8,10) {
		FXHorizontalFrame* cont = new FXHorizontalFrame(this, LAYOUT_SIDE_TOP, 0,0,0,0, 4,4,4,4, 8,8);

		new FXLabel(cont, "", ico_main32);

		new FXLabel(cont,
		"Bad App Version 0.0.1\n\n"
		"Demonstrate Microsoft Windows 3.1 handling\n"
		"of bad applications.\n\n"
		"Written by xcomposite",
		NULL, JUSTIFY_LEFT);

		FXButton* okbtn = new FXButton(this, "OK", NULL, this, ID_ACCEPT,
		BUTTON_DEFAULT|BUTTON_INITIAL|LAYOUT_RIGHT|FRAME_THICK|FRAME_RAISED,
		0,0,75,23, 26,26,2,3);

		okbtn->setFocus();
	}
	virtual void create() { FXDialogBox::create(); };
	void setFocus() {};
	virtual ~AboutBox() {};
};

FXIMPLEMENT(AboutBox, FXDialogBox, NULL, 0);


class OptionsBox: public FXDialogBox {
	FXDECLARE(OptionsBox);
protected:
	OptionsBox() {};
private:
	FXCheckButton* sndbox;
	FXHorizontalFrame* horcont;
	FXVerticalFrame* leftcont;
	FXVerticalFrame* rightcont;
	FXHorizontalFrame* zoomcont;
	FXSpinner* spin;
	FXHorizontalFrame* titlecont;
	FXTextField* text;


	FXButton* okbtn;

public:
	long onCmdAccept(FXObject* obj, FXSelector sel, void* ptr) {
		mainwin->setTitle(text->getText());
		getApp()->reg().writeStringEntry("Settings", "Title", text->getText().text());
		getApp()->reg().writeIntEntry("Settings", "Zoom", spin->getValue());

		resizeBombSprite(getApp(), spin->getValue());
		getApp()->reg().write();

		int canwidth = (32*imgscale)+13+14;
		if (canwidth < 144) {
			canwidth = 144;
		}

		canvas->setHeight((32*imgscale)+13+14);
		canvas->setWidth(canwidth);

		mainwin->recalc();
		mainwin->resize(mainwin->getDefaultWidth(), mainwin->getDefaultHeight());

		return FXDialogBox::onCmdAccept(obj, sel, ptr);
	}

	OptionsBox(FXWindow* owner): FXDialogBox(owner,"Options",DECOR_TITLE|DECOR_BORDER|DECOR_CLOSE|DECOR_MENU, 0,0,0,0, 4,8,8,8, 8,10) {
		horcont = new FXHorizontalFrame(this, LAYOUT_SIDE_TOP, 0,0,0,0, 0,0,0,0, 8,8);
		leftcont  = new FXVerticalFrame(horcont, LAYOUT_SIDE_TOP, 0,0,0,0, 0,0,0,0, 5,5);
		rightcont = new FXVerticalFrame(horcont, PACK_UNIFORM_WIDTH, 0,0,0,0, 0,0,0,0, 3,3);

		sndbox = new FXCheckButton(leftcont, "&Sound");
		sndbox->disable();

		zoomcont = new FXHorizontalFrame(leftcont, LAYOUT_SIDE_TOP, 0,0,0,0, 0,0,0,0, 0,0);
		new FXLabel(zoomcont, "&Zoom Factor: ", NULL, LABEL_NORMAL|LAYOUT_CENTER_Y);

		spin = new FXSpinner(zoomcont, 5, NULL, 0, SPIN_NORMAL|FRAME_NORMAL|LAYOUT_CENTER_Y, 0,0,0,0, 1,1,1,1);
		spin->setRange(1, 10);
		spin->setValue(imgscale);

		titlecont = new FXHorizontalFrame(leftcont, LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0, 0,0);
		new FXLabel(titlecont, "Title: ", NULL, LABEL_NORMAL|LAYOUT_CENTER_Y);
		text = new FXTextField(titlecont, 6, this, ID_ACCEPT, TEXTFIELD_NORMAL|TEXTFIELD_ENTER_ONLY|LAYOUT_FILL_X, 0,0,0,0, 2,2,1,1);
		text->setText(mainwin->getTitle());

		okbtn = new FXButton(rightcont, "OK", NULL, this, ID_ACCEPT,
			BUTTON_DEFAULT|BUTTON_INITIAL|LAYOUT_RIGHT|FRAME_THICK|FRAME_RAISED,
			0,0,75,23, 12,12,2,3);

		new FXButton(rightcont, "Cancel", NULL, this, ID_CANCEL,
			BUTTON_DEFAULT|BUTTON_INITIAL|LAYOUT_RIGHT|FRAME_THICK|FRAME_RAISED,
			0,0,75,23, 12,12,2,3);

		okbtn->setFocus();
	}
	virtual void create() { FXDialogBox::create(); };
	void setFocus() {};
	virtual ~OptionsBox() {};
};

FXDEFMAP(OptionsBox) OptionsBoxMap[] = {
	FXMAPFUNC(SEL_COMMAND,         OptionsBox::ID_ACCEPT,   OptionsBox::onCmdAccept),
};

FXIMPLEMENT(OptionsBox, FXDialogBox, OptionsBoxMap, ARRAYNUMBER(OptionsBoxMap));

class BadApp : public FXMainWindow {
	FXDECLARE(BadApp);

protected:
	BadApp() {}

private:
	FXMenuBar* menubar;
	FXMenuPane* actionpane;
	FXMenuPane* helppane;
	
	FXHorizontalFrame *contents;

public:
	long onPaintCanvas(FXObject*, FXSelector, void*);
	long onTimeoutCanvas(FXObject*, FXSelector, void*);

	long onPressCanvas(FXObject*, FXSelector, void*);

	long onCmdAbout(FXObject*, FXSelector, void*);
	long onCmdOptions(FXObject*, FXSelector, void*);
	long onCmdHang(FXObject*, FXSelector, void*);
	long onCmdSegfault(FXObject*, FXSelector, void*);





public:
	enum {
		ID_CANVAS = FXMainWindow::ID_LAST,
		ID_ABOUT,
		ID_OPTIONS,
		ID_HANG,
		ID_SEGFAULT,
		ID_LAST
	};

public:
	BadApp(FXApp* a);

	virtual void create();
	void setFocus() {};
	virtual ~BadApp();
};

FXDEFMAP(BadApp) BadAppMap[] = {
	FXMAPFUNC(SEL_PAINT,           BadApp::ID_CANVAS,  BadApp::onPaintCanvas),
	FXMAPFUNC(SEL_LEFTBUTTONPRESS, BadApp::ID_CANVAS,  BadApp::onPressCanvas),
	FXMAPFUNC(SEL_TIMEOUT,         BadApp::ID_CANVAS,  BadApp::onTimeoutCanvas),

	FXMAPFUNC(SEL_COMMAND,         BadApp::ID_ABOUT,   BadApp::onCmdAbout),
	FXMAPFUNC(SEL_COMMAND,         BadApp::ID_OPTIONS, BadApp::onCmdOptions),

	FXMAPFUNC(SEL_COMMAND,         BadApp::ID_SEGFAULT, BadApp::onCmdSegfault),
	FXMAPFUNC(SEL_COMMAND,         BadApp::ID_HANG, BadApp::onCmdHang),
};

FXIMPLEMENT(BadApp, FXMainWindow, BadAppMap, ARRAYNUMBER(BadAppMap));

BadApp::BadApp(FXApp *a) : FXMainWindow(a, "Bad App", NULL, ico_main32, DECOR_CLOSE|DECOR_BORDER|DECOR_MENU|DECOR_MINIMIZE|DECOR_TITLE, 0,0,0,0) {
	menubar = new FXMenuBar(this, LAYOUT_FILL_X, 0,0,0,0, 0,0,0,1, 0,0);

	actionpane = new FXMenuPane(this);
	new FXMenuCommand(actionpane, "&Seg-Fault", NULL, this, ID_SEGFAULT);
	new FXMenuCommand(actionpane, "&Hang", NULL, this, ID_HANG);
	new FXMenuSeparator(actionpane);
	new FXMenuCommand(actionpane, "&Options...", NULL, this, ID_OPTIONS);
	
	helppane = new FXMenuPane(this);
	new FXMenuCommand(helppane, "&About...", NULL, this, ID_ABOUT);

	new FXMenuTitle(menubar, "&Action", NULL, actionpane);
	new FXMenuTitle(menubar, "&Help", NULL, helppane);

	canvas = new FXCanvas(this, this, ID_CANVAS, LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT);

	int canwidth = (32*imgscale)+13+14;
	if (canwidth < 144) {
		canwidth = 144;
	}

	canvas->setHeight((32*imgscale)+13+14);
	canvas->setWidth(canwidth);
}

BadApp::~BadApp() {
	delete actionpane;
	delete helppane;
}

void BadApp::create() {
	FXMainWindow::create();

	show(PLACEMENT_SCREEN);
}

long BadApp::onPaintCanvas(FXObject* obj, FXSelector, void* ptr) {
	FXWindow* win = (FXWindow*)obj;
	FXEvent* ev = (FXEvent*)ptr;

	int winwidth = win->getWidth();
	int winheight = win->getHeight();

	FXDCWindow dc(win, ev);

	dc.setForeground(FXRGB(255,255,255));
	dc.fillRectangle(ev->rect.x, ev->rect.y, ev->rect.w, ev->rect.h);

	int imgwidth  = (BOMB_WIDTH*imgscale);
	int imgheight = (BOMB_HEIGHT*imgscale);

	dc.drawArea(img_bombspr,
			bomb_horframe*imgwidth,
			bomb_verframe*imgheight,
			
			imgwidth,
			imgheight,
			
			(winwidth-imgwidth)>>1,
			(winheight-imgheight)>>1);

	dc.end();
	return 1;
}

long BadApp::onPressCanvas(FXObject*, FXSelector, void*) {
	crashmethod = CRASH_SEGFAULT;

	FXWindow* win = actionpane->getFirst();

	while (win != NULL) {
		win->disable();
		win = win->getNext();
	}

	getApp()->addTimeout(this, ID_CANVAS, BOMB_DELAY);

	return 1;
}
long BadApp::onCmdSegfault(FXObject*, FXSelector, void*) {
	crashmethod = CRASH_SEGFAULT;

	FXWindow* win = actionpane->getFirst();

	while (win != NULL) {
		win->disable();
		win = win->getNext();
	}

	getApp()->addTimeout(this, ID_CANVAS, BOMB_DELAY);
	
	return 1;
}
long BadApp::onCmdHang(FXObject*, FXSelector, void*) {
	crashmethod = CRASH_HANG;

	FXWindow* win = actionpane->getFirst();

	while (win != NULL) {
		win->disable();
		win = win->getNext();
	}

	getApp()->addTimeout(this, ID_CANVAS, BOMB_DELAY);
	
	return 1;
}


long BadApp::onTimeoutCanvas(FXObject*, FXSelector, void*) {
	if (++bomb_horframe == BOMB_HORFRAMES) {
		bomb_horframe = 0;

		if (++bomb_verframe == BOMB_VERFRAMES) {
			bomb_verframe = 3;
			bomb_horframe = 7;
		}
	}

	canvas->update();
	if (bomb_horframe == 9) {
		if (bomb_verframe == BOMB_VERFRAMES-1) {
			if (++explodecount == 4) {
				if (crashmethod == CRASH_HANG) {
					while (1) {};
				} else {
					*(int*)0 = 0;
				}
			}
		}
	}
	getApp()->addTimeout(this, ID_CANVAS, BOMB_DELAY);
	//printf("%d\n", scantext->getBottomLine());

	return 1;
}

long BadApp::onCmdAbout(FXObject*, FXSelector, void*) {
	AboutBox dlg(this);
	dlg.execute();
	return 1;
}
long BadApp::onCmdOptions(FXObject*, FXSelector, void*) {
	OptionsBox dlg(this);
	dlg.execute();
	return 1;
}


int main(int argc, char *argv[]) {
	FXApp application("BadApp", "I2KProject");

	application.init(argc, argv);
	ico_main32 = new FXGIFIcon(&application, res_ico_main32);
	img_bombspr_og = new FXGIFImage(&application, res_img_bombspr);

	int zoom = application.reg().readIntEntry("Settings", "Zoom", 1);
	if (zoom < 1) {
		zoom = 1;
	} else if (zoom > 10) {
		zoom = 10;
	}

	resizeBombSprite(&application, zoom);


	mainwin = new BadApp(&application);
	mainwin->setTitle(application.reg().readStringEntry("Settings", "Title", "Bad App"));

	application.create();
	return application.run();
}
