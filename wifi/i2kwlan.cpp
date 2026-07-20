#include <fx.h>
#include <ice2k/comctl32.h>


#include "res/foxres.h"
#include <ice2k/wizard/I2KWizard.h>
#include <ice2k/wizard/I2KWizHeader.h>
#include <unistd.h>
#include <sys/wait.h>

#define ANIM_CONNECTING_WIDTH 104
#define ANIM_CONNECTING_HEIGHT 34

#define ANIM_CONNECTING_HORFRAMES 5
#define ANIM_CONNECTING_VERFRAMES 9
#define ANIM_CONNECTING_DELAY 67



#define ANIM_SCANNING_WIDTH 57
#define ANIM_SCANNING_HEIGHT 32

#define ANIM_SCANNING_HORFRAMES 5
#define ANIM_SCANNING_VERFRAMES 3
#define ANIM_SCANNING_DELAY 167


int connecting_verframe = 0;
int connecting_horframe = 0;
int scanning_verframe = 0;
int scanning_horframe = 0;

I2KWizHeader* wizhead = NULL;

FXIcon* mainIcon;
FXImage* wizIcon;

FXIcon* ico_wifi0;
FXIcon* ico_wifi1;
FXIcon* ico_wifi2;
FXIcon* ico_wifi3;
FXIcon* ico_wifi4;

FXIcon* ico_info;

FXImage* img_connectanim;
FXImage* img_scananim;


// window class
class WLANWizard : public FXMainWindow {
	FXDECLARE(WLANWizard);

protected:
	// required in every fox window class you define
	WLANWizard() {}

private:
	// put all the controls you will use for the class here
	FXHorizontalFrame *contents;
	I2KWizard* wiz;
	FXWizard* tabbook;
	FXCanvas* animcanvas;
	FXCanvas* scananimcanvas;
	FXText* scantext;
	FXText* nettext;
	FXIconList* iconlist;
	FXSwitcher* switcher;
	FXButton* scanbtn;

	FXLabel* waitlbl;


public:
	// class functions
	long onCmdHello(FXObject*, FXSelector, void*);
	long onCmdRefresh(FXObject*, FXSelector, void*);

	long onPageChange(FXObject*, FXSelector, void*);
	long onTimeoutAnimateConnecting(FXObject*, FXSelector, void*);
	long onPaintAnimateConnecting(FXObject*, FXSelector, void*);
	long onTimeoutAnimateScanning(FXObject*, FXSelector, void*);
	long onPaintAnimateScanning(FXObject*, FXSelector, void*);
	long onTimeoutScanning(FXObject*, FXSelector, void*);
	long onTimeoutConnecting(FXObject*, FXSelector, void*);

	long onListSelect(FXObject*, FXSelector, void*);
	long onListDeselect(FXObject*, FXSelector, void*);

	char* getSelectedNetwork();



	void wifiList();

public:
	// message ids
	enum {
		ID_MAINWIN = FXMainWindow::ID_LAST,
		ID_HELLO,
		ID_WIZARD,
		ID_REFRESH,
		ID_ANIMATE_CONNECTING,
		ID_ANIMATE_SCANNING,
		ID_SCAN,
		ID_LISTVIEW,
		ID_CONNECTING,
		ID_LAST
	};

public:

	// constructor
	WLANWizard(FXApp* a);
	
	void setFocus() {};

	virtual void create();
	virtual ~WLANWizard();
};

