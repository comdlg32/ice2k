#include <fx.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <ice2k/comctl32.h>
#include <ice2k/branding.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

#include "res/foxres.h"

FXMainWindow* mainwin;

char iface[16];


int ifexist(char* name) {
	struct ifaddrs *addrs, *tmp;

	getifaddrs(&addrs);
	tmp = addrs;

	while (tmp) {
	    if (tmp->ifa_addr == NULL) goto next;
		if (tmp->ifa_addr->sa_family != AF_PACKET && tmp->ifa_addr->sa_family != AF_INET) goto next;
		
		if (strcmp(tmp->ifa_name, name)) goto next;

		return 1;

next:
	    tmp = tmp->ifa_next;
	}

	freeifaddrs(addrs);

	return 0;
}


FXIcon* ico_main16;

FXIcon* ico_infra;
FXIcon* ico_infra_c;
FXIcon* ico_infra_h;

FXIcon* ico_adhoc;
FXIcon* ico_adhoc_c;
FXIcon* ico_adhoc_h;


class WLANProfiles : public FXMainWindow {
	FXDECLARE(WLANProfiles);

protected:
	WLANProfiles() {}

private:
	FXButton* okbtn;
	FXButton* cancelbtn;

	FXButton* removebtn;

	FXTabBook* tabbook;
	FXVerticalFrame* contents;
	FXVerticalFrame* grpcont;
	FXGroupBox* availgrp;
	FXGroupBox* prefgrp;

	FXPacker* iconlistbox;
	FXTreeList* iconlist;


public:
	void knownList();
	long onCmdWLAN(FXObject*, FXSelector, void*);
	long onCmdRemove(FXObject*, FXSelector, void*);


public:
	enum {
		ID_WLAN = FXMainWindow::ID_LAST,
		ID_REMOVE,
		ID_LIST,
		ID_LAST
	};

public:
	WLANProfiles(FXApp* a);

	virtual void create();
	void setFocus() {};
	virtual ~WLANProfiles();
};

FXDEFMAP(WLANProfiles) WLANProfilesMap[] = {
	FXMAPFUNC(SEL_COMMAND,           WLANProfiles::ID_WLAN,   WLANProfiles::onCmdWLAN),
	FXMAPFUNC(SEL_COMMAND,           WLANProfiles::ID_REMOVE, WLANProfiles::onCmdRemove),
	FXMAPFUNC(SEL_DOUBLECLICKED,     WLANProfiles::ID_LIST,   WLANProfiles::onCmdRemove),


};

FXIMPLEMENT(WLANProfiles, FXMainWindow, WLANProfilesMap, ARRAYNUMBER(WLANProfilesMap));


void WLANProfiles::knownList() {
	FILE* fp = popen(PREFIX"/libexec/ice2k/profiwd known", "r");
	
	int s = 0;
	int i = 0;
	int hidden;
	char line[32+2];

	if (fp == NULL) {
		removebtn->disable();
		return;
	}

	while (fgets(line, sizeof(line), fp)) {
		switch (i) {
			case 0:
				if (line[0] == '1') hidden = 1;
				else hidden = 0;
				++i;
				break;
			case 1:
				line[strcspn(line, "\n")] = '\0';
				if (hidden) iconlist->appendItem(NULL, line, ico_infra_h, ico_infra_h);
				else iconlist->appendItem(NULL, line, ico_infra, ico_infra);
				s = 1;
				i = 0;
				break;
			default:
				i = 0;
				break;
		}
	}

	pclose(fp);

	if (!s) {
		removebtn->disable();
		return;
	}

}

