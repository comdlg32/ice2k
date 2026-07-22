#include <fx.h>
#include <FXPNGIcon.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/if_link.h>
//#define _DO_NOT_OVERRIDE_FOX_CONTROLS
#include <ice2k/comctl32.h>
#include <ice2k/branding.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <pwd.h>

#include "res/foxres.h"
char iftraypath[PATH_MAX] = {0};

struct interfaceValues {
	char ifname[16];

	unsigned long long sent;
	unsigned long long received;

	union {
		uint32_t ip;

#if __BYTE_ORDER == __LITTLE_ENDIAN
		struct {
			uint8_t byte4;
			uint8_t byte3;
			uint8_t byte2;
			uint8_t byte1;
		} octets;
#else
		struct {
			uint8_t byte1;
			uint8_t byte2;
			uint8_t byte3;
			uint8_t byte4;
		} octets;
#endif
	} ip;

	union {
		uint32_t ip;

#if __BYTE_ORDER == __LITTLE_ENDIAN
		struct {
			uint8_t byte4;
			uint8_t byte3;
			uint8_t byte2;
			uint8_t byte1;
		} octets;
#else
		struct {
			uint8_t byte1;
			uint8_t byte2;
			uint8_t byte3;
			uint8_t byte4;
		} octets;
#endif
	} mask;

	int speed;

	int up;
	int exists;
	int trayshown;
};


struct interfaceValues ifvalues = {0};
struct interfaceValues ifvalues_old = {0};


FXIcon* ico_ncpa_dial;
FXIcon* ico_ncpa_dial_down;
FXIcon* ico_ncpa_dial_idle;
FXIcon* ico_ncpa_dial_nohw;
FXIcon* ico_ncpa_dial_rx;
FXIcon* ico_ncpa_dial_tx;
FXIcon* ico_ncpa_dial_txrx;
FXIcon* ico_ncpa_direct;
FXIcon* ico_ncpa_direct_down;
FXIcon* ico_ncpa_direct_idle;
FXIcon* ico_ncpa_direct_rx;
FXIcon* ico_ncpa_direct_tx;
FXIcon* ico_ncpa_direct_txrx;
FXIcon* ico_ncpa_lan;
FXIcon* ico_ncpa_lan_down;
FXIcon* ico_ncpa_lan_idle;
FXIcon* ico_ncpa_lan_nohw;
FXIcon* ico_ncpa_lan_rx;
FXIcon* ico_ncpa_lan_tx;
FXIcon* ico_ncpa_lan_txrx;
FXIcon* ico_ncpa_vpn;
FXIcon* ico_ncpa_vpn_idle;
FXIcon* ico_ncpa_vpn_rx;
FXIcon* ico_ncpa_vpn_tx;
FXIcon* ico_ncpa_vpn_txrx;

FXIcon* mainIcon;

FXIcon* loadPNGIcon(FXApp* app, const void* pix=NULL,
                    FXColor clr=FXRGB(192,192,192), FXColor blendclr=FXRGB(192,192,192),
                    FXuint opts=0, FXint w=1, FXint h=1) {
	FXIcon* icon = new FXPNGIcon(app, pix, clr, opts, w, h);

	if (blendclr == FXRGB(192,192,192)) {
		blendclr = app->getBaseColor();
	} else if (blendclr == FXRGB(255,255,255)) {
		blendclr = app->getBackColor();
	}

	icon->blend(blendclr);

	return(icon);
}


class NCPAPropWindow : public FXMainWindow {
	FXDECLARE(NCPAPropWindow);

protected:
	NCPAPropWindow() {}

private:
	FXVerticalFrame*   cont;

	FXHorizontalFrame* contents;
	FXTabBook*         tabbook;
	
	FXVerticalFrame*   generalcont;
	FXVerticalFrame*   supportcont;

	FXGroupBox*        connectiongrp;
	FXMatrix*          connectionmatrix;

	FXGroupBox*        activitygrp;
	FXGroupBox*        statusgrp;

	FXMatrix*          activitymtx;
	FXMatrix*          statusmtx;

	FXLabel*           rxlbl;
	FXLabel*           txlbl;

	FXLabel*           iconlbl;

	FXButton*          disablebtn;
	FXButton*          wlanbtn;

	FXLabel*           conlbl;

