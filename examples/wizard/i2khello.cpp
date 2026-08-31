#include <fx.h>
#include <ice2k/comctl32.h>
#include <ice2k/wizard/I2KWizard.h>
// #include <ice2k/wizard/I2KWizHeader.h>

#include "res/foxres.h"

FXIcon* ico_main16;
FXImage* img_banner;

class HelloWindow : public FXMainWindow {
	FXDECLARE(HelloWindow);

protected:
	HelloWindow() {}

private:
	I2KWizard* wiz;
	FXHorizontalFrame* userinfo_page;
	FXVerticalFrame* userinfo_main;

	FXHorizontalFrame* password_page;
	FXVerticalFrame* password_main;

	FXMatrix* userinfo_fld_mtx;
	FXMatrix* password_fld_mtx;



public:
	long onCmdHello(FXObject*, FXSelector, void*);


public:
	enum {
		ID_WIZARD = FXMainWindow::ID_LAST,
		ID_LAST
	};

public:
	HelloWindow(FXApp* a);

	virtual void create();
	virtual ~HelloWindow();
};

FXDEFMAP(HelloWindow) HelloWindowMap[] = {
};

FXIMPLEMENT(HelloWindow, FXMainWindow, HelloWindowMap, ARRAYNUMBER(HelloWindowMap));

HelloWindow::HelloWindow(FXApp *a) : FXMainWindow(a, "Add New User", ico_main16, NULL, DECOR_BORDER|DECOR_CLOSE|DECOR_TITLE, 0,0,0,0) {
	wiz = new I2KWizard(this, this, ID_WIZARD);

	userinfo_page = new FXHorizontalFrame(wiz->getSwitcher(), LAYOUT_FILL, 0,0,0,0, 0,1,0,0, 7,7);
	new FXImageFrame(userinfo_page, img_banner, LAYOUT_FIX_HEIGHT, 0,0,0,img_banner->getHeight()-2);
	userinfo_main = new FXVerticalFrame(userinfo_page, LAYOUT_FILL, 0,0,0,0, 9,9,9,9, 8,8);

	new FXLabel(userinfo_main, "Enter the basic information for the new user.");
	userinfo_fld_mtx = new FXMatrix(userinfo_main, 2, MATRIX_BY_COLUMNS, 0,0,0,0, 0,0,0,0, 16,6);
	new FXLabel(userinfo_fld_mtx, "&User name:", NULL, LAYOUT_CENTER_Y);
	new FXTextField(userinfo_fld_mtx, 33, NULL, 0, TEXTFIELD_NORMAL, 0,0,0,0, 4,0,1,4);
	new FXLabel(userinfo_fld_mtx, "&Full name:", NULL, LAYOUT_CENTER_Y);
	new FXTextField(userinfo_fld_mtx, 33, NULL, 0, TEXTFIELD_NORMAL, 0,0,0,0, 4,0,1,4);
	new FXLabel(userinfo_fld_mtx, "&Description:", NULL, LAYOUT_CENTER_Y);
	new FXTextField(userinfo_fld_mtx, 33, NULL, 0, TEXTFIELD_NORMAL, 0,0,0,0, 4,0,1,4);
	new FXLabel(userinfo_main, "To continue, click Next.", NULL, LABEL_NORMAL, 0,0,0,0, 2,2,24,2);


	password_page = new FXHorizontalFrame(wiz->getSwitcher(), LAYOUT_FILL, 0,0,0,0, 0,1,0,0, 7,7);
	new FXImageFrame(password_page, img_banner, LAYOUT_FIX_HEIGHT, 0,0,0,img_banner->getHeight()-2);
	password_main = new FXVerticalFrame(password_page, LAYOUT_FILL, 0,0,0,0, 9,9,9,9, 8,8);

	new FXLabel(password_main, "Type and confirm a password for this user.");
	password_fld_mtx = new FXMatrix(password_main, 2, MATRIX_BY_COLUMNS|LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0, 16,6);
	new FXLabel(password_fld_mtx, "&Password:", NULL, LAYOUT_CENTER_Y);
	new FXTextField(password_fld_mtx, 10, NULL, 0, TEXTFIELD_NORMAL|TEXTFIELD_PASSWD|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN, 0,0,0,0, 4,0,1,4);
	new FXLabel(password_fld_mtx, "&Confirm password:", NULL, LAYOUT_CENTER_Y);
	new FXTextField(password_fld_mtx, 10, NULL, 0, TEXTFIELD_NORMAL|TEXTFIELD_PASSWD|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN, 0,0,0,0, 4,0,1,4);
	new FXLabel(password_main, "To continue, click Next.", NULL, LABEL_NORMAL, 0,0,0,0, 2,2,24,2);
}

HelloWindow::~HelloWindow() {
}

void HelloWindow::create() {
	FXMainWindow::create();
	show(PLACEMENT_SCREEN);
}


int main(int argc, char *argv[]) {
	FXApp application("WizardTest", "I2KTest");
	ico_main16 = new FXGIFIcon(&application, res_ico_main16, 0, IMAGE_OPAQUE);
	img_banner = new FXGIFImage(&application, res_img_banner);


	application.init(argc, argv);

	new HelloWindow(&application);

	application.create();
	return application.run();
}
