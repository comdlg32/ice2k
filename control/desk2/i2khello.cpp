#include <fx.h>
// #include <ice2k/comctl32.h>
#include "res/foxres.h"

#define IMG_MODE_TILED 0
#define IMG_MODE_CENTER 1
#define IMG_MODE_STRETCH 2
#define IMG_MODE_FILL 3

FXIcon* mainIcon;

class HelloWindow : public FXMainWindow {
	FXDECLARE(HelloWindow);

protected:
	HelloWindow() {}

private:
	FXHorizontalFrame *contents;

public:
	long onCmdHello(FXObject*, FXSelector, void*);

	void writeBackCfg(const FXString&, FXColor, unsigned);

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


void writeBackCfg(const FXString& str, FXColor col, unsigned mode) {
	FILE* fp = fopen((FXSystem::getHomeDirectory()+"/.icewm/cfg/backmgr.ini").text(), "w");

	if (fp == NULL) return;

	fprintf(fp, "[Wallpaper]");

	switch (mode) {
		case IMG_MODE_CENTER:
			fprintf(fp, "Mode=Center");
			break;
		case IMG_MODE_STRETCH:
			fprintf(fp, "Mode=Stretch");
			break;
		case IMG_MODE_FILL:
			fprintf(fp, "Mode=Fill");
			break;

		case IMG_MODE_TILED:
		default:
			fprintf(fp, "Mode=Tiled");
	}
	fprintf("Color=#%02X%02X%02X", FXREDVAL(col), FXGREENVAL(col), FXBLUEVAL(col));

	fclose(fp);

}

HelloWindow::HelloWindow(FXApp *a) : FXMainWindow(a, "Hello World!", mainIcon, NULL, DECOR_ALL, 0,0,0,0) {
	new FXButton(this, "Hello World!", NULL, this, ID_HELLO, BUTTON_NORMAL);
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
