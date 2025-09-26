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
class MineSweeper : public FXMainWindow {

	// Macro for class hierarchy declarations
	FXDECLARE(MineSweeper)

private:
	FXHorizontalFrame *contents;                // Content frame
	FXVerticalFrame   *canvasFrame;             // Canvas frame
	FXVerticalFrame   *buttonFrame;             // Button frame
	FXCanvas          *canvas;                  // Canvas to draw into
	int                mdflag;                  // Mouse button down?
	int                dirty;                   // Canvas has been painted?
	FXColor            drawColor;               // Color for the line

protected:
	MineSweeper() {}

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

	// MineSweeper's constructor
	MineSweeper(FXApp* a);

	// Initialize
	virtual void create();

	virtual ~MineSweeper();
};



// Message Map for the Scribble Window class
FXDEFMAP(MineSweeper) MineSweeperMap[] = {
	FXMAPFUNC(SEL_PAINT,             MineSweeper::ID_CANVAS, MineSweeper::onPaint),
	FXMAPFUNC(SEL_LEFTBUTTONPRESS,   MineSweeper::ID_CANVAS, MineSweeper::onMouseDown),
	FXMAPFUNC(SEL_LEFTBUTTONRELEASE, MineSweeper::ID_CANVAS, MineSweeper::onMouseUp),
	FXMAPFUNC(SEL_MOTION,            MineSweeper::ID_CANVAS, MineSweeper::onMouseMove),
	FXMAPFUNC(SEL_COMMAND,           MineSweeper::ID_CLEAR,  MineSweeper::onCmdClear),
	FXMAPFUNC(SEL_UPDATE,            MineSweeper::ID_CLEAR,  MineSweeper::onUpdClear),
};



// Macro for the ScribbleApp class hierarchy implementation
FXIMPLEMENT(MineSweeper, FXMainWindow, MineSweeperMap, ARRAYNUMBER(MineSweeperMap))


class MSPacker : public FXPacker {
	FXDECLARE(MSPacker)

public:
	MSPacker(FXComposite* p, FXuint opts = 0,
		FXint x=0, FXint y=0, FXint w=0, FXint h=0,
		FXint pl=0, FXint pr=0, FXint pt=0, FXint pb=0,
		FXint hs=0, FXint vs=0);

	MSPacker(){}

	virtual ~MSPacker(){}

	long onPaint(FXObject*,FXSelector,void*);

protected:
	void drawFrame(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h);

};

#define FRAME_MASK        (FRAME_SUNKEN|FRAME_RAISED|FRAME_THICK)
#define LAYOUT_MASK       (LAYOUT_SIDE_MASK|LAYOUT_RIGHT|LAYOUT_CENTER_X|LAYOUT_BOTTOM|LAYOUT_CENTER_Y|LAYOUT_FIX_X|LAYOUT_FIX_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y)
#define LAYOUT_SIDE_MASK  (LAYOUT_SIDE_LEFT|LAYOUT_SIDE_RIGHT|LAYOUT_SIDE_TOP|LAYOUT_SIDE_BOTTOM)

void MSPacker::drawFrame(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h) {
	if (0 < w && 0 < h) {
		switch(options&FRAME_MASK) {
			case FRAME_SUNKEN:
				dc.setForeground(FXRGB(128,128,128));
				
				dc.fillRectangle(x,y,w-1,2);
				dc.fillRectangle(x,y,2,h-1);

				dc.setForeground(FXRGB(255,255,255));
				
				dc.fillRectangle(x+w-2,y+1,2,h);
				dc.fillRectangle(x+1,y+h-2,w,2);

				dc.setForeground(FXRGB(192,192,192));

				dc.drawPoint(x+1, y+h-2);
				dc.drawPoint(x+w-2, y+1);
				break;

			case FRAME_SUNKEN|FRAME_THICK:
				dc.setForeground(FXRGB(128,128,128));
				
				dc.fillRectangle(x,y,w-1,3);
				dc.fillRectangle(x,y,3,h-1);

				dc.setForeground(FXRGB(255,255,255));
				
				dc.fillRectangle(x+w-3,y+1,3,h);
				dc.fillRectangle(x+1,y+h-3,w,3);

				dc.setForeground(FXRGB(192,192,192));

				// top right border
				dc.drawPoint(x+2, y+h-3);
				dc.drawPoint(x+1, y+h-2);

				dc.drawPoint(x+w-2, y+1);
				dc.drawPoint(x+w-3, y+2);

				dc.setForeground(FXRGB(128,128,128));
				dc.drawPoint(x+w-3, y+1);
				dc.drawPoint(x+1, y+h-3);

				break;
		}
	}
}