	FXCheckButton*     showicon;

public:
	long onCmdAccept(FXObject*, FXSelector, void*);
	long onCmdDisable(FXObject*, FXSelector, void*);
	long onCmdWLAN(FXObject*, FXSelector, void*);


	long onTimeout(FXObject*, FXSelector, void*);

	void setFocus() { return; }

public:
	enum {
		ID_ACCEPT = FXMainWindow::ID_LAST,
		ID_DISABLE,
		ID_WLAN,
		ID_TIMEOUT,
		ID_LAST
	};

public:
	NCPAPropWindow(FXApp* a);

	virtual void create();
	virtual ~NCPAPropWindow();
};

FXDEFMAP(NCPAPropWindow) NCPAPropWindowMap[] = {
	FXMAPFUNC(SEL_COMMAND,           NCPAPropWindow::ID_ACCEPT,  NCPAPropWindow::onCmdAccept),
	FXMAPFUNC(SEL_COMMAND,           NCPAPropWindow::ID_DISABLE,  NCPAPropWindow::onCmdDisable),
	FXMAPFUNC(SEL_COMMAND,           NCPAPropWindow::ID_WLAN,     NCPAPropWindow::onCmdWLAN),


	FXMAPFUNC(SEL_TIMEOUT,           NCPAPropWindow::ID_TIMEOUT,  NCPAPropWindow::onTimeout),

};

FXIMPLEMENT(NCPAPropWindow, FXMainWindow, NCPAPropWindowMap, ARRAYNUMBER(NCPAPropWindowMap));

int getDefaultGateway(char* ip, int len, char* iface) {
	/* the stuff involved to get the default gateway using linux apis are abysmal
	 * i'd rather use popen at this point */
	FILE* file;
	/*char ip[sizeof("255.255.255.255")] = {0};*/

	char command[256] = {0};

	sprintf(command, "(route -n 2>/dev/null || "
			"/sbin/route -n 2>/dev/null || "
			"/usr/sbin/route -n 2>/dev/null) | "
			"awk '$8==\"%s\" && $4~\"G\" {print $2}'", iface);

	file = popen(command, "r");

	if (file == NULL) {
		strcpy(ip, "Unknown");
		return 0;
	}

	fgets(ip, len, file);
	ip[strcspn(ip, "\n")] = '\0';

	pclose(file);

	return 1;
}

