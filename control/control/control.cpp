#include <unistd.h>
#include <fx.h>
#include <FXPNGIcon.h>
#include <ice2k/comctl32.h>
#include <ice2k/branding.h>
#include "res/foxres.h"
#include <limits.h>
#include <pwd.h>
#include <X11/Xlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <string.h>


int xpmode = 0;

FXIcon* ico_control;

FXIcon* ico_exp_back;
FXIcon* ico_exp_forward;
FXIcon* ico_exp_up;

FXIcon* ico_exp_search;
FXIcon* ico_exp_folders;

FXIcon* ico_exp_copyto;
FXIcon* ico_exp_moveto;
FXIcon* ico_exp_delete;
FXIcon* ico_exp_properties;

FXIcon* ico_exp_views;

FXIcon* ico_exp_go;

FXIcon* ico_exp_throbber;


FXIcon* ico_arrow;

FXIcon* ico_appwiz_16;
FXIcon* ico_appwiz_32;
FXIcon* ico_desk_16;
FXIcon* ico_desk_32;
FXIcon* ico_fonts_16;
FXIcon* ico_fonts_32;
FXIcon* ico_inetcpl_16;
FXIcon* ico_inetcpl_32;
FXIcon* ico_joy_16;
FXIcon* ico_joy_32;
FXIcon* ico_keyb_16;
FXIcon* ico_keyb_32;
FXIcon* ico_mice_16;
FXIcon* ico_mice_32;
FXIcon* ico_mmsys_16;
FXIcon* ico_mmsys_32;
FXIcon* ico_modem_16;
FXIcon* ico_modem_32;
FXIcon* ico_ncpa_16;
FXIcon* ico_ncpa_32;
FXIcon* ico_powercfg_16;
FXIcon* ico_powercfg_32;
FXIcon* ico_timedate_16;
FXIcon* ico_timedate_32;
FXIcon* ico_sysdm_16;
FXIcon* ico_sysdm_32;
FXIcon* ico_printers_16;
FXIcon* ico_printers_32;

FXMainWindow* controlwin;

FXSettings xfesettings;
int xfeloaded = 0;

char iconspath[PATH_MAX];

int winx = 64;
int winy = 64;
int winw = 520;
int winh = 380;

int winsaved = 0;

#define CPL_ID_APPWIZ     1
#define CPL_ID_JOY        2
#define CPL_ID_TIMEDATE   3
#define CPL_ID_DESK       4
#define CPL_ID_FONTS      5
#define CPL_ID_KEYB       6
#define CPL_ID_MODEM      7
#define CPL_ID_MMSYS      8
#define CPL_ID_NCPA       9
#define CPL_ID_POWERCFG  10
#define CPL_ID_SYSDM     11
#define CPL_ID_INETCPL   12
#define CPL_ID_PRINTERS  13

#define SHF_ID_CONTROL 1
#define SHF_ID_NCPA 2

int shellfolder = SHF_ID_CONTROL;

class ControlPanel : public FXMainWindow {
	FXDECLARE(ControlPanel)

	private:
		FXDockSite*              topdock;

		FXPacker*                statusbarcont;
		FXLabel*                 statuslbl;

		FXPacker* corner;
		FXDragCorner* realcorner;

		FXMenuBar*               menubar;

		FXMenuButton*            viewbutton;

		FXMenuPane*              filemenu;
		FXMenuPane*              editmenu;
		FXMenuPane*              viewmenu;
		FXMenuPane*              favoritesmenu;
		FXMenuPane*              toolsmenu;
		FXMenuPane*              helpmenu;


		FXToolBarShell*          mbshell;
		FXToolBarShell*          tbshell;
		FXToolBarShell*          adshell;

		FXToolBarShell*          throbshell;

		FXToolBar*               toolbar;
		FXToolBar*               adtoolbar;
		FXToolBar*               throbtoolbar;

		FXPacker*                iconlistframe;
		FXIconList*              iconlist;

	protected:
		ControlPanel(){}

	public:
		long onCplActivate(FXObject*,FXSelector,void*);
		long onRightClick(FXObject*,FXSelector,void*);


		long doNothing(FXObject*,FXSelector,void*);
		long doOpen(FXObject*,FXSelector,void*);
		long onCmdAbout(FXObject*,FXSelector,void*);

		long onCmdSelectAll(FXObject*,FXSelector,void*);
		long onCmdInvertSelection(FXObject*,FXSelector,void*);

		long onCmdViewMenu(FXObject*,FXSelector,void*);
		long onCmdUp(FXObject*,FXSelector,void*);


		void setFocus();

	public:
		enum {
			ID_MAINWIN=FXMainWindow::ID_LAST,
			ID_CPL,
			ID_OPEN,
			ID_ABOUT,

			ID_SELECTALL,
			ID_INVERTSELECT,

			ID_VIEWMENU,
			ID_UP,
		};

	public:
		ControlPanel(FXApp* a);

		virtual void create();
		virtual ~ControlPanel();
};

