/*
FoxTray, system tray icon for the FOX Toolkit

Copyright (c) 2008, Hendrik Rittich
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

#ifndef FX_TRAY_ICON_H
#define FX_TRAY_ICON_H

#include "fx.h"
#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

namespace FX {

/** @defgroup tray_options FXTrayIcon options 
  @{*/

/** FXTrayIcon options.
 Describe what to do on mouse interaction with the icon.
 */
enum {
    TRAY_MENU_ON_LEFT = 0x0001, /**< Open popup menu on mouse left click. */
    TRAY_MENU_ON_RIGHT = 0x0002, /**< Open popup menu on mouse right click. */
    TRAY_CMD_ON_LEFT = 0x0004, /**< Send SEL_COMMAND message on mouse left click. */
    TRAY_CMD_ON_RIGHT = 0x0008 /**< Send SEL_COMMAND message on mouse right click. */
};
/** @} */

/** Display an icon in the system tray.
 Display an icon in the system tray, also
 called notification area.
 */
class FXTrayIcon : public FXTopWindow
{
    FXDECLARE(FXTrayIcon)
public: 
    /** @cond DEV */
#ifndef _WIN32
	enum {
        ID_BUTTON = FXTopWindow::ID_LAST,
        ID_LAST
    };
#endif
    /** @endcond */

    /** Create a tray icon.
     @param app the current FXApp instance
     @param text the tool tip text
     @param icon the icon to display in the tray
     @param pup the popup menu to display
     @param target send messages to
     @param sel id 
     @param opts options. See @ref tray_options .
     */
    FXTrayIcon(FXApp* app, const FXString& text, FXIcon* icon, 
            FXPopup* pup = NULL, FXObject* target = 0, FXSelector sel = 0,
            FXuint opts = TRAY_MENU_ON_LEFT);

    ~FXTrayIcon(); 

    /** Create the server side resource of the tray icon. */
    void create();

    /** @cond DEV */
    void mapToManager(); 

#ifdef _WIN32
	long onEvent(FXObject*, FXSelector, void* ptr);
#else
    long onLeft(FXObject*, FXSelector, void*);
    long onRight(FXObject* obj, FXSelector, void* ptr);
    long onClose(FXObject*, FXSelector, void*);
#endif
    /** @endcond */

    /** Set the popup menu. */
    void setMenu(FXPopup* pup) { mPup = pup; }
    /** Get the popup menu. */
    FXPopup* getMenu() const { return mPup; }

    /** Change the icon in the tray. */
    void setIcon(FXIcon* icon);
#ifdef _WIN32
	/** @cond DEV */
    inline FXIcon* getIcon() const { return mIcon; } 
	/** @endcond */
#else
    /** Get the icon. */
    FXIcon* getIcon() const;
#endif

    /** Change the tool tip text. */
    void setText(const FXString& text);
    /** Get the tool tip text. */
#ifdef _WIN32
	/** @cond DEV */
	inline const FXString& getText() const { return mTooltip; }
	/** @endcond */
#else
    const FXString& getText() const;
#endif

#ifdef _WIN32
	/** @cond DEV */
	static HICON createMswIcon(FXIcon* icon);
	/** @endcond */
#endif
private:
    FXPopup* mPup;
    FXuint mOpts;
#ifdef _WIN32
	HICON mWIcon;
	FXIcon* mIcon;
	DWORD mTrayID;

	FXString mTooltip;

	static DWORD sTrayIconCount;
#else
    FXButton* btn;
#endif

#ifdef _WIN32
	void setupNotifyData(NOTIFYICONDATA* data);
#else
    void popup(FXint x, FXint y);
#endif
    FXTrayIcon() {}
};

/** @example TrayApp.cpp
    A small example Application.
*/

}

#endif

