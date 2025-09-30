#include <fx.h>
#include <ice2k/branding.h>

#include <ice2k/comctl32.h>
#include <ice2k/ini.h>
#include "res/foxres.h"
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/extensions/Xcomposite.h>

#include <sys/wait.h>
#include <sys/stat.h>

#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#include <dirent.h>

#include <FXPNGIcon.h>


FXIcon*                  ico_mainico;
FXIcon*                  prvimage;

FXTreeItem* noneitem;


class DesktopProperties : public FXMainWindow {

  // Macro for class hierarchy declarations
  FXDECLARE(DesktopProperties)

private:
  // gui
  FXDockSite*              topdock;
  
  FXHorizontalFrame*       statusbarcont;
  FXLabel*                 statuslbl;

  FXToolBarShell*          mbshell;
  FXMenuBar*               menubar;
  
  FXMenuPane*              actionmenu;
  FXMenuPane*              viewmenu;

  FXToolBarShell*          tbshell;
  FXToolBarShell*          tb2shell;
  
  FXToolBar*               toolbar;
  FXToolBar*               toolbar2;

  FXSpinner*               waitspin;
  FXCheckButton*           passchk;

  FXTreeList* tree;


protected:
  DesktopProperties(){}

public:

  // Message handlers
  long onItemChange(FXObject*,FXSelector,void*);
  long onStatus(FXObject*,FXSelector,void*);
  long onChangeText(FXObject*,FXSelector,void*);
  long addDevices(FXObject*,FXSelector,void*);
  long onTimeoutScr(FXObject*,FXSelector,void*);
  long onTabChange(FXObject*,FXSelector,void*);
  long onScrChange(FXObject*,FXSelector,void*);

  long onClose(FXObject*,FXSelector,void*);
  long onPreview(FXObject*,FXSelector,void*);

  long onCmdOk(FXObject*,FXSelector,void*);
  long onCmdCancel(FXObject*,FXSelector,void*);
  long onCmdApply(FXObject*,FXSelector,void*);

  long onChange(FXObject*,FXSelector,void*);
  long onImageChange(FXObject*,FXSelector,void*);
  long onColorChange(FXObject*,FXSelector,void*);

  long onColorChangeCmd(FXObject*,FXSelector,void*);
  
  long onCloseCmd(FXObject*,FXSelector,void*);
  long onCmdBrowse(FXObject*,FXSelector,void*);
  

  long onResSliderChange(FXObject*,FXSelector,void*);

public:

  // Messages for our class
  enum {
    ID_MAINWIN=FXMainWindow::ID_LAST,
    ID_TREE,
    ID_ADDDEV,
    ID_SCR,
    ID_TABBOOK,
    ID_LISTBOX,
    ID_PREVIEW,

    ID_DLG_OK,
    ID_DLG_CANCEL,
    ID_DLG_APPLY,

    ID_CHANGE,
    ID_IMAGECHANGE,
    ID_COLORCHANGE,

    ID_RESSLIDER,

    ID_BROWSE
  };

public:

  // CtrlAltDelWindow's constructor
  DesktopProperties(FXApp* a);

  // Initialize
  virtual void create();

  virtual ~DesktopProperties();
};

FXDEFMAP(DesktopProperties) DesktopPropertiesMap[] = {
  /* FXMAPFUNC(SEL_UPDATE, 0, DesktopProperties::onStatus),
  FXMAPFUNC(SEL_COMMAND, DesktopProperties::ID_SETSTRINGVALUE, DesktopProperties::onChangeText),
  FXMAPFUNC(SEL_COMMAND, DesktopProperties::ID_ADDDEV, DesktopProperties::addDevices),

  FXMAPFUNC(SEL_CHANGED, DesktopProperties::ID_TREE, DesktopProperties::onItemChange), */
//  FXMAPFUNC(SEL_TIMEOUT, DesktopProperties::ID_SCR, DesktopProperties::onTimeoutScr),
  FXMAPFUNC(SEL_COMMAND, DesktopProperties::ID_TABBOOK, DesktopProperties::onTabChange),
  FXMAPFUNC(SEL_COMMAND, DesktopProperties::ID_PREVIEW, DesktopProperties::onPreview),

  FXMAPFUNC(SEL_COMMAND, DesktopProperties::ID_LISTBOX, DesktopProperties::onScrChange),

  FXMAPFUNC(SEL_COMMAND, DesktopProperties::ID_DLG_OK, DesktopProperties::onCmdOk),
  FXMAPFUNC(SEL_COMMAND, DesktopProperties::ID_DLG_CANCEL, DesktopProperties::onCmdCancel),
  FXMAPFUNC(SEL_COMMAND, DesktopProperties::ID_DLG_APPLY, DesktopProperties::onCmdApply),


  FXMAPFUNC(SEL_COMMAND, DesktopProperties::ID_CHANGE, DesktopProperties::onChange),
  
  FXMAPFUNC(SEL_COMMAND, DesktopProperties::ID_IMAGECHANGE, DesktopProperties::onImageChange),
  FXMAPFUNC(SEL_CHANGED, DesktopProperties::ID_COLORCHANGE, DesktopProperties::onColorChange),
  FXMAPFUNC(SEL_COMMAND, DesktopProperties::ID_COLORCHANGE, DesktopProperties::onColorChangeCmd),

  FXMAPFUNC(SEL_COMMAND, DesktopProperties::ID_RESSLIDER, DesktopProperties::onResSliderChange),
  FXMAPFUNC(SEL_CHANGED, DesktopProperties::ID_RESSLIDER, DesktopProperties::onResSliderChange),

  FXMAPFUNC(SEL_COMMAND, DesktopProperties::ID_BROWSE, DesktopProperties::onCmdBrowse),


  FXMAPFUNC(SEL_CLOSE, 0, DesktopProperties::onClose),

};

FXIMPLEMENT(DesktopProperties,FXMainWindow,DesktopPropertiesMap,ARRAYNUMBER(DesktopPropertiesMap))


FXColorWell* colorwell;


void DesktopProperties::create() {
	FXMainWindow::create();
}

DesktopProperties::~DesktopProperties() {
}


FXColor deskcol;

typedef struct {
	char* buffer;
	size_t size;
} iniString;


#define _IMGMODE_TILED   0
#define _IMGMODE_CENTER  1
#define _IMGMODE_STRETCH 2
#define _IMGMODE_FILL    3


typedef struct {
	int mode;
	const char* color;
	const char* image;
} wallConfiguration;