// message map
FXDEFMAP(WLANWizard) WLANWizardMap[] = {
	FXMAPFUNC(SEL_COMMAND,           WLANWizard::ID_HELLO,  WLANWizard::onCmdHello),
	FXMAPFUNC(SEL_COMMAND,           WLANWizard::ID_REFRESH,  WLANWizard::onCmdRefresh),
	FXMAPFUNC(SEL_SELECTED,          WLANWizard::ID_LISTVIEW,  WLANWizard::onListSelect),
	FXMAPFUNC(SEL_DESELECTED,          WLANWizard::ID_LISTVIEW,  WLANWizard::onListDeselect),


	FXMAPFUNC(SEL_COMMAND,           WLANWizard::ID_WIZARD,  WLANWizard::onPageChange),

	FXMAPFUNC(SEL_TIMEOUT,           WLANWizard::ID_ANIMATE_CONNECTING,  WLANWizard::onTimeoutAnimateConnecting),
	FXMAPFUNC(SEL_TIMEOUT,           WLANWizard::ID_SCAN,  WLANWizard::onTimeoutScanning),
	FXMAPFUNC(SEL_TIMEOUT,           WLANWizard::ID_CONNECTING,  WLANWizard::onTimeoutConnecting),


	FXMAPFUNC(SEL_PAINT,             WLANWizard::ID_ANIMATE_CONNECTING,  WLANWizard::onPaintAnimateConnecting),

	FXMAPFUNC(SEL_TIMEOUT,           WLANWizard::ID_ANIMATE_SCANNING,  WLANWizard::onTimeoutAnimateScanning),
	FXMAPFUNC(SEL_PAINT,             WLANWizard::ID_ANIMATE_SCANNING,  WLANWizard::onPaintAnimateScanning),
};

FXIMPLEMENT(WLANWizard, FXMainWindow, WLANWizardMap, ARRAYNUMBER(WLANWizardMap));

pid_t connect_pid = 0;
int connecting = 0;
int connect_err = -1;

void connectNetwork(const char* ssid, const char* key) {
	if (key != NULL && key[0] != '\0') setenv("WLAN_PSK", key, 1);
	connecting = 1;
	connect_pid = vfork();
	
	if (connect_pid < 0) {
		perror("vfork");
		exit(1);
	} else if (connect_pid == 0) {
		execlp("./wifiiwd", "./wifiiwd",
				"wlan0", "connect",
				ssid,
				(char*)NULL);

		perror("execlp");
		exit(1);
	} else {
		unsetenv("WLAN_PSK");
	}
}

char selectssid[64];

char* WLANWizard::getSelectedNetwork() {
	strncpy(selectssid,
			iconlist->getItemText(iconlist->getCurrentItem()).text()+2,
			sizeof(selectssid)-1);

	selectssid[strcspn(selectssid, "\t")] = '\0';

	return selectssid;
}


long WLANWizard::onPageChange(FXObject* obj,FXSelector sel, void* ptr) {
	FXuval action = (int)(FXuval)ptr;
	//printf("%lu	\n", action);
	int current = wiz->getCurrent();

	if (action == IWIZARD_ABACK) {
		//puts("Go back!!!");
		wiz->setCurrent(--current);
		wiz->setFinish(FALSE);

		if (current == 0) {
			wiz->getBackButton()->disable();
		}
	} else if (action == IWIZARD_ANEXT) {
		wiz->setCurrent(++current);
		wiz->getBackButton()->enable();

		if (current == wiz->getSwitcher()->numChildren()-1) {
			//puts("Go forward!!!");
			wiz->setFinish(TRUE);
		} else if (current == wiz->getSwitcher()->numChildren()) {
			getApp()->exit(0);
			//puts("Finished");
		} else {
			//puts("Go forward!!!");
		}

	} else {
		//puts("Cancel");
		getApp()->exit(1);
	}

	if (current == 1) {
		char networklbl[256];
		snprintf(networklbl, sizeof(networklbl),
				"Please wait while Windows connects to the '%s' network.", getSelectedNetwork());
		waitlbl->setText(networklbl);
		connecting_horframe = 0;
		connecting_verframe = 0;
		getApp()->addTimeout(this, ID_ANIMATE_CONNECTING, ANIM_CONNECTING_DELAY);
		connectNetwork(getSelectedNetwork(), NULL);
		getApp()->addTimeout(this, ID_CONNECTING, 100);
	} else {
		getApp()->removeTimeout(this, ID_ANIMATE_CONNECTING);
	}


	if (current == 0) {
		scanning_horframe = 0;
		scanning_verframe = 0;
	} else {
		getApp()->removeTimeout(this, ID_ANIMATE_SCANNING);
	}


	return 1;
}

