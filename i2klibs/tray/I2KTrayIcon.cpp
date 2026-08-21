/*
I2KTray, system tray for Ice2K.sys desktop based off
FoxTray, system tray icon for the FOX Toolkit

Copyright (c) 2008, Hendrik Rittich
Copyright (c) 2026, xcomposite
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in 
      the documentation and/or other materials provided with the 
      distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ice2k/i2ktray/I2KTrayIcon.h"

#ifndef _WIN32
#include "ice2k/i2ktray/I2KTrayApp.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

namespace FX {


#ifdef _WIN32
FXDEFMAP(I2KTrayIcon) I2KTrayIconMap[] = {
    FXMAPFUNC(SEL_MOTION, 0, I2KTrayIcon::onEvent)
};
#else
FXDEFMAP(I2KTrayIcon) I2KTrayIconMap[] = {
    FXMAPFUNC(SEL_MOUSEWHEEL, I2KTrayIcon::ID_BUTTON, I2KTrayIcon::onMouseWheel),

	FXMAPFUNC(SEL_LEFTBUTTONRELEASE, I2KTrayIcon::ID_BUTTON, I2KTrayIcon::onLeft),
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE, I2KTrayIcon::ID_BUTTON, I2KTrayIcon::onRight),
    FXMAPFUNC(SEL_CLOSE, 0, I2KTrayIcon::onClose)
};
#endif

FXIMPLEMENT(I2KTrayIcon, FXTopWindow, I2KTrayIconMap, ARRAYNUMBER(I2KTrayIconMap));
#ifdef _WIN32
DWORD I2KTrayIcon::sTrayIconCount = 1;
#else

static int trapped_error_code = 0;
static int (*old_error_handler) (Display *, XErrorEvent *);
#endif

#ifndef _WIN32
static int error_handler(Display* display, XErrorEvent* error) {
    trapped_error_code = error->error_code;
    return 0;
}

void trap_errors() {
    trapped_error_code = 0;
    old_error_handler = XSetErrorHandler(error_handler);
}

int untrap_errors() {
    XSetErrorHandler(old_error_handler);
    return trapped_error_code;
}

#define SYSTEM_TRAY_REQUEST_DOCK    0
#define SYSTEM_TRAY_BEGIN_MESSAGE   1
#define SYSTEM_TRAY_CANCEL_MESSAGE  2

bool send_tray_message(
     Display* dpy, /* display */
     Window w,     /* sender (tray icon window) */
     long message, /* message opcode */
     long data1=0,   /* message data 1 */
     long data2=0,   /* message data 2 */
     long data3=0    /* message data 3 */
) {
    XEvent ev;
  
    memset(&ev, 0, sizeof(ev));
    ev.xclient.type = ClientMessage;
    ev.xclient.window = w;
    ev.xclient.message_type = XInternAtom(dpy, "_NET_SYSTEM_TRAY_OPCODE", False );
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = CurrentTime;
    ev.xclient.data.l[1] = message;
    ev.xclient.data.l[2] = data1;
    ev.xclient.data.l[3] = data2;
    ev.xclient.data.l[4] = data3;

    trap_errors(); // XEMBED Specific
    XSendEvent(dpy, w, False, NoEventMask, &ev);
    XSync(dpy, False);
    if (untrap_errors()) {
	    return false;
    }
    return true;
}

#endif


#ifdef _WIN32