FXDEFMAP(MSPacker) MSPackerMap[]={
	FXMAPFUNC(SEL_PAINT,0,MSPacker::onPaint),
};


// Handle repaint
long MSPacker::onPaint(FXObject*,FXSelector,void* ptr){
	FXEvent *ev=(FXEvent*)ptr;
	FXDCWindow dc(this,ev);

	dc.setForeground(backColor);
	dc.fillRectangle(ev->rect.x,ev->rect.y,ev->rect.w,ev->rect.h);
	drawFrame(dc,0,0,width,height);

	return 1;
}

FXIMPLEMENT(MSPacker, FXPacker, MSPackerMap, ARRAYNUMBER(MSPackerMap));

MSPacker::MSPacker(FXComposite* p, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs):
	FXPacker(p, opts, x, y, w, h, pl, pr, pt, pb, hs, vs) {
	// from fox src
	border=(options&(FRAME_THICK))?3:(options&(FRAME_SUNKEN))?2:0;
}

// SEVEN SEGMENT WIDGET

#define _SEGMENTVAL(name, index) name = (index * -23)

class MSSevenSegment: public FXFrame {
	FXDECLARE(MSSevenSegment);
protected:
	FXImage*      segmentimg;
	FXint         number;
	FXint         dispnum;

	int           dispx;
	int           dispy;

	FXObject*     target;
	FXSelector    selector;

protected:
	MSSevenSegment() {}
			

public:

	MSSevenSegment(FXComposite* parent, FXuint opts=FRAME_NONE, FXint x=0, FXint y=0, FXint w=0, FXint h=0):
		FXFrame(parent, opts, x,y,w,h,0,0,0,0),
		number(0)
	{
		segmentimg = new FXBMPImage(getApp(), resico_segment, IMAGE_OPAQUE);
		segmentimg->create();
	}


	void setValue(FXint val) {
		if (val != number) {
			number = val;
			update();
		}
	}


	void drawDigit(FXDCWindow& dc, FXint digit, FXint x) {
		FXint ypos;

		switch (digit) {
			case -1: ypos = SEVEN_MINUS; break;
			case 0:  ypos = SEVEN_ZERO;  break;
			case 1:  ypos = SEVEN_ONE;   break;
			case 2:  ypos = SEVEN_TWO;   break;
			case 3:  ypos = SEVEN_THREE; break;
			case 4:  ypos = SEVEN_FOUR;  break;
			case 5:  ypos = SEVEN_FIVE;  break;
			case 6:  ypos = SEVEN_SIX;   break;
			case 7:  ypos = SEVEN_SEVEN; break;
			case 8:  ypos = SEVEN_EIGHT; break;
			case 9:  ypos = SEVEN_NINE;  break;
			default: ypos = SEVEN_NONE;  break;
		}

		dc.setClipRectangle(x+border+padleft, border+padtop, 39, 23);
		dc.drawImage(segmentimg, x * 13 + padleft+border, ypos + padtop+border);
		dc.clearClipRectangle();

	}

	FXint getValue() { return number; }

