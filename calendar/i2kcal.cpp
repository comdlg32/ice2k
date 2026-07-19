#include <fx.h>
#include <ice2k/FXExtra/FXCalendar.h>
#include <X11/Xlib.h>
// #include <ice2k/comctl32.h>
#include "res/foxres.h"

FXIcon* mainIcon;

class I2KCalendar : public FXMainWindow {
	FXDECLARE(I2KCalendar);

protected:
	I2KCalendar() {}

private:
	FXHorizontalFrame *contents;
	FXCalendar* cal;

public:
	long onUnfocus(FXObject*, FXSelector, void*);


public:
	enum {
		ID_MAINWIN = FXMainWindow::ID_LAST,
		ID_LAST
	};

public:
	I2KCalendar(FXApp* a);

	virtual void create();
	virtual ~I2KCalendar();
};



FXDEFMAP(I2KCalendar) I2KCalendarMap[] = {
	FXMAPFUNC(SEL_FOCUSOUT,           0,  I2KCalendar::onUnfocus),
};

FXIMPLEMENT(I2KCalendar, FXMainWindow, I2KCalendarMap, ARRAYNUMBER(I2KCalendarMap));

I2KCalendar::I2KCalendar(FXApp *a) : FXMainWindow(a, "Calendar", mainIcon, NULL, DECOR_BORDER|DECOR_CLOSE|DECOR_MENU, 0,0,0,0) {
	cal = new FXCalendar(this, NULL, 0, FRAME_NORMAL);
	cal->setWeekendColor(cal->getDayColor());
	cal->setOtherWeekendColor(cal->getOtherDayColor());
}

I2KCalendar::~I2KCalendar() {
}


void I2KCalendar::create() {
	FXMainWindow::create();

	FXRootWindow rootwin(getApp(), getApp()->getDefaultVisual());

	XMoveWindow((Display*)(getApp()->getDisplay()), id(),
			rootwin.getDefaultWidth()-getDefaultWidth()-3,
			rootwin.getDefaultHeight()-getDefaultHeight()-29-3);

	show();
}


long I2KCalendar::onUnfocus(FXObject*, FXSelector, void*) {
	getApp()->exit();
	return 1;
}
int main(int argc, char *argv[]) {
	FXApp application("Calendar", "I2KProject");
	mainIcon = new FXGIFIcon(&application, resico_mainicon);

	application.init(argc, argv);

	new I2KCalendar(&application);

	application.create();
	return application.run();
}