int inihandle(void* udata, const char* section, const char* name, const char* value) {
	wallConfiguration* wallcfg = (wallConfiguration*)udata;

	if (!strcmp(section, "Wallpaper")) {
		if (!strcmp(name, "Mode")) {
			if (!strcasecmp(value, "Tiled")) {
				wallcfg->mode = _IMGMODE_TILED;
			} else if (!strcasecmp(value, "Center")) {
				wallcfg->mode = _IMGMODE_CENTER;
			} else if (!strcasecmp(value, "Stretch")) {
				wallcfg->mode = _IMGMODE_STRETCH;
			} else {
				wallcfg->mode = _IMGMODE_FILL;
			}
		} else if (!strcmp(name, "Color")) {
			if (value[0] == '#' && strlen(value) == 7) {
				wallcfg->color = strdup(value);
			}
		} else if (!strcmp(name, "Image")) {
			wallcfg->image = strdup(value);
		} else {
			return 0;
		}
	} else {
		return 0;
	}

	return 1;
}


void handleChildProcess(int sig) {
    waitpid(-1, NULL, WNOHANG); 
}

char** getModes(int* count) { // probably the first time i learned how to parse something complex
	char  line[96];

	int   found=0;
	int   skip=0;
	
	int   cap=10;
	int   mcount=0;

	char** temp;

	char** modes = (char**)malloc(cap * sizeof(char*));
	if (!modes) return NULL;

	char* token;

	FILE* fp = popen("xlock . 2>&1", "r");

	if (!fp) {
		free(modes);
		return NULL;
	}

	while (fgets(line, sizeof(line), fp)) {
		if (!found) {
			if (strstr(line, "[-mode ")) {
				found=1;
			} else {
				continue;
			}
		}

		token = strtok(line, "[ |\n");
		while (token != NULL) {
			if (skip) {
				if (strchr(token, ']') != NULL) {
					goto endloop;
				}

				if (mcount >= cap) {
					cap *= 2;
					temp = (char**)realloc(modes, cap * sizeof(char*));

					if (!temp) {
freemodes:
						for (int i=0; i<mcount; ++i) free(modes[i]);

						free(modes);
						pclose(fp);
						return NULL;
					}

					modes = temp;
				}

				//printf("%s\n", token);
				modes[mcount] = strdup(token);

				if (!modes[mcount]) {
					goto freemodes;
				}
				
				mcount++;

				token = strtok(NULL, "[ |\n");
			} else {
				token = strtok(NULL, "[ |\n");
				skip = 1;
			}
		}
	}

endloop:

	int modeLen = strlen(token);
	//token[modeLen-1] = '\0';
	modes[mcount-1][modeLen-1] = '\0';
	//printf("%s\n", token);

	modes[mcount] = NULL;

	pclose(fp);

	*count = mcount;
	return modes;
}

pid_t pid = -1;
void previewScr(int x, int y, int w, int h, FXWindow* window, const char* scr) {
	pid = vfork();

	if (pid < 0) {
		perror("vfork");
		exit(1);
	} else if (pid == 0) {
		char geometry[32];
		sprintf(geometry, "%dx%d+%d+%d", w,h,x,y);
		//puts(geometry);

		char windowid[16];
		sprintf(windowid, "%u", (unsigned int)window->id());

		execlp("xlock", "xlock",
                   "-mode", scr,
                   "-startCmd", " ",
                   "-endCmd", " ",
                   "-inwindow",
                   "-geometry", geometry,
                   "-parent", windowid,
                   (char*)NULL);

		perror("execlp");
		exit(1);
	//} else {
		//printf("PID: %d\n", pid);
	}

	//wait(NULL);
}

long DesktopProperties::onClose(FXObject* obj,FXSelector sel,void* ptr) {
	//puts("closed");

	if (pid >= 0) {
		kill(pid, SIGTERM);
		pid = -1;
	}

	return FXMainWindow::onCmdClose(obj, sel, ptr);
}

long DesktopProperties::onResSliderChange(FXObject* obj,FXSelector sel,void* ptr) {
	FXSlider* slider = (FXSlider*)obj;

	int min;
	int max;

	slider->getRange(min, max);

	//printf("%d\n", max);

	slider->setValue(max);

	return 1;
}


FXButton* applybtn;
int valuesChanged = 0;


long DesktopProperties::onChange(FXObject* obj,FXSelector sel,void* ptr) {
	//puts("test");
	applybtn->enable();
	valuesChanged = 1;

	return 0;
}

long DesktopProperties::onColorChange(FXObject* obj,FXSelector sel,void* ptr) {
	deskcol = (FXColor)(intptr_t)ptr;

	return onChange(obj, sel, ptr);
}

FXColor lastColor;


FXIcon* monitorsource;
FXIcon* previewsource;
FXIcon* monitorimage;
FXIcon* monitornopimage;


FXLabel* scrmonitor;
FXMainWindow* controlwin;
I2KListBox* scrsel;
FXButton* previewbtn;
FXLabel* prvlbl;

FXIcon* genMonitorPreview(FXApp* app, FXIcon* output, FXImage* img2, FXColor col) {
	//img = loadImage(app, wallcfg.image);

	//if (!img)
	//	return NULL;

	//FXIcon* output = new FXIcon(app, NULL, 0, IMAGE_OPAQUE, 184, 170);
	FXIcon* img = (FXIcon*)img2;



	output->create();
	//puts("b");
	FXDCWindow prvdc(output);

	if (img != NULL) {
		img->scale(152, 112, 1);
		img->create();
	}

	prvdc.clipChildren(FALSE);

	prvdc.setForeground(app->getBaseColor());
	prvdc.fillRectangle(0, 0, 184, 170);
	prvdc.drawIcon(monitorsource, 0, 0);

	prvdc.setForeground(col);
	prvdc.fillRectangle(16, 17, 152, 112);

	prvdc.clipChildren(TRUE);

	if (img != NULL) {
		if (img->isMemberOf(&FXIcon::metaClass))
			prvdc.drawIcon(img, 16, 17);
		else
			prvdc.drawImage(img, 16, 17);
	}


	output->restore();
	output->render();



	//img->detach();
	//img->release();
	//delete img;

	return output;
}


long DesktopProperties::onColorChangeCmd(FXObject* obj,FXSelector sel,void* ptr) {
	//deskcol = (FXColor)(intptr_t)ptr;
	char* udata = (char*)tree->getItemData(tree->getCurrentItem());
	//puts(udata);

	FXColor newColor = (FXColor)(intptr_t)ptr;

	//puts("a");

	//FXColorWell* well = (FXColorWell*)obj;
	if (!lastColor)
		lastColor = newColor;

	if (newColor != lastColor) {
		deskcol = newColor;


		//printf("%d, %d, %d\n", FXREDVAL(newColor), FXGREENVAL(newColor), FXBLUEVAL(newColor));
		lastColor = newColor;

		onChange(obj, sel, ptr);

		genMonitorPreview(getApp(), monitorimage, previewsource, deskcol);
		genMonitorPreview(getApp(), monitornopimage, NULL, deskcol);
	
		if (udata == NULL) {
			prvimage->detach();
			prvimage->release();

			genMonitorPreview(getApp(), prvimage, NULL, deskcol);

			prvlbl->setIcon(NULL);
			prvlbl->setIcon(prvimage);
		}
		return 1;
	}



	return 0;
}