	enum numberPos {
		_SEGMENTVAL(SEVEN_MINUS, 0),
		_SEGMENTVAL(SEVEN_NONE, 1),
		
		_SEGMENTVAL(SEVEN_NINE, 2),
		_SEGMENTVAL(SEVEN_EIGHT, 3),
		_SEGMENTVAL(SEVEN_SEVEN, 4),
		_SEGMENTVAL(SEVEN_SIX, 5),
		_SEGMENTVAL(SEVEN_FIVE, 6),
		_SEGMENTVAL(SEVEN_FOUR, 7),
		_SEGMENTVAL(SEVEN_THREE, 8),
		_SEGMENTVAL(SEVEN_TWO, 9),
		_SEGMENTVAL(SEVEN_ONE, 10),
		_SEGMENTVAL(SEVEN_ZERO, 11)
	};



	long onPaint(FXObject* sender, FXSelector sel, void* ptr) {
		FXFrame::onPaint(sender, sel, ptr);
 
		FXEvent* event = (FXEvent*)ptr;
		FXDCWindow dc(this, event);

		dc.setForeground(FXRGB(0,0,0));
		dc.fillRectangle(padleft+border,padtop+border,width-(border<<1)-padright,height-(border<<1)-padbottom);

		int val = number;
		int neg = (val<0);

		if (neg) {
			val = -val;
			if (val>99) val = 99;
		} else {
			if (val>999) val = 999;
		}

		int d2 = val % 10; val /= 10;
		int d1 = val % 10; val /= 10;
		int d0 = val % 10;

		if (neg)
			drawDigit(dc, -1, 0);
		else
			drawDigit(dc, d0, 0);

		drawDigit(dc, d1, 1);
		drawDigit(dc, d2, 2);
		
		return 1;
	}


	FXint getDefaultWidth() {
		return 39+padleft+padright+(border<<1);
	}

	FXint getDefaultHeight() {
		return 23+padtop+padbottom+(border<<1);
	}

	long onCmdSetValue(FXObject*,FXSelector,void* ptr) {
		setValue((int)(long)ptr);
		return 1;
	}


	long onCmdSetIntValue(FXObject*,FXSelector,void* ptr) {
		setValue(*((int*)ptr));
		return 1;
	}



	long onCmdSetRealValue(FXObject*,FXSelector,void* ptr) {
		setValue((int)*((double*)ptr));
		return 1;
	}



	long onCmdGetIntValue(FXObject*,FXSelector,void* ptr){
		*((int*)ptr)=number;
		return 1;
	}


	long onCmdGetRealValue(FXObject*,FXSelector,void* ptr){
		*((double*)ptr)=(double)number;
		return 1;
	}


	enum {
		ID_CANVAS = FXFrame::ID_LAST,
		ID_LAST
	};



};

FXImage*      mineimg;


FXDEFMAP(MSSevenSegment) MSSevenSegmentMap[] = {
	FXMAPFUNC(SEL_PAINT, 0, MSSevenSegment::onPaint),
	FXMAPFUNC(SEL_UPDATE, 0, MSSevenSegment::onUpdate),

	FXMAPFUNC(SEL_COMMAND, MSSevenSegment::ID_SETVALUE,       MSSevenSegment::onCmdSetValue),

	FXMAPFUNC(SEL_COMMAND, MSSevenSegment::ID_SETINTVALUE,    MSSevenSegment::onCmdSetIntValue),
	FXMAPFUNC(SEL_COMMAND, MSSevenSegment::ID_SETREALVALUE,   MSSevenSegment::onCmdSetRealValue),

	FXMAPFUNC(SEL_COMMAND, MSSevenSegment::ID_GETINTVALUE,    MSSevenSegment::onCmdGetIntValue),
	FXMAPFUNC(SEL_COMMAND, MSSevenSegment::ID_GETREALVALUE,   MSSevenSegment::onCmdGetRealValue),
	//FXMAPFUNC(SEL_CHANGED, 0, MSSevenSegment::onChange),

};

