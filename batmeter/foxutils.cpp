#include <fx.h>
#include "foxutils.h"
#include <stdio.h>

#include "common.h"
#include "res/foxres.h"

static int iconsloaded;

FXIcon            *ico_bat_ac;                 // Plugged into AC
FXIcon            *ico_bat_charging;           // Plugged into AC and charging
FXIcon            *ico_bat_chargeov;           // Charging overlay

FXIcon            *ico_bat_chargin2;           // Second battery charging icon? No idea where used

FXIcon            *ico_bat_full;               // Battery full
FXIcon            *ico_bat_half;               // Battery half full
FXIcon            *ico_bat_low;                // Battery low
FXIcon            *ico_bat_zero;               // Battery has zero charge?

FXIcon            *ico_bat_notpresent;         // Battery not present
FXIcon            *ico_bat_unknown;            // Battery information is unknown?

FXIcon            *ico_bat_what;               // wtf?

FXIcon            *ico_mainapp_16;             // Small Window icon
FXIcon            *ico_mainapp_32;             // Large Window icon

/* FXIcon* loadGifIcon(FXApp* app, const void* pix, FXColor clr, FXColor blendclr, FXuint opts, FXint w, FXint h) {
	FXIcon* icon = new FXGIFIcon(app, pix, clr, opts, w, h);


	if ( blendclr == FXRGB(192,192,192) ) {
		blendclr = app->getBaseColor();
	}

	if ( blendclr == 0 ) {
		icon->blend(blendclr);
	}

	return(icon);
} */


void loadIcons(FXApp* ptrapp) {
	if (!iconsloaded) {
		ico_bat_ac = new FXGIFIcon(ptrapp, resico_bat_ac, FXRGB(192,192,192), IMAGE_NEAREST);
		ico_bat_charging = new FXGIFIcon(ptrapp, resico_bat_charging, FXRGB(192,192,192), IMAGE_NEAREST);
		ico_bat_chargeov = new FXGIFIcon(ptrapp, resico_bat_chargeov, FXRGB(192,192,192), IMAGE_NEAREST);

		ico_bat_chargin2 = new FXGIFIcon(ptrapp, resico_bat_chargin2, FXRGB(192,192,192), IMAGE_NEAREST);

		ico_bat_full = new FXGIFIcon(ptrapp, resico_bat_full, FXRGB(192,192,192), IMAGE_NEAREST);
		ico_bat_half = new FXGIFIcon(ptrapp, resico_bat_half, FXRGB(192,192,192), IMAGE_NEAREST);
		ico_bat_low = new FXGIFIcon(ptrapp, resico_bat_low, FXRGB(192,192,192), IMAGE_NEAREST);
		ico_bat_zero = new FXGIFIcon(ptrapp, resico_bat_zero, FXRGB(192,192,192), IMAGE_NEAREST);

		ico_bat_notpresent = new FXGIFIcon(ptrapp, resico_bat_notpresent, FXRGB(192,192,192), IMAGE_NEAREST);
		ico_bat_unknown = new FXGIFIcon(ptrapp, resico_bat_unknown, FXRGB(192,192,192), IMAGE_NEAREST);

		ico_bat_what = new FXGIFIcon(ptrapp, resico_bat_what, FXRGB(192,192,192), IMAGE_NEAREST);

		ico_mainapp_16 = new FXGIFIcon(ptrapp, resico_mainapp_16, FXRGB(192,192,192), IMAGE_NEAREST);
		ico_mainapp_32 = new FXGIFIcon(ptrapp, resico_mainapp_32, FXRGB(192,192,192), IMAGE_NEAREST);

		iconsloaded = 1;
	}
}


void unloadIcons(FXApp* ptrapp) {
	if (iconsloaded) {
		delete ico_bat_ac;
		delete ico_bat_charging;
		delete ico_bat_chargeov;

		delete ico_bat_chargin2;

		delete ico_bat_full;
		delete ico_bat_half;
		delete ico_bat_low;
		delete ico_bat_zero;

		delete ico_bat_notpresent;
		delete ico_bat_unknown;

		delete ico_bat_what;

		delete ico_mainapp_16;
		delete ico_mainapp_32;

		iconsloaded = 0;
	}
}


namespace UH {
	// Map
	FXDEFMAP(UHMenuBar) UHMenuBarMap[] = {
		FXMAPFUNC(SEL_ENTER,0,FXMenuBar::onEnter),
		FXMAPFUNC(SEL_LEAVE,0,FXMenuBar::onLeave),
		FXMAPFUNC(SEL_MOTION,0,FXMenuBar::onMotion),
		FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXMenuBar::onButtonPress),
		FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXMenuBar::onButtonRelease),
		FXMAPFUNC(SEL_MIDDLEBUTTONPRESS,0,FXMenuBar::onButtonPress),
		FXMAPFUNC(SEL_MIDDLEBUTTONRELEASE,0,FXMenuBar::onButtonRelease),
		FXMAPFUNC(SEL_RIGHTBUTTONPRESS,0,FXMenuBar::onButtonPress),
		FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,0,FXMenuBar::onButtonRelease),
		FXMAPFUNC(SEL_FOCUS_RIGHT,0,FXMenuBar::onFocusRight),
		FXMAPFUNC(SEL_FOCUS_LEFT,0,FXMenuBar::onFocusLeft),
		FXMAPFUNC(SEL_FOCUS_NEXT,0,FXMenuBar::onDefault),
		FXMAPFUNC(SEL_FOCUS_PREV,0,FXMenuBar::onDefault),
		FXMAPFUNC(SEL_FOCUS_UP,0,FXMenuBar::onDefault),
		FXMAPFUNC(SEL_FOCUS_DOWN,0,FXMenuBar::onDefault),
		FXMAPFUNC(SEL_COMMAND,FXWindow::ID_UNPOST,FXMenuBar::onCmdUnpost),
	};


	// Object implementation
	FXIMPLEMENT(UHMenuBar,FXMenuBar,UHMenuBarMap,ARRAYNUMBER(UHMenuBarMap))

	// Make a floatable menubar
	UHMenuBar::UHMenuBar(FXComposite* p, FXComposite* q, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs):
		FXMenuBar(p, q, opts, x, y, w, h, pl, pr, pt, pb, hs, vs) {
		flags|=FLAG_ENABLED;
		dragCursor=getApp()->getDefaultCursor(DEF_RARROW_CURSOR);
	}


	// Make a non-floatable menubar
	UHMenuBar::UHMenuBar(FXComposite* p, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs):
		FXMenuBar(p, opts, x, y, w, h, pl, pr, pt, pb, hs, vs) {
		flags|=FLAG_ENABLED;
		dragCursor=getApp()->getDefaultCursor(DEF_RARROW_CURSOR);
	}

}