// Patterns
// from fox imageviewer example
const FXchar patterns[] =
	"All Images (*.bmp,*.png,*.jpg,*.jpeg,*.gif,*.tga,*.ppm,*.pbm,*.pgm,*.tif,*.tiff)"
	"All Images (*.bmp,*.png,*.jpg,*.jpeg,*.gif,*.tga,*.ppm,*.pbm,*.pgm,*.tif,*.tiff)"
	"\nBitmap Image (*.bmp)"
	"\nPNG Image  (*.png)"
	"\nJPEG Image (*.jpg,*.jpeg)"
	"\nGIF Image (*.gif)"
	"\nTARGA Image  (*.tga)"
	"\nPPM/PBM/PGM Image  (*.ppm,*.pbm,*.pgm)"
	"\nTIFF Image (*.tif,*.tiff)"
	"\nAll Files (*)"
;

const char* imageExtensions[] = {
	".png", ".jpg", ".jpeg", ".gif", ".tif", ".tiff",
	".bmp", ".tga", ".pbm", ".ppm", ".pgm",
	".htm", ".html", ".mp4", ".flv"
};

const int imageExtensionsNum = sizeof(imageExtensions) / sizeof(imageExtensions[0]);


enum imageLoaders {
	_I2K_PNG,
	_I2K_JPG,
	_I2K_BMP,
	_I2K_GIF,
	_I2K_TIF,
	_I2K_TGA,
	_I2K_PPM
};

FXImage* loadImage(FXApp* app, const char* path) {
	FXIconSource* iconsrc = new FXIconSource(app);

	if (path) {
		if (access(path, F_OK) != 0) {
			return NULL;
		}
	} else {
		return NULL;
	}

	if (!iconsrc) {
		return NULL;
	}

	FXImage* img = iconsrc->loadImageFile(path);

	if (img) {
		return img;
	}

	//puts("Not supported");
	
	return NULL;
		
}

int isImageFormat(const char* ext) {
	if (!ext) return 0;

	for (int i = 0; i < imageExtensionsNum; i++) {
		if (strcasecmp(ext, imageExtensions[i]) == 0) {
			return 1;
		}
	}

	return 0;
}






long DesktopProperties::onImageChange(FXObject* obj,FXSelector sel,void* ptr) {
	//puts("test");

	char* udata = (char*)tree->getItemData(tree->getCurrentItem());

	onChange(obj, sel, ptr);

	prvimage->detach();
	prvimage->release();

	FXImage* img;

	if (udata)
		img = loadImage(getApp(), udata);
	//else
	//	img = loadImage(getApp(), NULL);

	//printf("%u\n", tree->getCurrentItem());
	prvimage->detach();
	prvimage->release();


	if (udata && img)
		genMonitorPreview(getApp(), prvimage, img, deskcol);
	else
		genMonitorPreview(getApp(), prvimage, NULL, deskcol);


	if (udata && img) {
		img->detach();
		img->release();
		delete img;
	}


	prvlbl->setIcon(NULL);
	prvlbl->setIcon(prvimage);
	
	return 1;
}

FXIcon* ico_bmp;

// from fox imageviewer example
long DesktopProperties::onCmdBrowse(FXObject*,FXSelector,void*){
	char* filepath = (char*)tree->getCurrentItem()->getData();

	FXFileDialog open(this,"Open Image");

	open.setFilename(filepath);
	open.setPatternList(patterns);

	if (open.execute()) {
		filepath = strdup(open.getFilename().text());

		FXTreeItem* item = tree->getFirstItem();
		//FXTreeItem* item;
		while (item) {
			//puts("test");
			const char* fpath = (const char*)item->getData();

			if (fpath && !strcmp(fpath, filepath)) {
				tree->makeItemVisible(item);
				tree->setCurrentItem(item, TRUE);

				tree->handle(this, FXSEL(SEL_COMMAND,0),(void*)item);

				//puts("found!");
				//puts(fpath);
				//puts(filepath);
				return 1;
			}

			item = item->getNext();
		}

		//if (tree->findItemByData(filepath)) {
		

		FXImage* img = loadImage(getApp(), filepath);

		if (img) {
			img->detach();
			img->release();

			delete img;
		} else {
			FXMessageBox::error(getApp(),MBOX_OK,"Error",
				"Could not load image!");

			return 1;
		}

		char* filename;

		filename = strrchr(filepath, '/');
		filename++;

		FXTreeItem* newitem = tree->appendItem(0, filename, ico_bmp, ico_bmp, filepath);

		tree->setSortFunc(FXTreeList::ascendingCase);
		tree->sortItems();

		tree->moveItem(tree->getFirstItem(), 0, noneitem);

		tree->makeItemVisible(newitem);
		tree->setCurrentItem(newitem, TRUE);
			//tree->hitItem(newitem, 10,10);
		tree->handle(this, FXSEL(SEL_COMMAND,0),(void*)newitem);
		//}
	}

	return 1;
}

void changeScr(const char* scr) {
	if (scrsel->getCurrentItem()) {
		int absx = 0;
		int absy = 0;

		scrmonitor->translateCoordinatesTo(absx, absy, controlwin, 0, 0);

		previewbtn->enable();
	
		previewScr(absx+16, absy+17, 152, 112, controlwin, scr);
	} else {
		previewbtn->disable();
	}

}


long DesktopProperties::onScrChange(FXObject* obj,FXSelector sel,void* ptr) {
	onChange(obj,sel,ptr);

	intptr_t index = (intptr_t)ptr;

	if (pid >= 0) {
		kill(pid, SIGTERM);
		pid = -1;
	}

	changeScr(scrsel->getItemText(index).text());

	return 1;
}



/* long DesktopProperties::onPreview(FXObject* obj,FXSelector sel,void* ptr) {
	int index = scrsel->getCurrentItem();

	if (pid >= 0) {
		kill(pid, SIGTERM);
		pid = -1;
	}

	char previewcmd[128];

	snprintf(previewcmd, sizeof(previewcmd), "xlock -nolock -mode %s", scrsel->getItemText(index).text());

	system(previewcmd);
	changeScr(scrsel->getItemText(index).text());
} */