FXIMPLEMENT(MSSevenSegment, FXFrame, MSSevenSegmentMap, ARRAYNUMBER(MSSevenSegmentMap));

#define _BUTTONVAL(name, index) name = (index * -16)


class MSMineButton: public FXFrame {
	FXDECLARE(MSMineButton);
protected:
	FXint         state;

	FXObject*     target;
	FXSelector    selector;

protected:
	MSMineButton() {}
			

public:

	MSMineButton(FXComposite* parent, FXuint opts=FRAME_NONE, FXint x=0, FXint y=0, FXint w=0, FXint h=0):
		FXFrame(parent, opts, x,y,w,h,0,0,0,0),
		state(MINEBUTTON_NORMAL)		
	{
		if (!mineimg) {
			mineimg = new FXBMPImage(getApp(), resico_coolmine, IMAGE_OPAQUE);
			mineimg->create();
		}
	}


	void setState(FXint val) {
		if (val != state) {
			state = val;
			update();
		}
	}


	FXint getState() { return state; }

	enum buttonPos {
		_BUTTONVAL(MINEBUTTON_NORMAL, 0),
		_BUTTONVAL(MINEBUTTON_FLAG, 1),
		_BUTTONVAL(MINEBUTTON_UNKNOWN, 2),
		
		_BUTTONVAL(MINEBUTTON_CLICKEDMINE, 3),
		_BUTTONVAL(MINEBUTTON_FLAGGEDMINE, 4),

		_BUTTONVAL(MINEBUTTON_MINE, 5),

		_BUTTONVAL(MINEBUTTON_UNKNOWN_PRESSED, 6),

		_BUTTONVAL(MINEBUTTON_EIGHT, 7),
		_BUTTONVAL(MINEBUTTON_SEVEN, 8), // six sevennnnn
		_BUTTONVAL(MINEBUTTON_SIX, 9),
		_BUTTONVAL(MINEBUTTON_FIVE, 10),
		_BUTTONVAL(MINEBUTTON_FOUR, 11),
		_BUTTONVAL(MINEBUTTON_THREE, 12),
		_BUTTONVAL(MINEBUTTON_TWO, 13),
		_BUTTONVAL(MINEBUTTON_ONE, 14),

		_BUTTONVAL(MINEBUTTON_NORMAL_PRESSED, 15),
	};



	long onPaint(FXObject* sender, FXSelector sel, void* ptr) {
		//FXFrame::onPaint(sender, sel, ptr);
 
		FXEvent* event = (FXEvent*)ptr;
		FXDCWindow dc(this, event);

		dc.setForeground(FXRGB(192,192,192));
		dc.fillRectangle(0,0,width,height);

		dc.setClipRectangle(0,0,width,height);
		dc.drawImage(mineimg, 0, state);
		dc.clearClipRectangle();
		
		return 1;
	}


	FXint getDefaultWidth() {
		return 16;
	}

	FXint getDefaultHeight() {
		return 16;
	}

	enum {
		ID_CANVAS = FXFrame::ID_LAST,
		ID_LAST
	};



};

FXDEFMAP(MSMineButton) MSMineButtonMap[] = {
	FXMAPFUNC(SEL_PAINT, 0, MSMineButton::onPaint),

	//FXMAPFUNC(SEL_CHANGED, 0, MSSevenSegment::onChange),

};

FXIMPLEMENT(MSMineButton, FXFrame, MSMineButtonMap, ARRAYNUMBER(MSMineButtonMap));