NCPAPropWindow::NCPAPropWindow(FXApp *a) : FXMainWindow(a, "Network Properties", mainIcon, NULL,
		DECOR_TITLE|DECOR_BORDER|DECOR_MENU|DECOR_CLOSE, 0,0,338,0) {
	char ip[sizeof("255.255.255.255")] = {0};
	char ipmask[sizeof("255.255.255.255")] = {0};

	char tmpstr[64];
	
	FXHorizontalSeparator* horsep;
	FXVerticalSeparator*   vertp;
	FXHorizontalFrame*     horfrm;
	FXHorizontalFrame*     horleftfrm;
	
	cont = new FXVerticalFrame(this, LAYOUT_FILL_Y|LAYOUT_FILL_X, 0,0,0,0, 6,6,6,6, 8,8);

	tabbook = new FXTabBook(cont, NULL, 0,
			TABBOOK_NORMAL|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0);

	new FXTabItem(tabbook, "General", NULL, TAB_TOP_NORMAL, 0,0,0,0, 6, 6, 1, 2);

	generalcont = new FXVerticalFrame(tabbook, FRAME_RAISED|FRAME_THICK, 0,0,0,0, 13,13,12,12, 8, 8);

	connectiongrp = new FXGroupBox(generalcont, "Connection",
			GROUPBOX_NORMAL|FRAME_GROOVE|LAYOUT_FILL_X, 0,0,0,0, 12,12,0,4, 0,0);

	connectionmatrix = new FXMatrix(connectiongrp, 2, MATRIX_BY_COLUMNS|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH, 0,0,0,0, 0,0,0,0, 0,0);

#if 0
	printf("Interface Name: %s\n", ifvalues.ifname);
	printf("Packets Sent: %llu\n", ifvalues.sent);
	printf("Packets Received: %llu\n", ifvalues.received);
	puts(ip);
	/*printf("Speed: %d\n", ifvalues.speed);*/
#endif
	sprintf(ip, "%hhu.%hhu.%hhu.%hhu", ifvalues.ip.octets.byte1, ifvalues.ip.octets.byte2, ifvalues.ip.octets.byte3, ifvalues.ip.octets.byte4);
	sprintf(ipmask, "%hhu.%hhu.%hhu.%hhu", ifvalues.mask.octets.byte1, ifvalues.mask.octets.byte2, ifvalues.mask.octets.byte3, ifvalues.mask.octets.byte4);


	new FXLabel(connectionmatrix, "Status:", NULL, LAYOUT_FILL_X|JUSTIFY_LEFT|LAYOUT_FILL_COLUMN);
	if (ifvalues.up) {
		conlbl = new FXLabel(connectionmatrix, "Connected", NULL, LAYOUT_FILL_X|JUSTIFY_RIGHT|LAYOUT_FILL_COLUMN);
	} else {
		conlbl = new FXLabel(connectionmatrix, "Disconnected", NULL, LAYOUT_FILL_X|JUSTIFY_RIGHT|LAYOUT_FILL_COLUMN);
	}
		
	new FXLabel(connectionmatrix, "Interface:", NULL, LAYOUT_FILL_X|JUSTIFY_LEFT|LAYOUT_FILL_COLUMN);
	new FXLabel(connectionmatrix, ifvalues.ifname, NULL, LAYOUT_FILL_X|JUSTIFY_RIGHT|LAYOUT_FILL_COLUMN);
	/*new FXLabel(connectionmatrix, "Speed:", NULL, LAYOUT_FILL_X|JUSTIFY_LEFT|LAYOUT_FILL_COLUMN);
	new FXLabel(connectionmatrix, "1.0 Gbps", NULL, LAYOUT_FILL_X|JUSTIFY_RIGHT|LAYOUT_FILL_COLUMN);*/
	new FXLabel(connectionmatrix, "IP Address:", NULL, LAYOUT_FILL_X|JUSTIFY_LEFT|LAYOUT_FILL_COLUMN);
	new FXLabel(connectionmatrix, ip, NULL, LAYOUT_FILL_X|JUSTIFY_RIGHT|LAYOUT_FILL_COLUMN);
	activitygrp = new FXGroupBox(generalcont, "Activity",
			GROUPBOX_NORMAL|FRAME_GROOVE|LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0, 0,0);

	activitymtx = new FXMatrix(activitygrp, 3, MATRIX_BY_COLUMNS|LAYOUT_FILL_X,
			0,0,0,0, 10,10,0,20, 8,0);

	/*new FXHorizontalSeparator(activitymtx, 
			LAYOUT_FILL_X|LAYOUT_FILL_COLUMN,
			0,0,0,0, 0,0,0,0);*/

	horfrm = new FXHorizontalFrame(activitymtx, LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_COLUMN,
			0,0,0,0, 0,0,0,0, 8,0);

	new FXLabel(horfrm, "Sent", NULL, LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN|JUSTIFY_RIGHT);
	
	horsep = new FXHorizontalSeparator(horfrm, 
			LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT,
			0,0,20,2, 0,0,0,0);

	horsep->setBackColor(getApp()->getShadowColor());

	iconlbl = new FXLabel(activitymtx, "", ico_ncpa_lan_idle, LABEL_NORMAL,
			0,0,0,0, 0,4,0,0);

	horfrm = new FXHorizontalFrame(activitymtx, LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_COLUMN,
			0,0,0,0, 0,0,0,0, 8,0);

	horsep = new FXHorizontalSeparator(horfrm, 
			LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT,
			0,0,20,2, 0,0,0,0);
	
	horsep->setBackColor(getApp()->getShadowColor());

	new FXLabel(horfrm, "Received", NULL, LAYOUT_CENTER_Y);

	horfrm = new FXHorizontalFrame(activitymtx, LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_COLUMN,
			0,0,0,0, 0,0,0,0, 0,0);

	new FXLabel(horfrm, "Packets:", NULL, LAYOUT_FILL_COLUMN);

	sprintf(tmpstr, "%llu", ifvalues.received);

	rxlbl = new FXLabel(horfrm, tmpstr, NULL, LAYOUT_RIGHT);

	vertp = new FXVerticalSeparator(activitymtx, 
			LAYOUT_CENTER_X|LAYOUT_FIX_WIDTH|LAYOUT_FILL_Y,
			0,0,2,0, 0,0,0,0);
	vertp->setBackColor(getApp()->getShadowColor());

	sprintf(tmpstr, "%llu", ifvalues.sent);
	txlbl = new FXLabel(activitymtx, tmpstr, NULL, LAYOUT_RIGHT);

	horfrm = new FXHorizontalFrame(generalcont, LAYOUT_FILL_X,
			0,0,0,0, 0,0,4,4, 4,4);

	FXHorizontalFrame* btnleftcont = new FXHorizontalFrame(horfrm,
			PACK_UNIFORM_WIDTH,
			0,0,0,0, 0,0,0,0, 4,4);


	new FXButton(btnleftcont, "&Properties", NULL, this,
			FXDialogBox::ID_ACCEPT,
			BUTTON_NORMAL|BUTTON_DEFAULT|BUTTON_INITIAL,
			0,0,0,0, 12, 12);
	disablebtn = new FXButton(btnleftcont, "&Disable", NULL, this,
			ID_DISABLE,
			BUTTON_NORMAL|BUTTON_DEFAULT|BUTTON_INITIAL,
			0,0,0,0, 12, 12);
	
	wlanbtn = new FXButton(horfrm, "&View Wireless Networks", NULL, this, ID_WLAN,
			BUTTON_NORMAL|BUTTON_DEFAULT|BUTTON_INITIAL|LAYOUT_FILL_X,
			0,0,0,0, 12, 12);

	if (!(ifvalues.ifname[0] == 'w' && ifvalues.ifname[1] == 'l')) {
		wlanbtn->hide();
	}

	showicon = new FXCheckButton(generalcont, "Sho&w icon in notification area when connected");
	showicon->setCheck(ifvalues.trayshown);

	new FXTabItem(tabbook, "Support", NULL, TAB_TOP_NORMAL, 0,0,0,0, 6, 6, 1, 2);

	supportcont = new FXVerticalFrame(tabbook, FRAME_RAISED|FRAME_THICK, 0,0,0,0, 13,13,12,12, 8, 8);
	
	statusgrp = new FXGroupBox(supportcont, "Connection status",
			GROUPBOX_NORMAL|FRAME_GROOVE|LAYOUT_FILL_X, 0,0,0,0, 12,12,0,4, 0,0);

	horfrm = new FXHorizontalFrame(statusgrp, LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_COLUMN,
			0,0,0,0, 0,0,6,6, 8,0);

	new FXLabel(horfrm, "", ico_ncpa_lan_txrx, LABEL_NORMAL,
			0,0,0,0, 0,4,0,0);

	statusmtx = new FXMatrix(horfrm, 2, MATRIX_BY_COLUMNS|LAYOUT_FILL_X,
			0,0,0,0, 0,0,0,0, 8,0);

	new FXLabel(statusmtx, "IP Address:", NULL, LAYOUT_FILL_COLUMN);
	new FXLabel(statusmtx, ip, NULL, LAYOUT_FILL_COLUMN);
	new FXLabel(statusmtx, "Subnet Mask:", NULL, LAYOUT_FILL_COLUMN);
	new FXLabel(statusmtx, ipmask, NULL, LAYOUT_FILL_COLUMN);
	new FXLabel(statusmtx, "Default Gateway:", NULL, LAYOUT_FILL_COLUMN);

	char gateway[sizeof("255.255.255.255")] = {0};
	getDefaultGateway(gateway, sizeof(gateway), ifvalues.ifname);
	new FXLabel(statusmtx, gateway, NULL, LAYOUT_FILL_COLUMN);
	//new FXLabel(statusmtx, gateway, NULL, LAYOUT_FILL_COLUMN);


	new FXButton(cont, "&Close", NULL, this,
			ID_ACCEPT,
			BUTTON_NORMAL|BUTTON_DEFAULT|BUTTON_INITIAL|LAYOUT_RIGHT,
			0,0,0,0, 20, 20);

	getApp()->addTimeout(this, ID_TIMEOUT, 1000);
}

