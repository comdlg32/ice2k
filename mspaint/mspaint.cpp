#include <fx.h>
#include <ice2k/comctl32.h>
#include "res/foxres.h"

FXIcon* ico_main16;

FXIcon* ico_tool_airbrush;
FXIcon* ico_tool_brush;
FXIcon* ico_tool_circle;
FXIcon* ico_tool_curve;
FXIcon* ico_tool_erase;
FXIcon* ico_tool_fill;
FXIcon* ico_tool_freeform;
FXIcon* ico_tool_line;
FXIcon* ico_tool_pencil;
FXIcon* ico_tool_picker;
FXIcon* ico_tool_polygon;
FXIcon* ico_tool_rect;
FXIcon* ico_tool_roundrect;
FXIcon* ico_tool_select;
FXIcon* ico_tool_text;
FXIcon* ico_tool_zoom;

// Main Window
class ScribbleWindow : public FXMainWindow {

	// Macro for class hierarchy declarations
	FXDECLARE(ScribbleWindow)

private:
	
	FXHorizontalFrame *contents;                // Content frame
	FXVerticalFrame   *canvasFrame;             // Canvas frame
	FXVerticalFrame   *toolbar;
	FXMatrix* toolbarbtns;
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

		ID_TOOL_FREEFORM,
		ID_TOOL_SELECT,
		ID_TOOL_ERASE,
		ID_TOOL_FILL,
		ID_TOOL_PICKER,
		ID_TOOL_ZOOM,
		ID_TOOL_PENCIL,
		ID_TOOL_BRUSH,
		ID_TOOL_AIRBRUSH,
		ID_TOOL_TEXT,
		ID_TOOL_LINE,
		ID_TOOL_CURVE,
		ID_TOOL_RECT,
		ID_TOOL_POLYGON,
		ID_TOOL_CIRCLE,
		ID_TOOL_ROUNDRECT,

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
FXIMPLEMENT(ScribbleWindow, FXMainWindow, ScribbleWindowMap, ARRAYNUMBER(ScribbleWindowMap));


class ToolBarButton : public FXButton {
	FXDECLARE(ToolBarButton);

protected:
	ToolBarButton() {}

public:
	virtual bool canFocus() const {return false;}
	virtual void setFocus() {}


public:
	ToolBarButton(FXComposite* p,
			FXIcon* ic=NULL, FXObject* tgt=NULL, FXSelector sel=0,
			FXuint opts=BUTTON_NORMAL,
			FXint x=0, FXint y=0, FXint w=0, FXint h=0,
			FXint pl=2,FXint pr=3,FXint pt=2,FXint pb=3) :
	FXButton(p,FXString::null,ic,tgt,sel,opts,x,y,w,h,pl,pr,pt,pb) {
	}
};

FXIMPLEMENT(ToolBarButton, FXButton, NULL, 0);

	enum {
		ID_CANVAS = FXMainWindow::ID_LAST,

		ID_TOOL_FREEFORM,
		ID_TOOL_SELECT,
		ID_TOOL_ERASE,
		ID_TOOL_FILL,
		ID_TOOL_PICKER,
		ID_TOOL_ZOOM,
		ID_TOOL_PENCIL,
		ID_TOOL_BRUSH,
		ID_TOOL_AIRBRUSH,
		ID_TOOL_TEXT,
		ID_TOOL_LINE,
		ID_TOOL_CURVE,
		ID_TOOL_RECT,
		ID_TOOL_POLYGON,
		ID_TOOL_CIRCLE,
		ID_TOOL_ROUNDRECT,