FXDEFMAP(ControlPanel) ControlPanelMap[] = {
	/* FXMAPFUNC(SEL_UPDATE, 0, ControlPanel::onStatus),
	   FXMAPFUNC(SEL_COMMAND, ControlPanel::ID_SETSTRINGVALUE, ControlPanel::onChangeText),
	   FXMAPFUNC(SEL_COMMAND, ControlPanel::ID_ADDDEV, ControlPanel::addDevices),

	   FXMAPFUNC(SEL_CHANGED, ControlPanel::ID_TREE, ControlPanel::onItemChange), */
	//FXMAPFUNC(SEL_CHANGED, ControlPanel::ID_CPL, ControlPanel::onCplActivate),
	FXMAPFUNC(SEL_DOUBLECLICKED, ControlPanel::ID_CPL, ControlPanel::onCplActivate),
	FXMAPFUNC(SEL_RIGHTBUTTONPRESS, ControlPanel::ID_CPL, ControlPanel::onRightClick),
	FXMAPFUNC(SEL_COMMAND, ControlPanel::ID_OPEN, ControlPanel::doOpen),
	FXMAPFUNC(SEL_COMMAND, ControlPanel::ID_ABOUT, ControlPanel::onCmdAbout),

	FXMAPFUNC(SEL_COMMAND, ControlPanel::ID_SELECTALL, ControlPanel::onCmdSelectAll),
	FXMAPFUNC(SEL_COMMAND, ControlPanel::ID_INVERTSELECT, ControlPanel::onCmdInvertSelection),

	FXMAPFUNC(SEL_COMMAND, ControlPanel::ID_VIEWMENU, ControlPanel::onCmdViewMenu),

	FXMAPFUNC(SEL_COMMAND, ControlPanel::ID_UP, ControlPanel::onCmdUp),
};

FXIMPLEMENT(ControlPanel,FXMainWindow,ControlPanelMap,ARRAYNUMBER(ControlPanelMap));

long ControlPanel::doNothing(FXObject*,FXSelector,void*) {
	return 1;
}

long ControlPanel::doOpen(FXObject*,FXSelector,void*) {
	return handle(this,FXSEL(SEL_DOUBLECLICKED,ID_CPL),(void*)(FXival)(iconlist->getCurrentItem()));
}


long ControlPanel::onCmdAbout(FXObject*,FXSelector,void*) {
	system("winver &");
	return 1;
}

int getHomeDir(char* buf, unsigned int size) {
	char *homedir;

	if ((homedir = getenv("HOME")) == NULL) {
		homedir = getpwuid(getuid())->pw_dir;
	}

	if (homedir == NULL) {
		sprintf(buf, "%.*s", size-1, "/");
		buf[size-1] = '\0';

		return 1;
	}

	sprintf(buf, "%.*s", size-1, homedir);
	buf[size-1] = '\0';

	return 1;
}

int getXdgConfigHome(char* buf, unsigned int size) {
	char homedir[PATH_MAX];
	char *xdgdir;

	if ((xdgdir = getenv("XDG_CONFIG_HOME")) == NULL) {
		getHomeDir(homedir, sizeof(homedir));
		
		sprintf(buf, "%.*s/.config", size-9, homedir); 
		buf[size-1] = '\0';

		return 1;
	}

	sprintf(buf, "%.*s", size-1, xdgdir);
	buf[size-1] = '\0';

	//puts(buf);

	return 1;
}

int getXfeConfig(char* buf, unsigned int size) {
	char xdghome[PATH_MAX];

	getXdgConfigHome(xdghome, sizeof(xdghome));
	sprintf(buf, "%.*s/xfe/xferc", (int)(size-11), xdghome);
	buf[size-1] = '\0';

	return 1;
}


void loadXfeSettings() {
	char xfeconfig[PATH_MAX];

	if (!xfeloaded) {
		getXfeConfig(xfeconfig, sizeof(xfeconfig));
		xfesettings.parseFile(xfeconfig, TRUE);

		xfeloaded = 1;
	}
}

int saveWindowPosition() {
	if (winsaved) return 1;

	int l = 0; int t = 0; int dummy;
	char xfeconfig[PATH_MAX];
	
	if (xfeloaded) {
		getXfeConfig(xfeconfig, sizeof(xfeconfig));

		xfesettings.clear();
		xfesettings.parseFile(xfeconfig, TRUE);

		controlwin->getWMBorders(l, dummy, t, dummy);
		
		xfesettings.writeIntEntry("OPTIONS", "xpos", controlwin->getX()-l);
		xfesettings.writeIntEntry("OPTIONS", "ypos", controlwin->getY()-t);
		xfesettings.writeIntEntry("OPTIONS", "width", controlwin->getWidth());
		xfesettings.writeIntEntry("OPTIONS", "height", controlwin->getHeight());

		xfesettings.unparseFile(xfeconfig);

		winsaved = 1;
	}

	return 1;
}


long ControlPanel::onCmdUp(FXObject*,FXSelector,void*) {
	saveWindowPosition();
	getApp()->exit();
	system("xfe / &");
	usleep(1000 * 700);
	return 1;
}



long ControlPanel::onCmdViewMenu(FXObject* obj,FXSelector,void*) {
	FXButton* btn = (FXButton*)obj;
	viewbutton->handle(viewbutton,FXSEL(SEL_COMMAND,FXMenuButton::ID_POST),NULL);
	btn->killFocus();
	return 1;
}
long ControlPanel::onCmdSelectAll(FXObject*,FXSelector,void*) {
	for (int i = 0; i < iconlist->getNumItems(); i++) {
		//printf("%d\n", i);
		iconlist->selectItem(i);
	}
	return 1;
}

