// this serves as an example on a basic ice2k program
// 
// the comments are maybe a bit unneccesary but maybe makes things
// more clear for someone
//
// based off scribble example

#include <fx.h>

// optional
// common controls library, replaces certain fox toolkit controls with
// more accurate widnows ones
//
// if you do not want to override fox toolkit controls use
// #define _DO_NOT_OVERRIDE_FOX_CONTROLS
//
// before including common controls!

// #include <ice2k/comctl32.h>


// if you are using one, you include your resources header here
#include "res/foxres.h"

FXIcon* mainIcon;

// window class
class HelloWindow : public FXMainWindow {
	FXDECLARE(HelloWindow);

protected:
	// required in every fox window class you define
	HelloWindow() {}

private:
	// put all the controls you will use for the class here
	FXHorizontalFrame *contents;

public:
	// class functions
	long onCmdHello(FXObject*, FXSelector, void*);

public:
	// message ids
	enum {
		ID_MAINWIN = FXMainWindow::ID_LAST,
		ID_HELLO,
		ID_LAST
	};

public:

	// constructor
	HelloWindow(FXApp* a);

	virtual void create();
	virtual ~HelloWindow();
};



// message map
FXDEFMAP(HelloWindow) HelloWindowMap[] = {
	FXMAPFUNC(SEL_COMMAND,           HelloWindow::ID_HELLO,  HelloWindow::onCmdHello),
};

FXIMPLEMENT(HelloWindow, FXMainWindow, HelloWindowMap, ARRAYNUMBER(HelloWindowMap));

// constructs window
HelloWindow::HelloWindow(FXApp *a) : FXMainWindow(a, "Hello World!", mainIcon, NULL, DECOR_ALL, 0,0,0,0) {
	new FXButton(this, "Hello World!", NULL, this, ID_HELLO, BUTTON_NORMAL);
}

// deconstructs window
// 
// you will need to put icons and menus here, if you loaded them in
// the constructor

HelloWindow::~HelloWindow() {
}


// i feel like the rest is self explanitory
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




