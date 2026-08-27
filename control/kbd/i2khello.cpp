#include <fx.h>
// #include <ice2k/comctl32.h>

#include "res/foxres.h"

FXIcon* mainIcon;

class HelloWindow : public FXMainWindow {
	FXDECLARE(HelloWindow);

protected:
	HelloWindow() {}

private:
	FXVerticalFrame* cont;
	FXTabBook* tabbook;
	FXVerticalFrame* speedcont;

public:
	long onCmdHello(FXObject*, FXSelector, void*);

public:
	enum {
		ID_MAINWIN = FXMainWindow::ID_LAST,
		ID_HELLO,
		ID_LAST
	};

public:
	HelloWindow(FXApp* a);

	virtual void create();
	virtual ~HelloWindow();
};

FXDEFMAP(HelloWindow) HelloWindowMap[] = {
	FXMAPFUNC(SEL_COMMAND,           HelloWindow::ID_HELLO,  HelloWindow::onCmdHello),
};

FXIMPLEMENT(HelloWindow, FXMainWindow, HelloWindowMap, ARRAYNUMBER(HelloWindowMap));

HelloWindow::HelloWindow(FXApp *a) : FXMainWindow(a, "Hello World!", mainIcon, NULL, DECOR_CLOSE|DECOR_BORDER|DECOR_TITLE, 0,0,0,0, 0,0,3,3, 0,6) {
	cont = new FXVerticalFrame(this, LAYOUT_FILL_Y|LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0, 0,0);
	
	tabbook = new FXTabBook(cont, NULL, 0, TABBOOK_NORMAL|LAYOUT_FILL, 0,0,0,0, 6,6,5,5);
	new FXTabItem(tabbook, "Power Schemes", NULL, TAB_TOP_NORMAL, 0,0,0,0, 4,4,1,2);
	speedcont = new FXVerticalFrame(tabbook, LAYOUT_FILL|FRAME_RAISED|FRAME_THICK, 0,0,0,0, 13,2,11,10, 8,8);
	
}

HelloWindow::~HelloWindow() {
}

void HelloWindow::create() {
	FXMainWindow::create();

	show(PLACEMENT_SCREEN);
}

long HelloWindow::onCmdHello(FXObject*, FXSelector, void*) {
	puts("Hello world!");
	return 1;
}

int main(int argc, char *argv[]) {
	FXApp application("Hello", "I2KTest");
	mainIcon = new FXGIFIcon(&application, resico_mainicon, 0, IMAGE_OPAQUE);

	application.init(argc, argv);

	new HelloWindow(&application);

	application.create();
	return application.run();
}
