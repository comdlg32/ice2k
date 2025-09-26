/********************************************************************************
*                                                                               *
*                         Scribble  Application coding sample                   *
*                                                                               *
********************************************************************************/
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


// if you are using one, you include your resources file here
#include "res/foxres.h"

FXIcon* mainIcon;

// Main Window
class ScribbleWindow : public FXMainWindow {

	// Macro for class hierarchy declarations
	FXDECLARE(ScribbleWindow)

private:
	FXHorizontalFrame *contents;                // Content frame
	FXVerticalFrame   *canvasFrame;             // Canvas frame
	FXVerticalFrame   *buttonFrame;             // Button frame
	FXCanvas          *canvas;                  // Canvas to draw into
	int                mdflag;                  // Mouse button down?
	int                dirty;                   // Canvas has been painted?
	FXColor            drawColor;               // Color for the line

protected:
	ScribbleWindow() {}

public:
	// Message handlers
	long onPaint(FXObject*, FXSelector, void*);
	long onMouseDown(FXObject*, FXSelector, void*);
	long onMouseUp(FXObject*, FXSelector, void*);
	long onMouseMove(FXObject*, FXSelector, void*);
	long onCmdClear(FXObject*, FXSelector, void*);
	long onUpdClear(FXObject*, FXSelector, void*);

public:
	// Messages for our class
	enum {
		ID_CANVAS = FXMainWindow::ID_LAST,
		ID_CLEAR,
		ID_LAST
	};

public:

	// ScribbleWindow's constructor
	ScribbleWindow(FXApp* a);

	// Initialize
	virtual void create();

	virtual ~ScribbleWindow();
};



// Message Map for the Scribble Window class
FXDEFMAP(ScribbleWindow) ScribbleWindowMap[] = {
	FXMAPFUNC(SEL_PAINT,             ScribbleWindow::ID_CANVAS, ScribbleWindow::onPaint),
	FXMAPFUNC(SEL_LEFTBUTTONPRESS,   ScribbleWindow::ID_CANVAS, ScribbleWindow::onMouseDown),
	FXMAPFUNC(SEL_LEFTBUTTONRELEASE, ScribbleWindow::ID_CANVAS, ScribbleWindow::onMouseUp),
	FXMAPFUNC(SEL_MOTION,            ScribbleWindow::ID_CANVAS, ScribbleWindow::onMouseMove),
	FXMAPFUNC(SEL_COMMAND,           ScribbleWindow::ID_CLEAR,  ScribbleWindow::onCmdClear),
	FXMAPFUNC(SEL_UPDATE,            ScribbleWindow::ID_CLEAR,  ScribbleWindow::onUpdClear),
};



// Macro for the ScribbleApp class hierarchy implementation
FXIMPLEMENT(ScribbleWindow, FXMainWindow, ScribbleWindowMap, ARRAYNUMBER(ScribbleWindowMap))



// Construct a ScribbleWindow
ScribbleWindow::ScribbleWindow(FXApp *a) : FXMainWindow(a, "Scribble Application", mainIcon, NULL, DECOR_ALL, 0,0,800,600) {

	FX7Segment* seven = new FX7Segment(this, "010", SEVENSEGMENT_NORMAL, 0,0,0,0, 0,0,0,0);

	seven->setBackColor(FXRGB(0,0,0));
	seven->setTextColor(FXRGB(255,0,0));

	seven->setCellWidth(11);
	seven->setCellHeight(21);



}


ScribbleWindow::~ScribbleWindow() {
}


// Create and initialize
void ScribbleWindow::create() {
	// Create the windows
	FXMainWindow::create();

	// Make the main window appear
	show(PLACEMENT_SCREEN);
}



// Mouse button was pressed somewhere
long ScribbleWindow::onMouseDown(FXObject*, FXSelector, void*) {
	canvas->grab();

	// While the mouse is down, we'll draw lines
	mdflag = 1;

	return 1;
}



// The mouse has moved, draw a line
long ScribbleWindow::onMouseMove(FXObject*, FXSelector, void* ptr) {
	FXEvent *ev =(FXEvent*) ptr;

	// Draw
	if (mdflag) {

		// Get DC for the canvas
		FXDCWindow dc(canvas);

		// Set foreground color
		dc.setForeground(drawColor);

		// Draw line
		dc.drawLine(ev->last_x, ev->last_y, ev->win_x, ev->win_y);

		// We have drawn something, so now the canvas is dirty
		dirty = 1;
	}

	return 1;
}



// The mouse button was released again
long ScribbleWindow::onMouseUp(FXObject*, FXSelector, void* ptr) {
	FXEvent *ev =(FXEvent*) ptr;

	canvas->ungrab();
	if (mdflag) {
		FXDCWindow dc(canvas);

		dc.setForeground(drawColor);
		dc.drawLine(ev->last_x, ev->last_y, ev->win_x, ev->win_y);

		// We have drawn something, so now the canvas is dirty
		dirty = 1;

		// Mouse no longer down
		mdflag = 0;
	}
	return 1;
}


// Paint the canvas
long ScribbleWindow::onPaint(FXObject*, FXSelector, void* ptr) {
	FXEvent *ev =(FXEvent*) ptr;
	FXDCWindow dc(canvas, ev);
	dc.setForeground(canvas->getBackColor());
	dc.fillRectangle(ev->rect.x, ev->rect.y, ev->rect.w, ev->rect.h);

	return 1;
}


// Handle the clear message
long ScribbleWindow::onCmdClear(FXObject*, FXSelector, void*) {
	FXDCWindow dc(canvas);
	dc.setForeground(canvas->getBackColor());
	dc.fillRectangle(0, 0, canvas->getWidth(), canvas->getHeight());
	dirty = 0;

	return 1;
}



// Update the clear button:- each gui element (widget) in FOX
// receives a message during idle processing asking it to be updated.
// For example, buttons can be sensitized or desensitized when the
// state of the application changes.
// In this case, we desensitize the sender (the clear button) when
// the canvas has already been cleared, and sensitize it when it has
// been painted (as indicated by the dirty flag).
long ScribbleWindow::onUpdClear(FXObject* sender, FXSelector, void*) {
	
	if (dirty)
		sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
	else
		sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);

	return 1;
}


// Here we begin
int main(int argc, char *argv[]) {
	// Make application
	FXApp application("Scribble", "FoxTest");

	// Load program icon
	mainIcon = new FXGIFIcon(&application, resico_mainicon, 0, IMAGE_OPAQUE);

	// Start app
	application.init(argc, argv);

	// Scribble window
	new ScribbleWindow(&application);

	// Create the application's windows
	application.create();

	// Run the application
	return application.run();
}




