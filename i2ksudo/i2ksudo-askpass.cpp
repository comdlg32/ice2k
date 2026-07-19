#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <fx.h>
#include <fxkeys.h>
#include <FXPNGImage.h>
#include <ice2k/comctl32.h>
#include <ice2k/branding.h>


#include "res/foxres.h"

FXIcon* mainIcon;
FXIcon* userIcon;

FXImage* authBannerImg;

char command[8192] = {0};

/*char pwusername[LOGIN_NAME_MAX+1] = {0};*/
char userid[HOST_NAME_MAX+1+LOGIN_NAME_MAX+1] = {0};


class CommandBox : public FXDialogBox {
	FXDECLARE(CommandBox)

	private:
		FXPacker* textframe;
		FXText* text;
		FXButton* okbtn;

	protected:
		CommandBox() {}

	public:
		void setFocus() { return; }

		CommandBox(FXWindow* owner) : FXDialogBox(owner, "Command", DECOR_TITLE|DECOR_BORDER|DECOR_CLOSE|DECOR_MENU, 0, 0, 320, 240,
                                  8,8,8,8, 8, 8) {
			okbtn = new FXButton(this, "OK", NULL, this, ID_ACCEPT,
					BUTTON_DEFAULT|LAYOUT_RIGHT|LAYOUT_SIDE_BOTTOM|FRAME_NORMAL,
					0,0,0,0, 26,26,3,2);
			okbtn->setFocus();

			new FXLabel(this, "Command:");

			textframe = new FXPacker(this, FRAME_NORMAL|LAYOUT_FILL_Y|LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0, 0,0);
			
			text = new FXText(textframe, NULL, 0, TEXT_READONLY|LAYOUT_FILL_Y|LAYOUT_FILL_X);
			text->setBackColor(getApp()->getBaseColor());

			char* tmpenv = getenv("SUDO_COMMAND");

			if (tmpenv == NULL) {
				strcpy(command, "Unknown");
			} else {
				sprintf(command, "%.*s\n", (int)sizeof(command)-2, tmpenv);
				command[sizeof(command)-1] = '\0';
			}
			
			text->setText(command);
		}

		virtual void create() { FXDialogBox::create(); };

		virtual ~CommandBox() {};
};

FXIMPLEMENT(CommandBox, FXDialogBox, NULL, 0);




class SudoWindow : public FXMainWindow {
	FXDECLARE(SudoWindow);

protected:
	SudoWindow() {}

private:
	FXVerticalFrame *contents;
	FXMatrix* credmtx;

	FXHorizontalFrame* userfield;
	FXLabel* userfieldicon;
	FXTextField* userfieldtxt;

	FXHorizontalFrame* btncont;

	FXTextField* pwfieldtxt;

	FXCheckButton* rememberchk;
	
	FXButton* okbtn;
	FXButton* cancelbtn;

	FXButton* commandbtn;

public:
	long onCmdAccept(FXObject*, FXSelector, void*);
	long onTimeoutAccept(FXObject*, FXSelector, void*);

	long onCmdCancel(FXObject*, FXSelector, void*);
	long onCmdCommand(FXObject*, FXSelector, void*);

	long onKeyPress(FXObject*, FXSelector, void*);
	long onKeyRelease(FXObject*, FXSelector, void*);

	void setFocus() { return; }


public:
	enum {
		ID_ACCEPT = FXMainWindow::ID_LAST,
		ID_CANCEL,
		ID_COMMAND,
		ID_LAST
	};

public:
	SudoWindow(FXApp* a);

	virtual void create();
	virtual ~SudoWindow();
};

FXDEFMAP(SudoWindow) SudoWindowMap[] = {
	FXMAPFUNC(SEL_KEYPRESS,          0,                      SudoWindow::onKeyPress),
	FXMAPFUNC(SEL_KEYRELEASE,        0,                      SudoWindow::onKeyRelease),

	FXMAPFUNC(SEL_TIMEOUT,           SudoWindow::ID_ACCEPT,  SudoWindow::onTimeoutAccept),
	FXMAPFUNC(SEL_COMMAND,           SudoWindow::ID_ACCEPT,  SudoWindow::onCmdAccept),

	FXMAPFUNC(SEL_COMMAND,           SudoWindow::ID_CANCEL,  SudoWindow::onCmdCancel),
	FXMAPFUNC(SEL_COMMAND,           SudoWindow::ID_COMMAND,  SudoWindow::onCmdCommand),

};

