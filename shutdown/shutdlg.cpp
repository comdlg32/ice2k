// Based on ctrl+alt+del program
// which is based on the scribble example

#include "fx.h"
#include <FXPNGImage.h>
#include "fxkeys.h"


#include "res/foxres.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>
#include <sys/sysinfo.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ice2k/I2KListBox.h>
#undef _DO_NOT_OVERRIDE_FOX_CONTROLS
#include <ice2k/comctl32.h>

// thank you stack overflow
// https://stackoverflow.com/questions/20733215/how-to-make-a-window-always-on-top
// since fox 1.6 doesnt support setting the window on top, we need to use the x11 api directly

#define _NET_WM_STATE_REMOVE        0    /* remove/unset property */
#define _NET_WM_STATE_ADD           1    /* add/set property */
#define _NET_WM_STATE_TOGGLE        2    /* toggle property  */

// comment to disable """secure""" desktop
//#define _SEC_DESKTOP

// comment to not exit on unfocus
//#define _EXIT_ON_UNFOCUS

// banner   = windows xp x64
// banner2k = windows 2000
// banner03 = windows server 2003 x64
#define _BANNER_IMG                 banner2k

int currentwksp;
int workspaces;
int deskshown;
int closedisable;
int buttonpressed;
char taskid[64];

Display* xdisplay;
Window rootwin;
FXDialogBox* cadwindow;
FXMainWindow* chkwindow;
#define _OPTION_LOGOFF 0
#define _OPTION_SHUTDOWN 1
#define _OPTION_RESTART 2
#define _OPTION_STANDBY 3

// change a window's _NET_WM_STATE property so that it can be kept on top.
// @display: x11 display singleton.
// @xid    : the window to set on top.

 // chatgpt helped me a bit with this
Status setOnTop (Display* display, Window xid) {
  XEvent event;
  event.xclient.type = ClientMessage;
  event.xclient.serial = 0;
  event.xclient.send_event = True;
  event.xclient.display = display;
  event.xclient.window  = xid;
  event.xclient.message_type = XInternAtom (display, "_WIN_LAYER", False);
  event.xclient.format = 32;

  event.xclient.data.l[0] = 8;

  return XSendEvent (display, DefaultRootWindow(display), False,
                     SubstructureRedirectMask|SubstructureNotifyMask, &event);
}

#define ZERO4 0, 0, 0, 0

int unfocus = 0;

// Main Window
class ShutdownDialog : public FXDialogBox {

  // Macro for class hierarchy declarations
  FXDECLARE(ShutdownDialog)

private:
  FXVerticalFrame   *cont;                    // Container
  FXGroupBox        *logongrp;                // Logon information group
  FXVerticalFrame   *logongrpc;               // Logon information group
  FXMatrix          *buttons;                 // Button container
  FXLabel           *actionlbl1;
  FXLabel           *actionlbl2;
  I2KListBox        *listbox;


protected:
  ShutdownDialog(){}

public:

  // Message handlers

  long onClickOption(FXObject*,FXSelector,void*);

  long onAccept(FXObject*,FXSelector,void*);
  long onClose(FXObject*,FXSelector,void*);

  long onFocus(FXObject*,FXSelector,void*);  
  long onUnfocus(FXObject*,FXSelector,void*);

  long onRealUnfocus(FXObject*,FXSelector,void*);
  long onRealFocus(FXObject*,FXSelector,void*);

  long onConfigure(FXObject*,FXSelector,void*);


public:

  // Messages for our class
  enum {
    ID_MAINWIN=FXDialogBox::ID_LAST,
    ID_CLICK_OPTION,
    ID_ACCEPT,
    ID_UNFOCUS,
    ID_FOCUS
  };

public:

  // ShutdownDialog's constructor
  ShutdownDialog(FXWindow* owner);

  // Initialize
  virtual void create();

  virtual ~ShutdownDialog();
};



// Message Map for the CtrlAltDel Window class
FXDEFMAP(ShutdownDialog) ShutdownDialogMap[] = {
  FXMAPFUNC(SEL_COMMAND, ShutdownDialog::ID_CLICK_OPTION, ShutdownDialog::onClickOption),
  FXMAPFUNC(SEL_COMMAND, ShutdownDialog::ID_ACCEPT, ShutdownDialog::onAccept),
  FXMAPFUNC(SEL_UNMAP, 0, ShutdownDialog::onClose),


  FXMAPFUNC(SEL_TIMEOUT, ShutdownDialog::ID_FOCUS, ShutdownDialog::onRealUnfocus),
  FXMAPFUNC(SEL_TIMEOUT, ShutdownDialog::ID_UNFOCUS, ShutdownDialog::onRealUnfocus),

  FXMAPFUNC(SEL_FOCUSIN, 0, ShutdownDialog::onFocus),
  FXMAPFUNC(SEL_FOCUSOUT, 0, ShutdownDialog::onUnfocus),
  FXMAPFUNC(SEL_CONFIGURE, 0, ShutdownDialog::onConfigure),

  //FXMAPFUNC(SEL_FOCUSOUT, 0, ShutdownDialog::onUnfocus),
};