		ID_CLEAR,
		ID_LAST
	};


// Construct a ScribbleWindow
ScribbleWindow::ScribbleWindow(FXApp *a) : FXMainWindow(a, "Scribble Application", ico_main16, NULL, DECOR_ALL, 0,0,800,600) {
	contents = new FXHorizontalFrame(this, LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0);

	toolbar = new FXVerticalFrame(contents, LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0);
	toolbarbtns = new FXMatrix(toolbar, 2, MATRIX_BY_COLUMNS, 0,0,0,0, 4,4,4,4, 0,0);

	new ToolBarButton(toolbarbtns, ico_tool_freeform,  this, ID_TOOL_FREEFORM);
	new ToolBarButton(toolbarbtns, ico_tool_select,    this, ID_TOOL_SELECT);
	new ToolBarButton(toolbarbtns, ico_tool_erase,     this, ID_TOOL_ERASE);
	new ToolBarButton(toolbarbtns, ico_tool_fill,      this, ID_TOOL_FILL);
	new ToolBarButton(toolbarbtns, ico_tool_picker,    this, ID_TOOL_PICKER);
	new ToolBarButton(toolbarbtns, ico_tool_zoom,      this, ID_TOOL_ZOOM);
	new ToolBarButton(toolbarbtns, ico_tool_pencil,    this, ID_TOOL_PENCIL);
	new ToolBarButton(toolbarbtns, ico_tool_brush,     this, ID_TOOL_BRUSH);
	new ToolBarButton(toolbarbtns, ico_tool_airbrush,  this, ID_TOOL_AIRBRUSH);
	new ToolBarButton(toolbarbtns, ico_tool_text,      this, ID_TOOL_TEXT);
	new ToolBarButton(toolbarbtns, ico_tool_line,      this, ID_TOOL_LINE);
	new ToolBarButton(toolbarbtns, ico_tool_curve,     this, ID_TOOL_CURVE);
	new ToolBarButton(toolbarbtns, ico_tool_rect,      this, ID_TOOL_RECT);
	new ToolBarButton(toolbarbtns, ico_tool_polygon,   this, ID_TOOL_POLYGON);
	new ToolBarButton(toolbarbtns, ico_tool_circle,    this, ID_TOOL_CIRCLE);
	new ToolBarButton(toolbarbtns, ico_tool_roundrect, this, ID_TOOL_ROUNDRECT);




	// LEFT pane to contain the canvas
	canvasFrame = new FXVerticalFrame(contents, FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_TOP|LAYOUT_LEFT, 0,0,0,0, 10,10,10,10);

	// Label above the canvas
	new FXLabel(canvasFrame, "Canvas Frame", NULL, JUSTIFY_CENTER_X|LAYOUT_FILL_X);

	// Horizontal divider line
	new FXHorizontalSeparator(canvasFrame, SEPARATOR_GROOVE|LAYOUT_FILL_X);

	// Drawing canvas
	canvas = new FXCanvas(canvasFrame, this, ID_CANVAS, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN);

	// Label above the buttons
	/*new FXLabel(buttonFrame, "Button Frame", NULL, JUSTIFY_CENTER_X|LAYOUT_FILL_X);

	// Horizontal divider line
	new FXHorizontalSeparator(buttonFrame, SEPARATOR_RIDGE|LAYOUT_FILL_X);

	// Button to clear
	new FXButton(buttonFrame, "&Clear", NULL, this, ID_CLEAR, FRAME_THICK|FRAME_RAISED|LAYOUT_FILL_X|LAYOUT_TOP|LAYOUT_LEFT, 0,0,0,0, 10,10,5,5);

	// Exit button
	new FXButton(buttonFrame, "&Exit", NULL, getApp(), FXApp::ID_QUIT, FRAME_THICK|FRAME_RAISED|LAYOUT_FILL_X|LAYOUT_TOP|LAYOUT_LEFT, 0,0,0,0, 10,10,5,5);*/

	// Initialize private variables
	drawColor = FXRGB(255, 0, 0);
	mdflag = 0;
	dirty = 0;
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


	// Start app
	application.init(argc, argv);


	// Load program icon
	ico_main16 = new FXGIFIcon(&application, res_ico_main16);

	ico_tool_airbrush = new FXGIFIcon(&application, res_tool_airbrush);
	ico_tool_brush = new FXGIFIcon(&application, res_tool_brush);
	ico_tool_circle = new FXGIFIcon(&application, res_tool_circle);
	ico_tool_curve = new FXGIFIcon(&application, res_tool_curve);
	ico_tool_erase = new FXGIFIcon(&application, res_tool_erase);
	ico_tool_fill = new FXGIFIcon(&application, res_tool_fill);
	ico_tool_freeform = new FXGIFIcon(&application, res_tool_freeform);
	ico_tool_line = new FXGIFIcon(&application, res_tool_line);
	ico_tool_pencil = new FXGIFIcon(&application, res_tool_pencil);
	ico_tool_picker = new FXGIFIcon(&application, res_tool_picker);
	ico_tool_polygon = new FXGIFIcon(&application, res_tool_polygon);
	ico_tool_rect = new FXGIFIcon(&application, res_tool_rect);
	ico_tool_roundrect = new FXGIFIcon(&application, res_tool_roundrect);
	ico_tool_select = new FXGIFIcon(&application, res_tool_select);
	ico_tool_text = new FXGIFIcon(&application, res_tool_text);
	ico_tool_zoom = new FXGIFIcon(&application, res_tool_zoom);


	// Scribble window
	new ScribbleWindow(&application);

	// Create the application's windows
	application.create();

	// Run the application
	return application.run();
}




