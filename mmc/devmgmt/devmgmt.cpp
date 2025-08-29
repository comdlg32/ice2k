#include <fx.h>
#include <ice2k/comctl32.h>
//#include <ice2k/branding.h>
#include "res/foxres.h"
#include <dirent.h>

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/XTest.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>



#include <stdio.h>
#include <string.h>
#include <cpuid.h>
#include <sys/sysinfo.h>
#include <stdint.h>
#include <locale.h>
#include <unistd.h>
#include <limits.h>

#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <errno.h>


#include <sys/types.h>
#include <pwd.h>
#include <ctype.h>
#include <fcntl.h>

#include <libserialport.h>

extern "C" {
#include <pci/pci.h>
}

FXApp* app;
FXLabel* statuslbl;

#define _SYSFS_BLOCK "/sys/block"

FXMainWindow* devmgmtwin;
FXIcon* ico_devmgmt;


// thank you https://en.wikipedia.org/wiki/CPUID#EAX=8000'0002h,8000'0003h,8000'0004h:_Processor_Brand_String
/* int getCpuString(char* output) {
  #ifdef __x86_64__
    #define _CPUID
  #endif

  #ifdef __i386__
    #define _CPUID
  #endif

  #ifdef _CPUID
    unsigned int regs[12];      // i know i can use the linux apis for this...
                                // but that's boring and i want to have fun :P
    __cpuid(0x80000000, regs[0], regs[1], regs[2], regs[3]);

    if (regs[0] < 0x80000004)
      return 1;

    __cpuid(0x80000002, regs[0], regs[1], regs[2], regs[3]);
    __cpuid(0x80000003, regs[4], regs[5], regs[6], regs[7]);
    __cpuid(0x80000004, regs[8], regs[9], regs[10], regs[11]);

    memcpy(output, regs, sizeof(regs));
    output[sizeof(regs)] = '\0';

    return 0;
  #else
    return 1;
  #endif
} */ // might get useful at one point

void formatnum(long unsigned num, char *buffer) { // from ice2kver
  char temp[1024];
  sprintf(temp, "%lu", num / 1024);

  int len = strlen(temp);
  int commas = (len - 1) / 3;
  int new_len = len + commas;

  buffer[new_len] = '\0';
  int i = len - 1, j = new_len - 1;

  int count = 0;
  while (i >= 0) {
    if (count == 3) {
      buffer[j--] = ',';
      count = 0;
    }
    buffer[j--] = temp[i--];
    count++;
  }
}



/* int checkAcpiSupport() {
	DIR* dir;
	dir = opendir("/sys/class/acpi");

	if (dir) {
		closedir(dir);
		dir = opendir("/proc/acpi");

		if (dir) {
			closedir(dir);
			dir = opendir("/sys/firmware/acpi");

			if (dir) {
				closedir(dir);
				return 0;
			}
		}
	}

	return 1;
} */


int checkAcpiSupport() {
	int i = 0;
	DIR* dir;

	acpicheck:

	switch(i) {
		case 0:
			dir = opendir("/sys/module/acpi");
			break;

		case 1:
			dir = opendir("/proc/acpi");
			break;

		case 2:
			dir = opendir("/sys/firmware/acpi");
			break;

		default:
			return 0;
	}

	closedir(dir);

	if (!dir) {
		return 1;
	}

	i++;

	goto acpicheck;
}

int checkAmd64() {
	// this ones easy...
	
	#ifdef __x86_64__
		//puts("System is 64 bit");
		return 0;
	#else
		return 1;
	#endif
}

