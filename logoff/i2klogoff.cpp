// Based on ctrl+alt+del program
// which is based on the scribble example

#include "fx.h"
#include <FXPNGImage.h>
#include <FXPNGIcon.h>
#include "fxkeys.h"

#include "res/foxres.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>
#include <sys/sysinfo.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ice2k/I2KListBox.h>
#undef _DO_NOT_OVERRIDE_FOX_CONTROLS
#include <ice2k/comctl32.h>
#include <ice2k/branding.h>


// thank you stack overflow
// https://stackoverflow.com/questions/20733215/how-to-make-a-window-always-on-top
// since fox 1.6 doesnt support setting the window on top, we need to use the x11 api directly

#define _NET_WM_STATE_REMOVE        0    /* remove/unset property */
#define _NET_WM_STATE_ADD           1    /* add/set property */
#define _NET_WM_STATE_TOGGLE        2    /* toggle property  */

char taskid[64];

FXIcon* logoffIcon;
Display* xdisplay;
Window rootwin;
FXDialogBox* cadwindow;
FXMainWindow* chkwindow;
#define _OPTION_LOGOFF 0
#define _OPTION_SHUTDOWN 1
#define _OPTION_RESTART 2
#define _OPTION_STANDBY 3

// change a window's _NET_WM_STATE property so that it can be kept on top.
// @display: x11 display singleton.
// @xid    : the window to set on top.

// chatgpt helped me a bit with this
Status setOnTop (Display* display, Window xid) {
	XEvent event;
	event.xclient.type = ClientMessage;
	event.xclient.serial = 0;
	event.xclient.send_event = True;
	event.xclient.display = display;
	event.xclient.window  = xid;
	event.xclient.message_type = XInternAtom (display, "_WIN_LAYER", False);
	event.xclient.format = 32;

	event.xclient.data.l[0] = 8;

	return XSendEvent (display, DefaultRootWindow(display), False,
			SubstructureRedirectMask|SubstructureNotifyMask, &event);
}

#define ZERO4 0, 0, 0, 0

int unfocus = 0;

// Main Window
class LogoffDialog : public FXDialogBox {

	// Macro for class hierarchy declarations
	FXDECLARE(LogoffDialog)

	private:
		FXHorizontalFrame* cont;
		FXHorizontalFrame* buttoncont;

	protected:
		LogoffDialog(){}

	public:

		// Message handlers

		long onClickOption(FXObject*,FXSelector,void*);

		long onAccept(FXObject*,FXSelector,void*);
		long onClose(FXObject*,FXSelector,void*);

		long onFocus(FXObject*,FXSelector,void*);  
		long onUnfocus(FXObject*,FXSelector,void*);

		long onRealUnfocus(FXObject*,FXSelector,void*);
		long onRealFocus(FXObject*,FXSelector,void*);

		long onConfigure(FXObject*,FXSelector,void*);


	public:

		// Messages for our class
		enum {
			ID_MAINWIN=FXDialogBox::ID_LAST,
			ID_CLICK_OPTION,
			ID_ACCEPT,
			ID_UNFOCUS,
			ID_FOCUS
		};

	public:

		// LogoffDialog's constructor
		LogoffDialog(FXWindow* owner);

		// Initialize
		virtual void create();
		void setFocus() {};

		virtual ~LogoffDialog();
};



// Message Map for the CtrlAltDel Window class
FXDEFMAP(LogoffDialog) LogoffDialogMap[] = {
	FXMAPFUNC(SEL_COMMAND, LogoffDialog::ID_ACCEPT, LogoffDialog::onAccept),
	FXMAPFUNC(SEL_UNMAP, 0, LogoffDialog::onClose),


	FXMAPFUNC(SEL_TIMEOUT, LogoffDialog::ID_FOCUS, LogoffDialog::onRealUnfocus),
	FXMAPFUNC(SEL_TIMEOUT, LogoffDialog::ID_UNFOCUS, LogoffDialog::onRealUnfocus),

	FXMAPFUNC(SEL_FOCUSIN, 0, LogoffDialog::onFocus),
	FXMAPFUNC(SEL_FOCUSOUT, 0, LogoffDialog::onUnfocus),
	FXMAPFUNC(SEL_CONFIGURE, 0, LogoffDialog::onConfigure),
};

long LogoffDialog::onFocus(FXObject* sender,FXSelector sel,void* ptr) {
	unfocus = 0;
	FXDialogBox::onFocusIn(sender, sel, ptr);
	return 1;
}


