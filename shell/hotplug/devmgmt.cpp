#include <fx.h>
#include <ice2k/comctl32.h>
#include <ice2k/branding.h>
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

FXMainWindow* hotplugwin;
FXIcon* ico_hotplug;
FXIcon* ico_hotplug_big;

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

// Main Window
class HotplugDialog : public FXMainWindow {

  // Macro for class hierarchy declarations
  FXDECLARE(HotplugDialog)

private:
  // gui
  FXDockSite*              topdock;

  FXLabel*                 devtext;

  // tree
  FXPacker*                treeframe;
  FXTreeList*              tree;

  FXTreeItem               *branch, *top;

  // icons
  FXIcon*                  ico_dev_cdrom;
  FXIcon*                  ico_dev_disk;
  FXIcon*                  ico_dev_floppy;


protected:
  HotplugDialog(){}

public:

  // Message handlers
  long onItemChange(FXObject*,FXSelector,void*);
  long onStatus(FXObject*,FXSelector,void*);
  long onChangeText(FXObject*,FXSelector,void*);
  long addDevices(FXObject*,FXSelector,void*);
  long changeStatus(FXTreeList*, FXTreeItem*, char*, int);

  long onCmdStop(FXObject*,FXSelector,void*);
  long onCmdClose(FXObject*,FXSelector,void*);



public:

  // Messages for our class
  enum {
    ID_MAINWIN=FXMainWindow::ID_LAST,
    ID_TREE,
    ID_ADDDEV,
    ID_STOP,
    ID_CLOSE
  };

public:

  // CtrlAltDelWindow's constructor
  HotplugDialog(FXApp* a);

  // Initialize
  virtual void create();

  virtual ~HotplugDialog();
};

FXDEFMAP(HotplugDialog) HotplugDialogMap[] = {
  /* FXMAPFUNC(SEL_UPDATE, 0, HotplugDialog::onStatus),
  FXMAPFUNC(SEL_COMMAND, HotplugDialog::ID_SETSTRINGVALUE, HotplugDialog::onChangeText), */
  FXMAPFUNC(SEL_COMMAND, HotplugDialog::ID_ADDDEV, HotplugDialog::addDevices),

  FXMAPFUNC(SEL_COMMAND, HotplugDialog::ID_STOP, HotplugDialog::onCmdStop),
  FXMAPFUNC(SEL_COMMAND, HotplugDialog::ID_CLOSE, HotplugDialog::onCmdClose),

  FXMAPFUNC(SEL_CHANGED, HotplugDialog::ID_TREE, HotplugDialog::onItemChange),
};

FXIMPLEMENT(HotplugDialog,FXMainWindow,HotplugDialogMap,ARRAYNUMBER(HotplugDialogMap))



HotplugDialog::~HotplugDialog() {
}
// from FXStatusLine src
long HotplugDialog::onStatus(FXObject* sender, FXSelector sel, void* ptr) {
  FXWindow *helpsource=getApp()->getCursorWindow();
  if (helpsource && getShell()->isOwnerOf(helpsource) && helpsource->handle(this,FXSEL(SEL_QUERY_HELP,0),NULL))
    return 1;

  statuslbl->setText(" ");
  return 1;
}

long HotplugDialog::onChangeText(FXObject* sender, FXSelector sel, void* ptr) {
  //statuslbl->setText(*((FXString*)ptr));
  
  return 1;
}

long HotplugDialog::onCmdClose(FXObject* sender, FXSelector sel, void* ptr) {
  //statuslbl->setText(*((FXString*)ptr));
  getApp()->exit(0);
  return 1;
}

char** mainargv;

long HotplugDialog::onCmdStop(FXObject* sender, FXSelector sel, void* ptr) {
  //statuslbl->setText(*((FXString*)ptr));
  FXTreeItem* currentItem = tree->getCurrentItem();
  char** dataptr = (char**)tree->getItemData(currentItem);

  char ejectcom[256];

  snprintf(ejectcom, sizeof(ejectcom), "pkexec eject /dev/%s", dataptr[0]);

  system(ejectcom);

  execv(mainargv[0], mainargv);

	  
  return 1;
}

