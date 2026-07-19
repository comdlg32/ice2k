#include <fx.h>
#include <ice2k/comctl32.h>
#include <ice2k/wizard/I2KWizHeader.h>


#include "res/foxres.h"

FXIcon* mainIcon;

class CrashWindow : public FXMainWindow {
	FXDECLARE(CrashWindow);

protected:
	CrashWindow() {}

private:
	FXFont* boldfont;
	FXVerticalFrame* contents;

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
	virtual ~CrashWindow();
};

FXDEFMAP(CrashWindow) CrashWindowMap[] = {
	FXMAPFUNC(SEL_COMMAND,           CrashWindow::ID_HELLO,  CrashWindow::onCmdHello),
};

FXIMPLEMENT(CrashWindow, FXMainWindow, CrashWindowMap, ARRAYNUMBER(CrashWindowMap));

CrashWindow::CrashWindow(FXApp *a) : FXMainWindow(a, "Hello World!", mainIcon, NULL, DECOR_CLOSE|DECOR_TITLE|DECOR_BORDER|DECOR_MENU, 0,0,0,0) {
	/*new I2KWizHeader(this, NULL, "test.exe has encountered a problem and needs to\nclose. We are sorry for the inconvenience.",
			"", LAYOUT_FIX_WIDTH|LAYOUT_FILL_X, 0,0,415,0);*/

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

CrashWindow::~CrashWindow() {
	if (boldfont) {
		delete boldfont;
		boldfont = NULL;
	}
}

void CrashWindow::create() {
	FXMainWindow::create();

	show(PLACEMENT_SCREEN);
}

long CrashWindow::onCmdHello(FXObject*, FXSelector, void*) {
	puts("Hello world!");
	return 1;
}

int main(int argc, char *argv[]) {
	FXApp application("CrashHandler", "I2KTest");
	mainIcon = new FXGIFIcon(&application, resico_mainicon, 0, IMAGE_OPAQUE);

	application.init(argc, argv);

	new CrashWindow(&application);

	application.create();
	return application.run();
}