long ControlPanel::onCmdInvertSelection(FXObject*,FXSelector,void*) {
	for (int i = 0; i < iconlist->getNumItems(); i++) {
		if (iconlist->isItemSelected(i)) {
			iconlist->deselectItem(i);
		} else {
			iconlist->selectItem(i);
		}
	}
	return 1;
}

long ControlPanel::onRightClick(FXObject* obj,FXSelector sel,void* ptr) {
	FXEvent* ev = (FXEvent*)ptr;
	int index = iconlist->getItemAt(ev->win_x, ev->win_y);
	
	if (index != -1) {
		iconlist->killSelection(TRUE);
		iconlist->selectItem(index, TRUE);
		iconlist->setAnchorItem(index);
		iconlist->setCurrentItem(index, TRUE);
		//printf("%d\n",iconlist->getAnchorItem());
	}

	return 1;
}

void ControlPanel::create() {
	FXMainWindow::create();
	show(PLACEMENT_DEFAULT);
	//position(winx+4, winy+23, winw, winh);
}


// hack to fix focusing issues on icewm
//
// fox toolkit calls setfocus on the main window one way or another
// and the way fox focuses the window causes issues and right now
// i'm too lazy to fork fox to fix it
void ControlPanel::setFocus() { return; }


ControlPanel::~ControlPanel() {
	saveWindowPosition();

	delete filemenu;
	delete editmenu;
	delete viewmenu;
	delete favoritesmenu;
	delete toolsmenu;
	delete helpmenu;
}