long DesktopProperties::onPreview(FXObject* obj,FXSelector sel,void* ptr) {
	int index = scrsel->getCurrentItem();

	if (pid >= 0) {
		kill(pid, SIGTERM);
		pid = -1;
	}


	pid_t previewpid = vfork();

	if (previewpid < 0) {
		perror("vfork");
		exit(1);
	} else if (previewpid == 0) {
		execlp("xlock", "xlock",
                   "-mode", scrsel->getItemText(index).text(),
                   "-nolock",
                   (char*)NULL);
		
		perror("execlp"); 

		exit(1);
	}

	waitpid(previewpid, NULL, 0);
		

	changeScr(scrsel->getItemText(index).text());

	return 1;
}




long DesktopProperties::onCmdOk(FXObject* obj,FXSelector sel,void* ptr) {
	valuesChanged && onCmdApply(obj,sel,ptr);

	if (pid >= 0) {
		kill(pid, SIGTERM);
		pid = -1;
	}


	getApp()->exit(0);

	return 1;
}

char* scrvalue;
int scrdelay = 15;
int scrpassword = 1;

int makeDirectory(const char *dir) {
	char tmp[PATH_MAX];
	char *p = NULL;
	size_t len;

	snprintf(tmp, sizeof(tmp), "%s", dir);
	len = strlen(tmp);
	
	if (tmp[len - 1] == '/')
		tmp[len - 1] = 0;

	for (p = tmp + 1; *p; p++) {
		if (*p == '/') {
			*p = 0;
			
			if (mkdir(tmp, S_IRWXU) == -1) {
				if (errno != EEXIST)
					return -1;
			}

			*p = '/';
		}
	}

	if (mkdir(tmp, S_IRWXU) == -1) {
		if (errno != EEXIST)
			return -1;
	}

	//puts("Success");

	return 1;
}

const char* getHomeDir() {
	const char *homedir;

	if ((homedir = getenv("HOME")) == NULL) {
		homedir = getpwuid(getuid())->pw_dir;
	}

	return homedir;
}


int writeWallpaperConfig(char* image, int imgmode, FXColor color, void* pattern=NULL) {
	char cfgpath[512] = "";
	snprintf(cfgpath, sizeof(cfgpath), "%s/%s", getHomeDir(), ".icewm/cfg");

	unsigned int cfgpathlen = strlen(cfgpath);

	if (makeDirectory(cfgpath)) {
		strncat(cfgpath, "/backmgr.ini", sizeof(cfgpath)-cfgpathlen-1);
		cfgpath[sizeof(cfgpath)-1] = '\0';
	}

	FILE* fp = fopen(cfgpath, "w");
	
	fputs("[Wallpaper]\n", fp);

	if (imgmode == _IMGMODE_TILED) {
		fputs("Mode=Tiled\n", fp);
	} else if (imgmode == _IMGMODE_CENTER) {
		fputs("Mode=Center\n", fp);
	} else if (imgmode == _IMGMODE_STRETCH) {
		fputs("Mode=Stretch\n", fp);
	} else {
		fputs("Mode=Fill\n", fp);
	}

	char hex[8];

	//color = ((color & 0xFF) << 16) | (color & 0xFF00) | ((color >> 16) & 0xFF);
	
	unsigned char r = FXREDVAL(color);
	unsigned char g = FXGREENVAL(color);
	unsigned char b = FXBLUEVAL(color);

	snprintf(hex, sizeof(hex), "#%02X%02X%02X", r, g, b);

	fprintf(fp, "Color=%s\n", hex);

	char* img = (char*)image;
	if (img)
	//	fprintf(fp, "Image=\n")else
		fprintf(fp, "Image=%s\n", image);

	//fprintf(stdout, 
	
	fclose(fp);

	return 1;
}

I2KListBox* picdisplay;

long DesktopProperties::onCmdApply(FXObject* obj,FXSelector sel,void* ptr) {
	if (!valuesChanged)
		return 1;

	applybtn->disable();
	
	char xrdbPath[PATH_MAX];
	snprintf(xrdbPath, sizeof(xrdbPath), "%s/.icewm/xrdb", getHomeDir());
	makeDirectory(xrdbPath);

	strncat(xrdbPath, "/xlock", sizeof(xrdbPath)-1);

	FILE *fptr = fopen(xrdbPath, "w");

	//scrvalue = scrsel->getItemText(index).text();

	int index = scrsel->getCurrentItem();

	//FXString* scr = scrsel->getItemText(index);
	scrvalue = strdup(scrsel->getItemText(index).text());
	scrdelay = waitspin->getValue();
	scrpassword = passchk->getCheck();

	//printf("XLock.mode: %s\n", scrvalue);

	fprintf(fptr, "XLock.mode: %s\n", scrvalue);
	fprintf(fptr, "XIdle.delay: %d\n", scrdelay * 60);
	fprintf(fptr, "XIdle.timeout: %d\n", scrdelay * 60);

	if (scrpassword) {
		fputs("XIdle.program: /usr/local/bin/xlock -lockdelay 5\n", fptr);
	} else {
		fputs("XIdle.program: /usr/local/bin/xlock -nolock\n",      fptr);
	}

	fclose(fptr);

	char xrdbCmd[strlen(xrdbPath)+13];

	// Let's just say I'm not implementing xrdb myself :joy: :rofl:
	snprintf(xrdbCmd, sizeof(xrdbCmd), "xrdb -merge %s", xrdbPath);
	system(xrdbCmd);

	system("killall -9 xidle && xidle &");

	void* walldat =  tree->getCurrentItem()->getData();
	char* wallpath = (char*)walldat;

	intptr_t wallmode = (intptr_t)picdisplay->getItemData(picdisplay->getCurrentItem());

	writeWallpaperConfig(wallpath, wallmode, colorwell->getRGBA());

	valuesChanged = 0;

	system("backmgr &");

	return 1;
}


long DesktopProperties::onCmdCancel(FXObject* obj,FXSelector sel,void* ptr) {
	if (pid >= 0) {
		kill(pid, SIGTERM);
		pid = -1;
	}

	getApp()->exit(0);

	return 1;
}

long DesktopProperties::onTabChange(FXObject* obj,FXSelector sel, void* ptr) {

	FXTabBook* tabbook = (FXTabBook*)obj;

	switch (tabbook->getCurrent()) {
		case 1:

			changeScr(scrsel->getItemText(scrsel->getCurrentItem()).text());
			break;

		default:
			if(pid >= 0) {
				kill(pid, SIGTERM);
				pid = -1;
			}
	}

	return 1;
}