long SudoWindow::onCmdCommand(FXObject*, FXSelector, void*) {
	CommandBox cmdbox(this);
	cmdbox.execute(PLACEMENT_OWNER);

	return 1;
}

FXIMPLEMENT(SudoWindow, FXMainWindow, SudoWindowMap, ARRAYNUMBER(SudoWindowMap));

int checkPassword(const char* pw) {
	/*char command[ (sizeof("su -c /bin/true ")-1) + LOGIN_NAME_MAX+1] = {0};*/
	int p[2];
	
	char input[1024];
	int len = sprintf(input, "%.*s\n", (int)sizeof(input)-2, pw);

	if (pipe(p) == -1) return 0;

	pid_t pid = fork();
	if (pid < 0) return 0;

	if (pid == 0) {
		dup2(p[0], STDIN_FILENO);
		close(p[0]);
		close(p[1]);
		
		int devnull = open("/dev/null", O_WRONLY);

		if (devnull != -1) {
			dup2(devnull, STDERR_FILENO);
			dup2(devnull, STDOUT_FILENO);
			close(devnull);
		}

		execlp("sudo", "sudo", "-Skvp", "", (char*)NULL);
		exit(1);
	} else {
		close(p[0]);

		write(p[1], input, len);
		close(p[1]);
		
		int status;

		waitpid(pid, &status, 0);

		if (WIFEXITED(status)) return !(WEXITSTATUS(status));
	}

	return 0;
}



SudoWindow::SudoWindow(FXApp *a) : FXMainWindow(a, "Run as root", userIcon, NULL, DECOR_BORDER|DECOR_TITLE|DECOR_CLOSE|DECOR_MENU, 0,0,0,0) {
	/*new FXButton(this, "Hello World!", NULL, this, ID_HELLO, BUTTON_NORMAL);*/

	int padtop;
	
	if (i2kBGetWinVersionInt() == ICE2K_BRAND_WIN2K) {
		new FXFrame(this, LAYOUT_FIX_HEIGHT|LAYOUT_FIX_WIDTH, 0,0,authBannerImg->getWidth(),0, 0,0,0,0);
		padtop = 11;
	} else {
		new FXImageFrame(this, authBannerImg, FRAME_NONE);
		padtop = 16;
	}

	contents = new FXVerticalFrame(this, LAYOUT_FILL_X, 0,0,0,0, 9,9,padtop,11, 0,0);
	new FXLabel(contents, "The action you requested needs root privileges.");

	credmtx = new FXMatrix(contents, 3, MATRIX_BY_COLUMNS|LAYOUT_FILL_X, 0,0,0,0, 2,2,11,44, 6,7);
	new FXLabel(credmtx, "&User name:              ", NULL, LAYOUT_CENTER_Y, 0,0,0,0, 0,1,0,0);

	userfield = new FXHorizontalFrame(credmtx, LAYOUT_FILL_X|LAYOUT_FILL_COLUMN|FRAME_NORMAL, 0,0,0,0, 0,0,0,0, 2,2);
	//userfield->setBackColor(getApp()->getBackColor());
	userfieldicon = new FXLabel(userfield, "", userIcon, LAYOUT_CENTER_Y, 0,0,0,0, 0,0,0,0);
	//userfieldicon->setBackColor(getApp()->getBackColor());
	userfieldtxt = new FXTextField(userfield, 10, NULL, 0, LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 3,2,2,2);
	userfieldtxt->setText(userid);
	userfieldtxt->disable();
	userfieldtxt->setTextColor(userfieldtxt->getShadowColor());

	FXArrowButton* arrowbtn = new FXArrowButton(userfield, NULL, 0, FRAME_NORMAL|ARROW_DOWN|LAYOUT_FILL_Y, 0,0,0,0, 1,2,0,0);
	arrowbtn->disable();
	arrowbtn->setBaseColor(arrowbtn->getHiliteColor());
	arrowbtn->setHiliteColor(arrowbtn->getBackColor());
	
	FXButton* dotbtn = new FXButton(credmtx, "&...", NULL,NULL, 0, BUTTON_NORMAL, 0,0,0,0, 1,2,1,2);
	dotbtn->disable();
	//userfieldtxt->setBaseColor(getApp()->getBackColor());
	//userfieldtxt->setBaseColor(getApp()->getBackColor());

	//new FXLabel(credmtx, "&Password:", NULL, LAYOUT_CENTER_Y, 0,0,0,0, 0,43,0,0);
	new FXLabel(credmtx, "&Password:", NULL, LAYOUT_CENTER_Y, 0,0,0,0, 0,1,0,0);
	pwfieldtxt = new FXTextField(credmtx, 10, this, ID_ACCEPT,
			TEXTFIELD_ENTER_ONLY|TEXTFIELD_PASSWD|
			FRAME_NORMAL|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN, 0,0,0,0, 4,4,1,1);
	new FXFrame(credmtx, FRAME_NONE);


	new FXFrame(credmtx, FRAME_NONE);
	rememberchk = new FXCheckButton(credmtx, "&Remember my password", NULL, 0, CHECKBUTTON_NORMAL|LAYOUT_FILL_COLUMN);
	rememberchk->setCheck(TRUE);
	rememberchk->disable();

	btncont = new FXHorizontalFrame(contents, LAYOUT_FILL_X|PACK_UNIFORM_WIDTH, 0,0,0,0, 3,2,0,0, 6,6);

	commandbtn = new FXButton(btncont, "Command...", NULL, this, ID_COMMAND, BUTTON_NORMAL|BUTTON_DEFAULT, 0,0,0,0, 6,6,2,3);
	cancelbtn = new FXButton(btncont, "Cancel", NULL, getApp(), FXApp::ID_QUIT, LAYOUT_RIGHT|BUTTON_NORMAL|BUTTON_DEFAULT, 0,0,0,0, 6,6,2,3);
	okbtn = new FXButton(btncont, "OK", NULL, this, ID_ACCEPT, LAYOUT_RIGHT|BUTTON_NORMAL|BUTTON_DEFAULT, 0,0,0,0, 6,6,2,3);

}