NCPAPropWindow::~NCPAPropWindow() {
}


void NCPAPropWindow::create() {
	char title[64];
	FXMainWindow::create();
	
	snprintf(title, sizeof(title), "%s Properties", ifvalues.ifname);

	this->setTitle(title);

	show(PLACEMENT_SCREEN);
}


long NCPAPropWindow::onCmdDisable(FXObject*, FXSelector, void*) {
	pid_t pid = vfork();

	if (pid < 0) {
		perror("vfork");
		exit(1);
	} else if (pid == 0) {
		if (ifvalues.up) {
			execlp("i2ksudo", "i2ksudo", "ifdown", ifvalues.ifname, (char*)NULL);
		} else {
			execlp("i2ksudo", "i2ksudo", "ifup", ifvalues.ifname, (char*)NULL);
		}

		perror("execlp");
		exit(1);
	}

	return 1;
}


long NCPAPropWindow::onCmdWLAN(FXObject*, FXSelector, void*) {
	pid_t pid = vfork();

	if (pid < 0) {
		perror("vfork");
		exit(1);
	} else if (pid == 0) {
		execlp("i2kwlan", "i2kwlan", ifvalues.ifname, (char*)NULL);

		perror("execlp");
		exit(1);
	}

	return 1;
}



int ifexist(struct interfaceValues* ifv, char* name) {
	struct ifaddrs *addrs, *tmp;
	/*char netitem[64];
	char nettype[256];*/

	//int iftype = NCPA_IFTYPE_LAN;

	getifaddrs(&addrs);
	tmp = addrs;

	while (tmp) {
	    if (tmp->ifa_addr == NULL) goto next;
		if (tmp->ifa_addr->sa_family != AF_PACKET && tmp->ifa_addr->sa_family != AF_INET) goto next;
		
		if (strcmp(tmp->ifa_name, name)) goto next;

		ifv->exists = 1;

		if (tmp->ifa_flags & IFF_UP) {
			ifv->up = 1;
		} else {
			ifv->up = 0;
		}

		if (tmp->ifa_addr->sa_family == AF_INET) {
			struct sockaddr_in* ipv4 = (struct sockaddr_in*)tmp->ifa_addr;
			unsigned char* bytes = (unsigned char*)&ipv4->sin_addr.s_addr;
			
			ifv->ip.octets.byte1 = bytes[0];
			ifv->ip.octets.byte2 = bytes[1];
			ifv->ip.octets.byte3 = bytes[2];
			ifv->ip.octets.byte4 = bytes[3];

			if (tmp->ifa_netmask != NULL) {
				struct sockaddr_in* mask = (struct sockaddr_in*)tmp->ifa_netmask;
				unsigned char* maskbytes = (unsigned char*)&mask->sin_addr.s_addr;
			
				ifv->mask.octets.byte1 = maskbytes[0];
				ifv->mask.octets.byte2 = maskbytes[1];
				ifv->mask.octets.byte3 = maskbytes[2];
				ifv->mask.octets.byte4 = maskbytes[3];
			}
		}

		if (tmp->ifa_addr->sa_family == AF_PACKET) {
			strcpy(ifv->ifname, tmp->ifa_name);

#ifdef __linux__
			struct rtnl_link_stats* stats = (rtnl_link_stats*)tmp->ifa_data;
			if (stats != NULL) {
				ifv->sent = stats->tx_packets;
				ifv->received = stats->rx_packets;
			}
#endif
		}
#if 0
		switch (iftype) {
			case NCPA_IFTYPE_WLAN:
				/*icl->appendItem(netitem, ico_ncpa_lan_idle, ico_ncpa_lan);*/
				break;

			case NCPA_IFTYPE_DIAL:
				/*icl->appendItem(netitem, ico_ncpa_dial_idle, ico_ncpa_dial);*/
				break;

			default:
			case NCPA_IFTYPE_LAN:
				/*icl->appendItem(netitem, ico_ncpa_lan_idle, ico_ncpa_lan);*/
				break;
		}
#endif
next:
	    tmp = tmp->ifa_next;
	}

	freeifaddrs(addrs);
}