char** getWallpapers(char* path, int* count) {
	// https://stackoverflow.com/a/4204758

	DIR* walldir = opendir(path);

	if (!walldir) {
		perror("opendir");
		return NULL;
	}

	char **files = NULL;
	struct dirent *dir;
	int size = 0;

	char fullpath[PATH_MAX];
	struct stat pstat;

	char* ext;

	while ((dir = readdir(walldir)) != NULL) {
		snprintf(fullpath, sizeof(fullpath), "%s/%s", path, dir->d_name);
		stat(fullpath, &pstat);

		if (S_ISDIR(pstat.st_mode))
			continue;

		ext = strrchr(dir->d_name, '.');

		if (!ext || !isImageFormat(ext))
			continue;

		files = (char**)realloc(files, (size + 1) * sizeof(char*));
		files[size] = strdup(fullpath);
		size++;
	}

	closedir(walldir);
	*count = size;

	return files;
}

FXColor hex2FXColor(const char* hex) {
	if (hex[0] == '#') hex++;

	char rs[3] = { hex[0], hex[1], '\0' };
	char gs[3] = { hex[2], hex[3], '\0' };
	char bs[3] = { hex[4], hex[5], '\0' };

	int r = strtol(rs, NULL, 16);
	int g = strtol(gs, NULL, 16);
	int b = strtol(bs, NULL, 16);

	return FXRGB(r,g,b);
}

wallConfiguration wallcfg;

