#include "AboutBox.h"

typedef const char const_char;
#define char const_char
#include "icons/icon32.xpm"
#undef char

FXIMPLEMENT(AboutBox, FXDialogBox, NULL, 0);

AboutBox::AboutBox(FXWindow* owner): FXDialogBox(owner,"About xtransfer",DECOR_TITLE|DECOR_BORDER|DECOR_CLOSE|DECOR_MENU, 0,0,0,0,10,10,10,10,8,12) {
	ico_main32 = new FXXPMIcon(getApp(), icon32_xpm);
	ico_main32->create();

	FXHorizontalFrame* cont = new FXHorizontalFrame(this, LAYOUT_SIDE_TOP, 0,0,0,0, 4,4,4,4, 10,10);

	new FXLabel(cont, "", ico_main32);

	new FXLabel(cont,
	"xtransfer 0.0.1\n"
	"Transfer files on Android devices.\n"
	"\n"
	"Written by xcomposite",
	NULL, JUSTIFY_LEFT);

	FXButton* okbtn = new FXButton(this, "OK", NULL, this, ID_ACCEPT,
	BUTTON_DEFAULT|BUTTON_INITIAL|LAYOUT_RIGHT|FRAME_THICK|FRAME_RAISED,
	0,0,75,23, 26,26,2,3);

	okbtn->setFocus();

}

void AboutBox::create() { FXDialogBox::create(); }

AboutBox::~AboutBox() {
	delete ico_main32;
}	
