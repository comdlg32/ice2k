#include <fx.h>
#include <ice2k/comctl32.h>

#include "res/foxres.h"

FXIcon* mainIcon;

class UsersAndPasswords : public FXMainWindow {
	FXDECLARE(UsersAndPasswords);

protected:
	UsersAndPasswords() {}

private:
	FXVerticalFrame*        cont;
	FXTabBook*              tabbook;

	FXButton*               okbtn;
	FXButton*               cancelbtn;
	FXButton*               applybtn;

	FXVerticalFrame*        userscont;


public:
	long onCmdHello(FXObject*, FXSelector, void*);

public:
	enum {
		ID_MAINWIN = FXMainWindow::ID_LAST,
		ID_HELLO,
		ID_LAST
	};

public:
	UsersAndPasswords(FXApp* a);

	virtual void create();
	virtual ~UsersAndPasswords();
};

FXDEFMAP(UsersAndPasswords) UsersAndPasswordsMap[] = {
	FXMAPFUNC(SEL_COMMAND,           UsersAndPasswords::ID_HELLO,  UsersAndPasswords::onCmdHello),
};

FXIMPLEMENT(UsersAndPasswords, FXMainWindow, UsersAndPasswordsMap, ARRAYNUMBER(UsersAndPasswordsMap));

UsersAndPasswords::UsersAndPasswords(FXApp *a) : FXMainWindow(a, "Users and Passwords", mainIcon, NULL, DECOR_ALL, 0,0,0,0) {
	cont = new FXVerticalFrame(this, LAYOUT_FILL_Y|LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0, 0,0);
	
	tabbook = new FXTabBook(cont, NULL, 0, TABBOOK_NORMAL|LAYOUT_FILL, 0,0,0,0, 6,6,5,5);
	new FXTabItem(tabbook, "Users ", NULL, TAB_TOP_NORMAL, 0,0,0,0, 4,4,1,2);
	userscont = new FXVerticalFrame(tabbook, LAYOUT_FILL|FRAME_RAISED|FRAME_THICK, 0,0,0,0, 13,2,11,10, 8,8);

	new FXLabel(userscont, "SMOKE ROCK BIETCHHHH =)");

}

UsersAndPasswords::~UsersAndPasswords() {
}

void UsersAndPasswords::create() {
	FXMainWindow::create();

	show(PLACEMENT_SCREEN);
}
	
long UsersAndPasswords::onCmdHello(FXObject*, FXSelector, void*) {
	puts("Hello world!");
	return 1;
}

int main(int argc, char *argv[]) {
	FXApp application("netplwiz", "I2KProject");
	mainIcon = new FXGIFIcon(&application, resico_mainicon, 0, IMAGE_OPAQUE);

	application.init(argc, argv);

	new UsersAndPasswords(&application);

	application.create();
	return application.run();
}