long LogoffDialog::onUnfocus(FXObject* sender,FXSelector sel,void* ptr) {
	unfocus = 1;
	FXDialogBox::onFocusOut(sender, sel, ptr);
	getApp()->addTimeout(this,LogoffDialog::ID_UNFOCUS,20);
	//onRealUnfocus(sender, sel, ptr);
	return 1;
}

long LogoffDialog::onRealUnfocus(FXObject* sender,FXSelector sel,void* ptr) {
	if (unfocus) {
		FXDialogBox::onFocusOut(sender, sel, ptr);
		FXDialogBox::onCmdCancel(cadwindow, sel, ptr);
	}
	return 1;
}

// Macro for the CtrlAltDelApp class hierarchy implementation
FXIMPLEMENT(LogoffDialog,FXDialogBox,LogoffDialogMap,ARRAYNUMBER(LogoffDialogMap));

// Construct a LogoffDialog
LogoffDialog::LogoffDialog(FXWindow* owner):FXDialogBox(owner, "Log Off Windows", DECOR_TITLE|DECOR_BORDER|DECOR_MENU|DECOR_CLOSE, 0,0,0,0, 12,12,10,13, 20,20) {
	cont = new FXHorizontalFrame(this, LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0, 8,8);
	new FXLabel(cont, "", logoffIcon, LAYOUT_CENTER_Y, 0,0,0,0, 0,0,0,0);
	new FXLabel(cont, "Are you sure you want to log off?", NULL, LAYOUT_BOTTOM, 0,0,0,0, 22,34,0,4);

	buttoncont = new FXHorizontalFrame(this, LAYOUT_CENTER_X|PACK_UNIFORM_WIDTH, 0,0,0,0, 6,0,0,0, 10,10);

	FXButton* logoffbtn = new FXButton(buttoncont, "&Log Off", NULL, this, ID_ACCEPT, BUTTON_NORMAL|BUTTON_DEFAULT, 0,0,0,0, 12,13,2,3);
	logoffbtn->setFocus();
	new FXButton(buttoncont, "&Cancel", NULL, this, ID_CANCEL, BUTTON_NORMAL|BUTTON_DEFAULT, 0,0,0,0, 12,13,2,3);
}


LogoffDialog::~LogoffDialog() {
}

// Create and initialize
void LogoffDialog::create() {
	FXDialogBox::create();
}

long LogoffDialog::onClose(FXObject* sender, FXSelector sel, void* ptr)
{
	chkwindow->close();
	return 0;
}

long LogoffDialog::onConfigure(FXObject* sender, FXSelector sel, void* ptr)
{
	unfocus = 0;
	FXDialogBox::onConfigure(sender, sel, ptr);
	return 0;
}


long LogoffDialog::onAccept(FXObject* sender, FXSelector sel, void* ptr) {
	system("~/.icewm/programs/shutdown/action logoff &");
	//system("xterm &");

	FXDialogBox::onCmdAccept(sender, sel, ptr);
	getApp()->exit(0);
	return 0;
}

class FadeWindow : public FXMainWindow {
	FXDECLARE(FadeWindow)
	private:
		FXImageFrame      *imgframe;
		FXImage* image;
		int satur;
		int chunky;
	protected:
		FadeWindow(){}

	public:

		// Message handlers

		long onClickOption(FXObject*,FXSelector,void*);

		long onAccept(FXObject*,FXSelector,void*);


		long onFocus(FXObject*,FXSelector,void*);
		long onUnfocus(FXObject*,FXSelector,void*);

		long onRealUnfocus(FXObject*,FXSelector,void*);
		long onTimeoutFade(FXObject*,FXSelector,void*);

		void XPFade(FXColor*, FXuint, FXuint, FXuint);


	public:

		// Messages for our class
		enum {
			ID_FADEWIN=FXMainWindow::ID_LAST,
			ID_FOCUS,
			ID_UNFOCUS,
			ID_FADE,
			ID_LAST
		};

	public:

		FadeWindow(FXApp* a);

		// Initialize
		virtual void create();
		void setFocus() {};

		virtual ~FadeWindow();
};


FadeWindow::FadeWindow(FXApp* a):FXMainWindow(a, "Log Off Windows", NULL, NULL, DECOR_TITLE|DECOR_BORDER|DECOR_MENU|DECOR_CLOSE, 0, 0, 0, 0, 0, 0, 0, 0) {
}

FadeWindow::~FadeWindow() {
}