long NCPAPropWindow::onTimeout(FXObject*, FXSelector, void*) {
	char ifname[16];
	strcpy(ifname, ifvalues.ifname);

	memcpy(&ifvalues_old, &ifvalues, sizeof(ifvalues));
	ifexist(&ifvalues, ifname);

	char tmpstr[64];
	
	sprintf(tmpstr, "%llu", ifvalues.received);
	rxlbl->setText(tmpstr);

	sprintf(tmpstr, "%llu", ifvalues.sent);
	txlbl->setText(tmpstr);

#if 0 
FXIcon* ico_ncpa_lan;
FXIcon* ico_ncpa_lan_down;
FXIcon* ico_ncpa_lan_idle;
FXIcon* ico_ncpa_lan_nohw;
FXIcon* ico_ncpa_lan_rx;
FXIcon* ico_ncpa_lan_tx;
FXIcon* ico_ncpa_lan_txrx;
#endif

	if (ifvalues.received > ifvalues_old.received) {
		if (ifvalues.sent > ifvalues_old.sent) {
			iconlbl->setIcon(ico_ncpa_lan_txrx);
		} else {
			iconlbl->setIcon(ico_ncpa_lan_rx);
		}
	} else if (ifvalues.sent > ifvalues_old.sent) {
		iconlbl->setIcon(ico_ncpa_lan_tx);
	} else {
		iconlbl->setIcon(ico_ncpa_lan_idle);
	}

	if (ifvalues.up) {
		disablebtn->setText("&Disable");
		conlbl->setText("Connected");
	} else {
		disablebtn->setText("&Enable");
		conlbl->setText("Disconnected");
	}


	/*sprintf(ip, "%hhu.%hhu.%hhu.%hhu\n", ifvalues.ip.octets.byte1, ifvalues.ip.octets.byte2, ifvalues.ip.octets.byte3, ifvalues.ip.octets.byte4);*/
	
	getApp()->addTimeout(this, ID_TIMEOUT, 1000);
}