int getDrives(char *buf, int bufsize, const char drivebeg[], const char drivebeg2[] = NULL) {
	DIR *dir = opendir(_SYSFS_BLOCK);
	struct dirent *entry;
	//size_t len = 0;
	int len = 0;

	//puts(drivebeg);

	//printf("%d", strlen(drivebeg));

	//puts(drivebeg2);

	if (!dir) {
		perror("opendir");
		if (bufsize > 0) buf[0] = '\0';
		return 1;
	}

	buf[0] = '\0';

	while ((entry = readdir(dir)) != NULL) {
		if  (strncmp(entry->d_name, drivebeg, strlen(drivebeg)) != 0 &&
               (drivebeg2 == NULL || strncmp(entry->d_name, drivebeg2, strlen(drivebeg2)) != 0))
			continue;

		char full_path[255 + 12];
		snprintf(full_path, sizeof(full_path), "%s/%s", _SYSFS_BLOCK, entry->d_name);

		//printf("%s", full_path);

		//size_t entry_len = strlen(entry->d_name);
		int entry_len = strlen(entry->d_name);

		if (len > 0) {
			if (len + 1 < bufsize) {
				buf[len] = ',';
				len++;
				buf[len] = '\0';
			} else {
				break;
			}
		}

		if (len + entry_len < bufsize) {
			strcat(buf, entry->d_name);
			len += entry_len;
		} else {
			break;
		}
	}

	closedir(dir);

	return 0;
}

// taken from ice2k batmeter
int getHardDrives(char *buf, int bufsize) {
	int ret = getDrives(buf, bufsize, "sd", "hd");
	return ret;
}

int getFloppyDrives(char *buf, int bufsize) {
	int ret = getDrives(buf, bufsize, "fd");
	return ret;
}

int getCdDrives(char *buf, int bufsize) {
	int ret = getDrives(buf, bufsize, "sr", "scd");
	return ret;
}


// taken from ice2k batmeter
int getHddInfo(const char* hardDrive, const char* info, char* buf, int bufsize ) {
	char hddpath[64];
	int fd;
	int len = 0;

	snprintf(hddpath, sizeof(hddpath), "%s/%s/%s", _SYSFS_BLOCK, hardDrive, info);

	fd = open(hddpath, O_RDONLY);

	if (fd < 0) {
		return 1;
	}

	len = read(fd, buf, bufsize - 1);
	if (len < 0) {
		close(fd);
		return 1;
	}

	buf[len] = '\0';

	for (int i = 0; buf[i] != '\0'; ++i) {
		if (buf[i] == '\n') {
			buf[i] = '\0';
			break;
		}
	}

	close(fd);
	return 0;
}

// i want to thank chatgpt for xrandr docs
// theres virtually no documentation for xrandr's api aside from the official ones

int getMonitors(char* buf, int bufsize ) {
//int getMonitors() {
	Display* dpy = (Display*)app->getDisplay();
	Window root = DefaultRootWindow(dpy);

	int eventBase, errorBase;
	XRRScreenResources* resources;

	if (!XRRQueryExtension(dpy, &eventBase, &errorBase))
		return 1;

	resources = XRRGetScreenResources(dpy, root);

	if (!resources)
		return 1;

	int y = 0;

	for (int i = 0; i < resources->noutput; i++) {
		XRROutputInfo *outputInfo = XRRGetOutputInfo(dpy, resources, resources->outputs[i]);
		if (outputInfo->connection == RR_Connected) {
			if (y) {
				strcat(buf, ",");
				strncat(buf, outputInfo->name, bufsize);
			} else {
				strcpy(buf, outputInfo->name);
			}
			y = 1;
		}
	}

	return 0;
}


// Main Window
class DeviceManager : public FXMainWindow {

  // Macro for class hierarchy declarations
  FXDECLARE(DeviceManager)

private:
  FXVerticalFrame*          generalframe;
  FXVerticalFrame*          networkframe;
  FXVerticalFrame*          hardwareframe;
  FXVerticalFrame*          userframe;
  FXVerticalFrame*          advframe;

  FXHorizontalFrame*        btncont;
  FXTabBook*                tabbook;
  FXButton*                 okbtn;
  FXButton*                 cancelbtn;
  FXButton*                 applybtn;