WLANProfiles::WLANProfiles(FXApp *a) : FXMainWindow(a, "Network Properties", ico_main16, NULL, DECOR_CLOSE|DECOR_MENU|DECOR_BORDER|DECOR_TITLE, 0,0,0,0, 0,0,0,0) {
	FXHorizontalFrame* btncont = new FXHorizontalFrame(this, LAYOUT_SIDE_BOTTOM|LAYOUT_RIGHT, 0,0,0,0, 0,6,1,7, 6,0);
	
	okbtn = new FXButton(btncont, "OK", NULL, getApp(), FXApp::ID_QUIT, BUTTON_DEFAULT|BUTTON_NORMAL|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0, 0, 75, 23, 0, 0, 0, 0);
	cancelbtn = new FXButton(btncont, "Cancel", NULL, getApp(), FXApp::ID_QUIT, BUTTON_NORMAL|BUTTON_DEFAULT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0, 0, 75, 23, 0, 0, 0, 0);
	//applybtn = new FXButton(btncont, "&Apply", NULL, this, ID_DLG_APPLY, BUTTON_NORMAL|BUTTON_DEFAULT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0, 0, 75, 23, 0, 0, 0, 0);

	//applybtn->disable();


	//new FXButton(this, "Hello World!", NULL, this, ID_HELLO, BUTTON_NORMAL);
	tabbook = new FXTabBook(this, NULL, 0, LAYOUT_FILL, 0,0,0,0, 6,6,5,5);

	new FXTabItem(tabbook, "Wireless Networks", NULL, TAB_TOP_NORMAL, 0,0,0,0, 5,5,1,2);
	contents = new FXVerticalFrame(tabbook, FRAME_THICK|FRAME_RAISED, 0,0,0,0, 11,10,11,11);
	FXCheckButton* chk = new FXCheckButton(contents, "Use &Windows to configure my wireless network settings");
	chk->setCheck(TRUE);
	chk->disable();

	grpcont = new FXVerticalFrame(contents, LAYOUT_FILL, 0,0,0,0, 2,2,8,0, 7,7);
	availgrp = new FXGroupBox(grpcont, "Available networks: ", FRAME_GROOVE|LAYOUT_FILL_X, 0,0,0,0, 7,9,0,7);
	new FXLabel(availgrp,
			"To connect to, disconnect from, or find out more information\n"
			"about wireless networks in range, click the button below.",
			NULL, JUSTIFY_LEFT, 0,0,0,0, 2,1,1,2);
	new FXButton(availgrp, "View Wireless &Networks", NULL, this, ID_WLAN, BUTTON_DEFAULT|BUTTON_NORMAL|LAYOUT_RIGHT, 0,0,0,0, 9,9,2,2);
	prefgrp = new FXGroupBox(grpcont, "Preferred networks: ", FRAME_GROOVE|LAYOUT_FILL_X, 0,0,0,0, 8,9,0,7, 7,7);

	new FXLabel(prefgrp,
			"Automatically connect to available networks in the list below:",
			NULL, JUSTIFY_LEFT, 0,0,0,0, 2,1,0,0);

	iconlistbox = new FXPacker(prefgrp, LAYOUT_FILL_X|FRAME_NORMAL, 0,0,0,0, 0,0,0,0);
	iconlist = new FXTreeList(iconlistbox, this, ID_LIST, LAYOUT_FILL|TREELIST_BROWSESELECT|SCROLLERS_DONT_TRACK);
	iconlist->setNumVisible(5);

	knownList();

	/*treelist->appendItem(NULL, "test network (PSK)", ico_infra, ico_infra);
	treelist->appendItem(NULL, "test network", ico_infra, ico_infra);
	treelist->appendItem(NULL, "test network", ico_infra, ico_infra);*/

	
	removebtn = new FXButton(prefgrp, "&Remove", NULL, this, ID_REMOVE, BUTTON_DEFAULT|BUTTON_NORMAL, 0,0,0,0, 16,16,2,2);


}

WLANProfiles::~WLANProfiles() {
}

void WLANProfiles::create() {
	FXMainWindow::create();

	show(PLACEMENT_SCREEN);
}

long WLANProfiles::onCmdWLAN(FXObject*, FXSelector, void*) {
	pid_t pid = vfork();

	if (pid < 0) {
		perror("vfork");
		_exit(1);
	} else if (pid == 0) {
		execlp("i2kwlan", "i2kwlan", iface, (char*)NULL);

		perror("execlp");
		_exit(1);
	}

	return 1;
}
long WLANProfiles::onCmdRemove(FXObject*, FXSelector, void*) {
	char ssid[32+1];
	char message[256];
	pid_t pid;

	strcpy(ssid, iconlist->getCurrentItem()->getText().text());

	sprintf(message, "Are you sure you want to forget the network %s?", ssid);

	if (FXMessageBox::question(mainwin, MBOX_YES_NO, "Forget network", message) == MBOX_CLICKED_YES) {
		pid = vfork();

		if (pid < 0) {
			perror("vfork");
			_exit(1);
		} else if (pid == 0) {
			execlp(PREFIX"/libexec/ice2k/profiwd",
			       PREFIX"/libexec/ice2k/profiwd",
			       "forget",
				   ssid,
			       (char*)NULL);
			perror("execlp");
			_exit(1);
		} else {
			wait(NULL);

			iconlist->clearItems();
			knownList();
		}
	}
			

	return 1;
}

void sigchld(int) {
	int olderrno = errno;

	while (waitpid(-1, NULL, WNOHANG) > 0) {};

	errno = olderrno;
}

int main(int argc, char *argv[]) {
	struct sigaction sa;
	sa.sa_handler = &sigchld;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	sigaction(SIGCHLD, &sa, NULL);

	FXApp application("WLANProfiles", "I2KProject");
	if (i2kBGetWinVersionInt() >= ICE2K_BRAND_WINXP) {
		ico_main16 = new FXGIFIcon(&application, resico_xplan);
	} else {
		ico_main16 = new FXGIFIcon(&application, resico_2klan);
	}

	ico_infra   = new FXGIFIcon(&application, resico_infra);
	ico_infra  -> create();
	ico_infra_c = new FXGIFIcon(&application, resico_infra_c);
	ico_infra_c-> create();
	ico_infra_h = new FXGIFIcon(&application, resico_infra_h);
	ico_infra_h-> create();

	ico_adhoc   = new FXGIFIcon(&application, resico_adhoc);
	ico_adhoc  -> create();
	ico_adhoc_c = new FXGIFIcon(&application, resico_adhoc_c);
	ico_adhoc_c-> create();
	ico_adhoc_h = new FXGIFIcon(&application, resico_adhoc_h);
	ico_adhoc_h-> create();


	application.init(argc, argv);
	if (argv[1] == NULL || argv[1][0] == '\0') {
		application.create();
		FXMessageBox::error(&application, MBOX_OK, "Error", "No interface is specified!");
		application.exit(1);
		return 1;
	}

	if (!ifexist(argv[1])) {
		application.create();
		FXMessageBox::error(&application, MBOX_OK, "Error", "Interface is non-existant!");
		application.exit(1);
		return 1;
	}

	strncpy(iface, argv[1], sizeof(iface)-1);
	iface[sizeof(iface)-1] = '\0';
		

	mainwin = new WLANProfiles(&application);
	char name[32];
	sprintf(name, "%s Properties", iface);
	mainwin->setTitle(name);

	application.create();
	return application.run();
}