FXButton* propbtn;
FXVerticalSeparator* propsep;

long HotplugDialog::changeStatus(FXTreeList* tree, FXTreeItem* currentItem, char* statusTextChar, int statusTextCharSize) {
  char** dataptr = (char**)tree->getItemData(currentItem);

  char* hardDrive;
  char* hardDriveModel;

  //char devtextchar[256];

  if (dataptr) {
    hardDrive = dataptr[0];
    hardDriveModel = dataptr[1];

    //puts(hardDrive);
    //puts(hardDriveModel);

    snprintf(statusTextChar, statusTextCharSize, "%s at Device %s", hardDriveModel, hardDrive);
    

    //devtext->setText(statusTextChar);
  }

  /* FXTreeItem* currentitem = tree->getCurrentItem();
  FXTreeItem* root = tree->getFirstItem();

  if (root == currentitem) {
    propbtn->hide();
    propsep->hide();
  } else {
    propbtn->show();
    propsep->show();
  }

  propbtn->getParent()->recalc(); */
  
  return 1;
}

void HotplugDialog::create() {
  FXMainWindow::create();
}


//FXButton* stopbtn;

long HotplugDialog::onItemChange(FXObject* sender, FXSelector sel, void* ptr) {
  FXTreeList* tree = (FXTreeList*)sender;
  FXTreeItem* currentItem = tree->getCurrentItem();

  char statustext[256];

  changeStatus(tree, currentItem, statustext, sizeof(statustext));
    
  devtext->setText(statustext);

  return 1;
}



long HotplugDialog::addDevices(FXObject* sender, FXSelector sel, void* ptr) {
  FXApp* app = getApp();
  // pci related code is based off here
  // thank you https://josuedhg.wordpress.com/2014/11/15/how-to-list-pci-devices-with-c-on-linux/


  // clear everything first...
  tree->clearItems();

	top = 0;

    char hardDrives[256];
    char* hardDrive;
    char hardDriveModel[256];

    char removablestr[3];
    char sizestr[3];


    getHardDrives(hardDrives, sizeof(hardDrives));

    hardDrive = strtok(hardDrives, ",");


    //puts(hardDrives);

      while (hardDrive) {
        getHddInfo(hardDrive, "device/model", hardDriveModel, sizeof(hardDriveModel));

        getHddInfo(hardDrive, "removable", removablestr, sizeof(removablestr));
        getHddInfo(hardDrive, "size", sizestr, sizeof(sizestr));

	  if (sizestr[0] == '0') {
          hardDrive = strtok(NULL, ",");
          continue;
	  }


	  removablestr[1] = '\00';
	  int removable = atoi(removablestr);
	  if (!removable) {
          hardDrive = strtok(NULL, ",");
	    continue;
	  }

	  int noname = 1;
	  int hardDriveModelLen = strlen(hardDriveModel);
	  
	  int i; for (i=0; i<hardDriveModelLen; i++) {
          if (hardDriveModel[i] != ' ')
		break;
	    else
            snprintf(hardDriveModel, sizeof(hardDriveModel), "Generic USB Mass Storage Device (/dev/%s)", hardDrive);
            noname = 0;
	  }

	  char* hardDriveStr = strdup(hardDrive);
	  char* hardDriveModelStr = strdup(hardDriveModel);

        char** driveInfo = (char**)malloc(sizeof(char*) * 2);
	  driveInfo[0] = hardDriveStr;
	  driveInfo[1] = hardDriveModelStr;

        removable && tree->appendItem(top,hardDriveModel,ico_dev_disk,ico_dev_disk, (char*)driveInfo);

        hardDrive = strtok(NULL, ",");
      }

    tree->update();	
    tree->recalc();

    char* cdDrives = hardDrives; // we save space, why not?
    getCdDrives(cdDrives, sizeof(cdDrives));

    char* cdDrive = strtok(cdDrives, ",");
    char cdDriveModel[256];

    while (cdDrive) {
      getHddInfo(cdDrive, "device/model", cdDriveModel, sizeof(cdDriveModel));

      getHddInfo(cdDrive, "removable", removablestr, sizeof(removablestr));
	removablestr[1] = '\00';
	int removable = atoi(removablestr);


	if (!removable) {
        cdDrive = strtok(NULL, ",");
	  continue;
	}

      char* cdDriveStr = strdup(cdDrive);
      char* cdDriveModelStr = strdup(cdDriveModel);

      char** driveInfo = (char**)malloc(sizeof(char*) * 2);
      driveInfo[0] = cdDriveStr;
      driveInfo[1] = cdDriveModelStr;

	removable && tree->appendItem(top,cdDriveModel,ico_dev_cdrom,ico_dev_cdrom, (char*)driveInfo);
      cdDrive = strtok(NULL, ",");
    }

    char* floppyDrives = hardDrives; // we save space, why not?
    getFloppyDrives(floppyDrives, sizeof(floppyDrives));

    char* floppyDrive = strtok(floppyDrives, ",");
    char floppyDriveModel[256];

    while (floppyDrive) {
      getHddInfo(floppyDrive, "device/model", floppyDriveModel, sizeof(floppyDriveModel));
      
      getHddInfo(cdDrive, "removable", removablestr, sizeof(removablestr));
	removablestr[1] = '\00';
	int removable = atoi(removablestr);


	if (!removable) {
        floppyDrive = strtok(NULL, ",");
	  continue;
	}

      char* floppyDriveStr = strdup(floppyDrive);
      char* floppyDriveModelStr = strdup(floppyDriveModel);

      char** driveInfo = (char**)malloc(sizeof(char*) * 2);
      driveInfo[0] = floppyDriveStr;
      driveInfo[1] = floppyDriveModelStr;


	removable && tree->appendItem(top,floppyDriveModel,ico_dev_floppy,ico_dev_floppy, (char*)driveInfo);
      floppyDrive = strtok(NULL, ",");
    }

  return 1;
    
}