void WLANWizard::wifiList() {
	FILE* fp = popen("./wifiiwd wlan0 list", "r");

	char item[48];
	char line[32+2];
	char ssid[32+2];
	
	int i = 0;
	int signal;
	char indic;

	FXIcon* wifiicon;

	while (fgets(line, sizeof(line), fp)) {
		//printf("%d: %s", i, line);
		switch (i) {
			case 0:
				signal = atoi(line);
				if (signal < 20) wifiicon = ico_wifi0;
				else if (signal < 40) wifiicon = ico_wifi1;
				else if (signal < 60) wifiicon = ico_wifi2;
				else if (signal < 80) wifiicon = ico_wifi3;
				else wifiicon = ico_wifi4;
				++i;
				break;
			case 1:
				indic = line[0];
				++i;
				break;
			case 2:
				sprintf(ssid, "%.*s", (int)sizeof(ssid), line);
				//sprintf(ssid, "CENSORED");
				ssid[strcspn(ssid, "\n")] = '\0';
				++i;
				break;
			case 3:
				switch(line[0]) {
					case 'p':
						sprintf(item, "%c\t%s\tPSK", indic, ssid);
						break;
					case '8':
						sprintf(item, "%c\t%s\t802.1.x", indic, ssid);
						break;
					default:
						sprintf(item, "%c\t%s\tOpen", indic, ssid);
				}
				iconlist->appendItem(item, wifiicon, wifiicon);
				i = 0;
				break;
			default:
				i = 0;
				break;
		}
	}
}

long WLANWizard::onTimeoutScanning(FXObject* obj,FXSelector sel, void* ptr) {
	//puts("Hello!");
	if (system("./wifiiwd wlan0 scanning")) {
		switcher->setCurrent(1);
		scanbtn->enable();
		//wiz->getNextButton()->enable();
		wifiList();
		getApp()->removeTimeout(this, ID_ANIMATE_SCANNING);
		scanning_horframe = 0;
		scanning_verframe = 0;
	} else {
		getApp()->addTimeout(this, ID_SCAN, 1000);
	}


	return 1;
}

long WLANWizard::onTimeoutConnecting(FXObject* obj,FXSelector sel, void* ptr) {
	//puts("Hello!");
	if (!connecting) {
		if (connect_err == 0) {
			getApp()->exit();
		} else {
			getApp()->removeTimeout(this, ID_ANIMATE_CONNECTING);
			wiz->setCurrent(2);
		}
	} else {
		getApp()->addTimeout(this, ID_CONNECTING, 1000);
	}


	return 1;
}


long WLANWizard::onCmdRefresh(FXObject* obj,FXSelector sel, void* ptr) {
	switcher->setCurrent(0);
	iconlist->clearItems();
	scanbtn->disable();
	wiz->getNextButton()->disable();
	getApp()->addTimeout(this, ID_ANIMATE_SCANNING, ANIM_SCANNING_DELAY);
	system("./wifiiwd wlan0 scan");
	getApp()->addTimeout(this, ID_SCAN, 1000);
	return 1;
}