void FadeWindow::XPFade(FXColor* data, FXuint w, FXuint h, FXuint y) {
	FXColor* pix = data + (w*y);
	//FXColor* pix = data;
	unsigned pixels = w*h;
	for (int i = pixels - 1; i >= 0; i--) {
		unsigned r = FXREDVAL(*pix);
		unsigned g = FXGREENVAL(*pix);
		unsigned b = FXBLUEVAL(*pix);
		
		unsigned gray = (54 * r + 183 * g + 19 * b) >> 8;
		unsigned temp = gray * (255 - 213);
		
		// >> 8 divides by / 256
		r = (r * 213 + temp) >> 8;
		g = (g * 213 + temp) >> 8;
		b = (b * 213 + temp) >> 8;
		
		*pix = FXRGB(r, g, b);
		pix++;
	}
}

long FadeWindow::onTimeoutFade(FXObject* sender, FXSelector sel, void* ptr) {
	int imageheight = image->getHeight();
	int chunkh = imageheight / 20;

	if (chunky + chunkh > imageheight) {
		chunkh = imageheight - chunky;
	}

	XPFade(image->getData(), image->getWidth(), chunkh, chunky);

	chunky += chunkh;

	if (chunky >= imageheight) {
		if (!(--satur > 0)) return 1;
		image->render();
		imgframe->update();
		chunky = 0;
	}

	getApp()->addTimeout(this, ID_FADE, 20);

	return 1;
}



void FadeWindow::create() {
	satur = 17;
	chunky = 0;

	FXMainWindow::create();

	FXApp* ptrapp = getApp();

	FXWindow* root = ptrapp->getRootWindow();
	int rootw = root->getWidth();
	int rooth = root->getHeight();

	image = new FXImage(ptrapp, NULL, IMAGE_OPAQUE|IMAGE_NEAREST|IMAGE_SHMP|IMAGE_SHMI, rootw, rooth);
	image->create();

	FXDCWindow dc(image);

	dc.clipChildren(FALSE);
	dc.setFunction(BLT_SRC);
	dc.drawArea(ptrapp->getRootWindow(), 0, 0, rootw, rooth, 0, 0);

	image->restore();
	FXColor* imgdata = image->getData();

	int imgwidth = image->getWidth();
	int imgheight = image->getHeight();
	int startx;

	if (i2kBGetWinVersionInt() == ICE2K_BRAND_WIN2K) {
		for (int y = 0; y < imgheight; y++) {
			if (y & 1) startx = 1;
			else startx = 0;

			for (int x = startx; x < imgwidth; x+=2) {
				imgdata[y*imgwidth+x] = FXRGB(0,0,0);
			}
		}
	} else {
		getApp()->addTimeout(this, ID_FADE, 20);
	}

	image->render();

	dc.setFillStyle(FILL_SOLID);
	dc.clipChildren(TRUE);

	imgframe = new FXImageFrame(chkwindow, image, FRAME_NONE);
	imgframe->create();

	FXMainWindow::create();
}


// Message Map for the CtrlAltDel Window class
FXDEFMAP(FadeWindow) FadeWindowMap[] = {
	FXMAPFUNC(SEL_TIMEOUT, FadeWindow::ID_FADE, FadeWindow::onTimeoutFade),
};

// Macro for the CtrlAltDelApp class hierarchy implementation
FXIMPLEMENT(FadeWindow,FXMainWindow,FadeWindowMap,ARRAYNUMBER(FadeWindowMap))


int main(int argc,char *argv[]) {
	FXApp application("LogoffDialog", "Ice2KProj");
	FXApp* ptrapp = &application;

	application.init(argc,argv);

	if (i2kBGetWinVersionInt() != ICE2K_BRAND_WIN2K) {
		logoffIcon = new FXPNGIcon(ptrapp, resico_xp_logoff);
	} else {
		logoffIcon = new FXPNGIcon(ptrapp, resico_2k_logoff);
	}
	logoffIcon->blend(ptrapp->getBaseColor());

	xdisplay = (Display*)ptrapp->getDisplay();
	chkwindow = new FadeWindow(ptrapp);
	cadwindow = new LogoffDialog(chkwindow);

	application.create();

	chkwindow->show();
	setOnTop(xdisplay, chkwindow->id());

	//cadwindow->create();
	cadwindow->position(
			(cadwindow->getRoot()->getWidth() - cadwindow->getWidth())/2,
			((cadwindow->getRoot()->getHeight()+40) - cadwindow->getHeight())/3,
			(cadwindow->getWidth()),
			(cadwindow->getHeight()) );
	cadwindow->show();

	application.runModalFor(cadwindow);

	return application.run();
}