// Construct a MineSweeper
MineSweeper::MineSweeper(FXApp *a) : FXMainWindow(a, "Scribble Application", mainIcon, NULL, DECOR_ALL, 0,0,320,200) {

	FXPacker* rootcont = new FXPacker(this, LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0, 0,0);
	rootcont->setBackColor(FXRGB(192,192,192));

	FXFrame* topborder = new FXFrame(rootcont, LAYOUT_FIX_HEIGHT|LAYOUT_FILL_X, 0,0,3,3);
	FXFrame* leftborder = new FXFrame(rootcont, LAYOUT_FIX_WIDTH|LAYOUT_FILL_Y|LAYOUT_SIDE_LEFT, 0,0,3,3);

	//FXHorizontalFrame*

	topborder->setBackColor(FXRGB(255,255,255));
	leftborder->setBackColor(FXRGB(255,255,255));

	FXPacker* cont = new FXPacker(rootcont, LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 6,5,6,5, 6,6);
	cont->setBackColor(FXRGB(192,192,192));

	//FXPacker* minegrid = new FXPacker(this, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0, 0,0);

	MSPacker* topstatscont = new MSPacker(cont, FRAME_SUNKEN|LAYOUT_FILL_X, 0,0,0,0, 5,7,4,4, 0,0);
	topstatscont->setBackColor(FXRGB(192,192,192));

	FXHorizontalFrame* topstats = new FXHorizontalFrame(topstatscont, LAYOUT_FILL_Y|LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0, 0,0);
	topstats->setBackColor(FXRGB(192,192,192));

	new MSSevenSegment(topstats, LAYOUT_LEFT|FRAME_NONE, 0,0,0,0);
	new MSSevenSegment(topstats, LAYOUT_CENTER_X|LAYOUT_CENTER_X|FRAME_NONE, 0,0,0,0);

	new MSSevenSegment(topstats, LAYOUT_RIGHT|LAYOUT_RIGHT|FRAME_NONE, 0,0,0,0);

	new MSMineButton(topstats, LAYOUT_RIGHT|LAYOUT_RIGHT|FRAME_NONE, 0,0,0,0);
	new MSMineButton(topstats, LAYOUT_RIGHT|LAYOUT_RIGHT|FRAME_NONE, 0,0,0,0);

	MSPacker* minegrid = new MSPacker(cont, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0, 0,0);
	minegrid->setBackColor(FXRGB(192,192,192));
}


MineSweeper::~MineSweeper() {
}


// Create and initialize
void MineSweeper::create() {
	// Create the windows
	FXMainWindow::create();

	// Make the main window appear
	show(PLACEMENT_SCREEN);
}



// Mouse button was pressed somewhere
long MineSweeper::onMouseDown(FXObject*, FXSelector, void*) {
	canvas->grab();

	// While the mouse is down, we'll draw lines
	mdflag = 1;

	return 1;
}



// The mouse has moved, draw a line
long MineSweeper::onMouseMove(FXObject*, FXSelector, void* ptr) {
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
long MineSweeper::onMouseUp(FXObject*, FXSelector, void* ptr) {
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
long MineSweeper::onPaint(FXObject*, FXSelector, void* ptr) {
	FXEvent *ev =(FXEvent*) ptr;
	FXDCWindow dc(canvas, ev);
	dc.setForeground(canvas->getBackColor());
	dc.fillRectangle(ev->rect.x, ev->rect.y, ev->rect.w, ev->rect.h);

	return 1;
}


// Handle the clear message
long MineSweeper::onCmdClear(FXObject*, FXSelector, void*) {
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
long MineSweeper::onUpdClear(FXObject* sender, FXSelector, void*) {
	
	if (dirty)
		sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
	else
		sender->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);

	return 1;
}


// Here we begin
int main(int argc, char *argv[]) {
	// Make application
	FXApp application("WinMine", "Ice2KProj");

	// Load program icon
	mainIcon = new FXGIFIcon(&application, resico_mainicon, 0, IMAGE_OPAQUE);

	// Start app
	application.init(argc, argv);

	// Scribble window
	new MineSweeper(&application);

	// Create the application's windows
	application.create();

	// Run the application
	return application.run();
}