WLANWizard::WLANWizard(FXApp *a) : FXMainWindow(a, "Connect to Wireless Network", NULL, NULL, DECOR_TITLE|DECOR_BORDER|DECOR_CLOSE|DECOR_MENU, 0,0,497,360) {
	wiz = new I2KWizard(this, this, ID_WIZARD);
	wiz->getNextButton()->disable();

	FXVerticalFrame* cont;
	FXPacker* scananimcont;

	/* FXLabel* lbl = new FXLabel(wiz->getSwitcher(),"This is some text");
	new FXLabel(wiz->getSwitcher(),"This is some other text"); */

	cont = new FXVerticalFrame(wiz->getSwitcher(),FRAME_NONE, 0,0,0,0, 0,0,0,0, 0,0);

	new I2KWizHeader(cont, wizIcon, "Set up a wireless connection", "Choose a wireless network to connect to.");
	cont = new FXVerticalFrame(cont, LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 22,20,23,12, 10,10);

	switcher = new FXSwitcher(cont, LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_NORMAL, 0,0,0,0, 0,0,0,0);
	scananimcont = new FXPacker(switcher, LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0, 0,0);
	
	scananimcont->setBackColor(getApp()->getBackColor());
	scananimcanvas = new FXCanvas(scananimcont, this, ID_ANIMATE_SCANNING,
			LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_CENTER_X|LAYOUT_CENTER_Y,
			0,0, ANIM_SCANNING_WIDTH, ANIM_SCANNING_HEIGHT);

	iconlist = new FXIconList(switcher, this, ID_LISTVIEW,
			LAYOUT_FILL_X|LAYOUT_FILL_Y|ICONLIST_DETAILED|ICONLIST_SINGLESELECT|SCROLLERS_DONT_TRACK);

	switcher->setCurrent(1);

	FXHorizontalFrame* scanbtm = new FXHorizontalFrame(cont, LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0, 4,4);
	new FXLabel(scanbtm, "", ico_info, FRAME_NONE, 0,0,0,0, 0,4,0,0);
	scantext = new FXText(scanbtm, NULL, 0, LAYOUT_FILL_X|LAYOUT_FIX_HEIGHT|TEXT_WORDWRAP);
	scantext->setText("If the desired network does not appear, "
			"verify that your wireless router/access point is turned on and click Search Again. "
			"If you do not own this network, please contact your adminstrator.");
	scantext->disable();
	scantext->setMarginTop(0);
	scantext->setMarginLeft(0);
	scantext->setMarginBottom(0);
	scantext->setMarginRight(0);
	scantext->setBackColor(getApp()->getBaseColor());
	scantext->setDefaultCursor(getApp()->getDefaultCursor(DEF_ARROW_CURSOR));


	scanbtn = new FXButton(scanbtm, "&Search Again", NULL, this, ID_REFRESH,
			BUTTON_DEFAULT|LAYOUT_BOTTOM|BUTTON_NORMAL,
			0,0,0,0, 14,14,2,2);
	//scanbtn->disable();

	iconlist->appendHeader("Present",NULL,16+18);
	iconlist->appendHeader("Network",NULL,335);
	iconlist->appendHeader("Security",NULL,65);

	iconlist->getHeader()->setPadTop(0);
	iconlist->getHeader()->setPadBottom(-1000);

	/*iconlist->appendItem("Really BIG and wide item to test\tPSK", ico_wifi0, ico_wifi4);
	iconlist->appendItem("Really BIG and wide item to test\tPSK", ico_wifi0, ico_wifi3);
	iconlist->appendItem("Really BIG and wide item to test\tPSK", ico_wifi0, ico_wifi3);
	iconlist->appendItem("Really BIG and wide item to test\t802.1.x", ico_wifi0, ico_wifi2);
	iconlist->appendItem("Really BIG and wide item to test\tPSK", ico_wifi0, ico_wifi2);
	iconlist->appendItem("Really BIG and wide item to test\tOpen", ico_wifi0, ico_wifi1);
	iconlist->appendItem("Really BIG and wide item to test\tOpen", ico_wifi0, ico_wifi1);
	iconlist->appendItem("Really BIG and wide item to test\tOpen", ico_wifi0, ico_wifi1);
	iconlist->appendItem("Really BIG and wide item to test\tOpen", ico_wifi0, ico_wifi1);
	iconlist->appendItem("Really BIG and wide item to test\tPSK", ico_wifi0, ico_wifi0);
	iconlist->appendItem("Really BIG and wide item to test\tPSK", ico_wifi0, ico_wifi0);
	iconlist->appendItem("Really BIG and wide item to test\tPSK", ico_wifi0, ico_wifi0);
	iconlist->appendItem("Really BIG and wide item to test\tPSK", ico_wifi0, ico_wifi0);
	iconlist->appendItem("Really BIG and wide item to test\tPSK", ico_wifi0, ico_wifi0);
	iconlist->appendItem("Really BIG and wide item to test\tPSK", ico_wifi0, ico_wifi0);
	iconlist->appendItem("Really BIG and wide item to test\tPSK", ico_wifi0, ico_wifi0);
	iconlist->appendItem("Really BIG and wide item to test\tPSK", ico_wifi0, ico_wifi0);*/

	cont = new FXVerticalFrame(wiz->getSwitcher(),FRAME_NONE, 0,0,0,0, 0,0,0,0, 0,0);

	new I2KWizHeader(cont, wizIcon, "Set up a wireless connection", "Please wait...");

	cont = new FXVerticalFrame(cont, LAYOUT_FILL_X, 0,0,0,0, 32,32,12,12, 6,6);

	waitlbl = new FXLabel(cont, "PLACEHOLDER");
	new FXLabel(cont, "Waiting for network to be ready...");
	animcanvas = new FXCanvas(cont, this, ID_ANIMATE_CONNECTING, LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_CENTER_X,
			0,0, ANIM_CONNECTING_WIDTH, ANIM_CONNECTING_HEIGHT);

	cont = new FXVerticalFrame(wiz->getSwitcher(),FRAME_NONE, 0,0,0,0, 0,0,0,0, 0,0);

	new I2KWizHeader(cont, wizIcon, "Set up a wireless connection", "Enter the key required for the network");

	cont = new FXVerticalFrame(cont, LAYOUT_FILL_X, 0,0,0,0, 32,10,10,10, 3,3);

	getApp()->addTimeout(this, ID_SCAN, 0);

	nettext = new FXText(cont, NULL, 0, LAYOUT_FILL_X|LAYOUT_FIX_HEIGHT|TEXT_WORDWRAP);
	nettext->setText("The network '...' requires a network key (also called a WEP key or WPA key). "
			"A network key helps prevent unknown intruders from connecting to this network.\n"
			"\n"
			"Type the key, and then click Next.");
	nettext->disable();
	nettext->setMarginTop(0);
	nettext->setMarginLeft(0);
	nettext->setMarginBottom(0);
	nettext->setMarginRight(0);
	nettext->setBackColor(getApp()->getBaseColor());
	nettext->setDefaultCursor(getApp()->getDefaultCursor(DEF_ARROW_CURSOR));
	FXMatrix* keymtx = new FXMatrix(cont, 2, MATRIX_BY_COLUMNS|LAYOUT_FILL_X, 0,0,0,0, 0,24,10,0, 20, 4);

	new FXLabel(keymtx,"Network key: ",	NULL, LAYOUT_CENTER_Y, 0,0,0,0, 0,0,0,0);
	new FXTextField(keymtx, 10, NULL, 0, LAYOUT_FILL_X|LAYOUT_FILL_COLUMN|TEXTFIELD_PASSWD|FRAME_NORMAL|LAYOUT_CENTER_Y, 0,0,0,0, 3,2,1,1);
	new FXLabel(keymtx,"Confirm network key: ",	NULL, LAYOUT_CENTER_Y, 0,0,0,0, 0,0,0,0);
	new FXTextField(keymtx, 10, NULL, 0, LAYOUT_FILL_X|LAYOUT_FILL_COLUMN|TEXTFIELD_PASSWD|FRAME_NORMAL|LAYOUT_CENTER_Y, 0,0,0,0, 3,2,1,1);
}