I2KTrayIcon::I2KTrayIcon(FXApp* app, const FXString& text, FXIcon* icon, 
        FXPopup* pup, FXObject* target, FXSelector sel,
        FXuint opts) :
	FXTopWindow(app, text, 0, 0, 0, 0,0,0,0, 0,0,0,0, 0,0),
	mIcon(icon),
	mPup(pup),
	mOpts(opts),
	mTooltip(text)
{
	mWIcon = 0;
	mTrayID = sTrayIconCount;
	sTrayIconCount++;

	setTarget(target);
	setSelector(sel);
}
#else
I2KTrayIcon::I2KTrayIcon(FXApp* app, const FXString& text, FXIcon* icon, 
            FXPopup* pup, FXObject* target, FXSelector sel,
            FXuint opts) : 
    FXTopWindow(app, app->getAppName(), 0,0,0, 0,0,64,64, 0,0,0,0, 0,0),
    mPup(pup),
    mOpts(opts)
{
    // Tooltip
    new FXToolTip(getApp());

	/*btn = new FXButton(this, "", icon, this, ID_BUTTON,
			LAYOUT_CENTER_X|LAYOUT_CENTER_Y, 0,0,0,0, 0,0,0,0);*/

	lbl = new FXLabel(this, "", icon, LAYOUT_CENTER_X|LAYOUT_CENTER_Y, 0,0,0,0, 0,0,0,0);
	// btn->setTipText(text);
	lbl->setTipText(text);
	lbl->setTarget(this);
	lbl->setSelector(ID_BUTTON);

    setTarget(target);
    setSelector(sel);

    ((I2KTrayApp*)app)->registerIcon(this);
}

#endif


#ifdef _WIN32

I2KTrayIcon::~I2KTrayIcon() {
	if (mWIcon != NULL) {
		// Icon entfernen
		NOTIFYICONDATA tray_data;
		ZeroMemory(&tray_data, sizeof(tray_data));
		tray_data.cbSize = sizeof(tray_data);
		tray_data.hWnd = (HWND)xid;
		tray_data.uID = mTrayID;
		Shell_NotifyIcon(NIM_DELETE, &tray_data);

		// Icon loeschen
		DestroyIcon(mWIcon);
	}
}

#else
I2KTrayIcon::~I2KTrayIcon() 
{
    ((I2KTrayApp*)getApp())->unregisterIcon(this);
}

#endif
void I2KTrayIcon::create() {
    // resizes to default size
    FXTopWindow::create();

#ifdef _WIN32
	if (mWIcon != NULL) return;

	mIcon->create();

	// Windows Icon erzeugen
    mWIcon = createMswIcon(mIcon);
#endif

    mapToManager();
}

#ifdef _WIN32

void I2KTrayIcon::mapToManager() {
	NOTIFYICONDATA tray_data;
    setupNotifyData(&tray_data);
	
	Shell_NotifyIcon(NIM_ADD, &tray_data);
}

#else
void I2KTrayIcon::mapToManager() 
{
    Display* dpy = (Display*)(getApp()->getDisplay());

    XSizeHints* hints = XAllocSizeHints();

	hints->flags = PMinSize | PMaxSize | PBaseSize;
    hints->base_width = hints->min_width = hints->max_width = 
        getDefaultWidth();
    hints->base_height = hints->min_height = hints->max_height = 
        getDefaultHeight();

    XSetWMNormalHints(dpy, xid, hints);
    XFree(hints);

    Window manager_id = ((I2KTrayApp*)getApp())->getTrayManager();
    if (manager_id == None) {
        return; // no tray
    } 

    // Reparent the Window to the System Tray
    if (send_tray_message(dpy, manager_id, 
            SYSTEM_TRAY_REQUEST_DOCK, xid)) {
        // success... show the window on embed message
    }
}

#endif

#ifdef _WIN32
void I2KTrayIcon::setupNotifyData(NOTIFYICONDATA* data) {
	ZeroMemory(data, sizeof(*data));
	
	data->cbSize = sizeof(*data);
	data->hWnd = (HWND)xid;
	data->uID = mTrayID;
	data->hIcon = mWIcon;
	data->uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP;
	data->uCallbackMessage = WM_MOUSEMOVE; // Change ID

	// Tooltip
#ifdef UNICODE
    utf2ncs(data->szTip, mTooltip.text(), 64);
#else
    //SetWindowTextA((HWND)xid,title.text());
#error USE DEFINE UNICODE
#endif
}