long ShutdownDialog::onFocus(FXObject* sender,FXSelector sel,void* ptr) {
  unfocus = 0;
  FXDialogBox::onFocusIn(sender, sel, ptr);
  return 1;
}


long ShutdownDialog::onUnfocus(FXObject* sender,FXSelector sel,void* ptr) {
  unfocus = 1;
  FXDialogBox::onFocusOut(sender, sel, ptr);
  getApp()->addTimeout(this,ShutdownDialog::ID_UNFOCUS,20);
  //onRealUnfocus(sender, sel, ptr);
  return 1;
}

long ShutdownDialog::onRealUnfocus(FXObject* sender,FXSelector sel,void* ptr) {
  if (unfocus) {
    FXDialogBox::onFocusOut(sender, sel, ptr);
    FXDialogBox::onCmdCancel(cadwindow, sel, ptr);
  }
  return 1;
}

// Macro for the CtrlAltDelApp class hierarchy implementation
FXIMPLEMENT(ShutdownDialog,FXDialogBox,ShutdownDialogMap,ARRAYNUMBER(ShutdownDialogMap))

// Construct a ShutdownDialog
ShutdownDialog::ShutdownDialog(FXWindow* owner):FXDialogBox(owner, "Shut Down Windows", DECOR_TITLE|DECOR_BORDER|DECOR_MENU|DECOR_CLOSE, 0, 0, 0, 0, 0, 0, 0, 0) {
  FXIcon* bannericon = new FXGIFIcon(getApp(), _BANNER_IMG,0,IMAGE_OPAQUE);
  FXIcon* shuticon = new FXGIFIcon(getApp(), shutdlg);
  bannericon->crop(0, 0, 411, 77);
  bannericon->render();
  bannericon->create();

  new FXLabel(this, "", bannericon, LABEL_NORMAL|LAYOUT_FILL_X|LAYOUT_CENTER_X, 0, 0, 411, 0, 0, 0, 0, 2);
  FXHorizontalFrame* horcont = new FXHorizontalFrame(this, LAYOUT_FILL_X, 0, 0, 0, 0, 20, 85, 9, 41, 14, 0);
  new FXLabel(horcont, "", shuticon, LABEL_NORMAL, 0, 0, 411, 0, 0, 0, 0, 2);

  FXVerticalFrame* vercont = new FXVerticalFrame(horcont, LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  
  new FXLabel(vercont, "What do you want the computer to do?", NULL, LABEL_NORMAL, 0, 0, 0, 0, 0, 0, 0, 8);

  listbox = new I2KListBox(vercont,this,ID_CLICK_OPTION,COMBOBOX_INSERT_LAST|LAYOUT_FILL_X|COMBOBOX_STATIC|FRAME_SUNKEN|FRAME_THICK, 0, 0, 0, 0, 2, 0, 2, 1);


  listbox->insertItem(_OPTION_LOGOFF,   "Log off");
  listbox->insertItem(_OPTION_SHUTDOWN, "Shut down");
  listbox->insertItem(_OPTION_RESTART,  "Restart");
  listbox->insertItem(_OPTION_STANDBY,  "Stand by");

  listbox->setNumVisible(listbox->getNumItems());
  listbox->setCurrentItem(_OPTION_SHUTDOWN);

  actionlbl1 = new FXLabel(vercont, "Ends your session and shuts down Windows so that", NULL, JUSTIFY_LEFT|LABEL_NORMAL, 0, 0, 0, 0, 0, 0, 11, -1);
  actionlbl2 = new FXLabel(vercont, "you can safely turn off power.", NULL, LABEL_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0);

  FXHorizontalFrame* btncont = new FXHorizontalFrame(this, LAYOUT_RIGHT, 0, 0, 0, 0, 0, 10, 0, 11, 6, 0);
  FXButton* okbtn = new FXButton(btncont, "OK", NULL, this, ID_ACCEPT, BUTTON_DEFAULT|BUTTON_NORMAL|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0, 0, 75, 23, 0, 0, 0, 0);
  new FXButton(btncont, "Cancel", NULL, this, ID_CANCEL, BUTTON_NORMAL|BUTTON_DEFAULT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0, 0, 75, 23, 0, 0, 0, 0);
  new FXButton(btncont, "&Help", NULL, this, ID_CANCEL, BUTTON_NORMAL|BUTTON_DEFAULT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0, 0, 75, 23, 0, 0, 0, 0);

  okbtn->setFocus();
}


ShutdownDialog::~ShutdownDialog() {
}

// Create and initialize
void ShutdownDialog::create() {
  FXDialogBox::create();
}

long ShutdownDialog::onClickOption(FXObject* sender, FXSelector sel, void* ptr)
{
  //printf("%d\n");
  //
  intptr_t option = (intptr_t)ptr;
  if (option == _OPTION_LOGOFF) {
	actionlbl1->setText("Ends your session, leaving the computer running on");
	actionlbl2->setText("full power.");
	return 1;
  } else if (option == _OPTION_SHUTDOWN) {
	actionlbl1->setText("Ends your session and shuts down Windows so that");
	actionlbl2->setText("you can safely turn off power.");
	return 1;
  } else if (option == _OPTION_RESTART) {
	actionlbl1->setText("Ends your session, shuts down Windows and starts");
	actionlbl2->setText("Windows again.");
	return 1;
  } else if (option == _OPTION_STANDBY) {
	actionlbl1->setText("Maintains your session, keeping the computer running");
	actionlbl2->setText("on low power with data still in memory.");
	return 1;
  } else {
	puts("What the helly :joy: :joy: :joy: :joy: :joy: :joy: :joy: :joy:");
  }
  return 0;
}

long ShutdownDialog::onClose(FXObject* sender, FXSelector sel, void* ptr)
{
  chkwindow->close();
  return 0;
}

long ShutdownDialog::onConfigure(FXObject* sender, FXSelector sel, void* ptr)
{
  unfocus = 0;
  FXDialogBox::onConfigure(sender, sel, ptr);
  return 0;
}


long ShutdownDialog::onAccept(FXObject* sender, FXSelector sel, void* ptr)
{
  int option = listbox->getCurrentItem();
  if (option == _OPTION_LOGOFF) {
	system("~/.icewm/programs/shutdown/action logoff &");
  } else if (option == _OPTION_SHUTDOWN) {
	system("~/.icewm/programs/shutdown/action shutdown &");
  } else if (option == _OPTION_RESTART) {
	system("~/.icewm/programs/shutdown/action restart &");
  } else if (option == _OPTION_STANDBY) {
	system("~/.icewm/programs/shutdown/action standby &");
  } else {
	puts("What the helly :joy: :joy: :joy: :joy: :joy: :joy: :joy: :joy:");
  }

  FXDialogBox::onCmdAccept(sender, sel, ptr);
  getApp()->exit(0);
  return 0;
}

class FadeWindow : public FXMainWindow {
  FXDECLARE(FadeWindow)
private:
  FXImageFrame      *imgframe;
protected:
  FadeWindow(){}

public:

  // Message handlers

  long onClickOption(FXObject*,FXSelector,void*);

  long onAccept(FXObject*,FXSelector,void*);


  long onFocus(FXObject*,FXSelector,void*);
  long onUnfocus(FXObject*,FXSelector,void*);

  long onRealUnfocus(FXObject*,FXSelector,void*);
  

public:

  // Messages for our class
  enum {
    ID_FADEWIN=FXMainWindow::ID_LAST,
    ID_FOCUS,
    ID_UNFOCUS
  };

public:

  FadeWindow(FXApp* a);

  // Initialize
  virtual void create();

  virtual ~FadeWindow();
};


FadeWindow::FadeWindow(FXApp* a):FXMainWindow(a, "Shut Down Windows", NULL, NULL, DECOR_TITLE|DECOR_BORDER|DECOR_MENU|DECOR_CLOSE, 0, 0, 0, 0, 0, 0, 0, 0) {
}

FadeWindow::~FadeWindow() {
}

void FadeWindow::create() {
  FXMainWindow::create();

  FXApp* ptrapp = getApp();

  FXWindow* root = ptrapp->getRootWindow();
  int rootw = root->getWidth();
  int rooth = root->getHeight();

  FXImage* image = new FXImage(ptrapp, NULL, IMAGE_OPAQUE|IMAGE_NEAREST|IMAGE_SHMP|IMAGE_SHMI, rootw, rooth);
  image->create();

  FXDCWindow dc(image);
  
  dc.clipChildren(FALSE);
  dc.setFunction(BLT_SRC);
  dc.drawArea(ptrapp->getRootWindow(), 0, 0, rootw, rooth, 0, 0);
  
  dc.setFillStyle(FILL_STIPPLED);
  dc.setStipple(STIPPLE_GRAY);
  dc.setForeground(FXRGB(0,0,0));
  dc.fillRectangle(0, 0, rootw, rooth);
  
  image->restore();

  dc.setFillStyle(FILL_SOLID);
  dc.clipChildren(TRUE);

  imgframe = new FXImageFrame(chkwindow, image, FRAME_NONE);
  imgframe->create();

  FXMainWindow::create();
}

// Message Map for the CtrlAltDel Window class
FXDEFMAP(FadeWindow) FadeWindowMap[] = {
};

// Macro for the CtrlAltDelApp class hierarchy implementation
FXIMPLEMENT(FadeWindow,FXMainWindow,FadeWindowMap,ARRAYNUMBER(FadeWindowMap))

// Here we begin
int main(int argc,char *argv[]) {
  FXApp application("CtrlAltDel", "Ice2KProj");
  FXApp* ptrapp = &application;

  application.init(argc,argv);

  xdisplay = (Display*)ptrapp->getDisplay();
  chkwindow = new FadeWindow(ptrapp);

  cadwindow = new ShutdownDialog(chkwindow);
  
  application.create();

  chkwindow->show();
  setOnTop(xdisplay, chkwindow->id());

  cadwindow->execute(PLACEMENT_SCREEN);
  cadwindow->setFocus();

  return application.run();
}