//int main(int argc, char *argv[]) {
HotplugDialog::HotplugDialog(FXApp *app):FXMainWindow(app, "Device Manager", ico_hotplug, NULL, DECOR_TITLE|DECOR_BORDER|DECOR_MENU|DECOR_CLOSE, 0,0,417,398,  11,10,16,11,  0,0) {	

  FXPacker* topinfo = new FXPacker(this, LAYOUT_FILL_X, 0,0,0,0,  0,0,0,8, 11, -1);


  new FXLabel(topinfo, "", ico_hotplug_big, LAYOUT_SIDE_LEFT|LABEL_NORMAL, 0,0,0,0, 0,0,0,0);

  new FXLabel(topinfo, "Select the device you want to unplug or eject, and then click Stop. When", NULL, LAYOUT_SIDE_TOP|LABEL_NORMAL, 0,0,0,0, 0,0,4,0);
  new FXLabel(topinfo, "Windows notifies you that it is safe to do so unplug the device from your", NULL, LAYOUT_SIDE_TOP|LABEL_NORMAL, 0,0,0,0, 0,0,0,0);
  new FXLabel(topinfo, "computer.", NULL, LAYOUT_SIDE_TOP|LABEL_NORMAL, 0,0,0,0, 0,0,0,0);

  new FXLabel(this, "&Hardware devices:", NULL, LAYOUT_SIDE_TOP|LABEL_NORMAL, 0,0,0,0, 0,0,0,3);

  FXPacker* bottominfo = new FXPacker(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X, 0,0,0,0,  0,0,0,0, 0,0);
  

  treeframe = new FXPacker(this, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_Y|LAYOUT_FILL_X, 0,0,0,0,  0,0,0,0);
  tree = new FXTreeList(treeframe,this,ID_TREE,SCROLLERS_DONT_TRACK|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y|TREELIST_BROWSESELECT);

  ico_dev_cdrom = new FXGIFIcon(app, resico_dev_cdrom, IMAGE_NEAREST); ico_dev_cdrom->create();
  ico_dev_disk = new FXGIFIcon(app, resico_dev_disk, IMAGE_NEAREST); ico_dev_disk->create();
  ico_dev_floppy = new FXGIFIcon(app, resico_dev_floppy, IMAGE_NEAREST); ico_dev_floppy->create();

  addDevices(NULL, 0, NULL);

  FXTreeItem* firstitem = tree->getFirstItem();
  devtext = new FXLabel(bottominfo, "No hotpluggable devices found.", NULL, LAYOUT_SIDE_TOP|LABEL_NORMAL, 0,0,0,0, 0,0,3,0);



  if (firstitem) {
    tree->selectItem(tree->getFirstItem());
    char statustext[256];
    changeStatus(tree, tree->getFirstItem(), statustext, sizeof(statustext));
    devtext->setText(statustext);
  }


  FXPacker* tbtncont = new FXPacker(bottominfo, LAYOUT_RIGHT, 0,0,0,0,  0,0,21,13, 4,4);

  FXButton* btn;

  btn = new FXButton(tbtncont, "&Properties", NULL, NULL, 0, LAYOUT_SIDE_LEFT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|BUTTON_NORMAL|BUTTON_DEFAULT, 0,0,83,23, 0,0,0,0); btn->disable();
  btn = new FXButton(tbtncont, "&Stop", NULL, this, ID_STOP, LAYOUT_SIDE_LEFT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|BUTTON_NORMAL|BUTTON_DEFAULT, 0,0,83,23, 0,0,0,0);

  if (!firstitem)
    btn->disable();

  new FXHorizontalSeparator(bottominfo, SEPARATOR_GROOVE|LAYOUT_FILL_X,0,0,0,0,  0,1,0,0);

  new FXHorizontalSeparator(bottominfo, SEPARATOR_NONE|LAYOUT_FIX_HEIGHT, 0,0,0,14); // semantics r cute

  FXVerticalFrame* chkcont = new FXVerticalFrame(bottominfo, LAYOUT_SIDE_LEFT|FRAME_NONE, 0,0,0,0,  0,0,0,8, 0,10);

  FXCheckButton* chkbtn;

  chkbtn = new FXCheckButton(chkcont, "&Display device components", NULL, 0, CHECKBUTTON_NORMAL, 0,0,0,0,  0,0,0,0);
  chkbtn->disable();

  chkbtn = new FXCheckButton(chkcont, "Show Unplug/Eject &icon on the taskbar", NULL, 0, CHECKBUTTON_NORMAL, 0,0,0,0,  0,0,0,0);
  chkbtn->setCheck(TRUE);
  chkbtn->disable();


  new FXButton(bottominfo, "&Close", NULL, this, ID_CLOSE, LAYOUT_SIDE_RIGHT|LAYOUT_BOTTOM|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|BUTTON_NORMAL|BUTTON_DEFAULT, 0,0,83,23, 0,0,0,0);

  //new FXButton(toolbar,"\tUp",ico_hist_up,NULL,0,BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT,0,0,0,0,  0,0,0,0);

}
int xp = 0;

int main(int argc,char *argv[]) {
  mainargv = argv;


  //fork();


  FXApp application("hotplug", "Ice2KProj");
  app = &application;

  application.init(argc,argv);

  char* windows = i2kBGetWinVersion();

  if ( !(strcmp(windows, "srv03")) )
    strcpy(windows, "xp");

  ico_hotplug = new FXGIFIcon(app, resico_hotplug);

  if ( !(strcmp(windows, "xp")) ) {
    xp = 1;
    ico_hotplug_big = new FXBMPIcon(app, resico_hotplug_big_xp);
    ico_hotplug_big->blend(app->getBaseColor());
  } else {
    ico_hotplug_big = new FXGIFIcon(app, resico_hotplug_big);
  }


  //ico_hotplug_big = new FXGIFIcon(app, resico_hotplug);
  
  hotplugwin = new HotplugDialog(&application);

  // create windows
  application.create();
  hotplugwin->killFocus();

  hotplugwin->show(PLACEMENT_OWNER);
  hotplugwin->killFocus();

  hotplugwin->changeFocus((FXWindow*)0);


  //hotplugwin->changeFocus(hotplugwin);


  //hotplugwin->show(PLACEMENT_SCREEN);

  // Run the application
  return application.run();
}

