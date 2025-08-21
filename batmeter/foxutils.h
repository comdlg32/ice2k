#pragma once
#include <fx.h>

#ifndef FXTOOLBAR_H
#include "FXToolBar.h"
#endif

FXIcon* loadGifIcon(FXApp* app, const void* pix = NULL, FXColor clr = FXRGB(192,192,192), FXColor blendclr = FXRGB(192,192,192), FXuint opts = 0, FXint w = 1, FXint h = 1);

void loadIcons(FXApp* ptrapp);
void unloadIcons(FXApp* ptrapp);


namespace UH {
	/// Menu bar
	class FXAPI UHMenuBar : public FXMenuBar {
		FXDECLARE(UHMenuBar)

		protected:
			UHMenuBar(){}

		private:
			UHMenuBar(const UHMenuBar&);
			UHMenuBar &operator=(const UHMenuBar&);

		public:

		/**
		* Construct a floatable menubar
		* Normally, the menubar is docked under window p.
		* When floated, the menubar can be docked under window q, which is
		* typically an FXToolbarShell window.
		*/
		UHMenuBar(FXComposite* p, FXComposite* q, FXuint opts=LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X,
		          FXint x=0, FXint y=0, FXint w=0, FXint h=0,
		          FXint pl=0, FXint pr=0, FXint pt=0, FXint pb=0, FXint hs=0, FXint vs=0);

		/**
		* Construct a non-floatable menubar.
		* The menubar can not be undocked.
		*/
		UHMenuBar(FXComposite* p, FXuint opts,
		          FXint x=0, FXint y=0, FXint w=0, FXint h=0,
                          FXint pl=0, FXint pr=0, FXint pt=0, FXint pb=0, FXint hs=0, FXint vs=0);

	};

}

