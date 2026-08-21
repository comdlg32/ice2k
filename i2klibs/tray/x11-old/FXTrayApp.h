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
#ifndef FX_TRAY_APP
#define FX_TRAY_APP

#include "fx.h"
#include "FXArray.h"

namespace FX {

class FXTrayIcon;

/** An application wich is able to display tray icons.
 Use this class instead of FX::FXApp, or FX::FXTrayIcon might
 not work, as expected.
 */
class FXTrayApp : public FXApp
{
    FXDECLARE(FXTrayApp)
public:
    FXTrayApp(const FXString& name="Application",
        const FXString& vendor="FoxDefault");

    void create();

    /** @cond DEV */
#ifndef _WIN32
    FXID getTrayManager();
    void createTrayAtom();

    void registerIcon(FXTrayIcon* ic);
    void unregisterIcon(FXTrayIcon* ic);
#endif
protected:
#ifdef _WIN32
    long dispatchEvent(FXID hwnd, unsigned int iMsg, unsigned int wParam,long lParam);
#else
	bool dispatchEvent(FXRawEvent& ev);
#endif
    /** @endcond */
private:
#ifdef _WIN32
    unsigned int mTaskbarRestartId;
#else
    FXObjectListOf<FXTrayIcon> mIcons;
    FXID manager_id;
    FXID mManagerAtom;
    FXID mTrayAtom;
    FXID mXembedAtom;
#endif
};

}

#endif

