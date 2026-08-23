#include <fx.h>
#include <stdint.h>

#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/if_link.h>
#include <FXPNGIcon.h>
#include <ice2k/branding.h>
#include <ice2k/i2ktray/I2KTrayApp.h>
#include <ice2k/i2ktray/I2KTrayIcon.h>

#include "res/foxres.h"

FXIcon* mainicon;

FXIcon* ico_lan_down;
FXIcon* ico_lan_idle;
FXIcon* ico_lan_rx;
FXIcon* ico_lan_tx;
FXIcon* ico_lan_txrx;
FXIcon* ico_wlan_down;
FXIcon* ico_wlan_idle;
FXIcon* ico_wlan_rx;
FXIcon* ico_wlan_tx;
FXIcon* ico_wlan_txrx;

int wireless = 0;

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

	unsigned char up:1;
	unsigned char running:1;
	unsigned char exists:1;
	unsigned char trayshown:1;
};

struct interfaceValues oifvalues = {0};
struct interfaceValues nifvalues = {0};



int ifexist(struct interfaceValues* ifv, char* name) {
	struct ifaddrs *addrs, *tmp;
	int exists = 0;
	/*char netitem[64];
	char nettype[256];*/

	//int iftype = NCPA_IFTYPE_LAN;

	getifaddrs(&addrs);
	tmp = addrs;

	if (ifv != NULL) {
		ifv->up = 0;
		ifv->running = 0;
	}

	while (tmp) {
	    if (tmp->ifa_addr == NULL) goto next;
		if (tmp->ifa_addr->sa_family != AF_PACKET && tmp->ifa_addr->sa_family != AF_INET) goto next;
		
		if (strcmp(tmp->ifa_name, name)) goto next;

		exists = 1;
		if (ifv == NULL) return 1;
		strcpy(ifv->ifname, name);
		ifv->exists = 1;

		ifv->up = (ifv->up || (tmp->ifa_flags & IFF_UP));
		ifv->running = (ifv->running || (tmp->ifa_flags & IFF_RUNNING));


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
#ifdef __linux__
			struct rtnl_link_stats* stats = (rtnl_link_stats*)tmp->ifa_data;
			if (stats != NULL) {
				ifv->sent = stats->tx_packets;
				ifv->received = stats->rx_packets;
			}
#endif
		}
next:
	    tmp = tmp->ifa_next;
	}

	freeifaddrs(addrs);

	return exists;
}

class NetTray : public FXObject {
	FXDECLARE(NetTray)

public:
    enum {
        ID_CONNECT = FXApp::ID_LAST,
		ID_FORGET,
		ID_ENABLE,
		ID_DISABLE,
		ID_DISCONNECT,
		ID_STATUS,
		ID_TIMEOUT,
        ID_LAST
    };

    NetTray(FXApp* app);

    long onCmdConnect(FXObject* sender, FXSelector sel, void*);
    long onCmdStatus(FXObject* sender, FXSelector sel, void*);

    long onCmdForget(FXObject* sender, FXSelector sel, void*);
    long onCmdDisconnect(FXObject* sender, FXSelector sel, void*);

    long onCmdEnable(FXObject* sender, FXSelector sel, void*);
    long onCmdDisable(FXObject* sender, FXSelector sel, void*);

	long onTimeout(FXObject* sender, FXSelector sel, void*);

private:
    NetTray() {}

    FXApp* app;

    FXPopup* popup;
    I2KTrayIcon* tray;
};

FXDEFMAP(NetTray) NetTrayMap[] = {
	FXMAPFUNC(SEL_COMMAND, NetTray::ID_CONNECT, NetTray::onCmdConnect),
	FXMAPFUNC(SEL_COMMAND, NetTray::ID_STATUS, NetTray::onCmdStatus),

	FXMAPFUNC(SEL_COMMAND, NetTray::ID_FORGET,  NetTray::onCmdForget),
	FXMAPFUNC(SEL_COMMAND, NetTray::ID_DISCONNECT, NetTray::onCmdDisconnect),

	FXMAPFUNC(SEL_COMMAND, NetTray::ID_ENABLE,  NetTray::onCmdEnable),
	FXMAPFUNC(SEL_COMMAND, NetTray::ID_DISABLE, NetTray::onCmdDisable),

	FXMAPFUNC(SEL_TIMEOUT, NetTray::ID_TIMEOUT, NetTray::onTimeout),
};

FXIMPLEMENT(NetTray, FXObject, NetTrayMap, ARRAYNUMBER(NetTrayMap));