HICON I2KTrayIcon::createMswIcon(FXIcon* icon) {
	// Windows Icon erzeugen
	ICONINFO iconinfo;
    ZeroMemory(&iconinfo, sizeof(iconinfo));
	
	iconinfo.fIcon = 1;
	iconinfo.hbmMask = (HBITMAP)icon->shape;
	iconinfo.hbmColor = (HBITMAP)icon->xid;
	return CreateIconIndirect(&iconinfo);
}

long I2KTrayIcon::onEvent(FXObject* obj, FXSelector, void* ptr) {
	FXEvent* ev = (FXEvent*)ptr;
	if (ptr == NULL) return 0;

	int lParam = ev->root_x;
	
	if ((lParam == WM_LBUTTONUP && (mOpts & TRAY_MENU_ON_LEFT)) ||
		(lParam == WM_RBUTTONUP && (mOpts & TRAY_MENU_ON_RIGHT))) {
		POINT p;
		GetCursorPos(&p);
		mPup->popup(0, p.x, p.y);

		return 1;
	}
	
	if (target != NULL &&
		(lParam == WM_LBUTTONUP && (mOpts & TRAY_CMD_ON_LEFT)) ||
		(lParam == WM_RBUTTONUP && (mOpts & TRAY_CMD_ON_RIGHT))) {		
		return target->tryHandle(obj, FXSEL(SEL_COMMAND, message), ptr);
	}
	return 0;
}

void I2KTrayIcon::setIcon(FXIcon* icon)
{
    mIcon = icon;

    if (mWIcon == NULL) return;

	DestroyIcon(mWIcon);

	mWIcon = createMswIcon(icon);

	NOTIFYICONDATA tray_data;
	setupNotifyData(&tray_data);
	Shell_NotifyIcon(NIM_MODIFY, &tray_data);
}

void I2KTrayIcon::setText(const FXString& text) {
    NOTIFYICONDATA tray_data;

	mTooltip = text;

    if (mWIcon = NULL) return;

    setupNotifyData(&tray_data);
	Shell_NotifyIcon(NIM_MODIFY, &tray_data);
}


#else

long I2KTrayIcon::onClose(FXObject*, FXSelector, void*) {
    return 1;
}

void I2KTrayIcon::setIcon(FXIcon* icon) {
	lbl->setIcon(icon);
}
    
FXIcon* I2KTrayIcon::getIcon() const {
	return lbl->getIcon();
}

void I2KTrayIcon::setText(const FXString& text) {
	lbl->setTipText(text);
}

const FXString& I2KTrayIcon::getText() const {
	return lbl->getTipText();
}

long I2KTrayIcon::onLeft(FXObject* obj, FXSelector sel, void* ptr) {
	FXEvent* ev = (FXEvent*)ptr;

	if (mOpts & TRAY_MENU_ON_LEFT) {
        popup(ev->click_x, ev->click_y);
        return 1;
    }
	
    if (mOpts & TRAY_CMD_ON_LEFT && target) {
		return target->tryHandle(obj, FXSEL(SEL_COMMAND, message), ptr);
	}
	
	return 0;
}

long I2KTrayIcon::onMouseWheel(FXObject* obj, FXSelector sel, void* ptr) {
	FXEvent* ev = (FXEvent*)ptr;
	
    if (target) {
		return target->tryHandle(obj, FXSEL(SEL_MOUSEWHEEL, message), ptr);
	}
	
	return 0;
}

long I2KTrayIcon::onRight(FXObject* obj, FXSelector, void* ptr) {
	FXEvent* ev = (FXEvent*)ptr;
    if (mOpts & TRAY_MENU_ON_RIGHT) {
        popup(ev->click_x, ev->click_y);
        return 1;
    }
	
    if (mOpts & TRAY_CMD_ON_RIGHT && target) {
		return target->tryHandle(obj, FXSEL(SEL_COMMAND, message), ptr);
	}

	return 0;
}

void I2KTrayIcon::popup(FXint x, FXint y) {
	translateCoordinatesTo(x,y,getRoot(), x,y);

	if (y > getRoot()->getHeight() / 2) {
		y -= mPup->getDefaultHeight();
	}

	mPup->popup(0, x, y);
}

#endif
}

