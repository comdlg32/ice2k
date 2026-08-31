#include <errno.h>
#include <fx.h>
#include <ice2k/comctl32.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
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

	FXButton* rem_btn;
	FXButton* prop_btn;

	FXVerticalFrame*        userscont;
	FXHorizontalFrame*      users_top_cont;

	FXPacker*   users_list_cont;
	FXPacker*   users_list_frame;

	FXIconList* users_list;

	FXHorizontalFrame* users_buttons;

	FXPacker* pass_cnt;
	FXGroupBox* pass_grp;

	FXText* pass_box;

	FXButton* reset_btn;

public:
	long onCmdRemove(FXObject*, FXSelector, void*);
	long onSelectUsersList(FXObject*, FXSelector, void*);
	long onDeSelectUsersList(FXObject*, FXSelector, void*);


public:
	enum {
		ID_DLG_OK = FXMainWindow::ID_LAST,
		ID_DLG_CANCEL,
		ID_DLG_APPLY,
		ID_REMOVE,
		ID_USERSLIST,
		ID_LAST
	};

public:
	UsersAndPasswords(FXApp* a);

	virtual void create();
	virtual void setFocus() {};
	virtual ~UsersAndPasswords();
};

FXDEFMAP(UsersAndPasswords) UsersAndPasswordsMap[] = {
	FXMAPFUNC(SEL_COMMAND,           UsersAndPasswords::ID_REMOVE,     UsersAndPasswords::onCmdRemove),
	FXMAPFUNC(SEL_SELECTED,          UsersAndPasswords::ID_USERSLIST,  UsersAndPasswords::onSelectUsersList),
	FXMAPFUNC(SEL_DESELECTED,        UsersAndPasswords::ID_USERSLIST,  UsersAndPasswords::onDeSelectUsersList),


};

FXIMPLEMENT(UsersAndPasswords, FXMainWindow, UsersAndPasswordsMap, ARRAYNUMBER(UsersAndPasswordsMap));

int getGroups(char arr[32][32], __uid_t uid) {
	struct passwd* pw = getpwuid(uid);
	if (pw == NULL) return 0;

	__gid_t groups[32];
	int ngroups = 31;

	getgrouplist(pw->pw_name, pw->pw_gid, groups, &ngroups);

	int i;
	for (i = 0; i < ngroups; ++i) {
		struct group* gr = getgrgid(groups[i]);
		if (gr != NULL) sprintf(arr[i], "%.*s", 31, gr->gr_name);
	}

	return i;
}

void genUsersList(FXIconList* list) {
	char name[2048];
	char groups[32][32];

	struct passwd* pw;
	while ( (pw = getpwent()) ) {
		if (pw->pw_uid == 0 || (pw->pw_uid >= 1000 && pw->pw_uid <= 65000)) {
			int ngroups = getGroups(groups, pw->pw_uid);
			int chars = sprintf(name, "%.*s\t", 31, pw->pw_name);

			for (int i = 0; i < ngroups; ++i) {
				if (chars < (int)sizeof(name)-1) {
					if (i == ngroups-1) {
						chars += sprintf(name+chars, "%.*s", 31, groups[i]);
					} else {
						chars += sprintf(name+chars, "%.*s, ", 31, groups[i]);
					}
				} else {
					break;
				}
			}

			list->appendItem(name, ico_user32, ico_user16);
		}
	}
}