long NCPAPropWindow::onCmdAccept(FXObject*, FXSelector, void*) {
	int showiconval = showicon->getCheck();
	if (showiconval != ifvalues.trayshown) {
		if (showiconval) {
			FILE* fptr = fopen(iftraypath, "w");
			if (fptr != NULL) fclose(fptr);
		} else {
			remove(iftraypath);
		}

		FXMessageBox::information(getApp(), MBOX_OK,
				"Network Properties",
				"You will need to log off for these changes to apply.");
	}

	getApp()->exit();

	return 1;
}

int getHomeDir(char* buf, size_t len) {
	const char *homedir;

	if ((homedir = getenv("HOME")) == NULL) {
		homedir = getpwuid(getuid())->pw_dir;
	}

	if (homedir != NULL && homedir[0] != '\0') {
		return sprintf(buf, "%.*s", (int)len-1, homedir);
		buf[len-1] = '\0';
	} else {
		strcpy(buf, "/");
		return 1;
	}
}

int main(int argc, char *argv[]) {
	FXApp application("NCPAProp", "I2KProject");
	//mainIcon = new FXPNGIcon(&application, resico_mainicon);

	application.init(argc, argv);

	if (argv[1] == NULL) {
		application.create();
		FXMessageBox::error(&application, MBOX_OK, "Error", "Please specify interface name in argument!");
		return 1;
	}

	ifexist(&ifvalues, argv[1]);

	if (ifvalues.exists == 0) {
		application.create();
		FXMessageBox::error(&application, MBOX_OK, "Error", "The interface doesn't exist!");
		application.exit();
		return 1;
	}

	char homedir[PATH_MAX] = {0};

	int homedirlen = getHomeDir(homedir, sizeof(homedir));

	sprintf(iftraypath, "%.*s/.icewm/cfg/nettray/%.*s", homedirlen, homedir,
			(int)(sizeof(iftraypath)-homedirlen-sizeof("/.icewm/cfg/nettray/")),
				argv[1]);

	if (iftraypath[0] != '\0' && access(iftraypath, F_OK) == 0) {
		ifvalues.trayshown = 1;
	} else {
		ifvalues.trayshown = 0;
	}

	/*ico_ncpa_dial = loadPNGIcon(&application, resico_2k_ncpa_dial); ico_ncpa_dial->create();
	ico_ncpa_dial_down = loadPNGIcon(&application, resico_2k_ncpa_dial_down); ico_ncpa_dial_down->create();
	ico_ncpa_dial_idle = loadPNGIcon(&application, resico_2k_ncpa_dial_idle); ico_ncpa_dial_idle->create();
	ico_ncpa_dial_nohw = loadPNGIcon(&application, resico_2k_ncpa_dial_nohw); ico_ncpa_dial_nohw->create();
	ico_ncpa_dial_rx = loadPNGIcon(&application, resico_2k_ncpa_dial_rx); ico_ncpa_dial_rx->create();
	ico_ncpa_dial_tx = loadPNGIcon(&application, resico_2k_ncpa_dial_tx); ico_ncpa_dial_tx->create();
	ico_ncpa_dial_txrx = loadPNGIcon(&application, resico_2k_ncpa_dial_txrx); ico_ncpa_dial_txrx->create();
	ico_ncpa_direct = loadPNGIcon(&application, resico_2k_ncpa_direct); ico_ncpa_direct->create();
	ico_ncpa_direct_down = loadPNGIcon(&application, resico_2k_ncpa_direct_down); ico_ncpa_direct_down->create();
	ico_ncpa_direct_idle = loadPNGIcon(&application, resico_2k_ncpa_direct_idle); ico_ncpa_direct_idle->create();
	ico_ncpa_direct_rx = loadPNGIcon(&application, resico_2k_ncpa_direct_rx); ico_ncpa_direct_rx->create();
	ico_ncpa_direct_tx = loadPNGIcon(&application, resico_2k_ncpa_direct_tx); ico_ncpa_direct_tx->create();
	ico_ncpa_direct_txrx = loadPNGIcon(&application, resico_2k_ncpa_direct_txrx); ico_ncpa_direct_txrx->create();*/
	if (i2kBGetWinVersionInt() == ICE2K_BRAND_WINXP) {
		ico_ncpa_lan = loadPNGIcon(&application, resico_xp_ncpa_lan); ico_ncpa_lan->create();
		ico_ncpa_lan_down = loadPNGIcon(&application, resico_xp_ncpa_lan_down); ico_ncpa_lan_down->create();
		ico_ncpa_lan_idle = loadPNGIcon(&application, resico_xp_ncpa_lan_idle); ico_ncpa_lan_idle->create();
		ico_ncpa_lan_nohw = loadPNGIcon(&application, resico_xp_ncpa_lan_nohw); ico_ncpa_lan_nohw->create();
		ico_ncpa_lan_rx = loadPNGIcon(&application, resico_xp_ncpa_lan_rx); ico_ncpa_lan_rx->create();
		ico_ncpa_lan_tx = loadPNGIcon(&application, resico_xp_ncpa_lan_tx); ico_ncpa_lan_tx->create();
		ico_ncpa_lan_txrx = loadPNGIcon(&application, resico_xp_ncpa_lan_txrx); ico_ncpa_lan_txrx->create();
	} else {
		ico_ncpa_lan = loadPNGIcon(&application, resico_2k_ncpa_lan); ico_ncpa_lan->create();
		ico_ncpa_lan_down = loadPNGIcon(&application, resico_2k_ncpa_lan_down); ico_ncpa_lan_down->create();
		ico_ncpa_lan_idle = loadPNGIcon(&application, resico_2k_ncpa_lan_idle); ico_ncpa_lan_idle->create();
		ico_ncpa_lan_nohw = loadPNGIcon(&application, resico_2k_ncpa_lan_nohw); ico_ncpa_lan_nohw->create();
		ico_ncpa_lan_rx = loadPNGIcon(&application, resico_2k_ncpa_lan_rx); ico_ncpa_lan_rx->create();
		ico_ncpa_lan_tx = loadPNGIcon(&application, resico_2k_ncpa_lan_tx); ico_ncpa_lan_tx->create();
		ico_ncpa_lan_txrx = loadPNGIcon(&application, resico_2k_ncpa_lan_txrx); ico_ncpa_lan_txrx->create();
	}

	mainIcon = ico_ncpa_lan;
	/*ico_ncpa_vpn = loadPNGIcon(&application, resico_2k_ncpa_vpn); ico_ncpa_vpn->create();
	ico_ncpa_vpn_idle = loadPNGIcon(&application, resico_2k_ncpa_vpn_idle); ico_ncpa_vpn_idle->create();
	ico_ncpa_vpn_rx = loadPNGIcon(&application, resico_2k_ncpa_vpn_rx); ico_ncpa_vpn_rx->create();
	ico_ncpa_vpn_tx = loadPNGIcon(&application, resico_2k_ncpa_vpn_tx); ico_ncpa_vpn_tx->create();
	ico_ncpa_vpn_txrx = loadPNGIcon(&application, resico_2k_ncpa_vpn_txrx); ico_ncpa_vpn_txrx->create();*/

	new NCPAPropWindow(&application);

	application.create();
	return application.run();
}