  FXHorizontalFrame*        horcont;
  FXVerticalFrame*          vercont;



protected:
  DeviceManager(){}

public:

  // Message handlers
  long onItemChange(FXObject*,FXSelector,void*);
  long onStatus(FXObject*,FXSelector,void*);
  long onChangeText(FXObject*,FXSelector,void*);


public:

  // Messages for our class
  enum {
    ID_MAINWIN=FXMainWindow::ID_LAST,
    ID_TREE,
  };

public:

  // CtrlAltDelWindow's constructor
  DeviceManager(FXApp* a);

  // Initialize
  virtual void create();

  virtual ~DeviceManager();
};

FXDEFMAP(DeviceManager) DeviceManagerMap[] = {
  FXMAPFUNC(SEL_UPDATE, 0, DeviceManager::onStatus),
  FXMAPFUNC(SEL_COMMAND, DeviceManager::ID_SETSTRINGVALUE, DeviceManager::onChangeText),

  FXMAPFUNC(SEL_CHANGED, DeviceManager::ID_TREE, DeviceManager::onItemChange),
};

FXIMPLEMENT(DeviceManager,FXMainWindow,DeviceManagerMap,ARRAYNUMBER(DeviceManagerMap))



DeviceManager::~DeviceManager() {
}
// from FXStatusLine src
long DeviceManager::onStatus(FXObject* sender, FXSelector sel, void* ptr) {
  //puts((char*)ptr);
  FXWindow *helpsource=getApp()->getCursorWindow();
  //FXFrame::onUpdate(sender,sel,ptr);
  if (helpsource && getShell()->isOwnerOf(helpsource) && helpsource->handle(this,FXSEL(SEL_QUERY_HELP,0),NULL)) {
    //FXString* helpstr = ((FXString*)ptr);
    //printf("%s\n", helpstr->text());
    return 1;
    
    //puts(helpstr->text());
  }

  statuslbl->setText(" ");
  //puts("a");
  return 1;
}

long DeviceManager::onChangeText(FXObject* sender, FXSelector sel, void* ptr) {
  //const FXString* helpstr = (*((FXString*)ptr));
  statuslbl->setText(*((FXString*)ptr));
  
  return 1;
}

FXButton* propbtn;
FXVerticalSeparator* propsep;


long DeviceManager::onItemChange(FXObject* sender, FXSelector sel, void* ptr) {
  FXTreeList* tree = (FXTreeList*)sender;

  FXTreeItem* currentitem = tree->getCurrentItem();
  FXTreeItem* root = tree->getFirstItem();

  if (root == currentitem) {
    propbtn->hide();
    propsep->hide();
  } else {
    propbtn->show();
    propsep->show();
  }

  propbtn->getParent()->recalc();
  
  return 1;
}

void DeviceManager::create() {
  FXMainWindow::create();
}

void getComputerName(char* computerType, int size) {
    // asked chatgpt to clean it up, did it's job well. was a bunch of nested ifs before
    int acpiSupport = checkAcpiSupport();
    int amd64Cpu = checkAmd64();
    int multiProcessor = 0;

    if (get_nprocs_conf() > 1)
      multiProcessor = 1;

    const char* arch = amd64Cpu ? "" : " x64-based";
    const char* biosType;

    if (!acpiSupport) {
      biosType = multiProcessor ? "ACPI Multiprocessor"
                                : "Advanced Configuration and Power Interface (ACPI)";
    } else {
      biosType = multiProcessor ? "MPS Multiprocessor"
                                : "Standard";
    }

    snprintf(computerType, size, "%s%s PC", biosType, arch);
}