DesktopProperties::DesktopProperties(FXApp *app):FXMainWindow(app, "Desktop Properties", ico_mainico, NULL, DECOR_TITLE|DECOR_BORDER|DECOR_MENU|DECOR_CLOSE, 0,0,398,423,  0,0,0,0,  0,0) {
  int xp = 0;
//writeWallpaperConfig("/home/tf/image.png", _IMGMODE_TILED, app->getBaseColor());

  const char* homedir = getHomeDir();

  char cfgpath[256] = {0};  // if your username is 230 characters, wtf are you doing?

  snprintf(cfgpath, sizeof(cfgpath), "%s/%s", homedir, ".icewm/cfg/backmgr.ini");

  wallcfg.image = "";
  wallcfg.color = strdup("#000000");

  ini_parse(cfgpath, inihandle, &wallcfg);
  //puts("test");

  deskcol = hex2FXColor(wallcfg.color);
  //puts("test");

  // create monitor images

  char* windows = i2kBGetWinVersion();

  if ( !(strcmp(windows, "srv03")) )
    strcpy(windows, "xp");

  if ( !(strcmp(windows, "xp")) ) {
    xp = 1;
    monitorsource = new FXGIFIcon(app, resico_monitorxp); monitorsource->create();
    previewsource = new FXGIFIcon(app, resico_previewxp); previewsource->create();
  } else {
    monitorsource = new FXGIFIcon(app, resico_monitor); monitorsource->create();
    previewsource = new FXGIFIcon(app, resico_preview); previewsource->create();
  }



  monitorimage = new FXIcon(app, NULL, 0, IMAGE_OPAQUE, 184, 170);
  //monitorimage->create();
  genMonitorPreview(app, monitorimage, previewsource, deskcol);

  /* FXDCWindow mondc(monitorimage);

  mondc.clipChildren(FALSE);

  mondc.setForeground(app->getBaseColor());
  mondc.fillRectangle(0, 0, 184, 170);
  mondc.drawIcon(monitorsource, 0, 0);

  mondc.setForeground(FXRGB(128,0,0));
  mondc.fillRectangle(16, 17, 152, 112);
  mondc.drawIcon(previewsource, 16, 17);

  monitorimage->restore();
  monitorimage->render(); */

  prvimage = new FXIcon(app, NULL, 0, IMAGE_OPAQUE, 184, 170);

  //FXIcon* monitorimage = NEW FXIcon(A

  /* prvimage->create();
  FXDCWindow prvdc(prvimage);

  prvdc.setForeground(app->getBaseColor());
  prvdc.fillRectangle(0, 0, 184, 170);
  prvdc.drawIcon(monitorsource, 0, 0);

  prvdc.setForeground(FXRGB(128,0,0));
  prvdc.fillRectangle(16, 17, 152, 112); */



  monitornopimage = new FXIcon(app, NULL, 0, IMAGE_OPAQUE, 184, 170);
  /* monitornopimage->create();
  FXDCWindow monndc(monitornopimage);

  monndc.clipChildren(FALSE);

  monndc.setForeground(app->getBaseColor());
  monndc.fillRectangle(0, 0, 184, 170);
  monndc.drawIcon(monitorsource, 0, 0);

  monndc.setForeground(FXRGB(128,0,0));
  monndc.fillRectangle(16, 17, 152, 112);

  monitornopimage->restore();
  monitornopimage->render(); */

  genMonitorPreview(app, monitorimage, previewsource, deskcol);
  genMonitorPreview(app, monitornopimage, NULL, deskcol);
  

  FXIcon* ico_nobg;

  if (xp) {
    ico_nobg = new FXBMPIcon(app, resico_nobg_xp);
    ico_nobg->blend(app->getBackColor());
    ico_bmp = new FXBMPIcon(app, resico_bmp_xp);
    ico_bmp->blend(app->getBackColor());
  } else {
    ico_nobg = new FXGIFIcon(app, resico_nobg);
    ico_bmp = new FXGIFIcon(app, resico_bmp);
  }

  //char buf[256] = {0};

  /* iniString config;

  config.buffer = buf;
  config.size = sizeof(buf); */

  // https://stackoverflow.com/questions/2910377/get-home-directory-in-linux

  /* const char *homedir;

  if ((homedir = getenv("HOME")) == NULL) {
    homedir = getpwuid(getuid())->pw_dir;
  } */






  //puts("a");




  /* int buflen = strlen(config.buffer);
  config.buffer[buflen-1] = '\0';
  buflen--;

  //puts(config.buffer);

  int i;

  if (config.buffer[0] == '"') {
    for (i = 1; i < buflen; i++) {
      config.buffer[i-1] = config.buffer[i];
    }

    config.buffer[buflen-1] = '\0';
  }
  //puts(config.buffer);

  char* ogpath = strdup(config.buffer);

  char buf2[sizeof(buf)] = {0};
  memcpy(buf2, buf, sizeof(buf2));

  char *basename = strrchr(buf2, '/');

  if (basename == NULL) {
    basename = strdup(config.buffer);
  }

  if (basename[0] == '/') {
    for (i = 1; i < buflen; i++) {
      basename[i-1] = basename[i];
    }
  } */

  

  //printf("%s\n",basename);

  //puts(config.buffer);



  // background tab
  FXVerticalFrame* tabframe = new FXVerticalFrame(this,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_NONE, 0,0,0,0, 0,0,0,0, 0,0);
  FXTabBook* tabbook = new FXTabBook(tabframe,this,ID_TABBOOK,LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_RIGHT, 0,0,0,0, 6,6,7,6);

  new FXTabItem(tabbook,"Background",NULL,TAB_TOP_NORMAL,0,0,0,0,4,4,1,3);
  FXVerticalFrame* bgframe = new FXVerticalFrame(tabbook,FRAME_THICK|FRAME_RAISED, 0,0,0,0, 13,12,13,13, 0,0); 
  prvlbl = new FXLabel(bgframe, "", prvimage, LABEL_NORMAL|LAYOUT_CENTER_X, 0,0,0,0,  0,0,0,30);

  new FXLabel(bgframe, "&Select a background picture or HTML document as Wallpaper:", NULL, LABEL_NORMAL|JUSTIFY_LEFT, 0,0,0,0,  0,0,0,2);

  FXHorizontalFrame* setbgframe = new FXHorizontalFrame(bgframe,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_NONE, 0,0,0,0, 0,0,0,0, 10,0); 

  //FXFrame* bglist = new FXFrame(setbgframe, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_Y|LAYOUT_FILL_X, 0,0,0,0,  0,0,0,0);
  //bglist->setBackColor(app->getBackColor());
 
  FXPacker* treeframe = new FXPacker(setbgframe, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_Y|LAYOUT_FILL_X, 0,0,0,0,  0,0,0,0);
  tree = new FXTreeList(treeframe,this,ID_IMAGECHANGE,SCROLLERS_DONT_TRACK|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y|TREELIST_BROWSESELECT);

  //FXTreeItem* ogselect = tree->appendItem(0,basename,ico_bmp,ico_bmp,ogpath);

  int count = 0;

  char wallpaperpath[] = "/home/tf/Pictures";


  char** wallpapers = getWallpapers(wallpaperpath, &count);
  char* wallname;
  char wallnamecurr[PATH_MAX] = "";
  char* wallnamecur = wallnamecurr;
  FXTreeItem* ogselect = NULL;

  // if you are going to question my order of adding treelist items
  // you're right
  //
  // i have to do this otherwise theres some item thats like selected
  // and not selected at the same time for some reason, even if i select
  // it in the code...
  //
  // the first item added becomes "semi selected"
  //
  // its really weird, i know

  FXImage* img;

  if (wallcfg.image[0] != '\0') {
    char* imgpath = strdup(wallcfg.image);

    wallnamecur = strrchr(imgpath, '/');
    wallnamecur++;

    img = loadImage(app, wallcfg.image);
    //createPreviewImage
    //prvimage = genMonitorPreview(app, img);
    genMonitorPreview(app, prvimage, img, deskcol);
    //prvimage->create();

    ogselect = tree->appendItem(0, wallnamecur, ico_bmp, ico_bmp, imgpath);
  } else {
    genMonitorPreview(app, prvimage, NULL, deskcol);
  }
	  
  noneitem = tree->appendItem(0,"(None)",ico_nobg,ico_nobg,NULL);


  for (int i = 0; i < count; i++) {
    wallname = strrchr(wallpapers[i], '/');
    wallname++;

    if (strcmp(wallname, wallnamecur) == 0)
      continue;

    tree->appendItem(0, wallname, ico_bmp, ico_bmp, strdup(wallpapers[i]));
    free(wallpapers[i]);
  }


  //new FXImageFrame(bgframe, prvimage, FRAME_NONE);


  free(wallpapers);

  //puts(wallname);


  tree->setSortFunc(FXTreeList::ascendingCase);
  tree->sortItems();



  if (ogselect) {
    tree->selectItem(ogselect);

  }

  tree->moveItem(tree->getFirstItem(), 0, noneitem);





  FXVerticalFrame* setbgcontrols = new FXVerticalFrame(setbgframe,LAYOUT_FILL_Y|FRAME_NONE, 0,0,0,0, 0,0,2,0, 0,0); 
  FXButton* btn;
  new FXButton(setbgcontrols, "&Browse...", NULL, this, ID_BROWSE, BUTTON_DEFAULT|BUTTON_NORMAL|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0,0,75,23,  0,0,0,0);
  new FXSeparator(setbgcontrols, SEPARATOR_NONE|LAYOUT_FIX_HEIGHT, 0,0,0,9); // i could use a fxframe, but semantics r cute
  new FXLabel(setbgcontrols, "Picture &Display:", NULL, LABEL_NORMAL|JUSTIFY_LEFT, 0,0,0,0,  1,0,0,2);
  picdisplay = new I2KListBox(setbgcontrols,this,ID_CHANGE,COMBOBOX_INSERT_LAST|LAYOUT_FILL_X|COMBOBOX_STATIC|FRAME_SUNKEN|FRAME_THICK, 0, 0, 0, 0, 3, 0, 2, 1);
  new FXSeparator(setbgcontrols, SEPARATOR_NONE|LAYOUT_FIX_HEIGHT, 0,0,0,9);
  new FXLabel(setbgcontrols, "Color:", NULL, LABEL_NORMAL|JUSTIFY_LEFT, 0,0,0,0,  0,0,0,2);
  
  //btn = new FXButton(setbgcontrols, "&Pattern...", NULL, NULL, 0, BUTTON_DEFAULT|BUTTON_NORMAL|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0,0,75,23,  0,0,0,0);
  //btn->disable();


  colorwell = new FXColorWell(setbgcontrols,deskcol,this,ID_COLORCHANGE,COLORWELL_OPAQUEONLY|LAYOUT_FIX_WIDTH|LAYOUT_FILL_Y, 0,0,75,0, 0,0,0,0);

  deskcol = colorwell->getRGBA();

  //colorwell->setRGBA(FXRGB(59, 110, 165));


  picdisplay->insertItem(0, "Center", NULL, (void*)_IMGMODE_CENTER);
  picdisplay->insertItem(1, "Tile", NULL, (void*)_IMGMODE_TILED);
  picdisplay->insertItem(2, "Stretch", NULL, (void*)_IMGMODE_STRETCH);
  picdisplay->insertItem(3, "Fill", NULL, (void*)_IMGMODE_FILL);

  picdisplay->setNumVisible(picdisplay->getNumItems());

  if (wallcfg.mode == _IMGMODE_CENTER) picdisplay->setCurrentItem(0);
  if (wallcfg.mode == _IMGMODE_TILED) picdisplay->setCurrentItem(1);
  if (wallcfg.mode == _IMGMODE_STRETCH) picdisplay->setCurrentItem(2);
  if (wallcfg.mode == _IMGMODE_FILL) picdisplay->setCurrentItem(3);


  // !! SCREENSAVER TAB
  // i think it should be obvious but i somehow get confused on what is what sometimes
  new FXTabItem(tabbook,"Screen Saver",NULL,TAB_TOP_NORMAL,0,0,0,0,4,4,1,3);
  FXVerticalFrame* scrframe = new FXVerticalFrame(tabbook,FRAME_THICK|FRAME_RAISED, 0,0,0,0, 13,12,13,13, 0,0);
  scrmonitor = new FXLabel(scrframe, "", monitornopimage, LABEL_NORMAL|LAYOUT_CENTER_X, 0,0,0,0,  0,0,0,0);

  FXGroupBox* scrgrp = new FXGroupBox(scrframe, "Screen Saver", FRAME_THICK|LAYOUT_FILL_X, 0,0,0,0, 7,12,-1,5);

  FXHorizontalFrame* scrgrptop = new FXHorizontalFrame(scrgrp,LAYOUT_FILL_X|FRAME_NONE, 0,0,0,0, 0,0,0,0, 0,0); 


  scrsel = new I2KListBox(scrgrptop,this,ID_LISTBOX,LAYOUT_BOTTOM|COMBOBOX_INSERT_LAST|LAYOUT_FILL_X|COMBOBOX_STATIC|FRAME_SUNKEN|FRAME_THICK, 0, 0, 0, 0, 3, 0, 2, 1);

  int modec=0;
  char** modes = getModes(&modec);

  if (modes == NULL) {
    puts("whoops!");
  }

  scrsel->insertItem(0, "(None)");

  Display* dpy = (Display*)app->getDisplay();
  char* resmgr = XResourceManagerString(dpy);
  XrmDatabase db;
  XrmValue value;

  char* type;

  if (resmgr) {
    XrmInitialize();
    db = XrmGetStringDatabase(resmgr);

    if (XrmGetResource(db, "XLock.mode", "Xlock.mode", &type, &value)) {
      scrvalue = value.addr;
    }

    if (XrmGetResource(db, "XIdle.delay", "Xidle.delay", &type, &value)) {
      scrdelay = atoi(value.addr) / 60;
    }

    if (XrmGetResource(db, "XIdle.program", "Xidle.program", &type, &value)) {
      char* token = strtok(value.addr, " ");

      while (token != NULL) {
        if (!strcmp(token, "-nolock")) {
          scrpassword = 0;
        }

	  //puts(token);
	  token = strtok(NULL, " ");
      }
    }
  }

  
  for (int i = 0; i < modec; ++i) {
    scrsel->insertItem(i+1, modes[i]);
    if (!strcmp(modes[i], scrvalue))
      scrsel->setCurrentItem(i+1);
    
    free(modes[i]);
  }

  if (modec < 12) {
    scrsel->setNumVisible(scrsel->getNumItems());
  } else {
    scrsel->setNumVisible(12);
  }

  free(modes);

  new FXSeparator(scrgrptop, SEPARATOR_NONE|LAYOUT_FIX_WIDTH, 0,0,5,0);
  
  btn = new FXButton(scrgrptop, "Se&ttings...", NULL, NULL, 0, BUTTON_DEFAULT|BUTTON_NORMAL|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0,0,75,23,  0,0,0,0);
  btn->disable();
  new FXSeparator(scrgrptop, SEPARATOR_NONE|LAYOUT_FIX_WIDTH, 0,0,6,0);

  previewbtn = new FXButton(scrgrptop, "Pre&view", NULL, this, ID_PREVIEW, BUTTON_DEFAULT|BUTTON_NORMAL|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0,0,75,23,  0,0,0,0);
  //btn->disable();

  
  FXHorizontalFrame* scrgrpbot = new FXHorizontalFrame(scrgrp,LAYOUT_FILL_X|FRAME_NONE, 0,0,0,0, 0,0,7,0, 0,0); 

  FXCheckButton* chk;
  // fix checkbox paddings eventually
  // too chopped to do comctl jawn
  passchk = new FXCheckButton(scrgrpbot, "&Password protected", this, ID_CHANGE, CHECKBUTTON_NORMAL,0,0,0,0,  0,1,1,1);

  passchk->setCheck(scrpassword);

  FXHorizontalFrame* waitcont = new FXHorizontalFrame(scrgrpbot,LAYOUT_RIGHT|FRAME_NONE, 0,0,0,0, 0,0,0,0, 0,0); 
  
  new FXLabel(waitcont, "Wait  ", NULL, LABEL_NORMAL|LAYOUT_CENTER_Y, 0,0,0,0,  0,0,0,0);
  waitspin = new FXSpinner(waitcont,4,this,ID_CHANGE,SPIN_NORMAL|LAYOUT_CENTER_Y|FRAME_SUNKEN|FRAME_THICK, 0,0,0,0, 0,0,1,1);
  waitspin->setValue(scrdelay);
  new FXLabel(waitcont, "  minutes", NULL, LABEL_NORMAL|LAYOUT_CENTER_Y, 0,0,0,0,  0,0,0,0);


  // !! EFFECTS TAB
  new FXTabItem(tabbook,"Effects",NULL,TAB_TOP_NORMAL,0,0,0,0,4,4,1,3);

  // coffee spelt backwards is eeffoc
  // because when i dont have my morning coffee
  // i dont give eeffoc

  FXVerticalFrame* effxframe = new FXVerticalFrame(tabbook,FRAME_THICK|FRAME_RAISED, 0,0,0,0, 13,12,13,13, 0,0); 

  FXGroupBox* effxbotgrp = new FXGroupBox(effxframe, "Visual effects", FRAME_THICK|LAYOUT_FILL_X, 0,0,0,0, 8,8,4,6);
  //FXHorizontalFrame* fxbotgrp = new FXHorizontalFrame(effxbotgrp,LAYOUT_FILL_X|FRAME_NONE, 0,0,0,0, 0,0,0,0, 0,0); 

  chk = new FXCheckButton(effxbotgrp, "&Smooth edges of screen fonts", NULL, 0, CHECKBUTTON_NORMAL,0,0,0,0,  0,1,1,1);
  chk->disable();
  chk = new FXCheckButton(effxbotgrp, "Show &window contents while dragging", NULL, 0, CHECKBUTTON_NORMAL,0,0,0,0,  0,1,1,1);
  chk->disable();


  // !! SETTINGS TAB

  Window root = DefaultRootWindow(dpy);

  XWindowAttributes attr;

  XGetWindowAttributes(dpy, root, &attr);
	  //printf("depth: %d\n", attr.depth);

  int evbase, errbase;
  char xcomposite;

  if (XCompositeQueryExtension(dpy, &evbase, &errbase)) {
    xcomposite = 1;
  } else {
    xcomposite = 0;
  }


  new FXTabItem(tabbook,"Settings",NULL,TAB_TOP_NORMAL,0,0,0,0,4,4,1,3);
  FXVerticalFrame* settingsframe = new FXVerticalFrame(tabbook,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_THICK|FRAME_RAISED, 0,0,0,0, 13,12,13,13, 0,0); 
  new FXLabel(settingsframe, "", monitorimage, LABEL_NORMAL|LAYOUT_CENTER_X, 0,0,0,0,  0,0,0,18);

  new FXLabel(settingsframe, "Display:", NULL, LABEL_NORMAL|JUSTIFY_LEFT, 0,0,0,0,  1,0,0,2);
  new FXLabel(settingsframe, "Default Monitor on Cirrus Logic 5446 Compatible Graphics Adapter", NULL, LABEL_NORMAL|JUSTIFY_LEFT, 0,0,0,0,  1,0,0,13);

  // bottom groupboxes
  //FXHorizontalFrame* settingsgrpc = new FXHorizontalFrame(settingsframe,LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0, 0,0);
  FXHorizontalFrame* settingsgrpc = new FXHorizontalFrame(settingsframe,PACK_UNIFORM_WIDTH|PACK_UNIFORM_HEIGHT|LAYOUT_FILL_X, 0,0,0,0, 0,1,0,0, 10,0);


  FXGroupBox* settingsgrpcol = new FXGroupBox(settingsgrpc, "Colors", LAYOUT_FILL_X|FRAME_THICK, 0,0,0,0, 7,7,0,7, 8,8);
  I2KListBox* moncolsel = new I2KListBox(settingsgrpcol,NULL,0,LAYOUT_FILL_X|FRAME_SUNKEN|FRAME_THICK, 0, 0, 0, 0, 3, 0, 2, 1);


  FXImage* img_hicolor = new FXGIFImage(app, resico_hicolor, IMAGE_OPAQUE);
  FXImage* img_medcolor = new FXGIFImage(app, resico_medcolor, IMAGE_OPAQUE);

  if (attr.depth <= 4) {
    moncolsel->appendItem("16 Color", NULL, (void*)4);
    new FXImageFrame(settingsgrpcol, img_medcolor, FRAME_SUNKEN);
  } else if (attr.depth <= 8) {
    moncolsel->appendItem("256 Colors", NULL, (void*)8);
    new FXImageFrame(settingsgrpcol, img_medcolor, FRAME_SUNKEN);
  } else if (attr.depth <= 16) {
    moncolsel->appendItem("High Color (16 bit)", NULL, (void*)16);
    new FXImageFrame(settingsgrpcol, img_hicolor, FRAME_SUNKEN);
  } else if (attr.depth <= 24) {
    // there isnt really a concept of 32 bit color in X
    // so i think the closest thing would be checking if
    // the x server is 24 bit color and checking if xcomposite
    // is supported

    if (xcomposite) moncolsel->appendItem("True Color (32 bit)", NULL, (void*)32);
    else moncolsel->appendItem("True Color (24 bit)", NULL, (void*)24);

    new FXImageFrame(settingsgrpcol, img_hicolor, FRAME_SUNKEN);
  }

  moncolsel->setNumVisible(moncolsel->getNumItems());
  moncolsel->disable();


  FXGroupBox* settingsgrpcr = new FXGroupBox(settingsgrpc, "Screen area", LAYOUT_FILL_X|FRAME_THICK, 0,0,0,0, 7,12,-1,5);
  FXHorizontalFrame* settingsgrpres = new FXHorizontalFrame(settingsgrpcr,LAYOUT_FILL_X|FRAME_NONE, 0,0,0,0, 0,0,0,0, 0,0); 


  //I2KListBox* reslistbox = new I2KListBox(settingsgrpcr,NULL,NULL,LAYOUT_BOTTOM|COMBOBOX_INSERT_LAST|LAYOUT_FILL_X|COMBOBOX_STATIC|FRAME_SUNKEN|FRAME_THICK, 0, 0, 0, 0, 3, 0, 2, 1);
  
  FXLabel* lbl;
  lbl = new FXLabel(settingsgrpres, "Less", NULL, LABEL_NORMAL, 0,0,0,0, 1,0,1,0);
  lbl->disable();

  FXSlider* resslider = new FXSlider(settingsgrpres, this, ID_RESSLIDER, SLIDER_TICKS_BOTTOM|SLIDER_ARROW_DOWN|LAYOUT_FIX_HEIGHT|SLIDER_NORMAL|LAYOUT_FILL_X, 0,0,0,29,  16,15,3,0);
  resslider->setRange(0, 3);
  resslider->setHeadSize(11);
  resslider->setSlotSize(4);
  
  //resslider->setTickDelta(4);

  lbl = new FXLabel(settingsgrpres, "More", NULL, LABEL_NORMAL, 0,0,0,0, 1,0,1,0);
  lbl->disable();

  /* FXWindow* fxroot = app->getRootWindow();

  int rwidth = fxroot->getWidth();
  int rheight = fxroot->getHeight(); */

  char restext[32];

  snprintf(restext, sizeof(restext), "%d by %d pixels", attr.width, attr.height);

  lbl = new FXLabel(settingsgrpcr, restext, NULL, LAYOUT_CENTER_X|LABEL_NORMAL, 0,0,0,0, 2,0,0,0);
  lbl->disable();


  //FXHorizontalFrame* rescolgrpbot = new FXHorizontalFrame(resgrpcc,LAYOUT_FILL_X|FRAME_NONE, 0,0,0,0, 0,0,7,0, 0,0); 




  FXHorizontalFrame* btncont = new FXHorizontalFrame(tabframe, LAYOUT_RIGHT, 0, 0, 0, 0, 0, 6, 0, 7, 6, 0);

  new FXButton(btncont, "OK", NULL, this, ID_DLG_OK, BUTTON_DEFAULT|BUTTON_NORMAL|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0, 0, 75, 23, 0, 0, 0, 0);
  new FXButton(btncont, "Cancel", NULL, this, ID_DLG_CANCEL, BUTTON_NORMAL|BUTTON_DEFAULT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0, 0, 75, 23, 0, 0, 0, 0);
  applybtn = new FXButton(btncont, "&Apply", NULL, this, ID_DLG_APPLY, BUTTON_NORMAL|BUTTON_DEFAULT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0, 0, 75, 23, 0, 0, 0, 0);

  if (!ogselect) {
    tree->selectItem(noneitem);
  }



  applybtn->disable();
}

int main(int argc, char *argv[]) {
	signal(SIGCHLD, handleChildProcess);

	FXApp application("DesktopProperties", "Ice2KProj");
	application.init(argc, argv);

	FXApp* app = &application;


	ico_mainico = new FXGIFIcon(app, resico_mainico, IMAGE_NEAREST);

	controlwin = new DesktopProperties(app);
	
	application.create();
	controlwin->show(PLACEMENT_OWNER);

	//app->addTimeout(controlwin,DesktopProperties::ID_SCR,2000);

	return application.run();
}