SudoWindow::~SudoWindow() {
}

void SudoWindow::create() {
	FXMainWindow::create();
	pwfieldtxt->setFocus();

	show(PLACEMENT_SCREEN);
}

long SudoWindow::onKeyPress(FXObject* obj, FXSelector sel, void* ptr) {
	if (FXTopWindow::onKeyPress(obj, sel, ptr)) return 1;
	
	if ( ((FXEvent*)ptr)->code == KEY_Escape) {
		getApp()->handle(getApp(), FXSEL(SEL_COMMAND, FXApp::ID_QUIT), NULL);
		return 1;
	}

	return 0;
}

long SudoWindow::onKeyRelease(FXObject* obj, FXSelector sel, void* ptr) {
	if (FXTopWindow::onKeyRelease(obj, sel, ptr)) return 1;
	if ( ((FXEvent*)ptr)->code == KEY_Escape) return 1;

	return 0;
}

long SudoWindow::onCmdAccept(FXObject*, FXSelector, void*) {
	pwfieldtxt->disable();
	pwfieldtxt->setTextColor(getApp()->getShadowColor());

	okbtn->disable();
	cancelbtn->disable();
	commandbtn->disable();
	//rememberchk->disable();

	getApp()->addTimeout(this, ID_ACCEPT, 1);
	
	return 1;
}


long SudoWindow::onTimeoutAccept(FXObject*, FXSelector, void*) {
	char pw[4096] = {0};
	strcpy(pw, pwfieldtxt->getText().text());

	if (checkPassword(pw)) {
		getApp()->exit(0);
		puts(pw);
		exit(0);
	} else {
		FXMessageBox::error(this, MBOX_OK, "Logon Message",
				"Unable to log on:\n"
				"\n"
				"Logon failure: unknown user name or bad password.");
	}

	pwfieldtxt->enable();
	pwfieldtxt->setTextColor(getApp()->getForeColor());
	okbtn->enable();
	cancelbtn->enable();
	commandbtn->enable();
	//rememberchk->enable();

	pwfieldtxt->setFocus();

	return 1;
}

long SudoWindow::onCmdCancel(FXObject*, FXSelector, void*) {
	puts("Hello");
	return 1;
}

int main(int argc, char *argv[]) {
	FXApp application("I2KSudo", "I2KProject");

	if (argv[1] == NULL) {
		char hostname[HOST_NAME_MAX+1];
		gethostname(hostname, sizeof(hostname));
		
		sprintf(userid, "%s\\%s", hostname, getlogin());
	} else {
		sprintf(userid, "%.*s", (int)sizeof(userid)-1, argv[1]);
	}

	//printf("%d\n", checkPassword(argv[2]));

	mainIcon = new FXGIFIcon(&application, resico_mainicon, 0, IMAGE_OPAQUE);
	userIcon = new FXGIFIcon(&application, resico_user);

	authBannerImg = new FXPNGImage(&application, resico_authbanner, IMAGE_OPAQUE);

	application.init(argc, argv);

	new SudoWindow(&application);

	application.create();
	return application.run();
}