FXIcon* getNetIcon(struct interfaceValues* ovalues, struct interfaceValues* nvalues) {
	if (nvalues == NULL) return ico_lan_down;
	
	int wireless = (nvalues->ifname[0] == 'w');
	int up = (nvalues->up && nvalues->running);

	//printf("up: %d, running: %d\n" , nvalues->up,  nvalues->running);
	if (!up) {
		//puts("not up!");
		if (wireless) {
			return ico_wlan_down;
		} else {
			return ico_lan_down;
		}
	}

	//puts("m");

	if (ovalues == NULL) {
		if (wireless) {
			return ico_wlan_idle;
		} else {
			return ico_lan_idle;
		}
	}

	long long n_rx = nvalues->received;
	long long n_tx = nvalues->sent;

	long long o_rx = ovalues->received;
	long long o_tx = ovalues->sent;

	long long rx = n_rx - o_rx;
	long long tx = n_tx - o_tx;

	if (wireless) {
		//puts("wlan");
		if (tx) {
			if (rx) {
				return ico_wlan_txrx;
			} else {
				return ico_wlan_tx;
			}
		} else {
			if (rx) {
				return ico_wlan_rx;
			} else {
				return ico_wlan_idle;
			}
		}
	} else {
		//puts("lan");
		if (tx) {
			if (rx) {
				return ico_lan_txrx;
			} else {
				return ico_lan_tx;
			}
		} else {
			if (rx) {
				return ico_lan_rx;
			} else {
				return ico_lan_idle;
			}
		}
	}

	return ico_lan_down;
}

NetTray::NetTray(FXApp* a) :
	app(a)
{
	char ifname[16];

	strcpy(ifname, nifvalues.ifname);
	ifexist(&nifvalues, ifname);
	
	FXFont* normalfont = a->getNormalFont();

	FXString fname = normalfont->getFont();
	FXuint fsize = normalfont->getSize()/10;
	FXuint fslant = normalfont->getSlant();
	FXuint fencoding = normalfont->getEncoding();
	FXuint fsetwidth = normalfont->getSetWidth();
	FXuint fhints = normalfont->getHints();

	FXFont* boldfont = new FXFont(a, fname, fsize, FXFont::Bold, fslant, fencoding, fsetwidth, fhints);
	if (boldfont == NULL) {
		boldfont = normalfont;
	} else {
		boldfont->create();
	}


	if (wireless) {
	    tray = new I2KTrayIcon(app, ifname, ico_wlan_down, 0, this, ID_CONNECT, TRAY_CMD_ON_LEFT|TRAY_MENU_ON_RIGHT);
	} else {
	    tray = new I2KTrayIcon(app, ifname, ico_lan_down, 0, this, ID_STATUS, TRAY_CMD_ON_LEFT|TRAY_MENU_ON_RIGHT);
	}

    popup = new FXPopup(tray);
	FXMenuCommand* status_cmd = new FXMenuCommand(popup, "Status", NULL, this, NetTray::ID_STATUS);

	if (!wireless) status_cmd->setFont(boldfont);
    new FXMenuSeparator(popup);
    new FXMenuCommand(popup, "Enable", NULL, this, NetTray::ID_ENABLE);
    new FXMenuCommand(popup, "Disable", NULL, this, NetTray::ID_DISABLE);

	if (wireless) {
	    new FXMenuSeparator(popup);
		new FXMenuCommand(popup, "Disconnect", NULL, this, NetTray::ID_DISCONNECT);
	    new FXMenuSeparator(popup);

		FXMenuCommand* connect_cmd = new FXMenuCommand(popup, "Connect", NULL, this, NetTray::ID_CONNECT);
		connect_cmd->setFont(boldfont);
		new FXMenuCommand(popup, "Forget", NULL, this, NetTray::ID_FORGET);
	}
		

	tray->setMenu(popup);

	app->addTimeout(this, ID_TIMEOUT, 500);
}

long NetTray::onCmdEnable(FXObject*, FXSelector, void*) {
	system("i2ksudo ifup \"$IFNAME\" &");
    return 1;
}

long NetTray::onCmdDisable(FXObject*, FXSelector, void*) {
	system("i2ksudo ifdown \"$IFNAME\" &");
    return 1;
}

long NetTray::onCmdConnect(FXObject*, FXSelector, void*) {
	system("i2kwlan \"$IFNAME\" &");
    return 1;
}

long NetTray::onCmdStatus(FXObject*, FXSelector, void*) {
	system("ncpastat \"$IFNAME\" &");
	return 1;
}