FXIcon* loadPNGIcon(FXApp* app, const void* pix=NULL,
                    FXColor clr=FXRGB(192,192,192), FXColor blendclr=FXRGB(255,255,255),
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

FXIcon* loadPNGIconFromFile(FXApp* app, const char* str,
                    FXColor clr=FXRGB(192,192,192), FXColor blendclr=FXRGB(255,255,255),
                    FXuint opts=0, FXint w=1, FXint h=1) {
	FXFileStream stream(app);
	if (!stream.open(str, FXStreamLoad)) return NULL;

	FXIcon* icon = new FXPNGIcon(app, NULL, clr, opts, w, h);
	icon->loadPixels(stream);
	stream.close();

	if (blendclr == FXRGB(192,192,192)) {
		blendclr = app->getBaseColor();
	} else if (blendclr == FXRGB(255,255,255)) {
		blendclr = app->getBackColor();
	}

	icon->blend(blendclr);
	icon->create();

	return(icon);
}

FXIcon* loadToolbarIcon(FXApp* app, const char* str) {
	char iconpath[4096];

	sprintf(iconpath, "%.*s/", (int)(sizeof(iconspath)-2), iconspath);
	strncat(iconpath, str, sizeof(iconspath)-strlen(iconspath)-1);

	FXIcon* icon = loadPNGIconFromFile(app, iconpath, FXRGB(192,192,192), FXRGB(192,192,192));

	return(icon);
}

FXIcon* makeTriangle(FXApp* app) {
	FXIcon* icon = new FXIcon(app, NULL, FXRGB(192,192,192), IMAGE_OPAQUE, 6, 3);
	icon->create();

	FXPoint points[3];

	FXDCWindow dc(icon);

	dc.setForeground(app->getBaseColor());
	dc.fillRectangle(0,0,icon->getWidth(),icon->getHeight());

	dc.setForeground(app->getForeColor());

	points[0].x = 0;  points[0].y = 0;
	points[1].x = 2;  points[1].y = 3;
	points[2].x = 5;  points[2].y = 0;

	dc.fillPolygon(points, 3);

	icon->restore();
	icon->render();

	return icon;
}

FXIcon* addTriangle(FXApp* app, FXIcon* icon) {
	int width = icon->getWidth();
	int height = icon->getWidth();

	FXIcon* newicon = new FXIcon(app, NULL, 0, IMAGE_OPAQUE, width+7, height);
	newicon->create();

	FXPoint points[3];

	FXDCWindow dc(newicon);

	dc.setForeground(app->getBaseColor());
	dc.fillRectangle(0,0,newicon->getWidth(),newicon->getHeight());

	dc.drawIcon(icon, 0, 0);

	dc.setForeground(app->getForeColor());

	points[0].x = width+2+0;  points[0].y = ((height-2)/2)+0;
	points[1].x = width+2+2;  points[1].y = ((height-2)/2)+3;
	points[2].x = width+2+5;  points[2].y = ((height-2)/2)+0;

	dc.fillPolygon(points, 3);

	icon->restore();
	icon->render();

	return newicon;
}


/*void loadXfeSettings() {
	char xdghome[PATH_MAX];
	char xfeconfig[PATH_MAX];

	if (!xfeloaded) {
		getXdgConfigHome(xdghome, sizeof(xdghome));
		//puts(xdghome);
		sprintf(xfeconfig, "%.*s/xfe/xferc", (int)(sizeof(xfeconfig)-11), xdghome);

		xfesettings.parseFile(xfeconfig, TRUE);

		xfeloaded = 1;
	}
}*/


void getIconPath(char* buf, unsigned int len) {
	const FXchar* iconpath;

	if (!xfeloaded) {
		loadXfeSettings();
	}

	iconpath = xfesettings.readStringEntry("SETTINGS", "iconpath", "/usr/local/share/xfe/icons/windows-theme");
	sprintf(buf, "%.*s", (int)len-1, iconpath);
}

int runCpl(int cpl) {
	switch(cpl) {
		case CPL_ID_APPWIZ:
			system("pkexec synaptic &");
			break;
		case CPL_ID_TIMEDATE:
			system("~/.icewm/programs/control/cpls-bin/timedate/timedate &");
			break;
		case CPL_ID_JOY:
			system("jstest-gtk &");
			break;
		case CPL_ID_FONTS:
			system("yad --font --window-icon font-x-generic --title Fonts --borders=12 --no-buttons --width=500 --height=350 --center &");
			break;
		case CPL_ID_KEYB:
			system("lxinput &");
			break;
		case CPL_ID_INETCPL:
			system("firefox about:preferences &");
			break;
		case CPL_ID_MODEM:
			system("pkexec xterm -e pppconfig &");
			break;
		case CPL_ID_MMSYS:
			system("fxmixer &");
			break;
		case CPL_ID_NCPA:
			system("~/.icewm/programs/control/cpls-bin/ncpa/ncpa &");
			break;
		case CPL_ID_POWERCFG:
			system("batmeter &");
			break;
		case CPL_ID_SYSDM:
			system("sysdmcpl &");
			break;		
		case CPL_ID_DESK:
			system("deskcpl &");
			break;
		case CPL_ID_PRINTERS:
			system("firefox localhost:631 &");
			break;
		default:
			fputs("No action asociated!\n", stderr);
			return 0;
	}
	return 1;
}

long ControlPanel::onCplActivate(FXObject*,FXSelector,void* ptr) {
	switch(shellfolder) {
		case SHF_ID_CONTROL:
			runCpl((int)(FXival)iconlist->getItemData((FXival)ptr));
			break;
		default:
			fputs("Invalid shell folder!\n", stderr);
			return 0;
	}

	return 1;
}

void controlPanelList(FXIconList* icl) {
	icl->appendHeader("Name", NULL, 150);
	icl->appendHeader("Comment", NULL, 300);

	icl->appendItem("Add/Remove\tInstalls and removes programs", ico_appwiz_32, ico_appwiz_16, (void*)CPL_ID_APPWIZ);
	icl->appendItem("Controllers\tAdds, removes, and configures game controller hardware such as joysticks and gamepads", ico_joy_32, ico_joy_16, (void*)CPL_ID_JOY);
	icl->appendItem("Date/Time\tSets the date, time, and time zone for your computer", ico_timedate_32, ico_timedate_16, (void*)CPL_ID_TIMEDATE);
	icl->appendItem("Display\tCustomizes your desktop display and screen saver", ico_desk_32, ico_desk_16, (void*)CPL_ID_DESK);
	icl->appendItem("Fonts\tDisplays and manages fonts on your computer", ico_fonts_32, ico_fonts_16, (void*)CPL_ID_FONTS);
	icl->appendItem("Input\tCustomizes your keyboard and mouse settings", ico_keyb_32, ico_keyb_16, (void*)CPL_ID_KEYB);
	icl->appendItem("Internet\tCustomizes your Internet display and connections settings", ico_inetcpl_32, ico_inetcpl_16, (void*)CPL_ID_INETCPL);
	icl->appendItem("Modems\tConfigures your modem properties", ico_modem_32, ico_modem_16, (void*)CPL_ID_MODEM);
	icl->appendItem("Printers\tAdds, removes, and configures local and network printers", ico_printers_32, ico_printers_16, (void*)CPL_ID_PRINTERS);
	icl->appendItem("Sound\tConfigures sound devices", ico_mmsys_32, ico_mmsys_16, (void*)CPL_ID_MMSYS);
	icl->appendItem("Network Connections\tConnects to other computers, networks, and the Internet", ico_ncpa_32, ico_ncpa_16, (void*)CPL_ID_NCPA);
	icl->appendItem("Power\tConfigures energy-saving settings for your computer", ico_powercfg_32, ico_powercfg_16, (void*)CPL_ID_POWERCFG);
	icl->appendItem("System\tProvides system information and changes environment settings", ico_sysdm_32, ico_sysdm_16, (void*)CPL_ID_SYSDM);
}

#define NCPA_IFTYPE_LAN 1
#define NCPA_IFTYPE_WLAN 2
#define NCPA_IFTYPE_DIAL 3

void ncpaList(FXIconList* icl) {
	struct ifaddrs *addrs, *tmp;
	char netitem[64];
	char nettype[256];

	FXbool ifup = FALSE;
	int iftype = NCPA_IFTYPE_LAN;

	getifaddrs(&addrs);
	tmp = addrs;

	while (tmp) {
	    if (tmp->ifa_addr == NULL) goto next;
		if (tmp->ifa_addr->sa_family != AF_PACKET) goto next;
		
		if (!strcmp(tmp->ifa_name, "lo")) goto next;

		if (tmp->ifa_flags & IFF_UP) {
			ifup = TRUE;
		} else {
			ifup = FALSE;
		}


		if (!strncmp(tmp->ifa_name, "wl", 2)) {
			sprintf(netitem, "%.15s\tWireless", tmp->ifa_name);
		} else {
			sprintf(netitem, "%.15s\tLAN", tmp->ifa_name);
		}
next:
	    tmp = tmp->ifa_next;
	}

	

	freeifaddrs(addrs);
}


ControlPanel::ControlPanel(FXApp *app):FXMainWindow(app, "Control Panel", ico_control, NULL, DECOR_ALL, winx,winy,winw,winh,  0,0,0,0,  0,0) {
	topdock = new FXDockSite(this, FRAME_SUNKEN|DOCKSITE_NO_WRAP|LAYOUT_SIDE_TOP|LAYOUT_FILL_X);
	new FXHorizontalSeparator(this, LAYOUT_SIDE_TOP|LAYOUT_FIX_HEIGHT, 0,0,0,3, 0,0,0,0);

	statusbarcont = new FXPacker(this, JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_SIDE_BOTTOM, 0, 0, 0, 0, 0, 0, 1, 0, 2, 2);
	statuslbl = new FXLabel(statusbarcont, " ", NULL, LAYOUT_FIX_Y|LABEL_NORMAL|FRAME_SUNKEN|LAYOUT_FILL_X|JUSTIFY_LEFT, 0,2,0,0, 1,1,1,1);

	corner = new FXPacker(statusbarcont, LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_SIDE_RIGHT|LAYOUT_BOTTOM, 0, 0, 13, 13, 0, 0, 0, 0, 0, 0);
	realcorner = new FXDragCorner(corner);

	mbshell = new FXToolBarShell(this,FRAME_SUNKEN);


	menubar = new FXMenuBar(topdock,mbshell,LAYOUT_DOCK_SAME|LAYOUT_FILL_X|LAYOUT_SIDE_TOP|LAYOUT_FILL_Y|FRAME_RAISED,0,0,0,0,  2,6,2,2,  4,4);

	throbtoolbar = new FXToolBar(topdock, throbshell, LAYOUT_FILL_Y|LAYOUT_DOCK_SAME|LAYOUT_SIDE_TOP|FRAME_RAISED, 0,0,0,0, 0,0,0,0, 0,0);
	FXPacker* throbcont = new FXPacker(throbtoolbar, LAYOUT_CENTER_Y|LAYOUT_FIX_HEIGHT|LAYOUT_FIX_WIDTH, 0, 0, 38, 22, 0, 0, 0, 0, 0, 0);
	throbcont->setBackColor(FXRGB(0,0,0));
	throbtoolbar->setBackColor(FXRGB(0,0,0));

	new FXLabel(throbcont, "", ico_exp_throbber, LAYOUT_CENTER_X|LAYOUT_FIX_Y|ICON_BELOW_TEXT|LAYOUT_TOP|LAYOUT_LEFT, 0, 0, 0, 0, 0, 0, 0, 0);
 
	new FXToolBarGrip(menubar,menubar,FXMenuBar::ID_TOOLBARGRIP,TOOLBARGRIP_SINGLE, 0,0,0,0, 0,2,0,0);

	filemenu = new FXMenuPane(this);
	editmenu = new FXMenuPane(this);
	viewmenu = new FXMenuPane(this);
	favoritesmenu = new FXMenuPane(this);
	toolsmenu = new FXMenuPane(this);
	helpmenu = new FXMenuPane(this);


	FXMenuCommand* menucmd; FXMenuRadio* menurad; FXMenuCheck* menuchk;
	
	new FXMenuTitle(menubar, "&File", NULL, filemenu);
	new FXMenuCommand(filemenu,"&Open",NULL,this,ID_OPEN);
	new FXMenuSeparator(filemenu);
	menucmd = new FXMenuCommand(filemenu,"Rena&me");
	menucmd->disable();
	menucmd = new FXMenuCommand(filemenu,"&Delete");
	menucmd->disable();
	menucmd = new FXMenuCommand(filemenu,"P&roperties");
	menucmd->disable();
	new FXMenuSeparator(filemenu);
	new FXMenuCommand(filemenu,"&Close\tCtl-Q",NULL,getApp(),FXApp::ID_QUIT);

	menucmd = new FXMenuCommand(helpmenu, "&Help Topics"); menucmd->disable();
	new FXMenuSeparator(helpmenu);
	new FXMenuCommand(helpmenu, "&About", NULL, this, ID_ABOUT);

	new FXMenuTitle(menubar, "&Edit", NULL, editmenu);

	menucmd = new FXMenuCommand(editmenu,"&Undo");
	menucmd->disable();
	new FXMenuSeparator(editmenu);
	menucmd = new FXMenuCommand(editmenu,"Cu&t");
	menucmd->disable();
	menucmd = new FXMenuCommand(editmenu,"&Copy");
	menucmd->disable();
	menucmd = new FXMenuCommand(editmenu,"&Paste");
	menucmd->disable();
	menucmd = new FXMenuCommand(editmenu,"Paste &Shortcut");
	menucmd->disable();
	new FXMenuSeparator(editmenu);
	menucmd = new FXMenuCommand(editmenu,"Copy to &Folder...");
	menucmd->disable();
	menucmd = new FXMenuCommand(editmenu,"Mo&ve to Folder...");
	menucmd->disable();

	new FXMenuSeparator(editmenu);
	menucmd = new FXMenuCommand(editmenu,"Select &All\tCtl-A", NULL, this, ID_SELECTALL);
	menucmd = new FXMenuCommand(editmenu,"&Invert Selection", NULL, this, ID_INVERTSELECT);

	new FXMenuTitle(menubar, "&View", NULL, viewmenu);
	new FXMenuTitle(menubar, "F&avorites", NULL, favoritesmenu);
	menucmd = new FXMenuCommand(favoritesmenu,"&Add to Favorites...");
	menucmd->disable();
	menucmd = new FXMenuCommand(favoritesmenu,"&Organize Favorites...");
	menucmd->disable();
	new FXMenuSeparator(favoritesmenu);
	menucmd = new FXMenuCommand(favoritesmenu,"(Empty)");
	menucmd->disable();

	new FXMenuTitle(menubar, "&Tools", NULL, toolsmenu);
	menucmd = new FXMenuCommand(toolsmenu,"Folder &Options...");
	menucmd->disable();

	new FXMenuTitle(menubar, "&Help", NULL, helpmenu);


	/*menucmd = new FXMenuCommand(viewmenu, "C&ustomize...\t\tCustomizes the view"); menucmd->disable();
	new FXMenuSeparator(viewmenu);*/


	tbshell = new FXToolBarShell(this,FRAME_SUNKEN);

	toolbar = new FXToolBar(topdock,tbshell,LAYOUT_FILL_Y|LAYOUT_FILL_X|LAYOUT_DOCK_NEXT|LAYOUT_SIDE_TOP|FRAME_RAISED,0,0,0,0, 0,0,0,0,  0,0);
	new FXToolBarGrip(toolbar, toolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_SINGLE,0,0,0,0,2,3,2,2);

	adshell = new FXToolBarShell(this,FRAME_SUNKEN);

	adtoolbar = new FXToolBar(topdock,tbshell,LAYOUT_FILL_Y|LAYOUT_FILL_X|LAYOUT_DOCK_NEXT|LAYOUT_SIDE_TOP|FRAME_RAISED,0,0,0,0, 0,0,0,0,  0,0);
	new FXToolBarGrip(adtoolbar, adtoolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_SINGLE,0,0,0,0,2,3,2,2);

	new FXLabel(adtoolbar, "A&ddress",NULL,LAYOUT_CENTER_Y,0,0,0,0,1,4,0,0);
	
	FXHorizontalFrame* addresscont = new FXHorizontalFrame(adtoolbar, FRAME_NORMAL|LAYOUT_FILL_X|LAYOUT_CENTER_Y, 0,0,0,0, 1,0,0,0, 1,0);
	addresscont->setBackColor(app->getBackColor());

	FXLabel* addressicon = new FXLabel(addresscont, "", ico_control, LAYOUT_CENTER_Y,0,0,0,0,0,0,0,0); 
	addressicon->setBackColor(app->getBackColor());


	FXComboBox* address = new FXComboBox(addresscont, 10, NULL, 0, COMBOBOX_INSERT_LAST|JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_CENTER_Y, 0,0,0,0, 2,2,3,1);
	address->setEditable(FALSE);
	address->setText("Control Panel");


	if (xpmode) {
		new FXButton(adtoolbar, "Go", ico_exp_go, NULL, 0, BUTTON_TOOLBAR|ICON_BEFORE_TEXT|FRAME_RAISED|LAYOUT_FILL_Y, 0,0,0,0, 4,14,0,0);
	} else {
		new FXButton(adtoolbar, "Go", ico_exp_go, NULL, 0, BUTTON_TOOLBAR|ICON_BEFORE_TEXT|FRAME_RAISED|LAYOUT_FILL_Y, 0,0,0,0, 2,6,0,0);
	}


	FXButton* btn;


    btn = new FXButton(toolbar, "Back", ico_exp_back, NULL, 0, BUTTON_TOOLBAR|ICON_BEFORE_TEXT|FRAME_RAISED|LAYOUT_FILL_Y,0,0,0,0,5,3,2,2);
	btn->disable();
	btn = new FXButton(toolbar, "", ico_arrow, NULL, 0, LAYOUT_FILL_Y,0,0,0,0, 4,3,2,0);
	btn->disable();
    btn = new FXButton(toolbar, "", ico_exp_forward, NULL, 0, BUTTON_TOOLBAR|ICON_BEFORE_TEXT|FRAME_RAISED|LAYOUT_FILL_Y,0,0,0,0,3,2,2,2);
	btn->disable();
	btn = new FXButton(toolbar, "", ico_arrow, NULL, 0, LAYOUT_FILL_Y,0,0,0,0,4,3,2,0);
	btn->disable();
    btn = new FXButton(toolbar, "", ico_exp_up, this, ID_UP, BUTTON_TOOLBAR|ICON_BEFORE_TEXT|FRAME_RAISED|LAYOUT_FILL_Y,0,0,0,0,3,3,2,2);

	new FXVerticalSeparator(toolbar, SEPARATOR_GROOVE|LAYOUT_FILL_Y, 0, 0, 0, 0, 1, 2, 2, 2);

	btn = new FXButton(toolbar, "Search", ico_exp_search, NULL, 0, BUTTON_TOOLBAR|ICON_BEFORE_TEXT|FRAME_RAISED|LAYOUT_FILL_Y,0,0,0,0,5,3,2,2);
	btn = new FXButton(toolbar, "Folders", ico_exp_folders, NULL, 0, BUTTON_TOOLBAR|ICON_BEFORE_TEXT|FRAME_RAISED|LAYOUT_FILL_Y,0,0,0,0,5,3,2,2);

	new FXVerticalSeparator(toolbar, SEPARATOR_GROOVE|LAYOUT_FILL_Y, 0, 0, 0, 0, 1, 2, 2, 2);

    btn = new FXButton(toolbar, "", ico_exp_copyto, NULL, 0, BUTTON_TOOLBAR|ICON_BEFORE_TEXT|FRAME_RAISED|LAYOUT_FILL_Y,0,0,0,0,3,2,2,2);
	btn->disable();
    btn = new FXButton(toolbar, "", ico_exp_moveto, NULL, 0, BUTTON_TOOLBAR|ICON_BEFORE_TEXT|FRAME_RAISED|LAYOUT_FILL_Y,0,0,0,0,3,2,2,2);
	btn->disable();
    btn = new FXButton(toolbar, "", ico_exp_delete, NULL, 0, BUTTON_TOOLBAR|ICON_BEFORE_TEXT|FRAME_RAISED|LAYOUT_FILL_Y,0,0,0,0,3,2,2,2);
	btn->disable();
    btn = new FXButton(toolbar, "", ico_exp_properties, NULL, 0, BUTTON_TOOLBAR|ICON_BEFORE_TEXT|FRAME_RAISED|LAYOUT_FILL_Y,0,0,0,0,3,2,2,2);
	btn->disable();

	new FXVerticalSeparator(toolbar, SEPARATOR_GROOVE|LAYOUT_FILL_Y, 0, 0, 0, 0, 1, 2, 2, 2);


   	viewbutton =new FXMenuButton(toolbar, "", NULL, viewmenu, LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_BOTTOM|MENUBUTTON_NOARROWS, 0,0,0,0, 0,0,0,0);
	viewbutton->setHiliteColor(viewbutton->getBackColor());
	viewbutton->setShadowColor(viewbutton->getBackColor());
	viewbutton->setBorderColor(viewbutton->getBackColor());
	btn = new FXButton(toolbar, "", ico_exp_views, this, ID_VIEWMENU, BUTTON_TOOLBAR|ICON_BEFORE_TEXT|FRAME_RAISED|LAYOUT_FILL_Y,0,0,0,0,3,2,2,2);
	//viewbutton->hide();


	iconlistframe = new FXPacker(this, LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_NORMAL, 0,0,0,0, 0,0,0,0);

	iconlist = new FXIconList(iconlistframe, this, ID_CPL, LAYOUT_FILL_X|LAYOUT_FILL_Y|
			ICONLIST_EXTENDEDSELECT|ICONLIST_BIG_ICONS|ICONLIST_COLUMNS);


	new FXMenuRadio(viewmenu,"&Details",iconlist,FXIconList::ID_SHOW_DETAILS);
	new FXMenuRadio(viewmenu,"&Small Icons",iconlist,FXIconList::ID_SHOW_MINI_ICONS);
	new FXMenuRadio(viewmenu,"&Big Icons",iconlist,FXIconList::ID_SHOW_BIG_ICONS);
	new FXMenuSeparator(viewmenu);
	new FXMenuRadio(viewmenu,"&Rows",iconlist,FXIconList::ID_ARRANGE_BY_ROWS);
	new FXMenuRadio(viewmenu,"&Columns",iconlist,FXIconList::ID_ARRANGE_BY_COLUMNS);

	iconlist->setItemSpace(76);

	switch(shellfolder) {
		case SHF_ID_CONTROL:
			controlPanelList(iconlist);
			break;
		default:
			fputs("Invalid shell folder!\n", stderr);
	}
}

int main(int argc, char *argv[]) {
	char iconpath[PATH_MAX];
	FXApp application("Control", "Ice2KProj");
	application.init(argc, argv);

	FXApp* app = &application;

	loadXfeSettings();

	if (xfeloaded) {
		winx = xfesettings.readIntEntry("OPTIONS", "xpos", 64);
		winy = xfesettings.readIntEntry("OPTIONS", "ypos", 64);
		winw = xfesettings.readIntEntry("OPTIONS", "width", 520);
		winh = xfesettings.readIntEntry("OPTIONS", "height", 380);
	}

	winx += 4;
	winy += 23;

	getIconPath(iconspath, sizeof(iconpath));

	ico_exp_back = loadToolbarIcon(app, "dirback.png");
	ico_exp_forward = loadToolbarIcon(app, "dirforward.png");
	ico_exp_up = loadToolbarIcon(app, "dirup.png");
	
	ico_exp_search = loadToolbarIcon(app, "search.png");
	ico_exp_folders = loadToolbarIcon(app, "treeonepanel.png");

	ico_exp_copyto = loadToolbarIcon(app, "copy_clp.png");
	ico_exp_moveto = loadToolbarIcon(app, "moveit.png");
	ico_exp_delete = loadToolbarIcon(app, "filedelete.png");
	ico_exp_properties = loadToolbarIcon(app, "attrib.png");

	ico_exp_go = loadToolbarIcon(app, "enter.png");

	ico_exp_throbber = loadToolbarIcon(app, "throbanim.png");

	ico_arrow = makeTriangle(app);

	char *winver = strdup(i2kBGetWinVersion());

	if (!strcmp(winver, "xp")) {
		xpmode = 1;

		ico_appwiz_16 = loadPNGIcon(app, resico_xp_appwiz_16);
		ico_appwiz_32 = loadPNGIcon(app, resico_xp_appwiz_32);
		ico_desk_16 = loadPNGIcon(app, resico_xp_desk_16);
		ico_desk_32 = loadPNGIcon(app, resico_xp_desk_32);
		ico_fonts_16 = loadPNGIcon(app, resico_xp_fonts_16);
		ico_fonts_32 = loadPNGIcon(app, resico_xp_fonts_32);
		ico_inetcpl_16 = loadPNGIcon(app, resico_xp_inetcpl_16);
		ico_inetcpl_32 = loadPNGIcon(app, resico_xp_inetcpl_32);
		ico_joy_16 = loadPNGIcon(app, resico_xp_joy_16);
		ico_joy_32 = loadPNGIcon(app, resico_xp_joy_32);
		ico_keyb_16 = loadPNGIcon(app, resico_xp_keyb_16);
		ico_keyb_32 = loadPNGIcon(app, resico_xp_keyb_32);
		ico_mice_16 = loadPNGIcon(app, resico_xp_mice_16);
		ico_mice_32 = loadPNGIcon(app, resico_xp_mice_32);
		ico_mmsys_16 = loadPNGIcon(app, resico_xp_mmsys_16);
		ico_mmsys_32 = loadPNGIcon(app, resico_xp_mmsys_32);
		ico_modem_16 = loadPNGIcon(app, resico_xp_modem_16);
		ico_modem_32 = loadPNGIcon(app, resico_xp_modem_32);
		ico_ncpa_16 = loadPNGIcon(app, resico_xp_ncpa_16);
		ico_ncpa_32 = loadPNGIcon(app, resico_xp_ncpa_32);
		ico_powercfg_16 = loadPNGIcon(app, resico_xp_powercfg_16);
		ico_powercfg_32 = loadPNGIcon(app, resico_xp_powercfg_32);
		ico_timedate_16 = loadPNGIcon(app, resico_xp_timedate_16);
		ico_timedate_32 = loadPNGIcon(app, resico_xp_timedate_32);
		ico_sysdm_16 = loadPNGIcon(app, resico_xp_sysdm_16);
		ico_sysdm_32 = loadPNGIcon(app, resico_xp_sysdm_32);
		ico_printers_16 = loadPNGIcon(app, resico_xp_printers_16);
		ico_printers_32 = loadPNGIcon(app, resico_xp_printers_32);

		ico_control = loadPNGIcon(app, resico_xp_control);

		FXIcon* ico_exp_views2 = loadToolbarIcon(app, "bigicons.png");
		ico_exp_views = addTriangle(app, ico_exp_views2);

		delete ico_exp_views2;
	} else {
		xpmode = 0;

		ico_appwiz_16 = loadPNGIcon(app, resico_2k_appwiz_16);
		ico_appwiz_32 = loadPNGIcon(app, resico_2k_appwiz_32);
		ico_desk_16 = loadPNGIcon(app, resico_2k_desk_16);
		ico_desk_32 = loadPNGIcon(app, resico_2k_desk_32);
		ico_fonts_16 = loadPNGIcon(app, resico_2k_fonts_16);
		ico_fonts_32 = loadPNGIcon(app, resico_2k_fonts_32);
		ico_inetcpl_16 = loadPNGIcon(app, resico_2k_inetcpl_16);
		ico_inetcpl_32 = loadPNGIcon(app, resico_2k_inetcpl_32);
		ico_joy_16 = loadPNGIcon(app, resico_2k_joy_16);
		ico_joy_32 = loadPNGIcon(app, resico_2k_joy_32);
		ico_keyb_16 = loadPNGIcon(app, resico_2k_keyb_16);
		ico_keyb_32 = loadPNGIcon(app, resico_2k_keyb_32);
		ico_mice_16 = loadPNGIcon(app, resico_2k_mice_16);
		ico_mice_32 = loadPNGIcon(app, resico_2k_mice_32);
		ico_mmsys_16 = loadPNGIcon(app, resico_2k_mmsys_16);
		ico_mmsys_32 = loadPNGIcon(app, resico_2k_mmsys_32);
		ico_modem_16 = loadPNGIcon(app, resico_2k_modem_16);
		ico_modem_32 = loadPNGIcon(app, resico_2k_modem_32);
		ico_ncpa_16 = loadPNGIcon(app, resico_2k_ncpa_16);
		ico_ncpa_32 = loadPNGIcon(app, resico_2k_ncpa_32);
		ico_powercfg_16 = loadPNGIcon(app, resico_2k_powercfg_16);
		ico_powercfg_32 = loadPNGIcon(app, resico_2k_powercfg_32);
		ico_timedate_16 = loadPNGIcon(app, resico_2k_timedate_16);
		ico_timedate_32 = loadPNGIcon(app, resico_2k_timedate_32);
		ico_sysdm_16 = loadPNGIcon(app, resico_2k_sysdm_16);
		ico_sysdm_32 = loadPNGIcon(app, resico_2k_sysdm_32);
		ico_printers_16 = loadPNGIcon(app, resico_2k_printers_16);
		ico_printers_32 = loadPNGIcon(app, resico_2k_printers_32);
	
		ico_control = loadPNGIcon(app, resico_2k_control);

		FXIcon* ico_exp_views2 = loadToolbarIcon(app, "smallicons.png");
		ico_exp_views = addTriangle(app, ico_exp_views2);

		delete ico_exp_views2;
	}

	controlwin = new ControlPanel(app);
	application.create();

	return application.run();
}