// deconstructs window
// 
// you will need to put icons and menus here, if you loaded them in
// the constructor
//
// ref. http://www.fox-toolkit.org/faq.html#MENUPANES
//
// "As a general rule, shared resources such as Icons, Fonts, Cursors, Bitmaps,
//  and also MenuPanes, must be explicitly deleted by the widgets that owns them."

WLANWizard::~WLANWizard() {
}


// i feel like the rest is self explanitory
void WLANWizard::create() {
	FXMainWindow::create();

	show(PLACEMENT_SCREEN);
	scantext->setHeight(scantext->getContentHeight());
	nettext->setHeight(nettext->getContentHeight());

	//getApp()->addTimeout(this, ID_ANIMATE_SCANNING, ANIM_SCANNING_DELAY);
	//wiz->execute();
}


long WLANWizard::onCmdHello(FXObject*, FXSelector, void*) {
	puts("Hello world!");
	return 1;
}

long WLANWizard::onListDeselect(FXObject*, FXSelector, void*) {
	wiz->getNextButton()->disable();
	return 1;
}

long WLANWizard::onListSelect(FXObject*, FXSelector, void*) {
	wiz->getNextButton()->enable();
	return 1;
}



long WLANWizard::onTimeoutAnimateConnecting(FXObject*, FXSelector, void*) {
	if (++connecting_horframe == ANIM_CONNECTING_HORFRAMES) {
		connecting_horframe = 0;

		if (++connecting_verframe == ANIM_CONNECTING_VERFRAMES) {
			connecting_verframe = 0;
		}
	}

	animcanvas->update();
	getApp()->addTimeout(this, ID_ANIMATE_CONNECTING, ANIM_CONNECTING_DELAY);

	return 1;
}
long WLANWizard::onPaintAnimateConnecting(FXObject* obj, FXSelector, void* ptr) {
	FXEvent* ev = (FXEvent*)ptr;
	FXWindow* win = (FXWindow*)obj;

	FXDCWindow dc(win, ev);

	dc.drawArea(img_connectanim,
			connecting_horframe*ANIM_CONNECTING_WIDTH,
			connecting_verframe*ANIM_CONNECTING_HEIGHT,
			ANIM_CONNECTING_WIDTH,
			ANIM_CONNECTING_HEIGHT, 0,0);

	dc.end();

	return 1;
}