long NetTray::onCmdForget(FXObject*, FXSelector, void*) {
	system("i2kwlanprof \"$IFNAME\" &");
    return 1;
}


long NetTray::onCmdDisconnect(FXObject*, FXSelector, void*) {
	system("iwctl station \"$IFNAME\" disconnect &");
    return 1;
}

long NetTray::onTimeout(FXObject*, FXSelector, void*) {
	char ifname[16];
	//FXMessageBox::information(app, MBOX_OK, "Tray Test", "Hello World!");

	memcpy(&oifvalues, &nifvalues, sizeof(nifvalues));
	strcpy(ifname, nifvalues.ifname);
	ifexist(&nifvalues, ifname);

	tray->setIcon(getNetIcon(&oifvalues, &nifvalues));

	app->addTimeout(this, ID_TIMEOUT, 500);

    return 1;
}




int main(int argc, char* argv[]) {
	if (argv[1] == NULL) {
		fputs("Please specify interface name in argument!\n", stderr);
		return 1;
	}

	if (ifexist(&nifvalues, argv[1]) == 0) {
		fputs("The interface does not exist!\n", stderr);
		return 1;
	}



    I2KTrayApp application("NetTray", "I2KProject");
   
	application.init(argc, argv);

	setenv("IFNAME", argv[1], 1);

	if (i2kBGetWinVersionInt() < ICE2K_BRAND_WINXP) {
		if (argv[1][0] == 'w') {
			wireless = 1;

			ico_wlan_down = new FXPNGIcon(&application, resico_2k_wlan_down);
			ico_wlan_idle = new FXPNGIcon(&application, resico_2k_wlan_idle);
			ico_wlan_rx = new FXPNGIcon(&application, resico_2k_wlan_rx);
			ico_wlan_tx = new FXPNGIcon(&application, resico_2k_wlan_tx);
			ico_wlan_txrx = new FXPNGIcon(&application, resico_2k_wlan_txrx);
		} else {
			ico_lan_down = new FXPNGIcon(&application, resico_2k_lan_down);
			ico_lan_idle = new FXPNGIcon(&application, resico_2k_lan_idle);
			ico_lan_rx = new FXPNGIcon(&application, resico_2k_lan_rx);
			ico_lan_tx = new FXPNGIcon(&application, resico_2k_lan_tx);
			ico_lan_txrx = new FXPNGIcon(&application, resico_2k_lan_txrx);
		}
	} else {
		if (argv[1][0] == 'w') {
			wireless = 1;

			ico_wlan_down = new FXPNGIcon(&application, resico_xp_wlan_down);
			ico_wlan_down->blend(application.getBaseColor());
		
			ico_wlan_idle = new FXPNGIcon(&application, resico_xp_wlan_idle);
			ico_wlan_idle->blend(application.getBaseColor());
		
			ico_wlan_rx = new FXPNGIcon(&application, resico_xp_wlan_rx);
			ico_wlan_rx->blend(application.getBaseColor());
		
			ico_wlan_tx = new FXPNGIcon(&application, resico_xp_wlan_tx);
			ico_wlan_tx->blend(application.getBaseColor());

			ico_wlan_txrx = new FXPNGIcon(&application, resico_xp_wlan_txrx);
			ico_wlan_txrx->blend(application.getBaseColor());
		} else {
			ico_lan_down = new FXPNGIcon(&application, resico_xp_lan_down);
			ico_lan_down->blend(application.getBaseColor());
		
			ico_lan_idle = new FXPNGIcon(&application, resico_xp_lan_idle);
			ico_lan_idle->blend(application.getBaseColor());
		
			ico_lan_rx = new FXPNGIcon(&application, resico_xp_lan_rx);
			ico_lan_rx->blend(application.getBaseColor());
		
			ico_lan_tx = new FXPNGIcon(&application, resico_xp_lan_tx);
			ico_lan_tx->blend(application.getBaseColor());
		
			ico_lan_txrx = new FXPNGIcon(&application, resico_xp_lan_txrx);
			ico_lan_txrx->blend(application.getBaseColor());
		}
	}

	if (argv[1][0] == 'w') {
		ico_wlan_down->create();
		ico_wlan_idle->create();
		ico_wlan_rx->create();
		ico_wlan_tx->create();
		ico_wlan_txrx->create();
	} else {
		ico_lan_down->create();
		ico_lan_idle->create();
		ico_lan_rx->create();
		ico_lan_tx->create();
		ico_lan_txrx->create();
	}

    NetTray tray_app(&application);

    application.create();

    return application.run();
}