UsersAndPasswords::UsersAndPasswords(FXApp *a) : FXMainWindow(a, "Users and Passwords", ico_main16, NULL, DECOR_CLOSE|DECOR_BORDER|DECOR_TITLE, 0,0,0,0) {
	cont = new FXVerticalFrame(this, LAYOUT_FILL_Y|LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0, 0,0);
	
	tabbook = new FXTabBook(cont, NULL, 0, TABBOOK_NORMAL|LAYOUT_FILL, 0,0,0,0, 6,6,7,5);

	new FXTabItem(tabbook, "Users ", NULL, TAB_TOP_NORMAL, 0,0,0,0, 6,6,1,2);
	userscont = new FXVerticalFrame(tabbook, LAYOUT_FILL|FRAME_RAISED|FRAME_THICK, 0,0,0,0, 11,10,10,12, 2,2);
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
	users_list = new FXIconList(users_list_frame, this, ID_USERSLIST, LAYOUT_FILL|ICONLIST_DETAILED|ICONLIST_SINGLESELECT|HSCROLLER_NEVER);

	users_list->appendHeader("User Name", NULL, 168);
	users_list->appendHeader("Group", NULL, 168);


	/*users_list->getHeader()->setPadLeft(4);
	users_list->getHeader()->setPadTop(0);

	users_list->getHeader()->setPadBottom(-1);*/
	users_list->getHeader()->setPadTop(0);
	users_list->getHeader()->setPadBottom(0);

	/*users_list->appendItem("Administrator\tAdministrators", ico_user32, ico_user16);
	users_list->appendItem("Guest\tGuests", ico_user32, ico_user16);
	users_list->appendItem("xcomp\tAdministrators", ico_user32, ico_user16);*/

	genUsersList(users_list);


	new FXFrame(users_list_cont, FRAME_NONE|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0,0,357,0, 0,0,0,0);

	users_buttons = new FXHorizontalFrame(userscont, LAYOUT_RIGHT|PACK_UNIFORM_WIDTH, 0,0,0,0, 0,2,0,0, 6,6);

	new FXButton(users_buttons, "A&dd...", NULL, NULL, 0, BUTTON_NORMAL|BUTTON_DEFAULT, 0,0,0,0, 11,11,2,3);
	rem_btn = new FXButton(users_buttons, "&Remove", NULL, this, ID_REMOVE, BUTTON_NORMAL|BUTTON_DEFAULT, 0,0,0,0, 11,11,2,3);
	prop_btn = new FXButton(users_buttons, "Pr&operties", NULL, NULL, 0, BUTTON_NORMAL|BUTTON_DEFAULT, 0,0,0,0, 11,11,2,3);


	pass_cnt = new FXPacker(userscont, LAYOUT_FILL_X, 0,0,0,0, 2,2,6,1);
	pass_grp = new FXGroupBox(pass_cnt, "Password for tf", LAYOUT_FILL_X|FRAME_GROOVE, 0,0,0,0, 9,10,5,11, 3,3);
	new FXLabel(pass_grp, "", ico_user32, LAYOUT_SIDE_LEFT);
	pass_box = new FXText(pass_grp, NULL, 0, LAYOUT_FILL_X|TEXT_WORDWRAP|VSCROLLER_NEVER|HSCROLLER_NEVER, 0,0,0,0, 0,0,0,0);
	pass_box->disable();
	pass_box->setBackColor(getApp()->getBaseColor());
	pass_box->setDefaultCursor(getApp()->getDefaultCursor(DEF_ARROW_CURSOR));
	pass_box->setVisibleRows(2);
	pass_box->setText("To change the password for tf, click Set Password.");
	//new FXLabel(pass_grp, "To change the password for tf, click Set Password.", NULL, LAYOUT_SIDE_TOP);
	reset_btn = new FXButton(pass_grp, "Set &Password...", NULL, NULL, 0, BUTTON_NORMAL|BUTTON_DEFAULT|LAYOUT_SIDE_BOTTOM|LAYOUT_RIGHT, 0,0,0,0, 15,15,2,3);
	reset_btn->disable();


	FXHorizontalFrame* btncont = new FXHorizontalFrame(cont, LAYOUT_RIGHT|PACK_UNIFORM_WIDTH, 0,0,0,0, 0,6,1,7, 6,0);

	okbtn = new FXButton(btncont, "OK", NULL, this, ID_DLG_OK, BUTTON_DEFAULT|BUTTON_NORMAL, 0,0,0,0, 19,20,2,3);
	cancelbtn = new FXButton(btncont, "Cancel", NULL, this, ID_DLG_CANCEL, BUTTON_NORMAL|BUTTON_DEFAULT, 0,0,0,0, 19,20,2,3);
	applybtn = new FXButton(btncont, "&Apply", NULL, this, ID_DLG_APPLY, BUTTON_NORMAL|BUTTON_DEFAULT, 0,0,0,0, 19,19,2,3);

	applybtn->disable();
	rem_btn->disable();
	prop_btn->disable();
	//users_list->setNumVisible(4);
	//new FXLabel(userscont, "SMOKE ROCK BIETCHHHH =)");

}

UsersAndPasswords::~UsersAndPasswords() {
}

void UsersAndPasswords::create() {
	FXMainWindow::create();

	show(PLACEMENT_SCREEN);
}
	
long UsersAndPasswords::onSelectUsersList(FXObject*, FXSelector, void*) {
	char user[32];
	char info[256];

	sprintf(user, "%.31s", users_list->getItem(users_list->getCurrentItem())->getText().text());
	user[strcspn(user, "\t")] = 0;

	//size_t len = strcspn(item, "\t");

	sprintf(info, "To change the password for %.31s, click Set Password.", user);
	pass_box->setText(info);
	reset_btn->enable();
	rem_btn->enable();
	prop_btn->enable();

	return 1;
}
long UsersAndPasswords::onDeSelectUsersList(FXObject*, FXSelector, void*) {
	reset_btn->disable();

	rem_btn->disable();
	prop_btn->disable();

	return 1;
}


long UsersAndPasswords::onCmdRemove(FXObject*, FXSelector, void*) {
	char item[512];
	char curuser[32];

	uid_t uid = getuid();
	struct passwd* pw = getpwuid(uid);
	sprintf(item, "%.511s", users_list->getItem(users_list->getCurrentItem())->getText().text());

	size_t len = strcspn(item, "\t");

	if (pw != NULL) {
		sprintf(curuser, "%.31s", pw->pw_name);
	} else {
		strcpy(curuser, "?");
	}

	if (strncmp(item, "root", len) == 0) {
		FXMessageBox::error(this, MBOX_OK, "Error", "You cannot delete the root user!");
	} else if (strncmp(item, curuser, len) == 0) {
		FXMessageBox::error(this, MBOX_OK, "Error", "You cannot delete your own user!");
	}

	//puts("Hello world!");
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