long WLANWizard::onTimeoutAnimateScanning(FXObject*, FXSelector, void*) {
	if (++scanning_horframe == ANIM_SCANNING_HORFRAMES) {
		scanning_horframe = 0;

		if (++scanning_verframe == ANIM_SCANNING_VERFRAMES) {
			scanning_verframe = 0;
		}
	}

	scananimcanvas->update();
	getApp()->addTimeout(this, ID_ANIMATE_SCANNING, ANIM_SCANNING_DELAY);
	//printf("%d\n", scantext->getBottomLine());

	return 1;
}
long WLANWizard::onPaintAnimateScanning(FXObject* obj, FXSelector, void* ptr) {
	FXEvent* ev = (FXEvent*)ptr;
	FXWindow* win = (FXWindow*)obj;

	FXDCWindow dc(win, ev);

	dc.drawArea(img_scananim,
			scanning_horframe*ANIM_SCANNING_WIDTH,
			scanning_verframe*ANIM_SCANNING_HEIGHT,
			ANIM_SCANNING_WIDTH,
			ANIM_SCANNING_HEIGHT, 0,0);

	dc.end();

	return 1;
}

void sigchld_handler(int sig) {
	int status;
	pid_t child;

	while ((child = waitpid(-1, &status, WNOHANG)) > 0) {
		if (child == connect_pid) {
			connecting = 0;

			if (WIFEXITED(status)) {
				connect_err = WEXITSTATUS(status);
			} else {
				connect_err = -1;
			}
		}
	}
}


int main(int argc, char *argv[]) {
	signal(SIGCHLD, sigchld_handler);

	//printf("%d\n", !system("./wifiiwd wlan0 netexists Oliebol"));

	FXApp application("WLAN", "I2KProject");
	application.init(argc, argv);

	mainIcon = new FXGIFIcon(&application, resico_mainicon, 0, IMAGE_OPAQUE);
	wizIcon = new FXGIFImage(&application, resico_wizicon, 0, IMAGE_OPAQUE);

	ico_wifi0 = new FXGIFIcon(&application, resico_wifi0, 0, IMAGE_NEAREST); ico_wifi0->create();
	ico_wifi1 = new FXGIFIcon(&application, resico_wifi1, 0, IMAGE_NEAREST); ico_wifi1->create();
	ico_wifi2 = new FXGIFIcon(&application, resico_wifi2, 0, IMAGE_NEAREST); ico_wifi2->create();
	ico_wifi3 = new FXGIFIcon(&application, resico_wifi3, 0, IMAGE_NEAREST); ico_wifi3->create();
	ico_wifi4 = new FXGIFIcon(&application, resico_wifi4, 0, IMAGE_NEAREST); ico_wifi4->create();

	ico_info = new FXGIFIcon(&application, resico_info, 0, IMAGE_NEAREST);


	img_connectanim = new FXGIFImage(&application, resico_connect, IMAGE_NEAREST);
	img_connectanim->blend(application.getBaseColor());
	img_connectanim->create();

	img_scananim = new FXGIFImage(&application, resico_scanning, IMAGE_NEAREST);
	img_scananim->blend(application.getBackColor());
	img_scananim->create();

	new WLANWizard(&application);

	application.create();
	//win->close();
	application.run();

}




