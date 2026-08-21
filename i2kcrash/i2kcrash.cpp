#include <fx.h>
#include <FXPNGIcon.h>

#include <ice2k/comctl32.h>
#include <ice2k/branding.h>

#include <ice2k/wizard/I2KWizHeader.h>


#include "res/foxres.h"

//FXIcon* ico_main16;

FXIcon* ico_error;

#define CRASHDLG_STYLE_9X 0
#define CRASHDLG_STYLE_2K 1

int style = CRASHDLG_STYLE_2K;

class CrashWindow : public FXMainWindow {
	FXDECLARE(CrashWindow);

protected:
	CrashWindow() {}

private:
	FXFont* boldfont;
	FXVerticalFrame* contents;
	FXVerticalFrame* btncont;

	FXHorizontalFrame* horcont;
	FXText* text;


public:
	long onCmdHello(FXObject*, FXSelector, void*);

public:
	enum {
		ID_MAINWIN = FXMainWindow::ID_LAST,
		ID_HELLO,
		ID_LAST
	};

public:
	CrashWindow(FXApp* a);

	virtual void create();
	void setFocus() {};
	virtual ~CrashWindow();
};

FXDEFMAP(CrashWindow) CrashWindowMap[] = {
	FXMAPFUNC(SEL_COMMAND,           CrashWindow::ID_HELLO,  CrashWindow::onCmdHello),
};

FXIMPLEMENT(CrashWindow, FXMainWindow, CrashWindowMap, ARRAYNUMBER(CrashWindowMap));

CrashWindow::CrashWindow(FXApp *a) : FXMainWindow(a, "a.out", NULL, NULL, DECOR_CLOSE|DECOR_TITLE|DECOR_BORDER, 0,0,0,0) {
	/*new I2KWizHeader(this, NULL, "test.exe has encountered a problem and needs to\nclose. We are sorry for the inconvenience.",
			"", LAYOUT_FIX_WIDTH|LAYOUT_FILL_X, 0,0,415,0);*/

	if (style == CRASHDLG_STYLE_9X) {
		boldfont = NULL;
		horcont = new FXHorizontalFrame(this, LAYOUT_FILL, 0,0,0,0, 9,9,9,9, 4,4);
		new FXLabel(horcont, "", ico_error);
		text = new FXText(horcont, NULL, 0, LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|TEXT_WORDWRAP, 0,0, 255+5+2,0);
		text->setText("This program has performed an illegal operation and will be shut down.\n"
				"\n"
				"If the problem persists, contact the program vendor.");
		text->disable();
		text->setMarginTop(1);
		text->setMarginLeft(5);
		text->setMarginBottom(8);
		text->setMarginRight(2);
		text->setBackColor(getApp()->getBaseColor());
		text->setDefaultCursor(getApp()->getDefaultCursor(DEF_ARROW_CURSOR));

		btncont = new FXVerticalFrame(horcont, LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH, 0,0,0,0, 0,0,2,2);

		FXButton* closebtn = new FXButton(btncont, "&Close", NULL, getApp(), FXApp::ID_QUIT, BUTTON_NORMAL|BUTTON_DEFAULT, 0,0,0,0, 12,11,2,3);
		closebtn->setFocus();
		FXButton* detbtn = new FXButton(btncont, "&Details>>", NULL, NULL, 0, BUTTON_NORMAL|BUTTON_DEFAULT|LAYOUT_BOTTOM, 0,0,0,0, 12,11,2,3);
		detbtn->disable();





	} else {
		FXFontDesc desc;
		getApp()->getNormalFont()->getFontDesc(desc);
		desc.weight = FXFont::Bold;

		boldfont = new FXFont(getApp(), desc);

		FXHorizontalFrame* header = new FXHorizontalFrame(this, LAYOUT_FILL_X, 0,0,0,0, 21,8,13,12, 8,8);
		FXLabel* headerlbl = new FXLabel(header, "test.exe has encountered a problem and needs to close.\n"
				"We are sorry for the inconvenience.", NULL, JUSTIFY_LEFT|LAYOUT_CENTER_Y);
		headerlbl->setFont(boldfont);

		new FXHorizontalSeparator(this, SEPARATOR_GROOVE|LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0);

		header->setBackColor(getApp()->getBackColor());
		headerlbl->setBackColor(getApp()->getBackColor());

		contents = new FXVerticalFrame(this, LAYOUT_FILL_Y|LAYOUT_FILL_X, 0,0,0,0, 24,11,8,8, 12,12);

		new FXLabel(contents, "If you were in the middle of something, the information you were working on\n"
				"might be lost.", NULL, JUSTIFY_LEFT, 0,0,0,0, 7,4,2,2);
		//new FXLabel(contents, "We have saved a dump of the program at ...", NULL, JUSTIFY_LEFT);

		FXButton* btn = new FXButton(contents, "&Close", NULL, getApp(), FXApp::ID_QUIT,
				LAYOUT_RIGHT|BUTTON_NORMAL|BUTTON_DEFAULT, 0,0,0,0, 23,22,2,3);

		btn->setFocus();
	}
}


CrashWindow::~CrashWindow() {
	if (boldfont != NULL) {
		delete boldfont;
		boldfont = NULL;
	}
}

void CrashWindow::create() {
	FXMainWindow::create();

	if (style == CRASHDLG_STYLE_9X) {
		text->setHeight(text->getContentHeight());
		setHeight(getDefaultHeight());
		setWidth(getDefaultWidth());
	}
	show(PLACEMENT_SCREEN);
}

long CrashWindow::onCmdHello(FXObject*, FXSelector, void*) {
	puts("Hello world!");
	return 1;
}

int main(int argc, char *argv[]) {
	char str[8];
	FXApp application("CrashHandler", "I2KProject");

	application.init(argc, argv);
	//ico_main16 = new FXGIFIcon(&application, resico_mainicon, 0, IMAGE_OPAQUE);

	if (i2kBGetWinVersionInt() >= ICE2K_BRAND_WINXP) {
		ico_error = new FXPNGIcon(&application, resico_xp_error);
		ico_error->blend(application.getBaseColor());
	} else {
		ico_error = new FXGIFIcon(&application, resico_2k_error);
	}

	strncpy(str,
			application.reg().readStringEntry("ICE2K", "crashdlg", "9x"),
			sizeof(str)-1);

	str[sizeof(str)-1] = '\0';

	if (strcmp(str, "9x") == 0) {
		style = CRASHDLG_STYLE_9X;
	}

	CrashWindow* win = new CrashWindow(&application);
	if (argv[1] != NULL && argv[1][0] != '\0') {
		win->setTitle(argv[1]);
	}

	application.create();
	return application.run();
}