//int main(int argc, char *argv[]) {
DeviceManager::DeviceManager(FXApp *app):FXMainWindow(app, "Device Manager", ico_devmgmt, NULL, DECOR_ALL, 0,0,520,380,  0,0,0,0,  0,0) {
  FXDockSite* topdock = new FXDockSite(this, FRAME_SUNKEN|DOCKSITE_NO_WRAP|LAYOUT_SIDE_TOP|LAYOUT_FILL_X);

  FXPacker* statusbarcont = new FXHorizontalFrame(this, JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_SIDE_BOTTOM, 0, 0, 0, 0, 0, 1, 2, 0, 2, 2);
  statuslbl = new FXLabel(statusbarcont, " ", NULL, LABEL_NORMAL|FRAME_SUNKEN|LAYOUT_FILL_X|JUSTIFY_LEFT, 0,0,0,0, 1,1,1,1);
  new FXFrame(statusbarcont, FRAME_SUNKEN|LAYOUT_FIX_WIDTH, 0,0,131,0, 0,0,0,0);
  new FXFrame(statusbarcont, FRAME_SUNKEN|LAYOUT_FIX_WIDTH, 0,0, 83,0, 0,0,0,0);

  FXToolBarShell* mbshell = new FXToolBarShell(this,FRAME_SUNKEN);


  FXMenuBar* menubar = new FXMenuBar(topdock,mbshell,LAYOUT_DOCK_SAME|LAYOUT_SIDE_TOP|LAYOUT_FILL_Y|FRAME_RAISED,0,0,0,0,  2,6,2,2,  4,4);
  new FXToolBarGrip(menubar,menubar,FXMenuBar::ID_TOOLBARGRIP,TOOLBARGRIP_SINGLE, 0,0,0,0, 0,2,0,0);

  FXMenuPane* actionmenu = new FXMenuPane(this);
  FXMenuPane* viewmenu = new FXMenuPane(this);

  FXMenuCommand* menucmd;

  new FXMenuTitle(menubar, "&Action", NULL, actionmenu);
  menucmd = new FXMenuCommand(actionmenu, "&Help"); menucmd->disable();

  new FXMenuTitle(menubar, "&View", NULL, viewmenu);

  menucmd = new FXMenuCommand(viewmenu, "D&evices by type\t\tDisplays devices by hardware type."); menucmd->disable();
  menucmd = new FXMenuCommand(viewmenu, "De&vices by connection\t\tDisplays devices by connection."); menucmd->disable();
  menucmd = new FXMenuCommand(viewmenu, "Resources by t&ype\t\tDisplays resources by type."); menucmd->disable();
  menucmd = new FXMenuCommand(viewmenu, "Resources by co&nnection\t\tDisplays resources by connection"); menucmd->disable();

  new FXMenuSeparator(viewmenu);

  menucmd = new FXMenuCommand(viewmenu, "Sho&w hidden devices\t\tDisplays legacy devices and devices that are no longer installed."); menucmd->disable();

  new FXMenuSeparator(viewmenu);
  menucmd = new FXMenuCommand(viewmenu, "&Print\t\tPrints a report of the devices that are installed."); menucmd->disable();
  new FXMenuSeparator(viewmenu);
  menucmd = new FXMenuCommand(viewmenu, "C&ustomize...\t\tCustomizes the view"); menucmd->disable();

  // Toolbar
  FXToolBarShell* tbshell = new FXToolBarShell(this,FRAME_SUNKEN);

  FXToolBar* toolbar = new FXToolBar(topdock,tbshell,LAYOUT_FILL_Y|LAYOUT_DOCK_SAME|LAYOUT_SIDE_TOP|FRAME_RAISED,0,0,0,0, 0,5,0,0,  1,1);
  new FXToolBarGrip(toolbar, toolbar, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_SINGLE,0,0,0,0,2,3,2,2);

  FXIcon* ico_back = new FXGIFIcon(app, resico_mmc_back);
  FXIcon* ico_forward = new FXGIFIcon(app, resico_mmc_forward);

  FXIcon* ico_up = new FXGIFIcon(app, resico_mmc_up);
  FXIcon* ico_contree = new FXGIFIcon(app, resico_mmc_contree);

  FXIcon* ico_properties = new FXGIFIcon(app, resico_mmc_properties);

  FXIcon* ico_help = new FXGIFIcon(app, resico_mmc_help);

  FXIcon* ico_scan = new FXGIFIcon(app, resico_dmg_scan);


  //FXIcon* ico_hist_up = new FXGIFIcon(app, resico_hist_up);
  FXButton* btn;

  btn = new FXButton(toolbar,"\tBack",ico_back,NULL,0,BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0,  2,2,2,2);
  btn->disable();
  btn = new FXButton(toolbar,"\tForward",ico_forward,NULL,0,BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0,  2,2,2,2);
  btn->disable();
  new FXVerticalSeparator(toolbar, SEPARATOR_GROOVE|LAYOUT_FILL_Y, 0,0,0,0,  3,2,2,2);
  btn = new FXButton(toolbar,"\tUp one level",ico_up,NULL,0,BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0,  2,2,2,2);
  btn->disable();
  btn = new FXButton(toolbar,"\tShow/Hide Console Tree/Favorites",ico_contree,NULL,0,BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0,  2,2,2,2);
  new FXVerticalSeparator(toolbar, SEPARATOR_GROOVE|LAYOUT_FILL_Y, 0,0,0,0,  3,2,2,2);

  propbtn = new FXButton(toolbar,"\tProperties",ico_properties,NULL,0,BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0,  2,2,2,2); propbtn->hide();
  propsep = new FXVerticalSeparator(toolbar, SEPARATOR_GROOVE|LAYOUT_FILL_Y, 0,0,0,0,  3,2,2,2); propsep->hide();

  btn = new FXButton(toolbar,"\tHelp",ico_help,NULL,0,BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0,  2,2,2,2);

  FXToolBarShell* tb2shell = new FXToolBarShell(this,FRAME_SUNKEN);

  FXToolBar* toolbar2 = new FXToolBar(topdock,tb2shell,LAYOUT_FILL_Y|LAYOUT_DOCK_SAME|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|FRAME_RAISED,0,0,0,0, 0,0,0,0,  1,1);
  new FXToolBarGrip(toolbar2, toolbar2, FXToolBar::ID_TOOLBARGRIP, TOOLBARGRIP_SINGLE,0,0,0,0,2,3,2,2);

  btn = new FXButton(toolbar2,"\tScan for hardware changes",ico_scan,NULL,0,BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0,  2,2,2,2);

  new FXSeparator(this, SEPARATOR_NONE|LAYOUT_FIX_HEIGHT, 0,0,0,2); // semantics r cute  

  FXIcon* ico_dev_computer = new FXGIFIcon(app, resico_dev_computer);
  FXIcon* ico_dev_cdrom = new FXGIFIcon(app, resico_dev_cdrom);
  FXIcon* ico_dev_disk = new FXGIFIcon(app, resico_dev_disk);
  FXIcon* ico_dev_disp = new FXGIFIcon(app, resico_dev_disp);
  FXIcon* ico_dev_ide = new FXGIFIcon(app, resico_dev_ide);
  FXIcon* ico_dev_floppy = new FXGIFIcon(app, resico_dev_floppy);
  FXIcon* ico_dev_mice = new FXGIFIcon(app, resico_dev_mice);
  FXIcon* ico_dev_keyb = new FXGIFIcon(app, resico_dev_keyb);
  FXIcon* ico_dev_network = new FXGIFIcon(app, resico_dev_network);
  FXIcon* ico_dev_unknown = new FXGIFIcon(app, resico_dev_unknown);
  FXIcon* ico_dev_serial = new FXGIFIcon(app, resico_dev_serial);
  FXIcon* ico_dev_sound = new FXGIFIcon(app, resico_dev_sound);
  FXIcon* ico_dev_usb = new FXGIFIcon(app, resico_dev_usb);
  FXIcon* ico_dev_firewire = new FXGIFIcon(app, resico_dev_firewire);

  FXPacker* treeframe = new FXPacker(this, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_Y|LAYOUT_FILL_X, 0,0,0,0,  0,0,0,0);
  FXTreeList* tree = new FXTreeList(treeframe,this,ID_TREE,SCROLLERS_DONT_TRACK|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y|TREELIST_SHOWS_BOXES|TREELIST_SHOWS_LINES|TREELIST_BROWSESELECT|TREELIST_ROOT_BOXES);

  FXTreeItem *branch, *top;

  char hostname[HOST_NAME_MAX+1];
  gethostname(hostname, HOST_NAME_MAX+1);

  char *upper = hostname;

  while (*upper) {
    *upper = toupper((unsigned char) *upper);
    upper++;
  }

  // pci related code is based off here
  // thank you https://josuedhg.wordpress.com/2014/11/15/how-to-list-pci-devices-with-c-on-linux/

  struct pci_access* pciaccess;
  struct pci_dev* dev;
  char namebuf[1024];

  pciaccess = pci_alloc();
  pci_init(pciaccess);
  pci_scan_bus(pciaccess);

  top = tree->appendItem(0,hostname,ico_devmgmt,ico_devmgmt);
  tree->expandTree(top);
    char computerType[64];
    getComputerName(computerType, sizeof(computerType));

    branch = tree->appendItem(top,"Computer",ico_dev_computer,ico_dev_computer);    
      tree->appendItem(branch,computerType,ico_dev_computer,ico_dev_computer);


    char hardDrives[256];
    getHardDrives(hardDrives, sizeof(hardDrives));

    char* hardDrive = strtok(hardDrives, ",");

    char hardDriveModel[256];

    //puts(hardDrives);

    branch = tree->appendItem(top,"Disk drives",ico_dev_disk,ico_dev_disk);
      while (hardDrive) {
        getHddInfo(hardDrive, "device/model", hardDriveModel, sizeof(hardDriveModel));
        tree->appendItem(branch,hardDriveModel,ico_dev_disk,ico_dev_disk);
        hardDrive = strtok(NULL, ",");
      }


    FXTreeItem* devVga = tree->appendItem(top,"Display adapters",ico_dev_disp,ico_dev_disp);
    FXTreeItem* devCdRom = tree->appendItem(top,"DVD/CD-ROM drives",ico_dev_cdrom,ico_dev_cdrom);
    FXTreeItem* devFloppyCon = tree->appendItem(top,"Floppy disk controllers",ico_dev_ide,ico_dev_ide);
    FXTreeItem* devFloppyDrive = tree->appendItem(top,"Floppy disk drives",ico_dev_floppy,ico_dev_floppy);
    FXTreeItem* devStorage = tree->appendItem(top,"IDE ATA/ATAPI controllers",ico_dev_ide,ico_dev_ide);
    FXTreeItem* devFirewire = tree->appendItem(top,"IEEE 1394 Bus host controllers",ico_dev_firewire,ico_dev_firewire);
    FXTreeItem* devKeyboards = tree->appendItem(top,"Keyboards",ico_dev_keyb,ico_dev_keyb);
    FXTreeItem* devMice = tree->appendItem(top,"Mice and other pointing devices",ico_dev_mice,ico_dev_mice);
    FXTreeItem* devMonitors = tree->appendItem(top,"Monitors",ico_dev_disp,ico_dev_disp);
    FXTreeItem* devNetwork = tree->appendItem(top,"Network adapters",ico_dev_network,ico_dev_network);
    FXTreeItem* devUnknown = tree->appendItem(top,"Other devices",ico_dev_unknown,ico_dev_unknown);

    FXTreeItem* devSerial = tree->appendItem(top,"Ports (COM & LPT)",ico_dev_serial,ico_dev_serial);
    FXTreeItem* devSound = tree->appendItem(top,"Sound, video and game controllers",ico_dev_sound,ico_dev_sound);
    //branch = tree->appendItem(top,"Storage volumes",ico_dev_disk,ico_dev_disk);

    FXTreeItem* devSystem = tree->appendItem(top,"System devices",ico_dev_computer,ico_dev_computer);
    FXTreeItem* devUsbCon = tree->appendItem(top,"Universal Serial Bus controllers",ico_dev_usb,ico_dev_usb);    

    char* cdDrives = hardDrives; // we save space, why not?
    getCdDrives(cdDrives, sizeof(cdDrives));

    char* cdDrive = strtok(cdDrives, ",");
    char cdDriveModel[256];

    while (cdDrive) {
      getHddInfo(cdDrive, "device/model", cdDriveModel, sizeof(cdDriveModel));
      tree->appendItem(devCdRom,cdDriveModel,ico_dev_cdrom,ico_dev_cdrom);
      cdDrive = strtok(NULL, ",");
    }

    char* floppyDrives = hardDrives; // we save space, why not?
    getFloppyDrives(floppyDrives, sizeof(floppyDrives));

    char* floppyDrive = strtok(floppyDrives, ",");
    char floppyDriveModel[256];

    while (floppyDrive) {
      getHddInfo(floppyDrive, "device/model", floppyDriveModel, sizeof(floppyDriveModel));
      tree->appendItem(devFloppyDrive,floppyDriveModel,ico_dev_floppy,ico_dev_floppy);
      floppyDrive = strtok(NULL, ",");
    }

      //tree->appendItem(branch,computerType,ico_dev_computer,ico_dev_computer);

    char monitors[512];
    getMonitors(monitors, sizeof(monitors));
    if (!getMonitors(monitors, sizeof(monitors))) {
	char* monitortok = strtok(monitors, ",");

      while (monitortok) {
        tree->appendItem(devMonitors,monitortok,ico_dev_disp,ico_dev_disp);
        monitortok = strtok(NULL, ",");
      }
    } else { 
      tree->appendItem(devMonitors,"Default Monitor",ico_dev_disp,ico_dev_disp);

    }
    //puts(monitors);

    tree->appendItem(devSystem, "Black Mesa Mark IV Hazardous Environment Suit", ico_dev_computer, ico_dev_computer);

    for (dev = pciaccess->devices; dev; dev = dev->next) {
      pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS);
      pci_lookup_name(pciaccess, namebuf, sizeof(namebuf), PCI_LOOKUP_VENDOR|PCI_LOOKUP_DEVICE, dev->vendor_id, dev->device_id);

      if ((dev->device_class >> 8) == PCI_BASE_CLASS_NETWORK) {
        //printf("%s\n", namebuf);
        tree->appendItem(devNetwork, namebuf, ico_dev_network, ico_dev_network);
	} else if (dev->device_class == PCI_CLASS_STORAGE_FLOPPY) {
        tree->appendItem(devFloppyCon, namebuf, ico_dev_ide, ico_dev_ide);
	} else if ((dev->device_class >> 8) == PCI_BASE_CLASS_STORAGE) {
        tree->appendItem(devStorage, namebuf, ico_dev_ide, ico_dev_ide);
	} else if ((dev->device_class >> 8) == PCI_BASE_CLASS_MULTIMEDIA) {
        tree->appendItem(devSound, namebuf, ico_dev_sound, ico_dev_sound);
	//} else if ((dev->device_class >> 8) == PCI_BASE_CLASS_BRIDGE) {
      //  tree->appendItem(devSystem, namebuf, ico_dev_computer, ico_dev_computer);
	} else if ((dev->device_class >> 8) == PCI_CLASS_OTHERS) {
        tree->appendItem(devUnknown, namebuf, ico_dev_unknown, ico_dev_unknown);
	} else if ((dev->device_class >> 8) == PCI_BASE_CLASS_DISPLAY) {
        tree->appendItem(devVga, namebuf, ico_dev_disp, ico_dev_disp);
	} else if (dev->device_class == PCI_CLASS_SERIAL_USB) {
        tree->appendItem(devUsbCon, namebuf, ico_dev_usb, ico_dev_usb);
	} else if (dev->device_class == PCI_CLASS_SERIAL_FIREWIRE) {
        tree->appendItem(devFirewire, namebuf, ico_dev_firewire, ico_dev_firewire);
	} else {
        tree->appendItem(devSystem, namebuf, ico_dev_computer, ico_dev_computer);
	}
    }

    int inputDevices, inputDevice=0;
    XDeviceInfo *devList, *curDev;

    Display *dpy = (Display*)app->getDisplay();

    // is xinput2 supported?
    int major = 2; int minor = 0;
    int opcode, event, error;

    int xinput2 = 0;


    if (XQueryExtension(dpy, "XInputExtension", &opcode, &event, &error)) {
      if (XIQueryVersion(dpy, &major, &minor) == Success) {
        if (major >=2)
            xinput2 = 1;
      }
    }

    //printf("%d\n", xinput2);

    devList = XListInputDevices(dpy, &inputDevices);
    if (xinput2) {
      if (devList) {
        while (inputDevice < inputDevices) {
          curDev = devList + inputDevice;
          if (!(curDev->use == XIMasterPointer || curDev->use == XISlavePointer))
            tree->appendItem(devMice, curDev->name, ico_dev_mice, ico_dev_mice);
          else
            tree->appendItem(devKeyboards, curDev->name, ico_dev_keyb, ico_dev_keyb);

          inputDevice++;
        }
      }
    } else {
      tree->appendItem(devMice, "Microsoft PS/2 Mouse", ico_dev_mice, ico_dev_mice);
      tree->appendItem(devKeyboards, "PC/AT Enhanced PS/2 Keyboard (101/102-Key)", ico_dev_keyb, ico_dev_keyb);
    }

    tree->expandTree(devUnknown);

    // serial ports
    // https://sigrok.org/api/libserialport/unstable/a00002.html
    struct sp_port **portList;

    enum sp_return result = sp_list_ports(&portList);

    if (result == SP_OK) {
      int i;
      for (i = 0; portList[i] != NULL; i++) {
	  struct sp_port *port = portList[i];
	  tree->appendItem(devSerial, sp_get_port_name(port), ico_dev_serial, ico_dev_serial);
	}
    } else
	 puts("not ok");

    FXTreeItem* loopthruprev;
    FXTreeItem* loopthru = top->getFirst();

    int i = 0;

    while (loopthru) {

	i++;

	loopthruprev = loopthru;
	loopthru = loopthru->getNext();

	if (!loopthruprev->getNumChildren()) {
	  tree->removeItem(loopthruprev);
	  //delete loopthru;
	  //puts("No");
	}

	//printf("%d\n", i);
	if (loopthruprev == top)
	  break;



    }




  //new FXButton(toolbar,"\tUp",ico_hist_up,NULL,0,BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0,  0,0,0,0);

}


int main(int argc,char *argv[]) {


  FXApp application("sysdm", "Ice2KProj");
  app = &application;

  application.init(argc,argv);

  ico_devmgmt = new FXGIFIcon(app, resico_devmgmt);
  
  devmgmtwin = new DeviceManager(&application);

  // create windows
  application.create();
  devmgmtwin->killFocus();

  devmgmtwin->show(PLACEMENT_OWNER);
  devmgmtwin->killFocus();

  devmgmtwin->changeFocus((FXWindow*)0);

  //devmgmtwin->changeFocus(devmgmtwin);


  //devmgmtwin->show(PLACEMENT_SCREEN);

  // Run the application
  return application.run();
}

