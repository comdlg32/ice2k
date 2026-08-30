#include <fx.h>
#include <ice2k/comctl32.h>

#include "res/foxres.h"

FXIcon* ico_main16;
FXIcon* ico_main32;

FXIcon* ico_user16;
FXIcon* ico_user32;

FXImage* img_banner;

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
	FXHorizontalFrame*      users_top_cont;

	FXPacker*   users_list_cont;
	FXPacker*   users_list_frame;

	FXIconList* users_list;

	FXHorizontalFrame* users_buttons;

	FXPacker* pass_cnt;
	FXGroupBox* pass_grp;

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

UsersAndPasswords::UsersAndPasswords(FXApp *a) : FXMainWindow(a, "Users and Passwords", ico_main16, NULL, DECOR_CLOSE|DECOR_BORDER|DECOR_TITLE, 0,0,0,0) {
	cont = new FXVerticalFrame(this, LAYOUT_FILL_Y|LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0, 0,0);
	
	tabbook = new FXTabBook(cont, NULL, 0, TABBOOK_NORMAL|LAYOUT_FILL, 0,0,0,0, 6,6,7,5);

	new FXTabItem(tabbook, "Users ", NULL, TAB_TOP_NORMAL, 0,0,0,0, 6,6,1,2);
	userscont = new FXVerticalFrame(tabbook, LAYOUT_FILL|FRAME_RAISED|FRAME_THICK, 0,0,0,0, 11,10,10,10, 2,2);
	users_top_cont = new FXHorizontalFrame(userscont, LAYOUT_FILL_X, 0,0,0,0, 0,0,0,7, 5,5);

	new FXLabel(users_top_cont, "", ico_main32);

	new FXLabel(users_top_cont,
			"Use the list below to grant or deny users access to your\n"
			"computer, and to change passwords and other settings.");

	new FXCheckButton(userscont, "Users must &enter a user name and password to use this computer.",
			NULL, 0, CHECKBUTTON_NORMAL, 0,0,0,0, 2,2,2,6);

	new FXLabel(userscont, "&Users for this computer:");

	users_list_cont = new FXPacker(userscont, LAYOUT_FILL_X, 0,0,0,0, 2,2,0,1);
	users_list_frame = new FXPacker(users_list_cont, LAYOUT_FILL_X|FRAME_NORMAL|LAYOUT_FIX_HEIGHT, 0,0,0,117, 0,0,0,0);
	users_list = new FXIconList(users_list_frame, NULL, 0, LAYOUT_FILL|ICONLIST_DETAILED|HSCROLLER_NEVER);

	users_list->appendHeader("User Name", NULL, 168);
	users_list->appendHeader("Group", NULL, 168);


	/*users_list->getHeader()->setPadLeft(4);
	users_list->getHeader()->setPadTop(0);

	users_list->getHeader()->setPadBottom(-1);*/
	users_list->getHeader()->setPadTop(1);
	users_list->getHeader()->setPadBottom(1);

	users_list->appendItem("Administrator\tAdministrators", ico_user32, ico_user16);
	users_list->appendItem("Guest\tGuests", ico_user32, ico_user16);
	users_list->appendItem("xcomp\tAdministrators", ico_user32, ico_user16);


	new FXFrame(users_list_cont, FRAME_NONE|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0,0,357,0, 0,0,0,0);

	users_buttons = new FXHorizontalFrame(userscont, LAYOUT_RIGHT|PACK_UNIFORM_WIDTH, 0,0,0,0, 0,2,0,0, 6,6);

	new FXButton(users_buttons, "A&dd...", NULL, NULL, 0, BUTTON_NORMAL|BUTTON_DEFAULT, 0,0,0,0, 11,11,2,3);
	new FXButton(users_buttons, "&Remove", NULL, NULL, 0, BUTTON_NORMAL|BUTTON_DEFAULT, 0,0,0,0, 11,11,2,3);
	new FXButton(users_buttons, "Pr&operties", NULL, NULL, 0, BUTTON_NORMAL|BUTTON_DEFAULT, 0,0,0,0, 11,11,2,3);


	pass_cnt = new FXPacker(userscont, LAYOUT_FILL_X, 0,0,0,0, 2,2,6,1);
	pass_grp = new FXGroupBox(pass_cnt, "Password for xcomp", LAYOUT_FILL_X|FRAME_GROOVE, 0,0,0,0, 9,10,5,11, 3,13);
	new FXLabel(pass_grp, "", ico_user32, LAYOUT_SIDE_LEFT);
	new FXLabel(pass_grp, "To change the password for xcomp, click Set Password.", NULL, LAYOUT_SIDE_TOP);
	new FXButton(pass_grp, "Set &Password...", NULL, NULL, 0, BUTTON_NORMAL|BUTTON_DEFAULT|LAYOUT_SIDE_BOTTOM|LAYOUT_RIGHT, 0,0,0,0, 15,15,2,3);


	//users_list->setNumVisible(4);
	//new FXLabel(userscont, "SMOKE ROCK BIETCHHHH =)");

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

	ico_main16 = new FXGIFIcon (&application, res_ico_main16);
	ico_main32 = new FXGIFIcon (&application, res_ico_main32);

	ico_user16 = new FXGIFIcon (&application, res_ico_user16);
	ico_user32 = new FXGIFIcon (&application, res_ico_user32);


	img_banner = new FXGIFImage(&application, res_img_banner);

	application.init(argc, argv);

	new UsersAndPasswords(&application);

	application.create();
	return application.run();
}
