#include <fx.h>
#include <X11/Xlib.h>

#include <FXPNGIcon.h>
#include <FXId.h>
#include <iostream>
#include <FXApp.h>
#include <FXFont.h>
#include <limits.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <locale.h>

#include "res/foxres.h"

// set windows version here!
//#define _WINVER_WIN2K
//#define _WINVER_WINXP
//#define _WINVER_WINSRV03


void formatnum(long unsigned num, char *buffer) {
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


/* namespace FX {

class FXAPI Ice2KVer : public FXMainWindow {

};

FXDEFMAP(Ice2KVer) Ice2KVerMap[]={
  FXMAPFUNC(SEL_COMMAND,Ice2KVer::ID_NEXT,Ice2KVer::onCmdNext),
  FXMAPFUNC(SEL_UPDATE,Ice2KVer::ID_NEXT,Ice2KVer::onUpdNext),
  FXMAPFUNC(SEL_COMMAND,Ice2KVer::ID_BACK,Ice2KVer::onCmdBack),
  FXMAPFUNC(SEL_UPDATE,Ice2KVer::ID_BACK,Ice2KVer::onUpdBack),
  FXMAPFUNC(SEL_UPDATE,Ice2KVer::ID_ACCEPT,Ice2KVer::onUpdFinish),
};


// Object implementation
FXIMPLEMENT(Ice2KVer,FXDialogBox,ice2kverMap,ARRAYNUMBER(ice2kverMap))

} */



// thank you stack overflow
// https://stackoverflow.com/questions/20733215/how-to-make-a-window-always-on-top
// since fox 1.6 doesnt support setting the window on top, we need to use the x11 api directly

#define _NET_WM_STATE_REMOVE        0    /* remove/unset property */
#define _NET_WM_STATE_ADD           1    /* add/set property */
#define _NET_WM_STATE_TOGGLE        2    /* toggle property  */


// change a window's _NET_WM_STATE property so that it can be kept on top.
// @display: x11 display singleton.
// @xid    : the window to set on top.
Status setOnTop(Display* display, Window xid)
{
    XEvent event;
    event.xclient.type = ClientMessage;
    event.xclient.serial = 0;
    event.xclient.send_event = True;
    event.xclient.display = display;
    event.xclient.window  = xid;
    event.xclient.message_type = XInternAtom (display, "_NET_WM_STATE", False);
    event.xclient.format = 32;

    event.xclient.data.l[0] = _NET_WM_STATE_ADD;
    event.xclient.data.l[1] = XInternAtom (display, "_NET_WM_STATE_ABOVE", False);
    event.xclient.data.l[2] = 0; //unused.
    event.xclient.data.l[3] = 0;
    event.xclient.data.l[4] = 0;

    return XSendEvent (display, DefaultRootWindow(display), False,
                       SubstructureRedirectMask|SubstructureNotifyMask, &event);
}


int endswith(char* str, char* suff) {
	char* p;

	if ( (p = strstr(str, suff)) != NULL) {
		size_t plen = strlen(p);
		size_t sufflen = strlen(suff);

		if (plen == sufflen) return 1;
	}

	return 0;
}



int main(int argc, char *argv[]) {
  int winver = 0;
  if (endswith(argv[0], "winver"))
    winver = 1; // for future use...
		    // TODO: put ice2kver and winver in one binary. show windows larp when ran
		    //       from the winver symlink, otherwise, show ice2k.sys version.

  FXApp application("ice2kver", "Ice2KProj");
  FXApp* ptrapp = &application;

  #ifdef _WINVER_WIN2K
    #define _BANNER banner2k
  #endif
  #ifdef _WINVER_WINXP
    #define _BANNER banner
  #endif
  #ifdef _WINVER_WINSRV03
    #define _BANNER banner03
  #endif

  FXIcon* bannericon = new FXGIFIcon(&application, _BANNER,0,IMAGE_OPAQUE);

  application.init(argc, argv);
  // FXMainWindow *main=new FXMainWindow(&application, "About Windows", NULL, NULL, DECOR_TITLE|DECOR_BORDER|DECOR_MENU|DECOR_CLOSE, 0, 0, 413, 304);
  FXMainWindow *main=new FXMainWindow(&application, "About Windows", NULL, NULL, DECOR_TITLE|DECOR_BORDER|DECOR_MENU|DECOR_CLOSE, 0, 0, 0, 0);

  new FXLabel(main, "", bannericon, LAYOUT_CENTER_X, 0, 0, 0, 0, 0, 0, 0);

  FXVerticalFrame* winverinfo = new FXVerticalFrame(main, LAYOUT_FILL_X, 0, 0, 0, 0, 53, 9, 10, 0, 0, 0);
  #ifdef _WINVER_WIN2K
    new FXLabel(winverinfo, "Microsoft (R) Windows", NULL, 0, 0, 0, 0, 0, 0, 0, 0, 3);
    new FXLabel(winverinfo, "Version 5.0 (Build 2195: Service Pack 4)", NULL, 0, 0, 0, 0, 0, 0, 0, 0, 2);
    new FXLabel(winverinfo, "Copyright (C) 1981-1999 Microsoft Corp.", NULL, 0, 0, 0, 0, 0, 0, 0, 0, 3);
    new FXLabel(winverinfo, " ", NULL, 0, 0, 0, 0, 0, 0, 0, 0, 3);
    new FXLabel(winverinfo, " ", NULL, 0, 0, 0, 0, 0, 0, 0, 0, 1);
  #else

  #ifdef _WINVER_WINSRV03
    #define _WINVER_WINXP
  #endif

  #ifdef _WINVER_WINXP
    new FXLabel(winverinfo, "Microsoft ® Windows", NULL, 0, 0, 0, 0, 0, 0, 0, 0, 3);
    new FXLabel(winverinfo, "Version 5.2 (Build 3790.srv03_sp1_rtm.050324-1447 : Service Pack 1)", NULL, 0, 0, 0, 0, 0, 1, 0, 0, 2);
    new FXLabel(winverinfo, "Copyright © 1985-2005 Microsoft Corporation", NULL, 0, 0, 0, 0, 0, 0, 0, 0, 3);
    new FXLabel(winverinfo, " ", NULL, 0, 0, 0, 0, 0, 0, 0, 0, 3);
    new FXLabel(winverinfo, " ", NULL, 0, 0, 0, 0, 0, 0, 0, 0, 1);
  #endif

  // #ifdef _WINVER_WINSRV03
  //   #undef _WINVER_WINXP
  // #endif // we are gonna use this later anyway, so why not?
  #endif

  #ifdef _WINVER_WIN2K
    FXHorizontalFrame* eulacont1 = new FXHorizontalFrame(winverinfo, LAYOUT_MIN_WIDTH, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1);
    FXLabel* abouttext1 = new FXLabel(eulacont1, "This product is licensed to:", NULL, 0, 0, 0, 0, 0, 0, 0, 0, 2);
  #else
  #ifdef _WINVER_WINXP // im sorry...
    FXHorizontalFrame* eulacont1 = new FXHorizontalFrame(winverinfo, LAYOUT_MIN_WIDTH, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1);
    FXLabel* abouttext1 = new FXLabel(eulacont1, "This product is licensed under the terms of the ", NULL, 0, 0, 0, 0, 0, 0, 0, 0, -1);
    FXLabel* eulalink1 = new FXLabel(eulacont1, "End-User", NULL, 0, 0, 0, 0, 0, 0, 0, 0, -1);

    FXColor linkcolor;

    if ( ( ptrapp->getSelbackColor() == FXRGB(10,35,104)) || ( ptrapp->getSelbackColor() == FXRGB(0,0,123)) ) {
      linkcolor = FXRGB(0,0,128);
    } else {
      linkcolor = ptrapp->getSelbackColor();
    }

    FXFrame* eulaunderscore1 = new FXFrame(eulacont1, LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FIX_X|LAYOUT_FIX_Y, 0, 0, 1, 1, 0, 0, 0, 0);
    eulalink1->setTextColor(linkcolor);
    eulaunderscore1->setBackColor(linkcolor);

    FXHorizontalFrame* eulacont2 = new FXHorizontalFrame(winverinfo, LAYOUT_MIN_WIDTH, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1);
    FXLabel* eulalink2 = new FXLabel(eulacont2, "License Agreement", NULL, 0, 0, 0, 0, 0, 0, 0, 0, 5);
    FXLabel* abouttext2 = new FXLabel(eulacont2, " to:", NULL, 0, 0, 0, 0, 0, 0, 0, 0, 5);

    FXFrame* eulaunderscore2 = new FXFrame(eulacont2, LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FIX_X|LAYOUT_FIX_Y, 0, 0, 1, 1, 0, 0, 0, 0);

    eulalink2->setTextColor(linkcolor);
    eulaunderscore2->setBackColor(linkcolor);
  #endif
  #endif


  // get hostname thank you https://stackoverflow.com/questions/5190553/linux-c-get-server-hostname
  char hostname[HOST_NAME_MAX+1];
  gethostname(hostname, HOST_NAME_MAX+1);

  #ifdef _WINVER_WINXP
    new FXLabel(winverinfo, getlogin(), NULL, 0, 0, 0, 0, 0, 15, 0, 0, 2);
    new FXLabel(winverinfo, hostname, NULL, 0, 0, 0, 0, 0, 15, 0, 0, 6);
  #else
    new FXLabel(winverinfo, getlogin(), NULL, 0, 0, 0, 0, 0, 0, 0, 0, 2);
    new FXLabel(winverinfo, hostname, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 7);
  #endif

  FXHorizontalFrame* headerspcont = new FXHorizontalFrame(winverinfo, LAYOUT_FILL_X|LAYOUT_FIX_HEIGHT, 0, 58, 0, 2, 0, 1, 0, 0, 0, 0);
  headerspcont->setBackColor(ptrapp->getHiliteColor());

  FXHorizontalFrame* headerspline1 = new FXHorizontalFrame(headerspcont, LAYOUT_FILL_X|LAYOUT_FIX_HEIGHT, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0);
  headerspline1->setBackColor(ptrapp->getShadowColor());


  struct sysinfo sys_info;

  if( sysinfo(&sys_info) != 0)
    perror("sysinfo");

  //printf("%lu\n", sys_info.totalram);
  char physmemtext[1024];
  formatnum(sys_info.totalram, physmemtext);

  //printf("%s\n", physmemtext);

  char *tmp = strdup(physmemtext);

  //strcpy("Physical memory available to Windows:  ", physmemtext);
  strcpy(physmemtext, "Physical memory available to Windows:    ");
  strcat(physmemtext, tmp);

  free(tmp);

  strcat(physmemtext, " KB");

  //printf("%s\n", physmemtext);

  #ifdef _WINVER_WINXP
  new FXLabel(winverinfo, physmemtext, NULL, 0, 0, 0, 0, 0, 0, 0, 6, 6);
  #else
  new FXLabel(winverinfo, physmemtext, NULL, 0, 0, 0, 0, 0, 0, 0, 5, 6);
  #endif
  FXPacker *okcont=new FXPacker(main,LAYOUT_SIDE_BOTTOM|LAYOUT_RIGHT, 0, 0, 0, 0, 10, 8, 22, 10);

  FXButton* okbutton = new FXButton(okcont, "OK", NULL, &application, FXApp::ID_QUIT, BUTTON_NORMAL|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|BUTTON_DEFAULT, 0, 0, 75, 23, 3, 3, 2, 3);

  okbutton->setFocus();



  application.create();

  #ifdef _WINVER_WINXP 
    eulaunderscore1->setX(eulalink1->getX());
    eulaunderscore1->setY(eulalink1->getY() +  eulalink1->getDefaultHeight() - 1);
    eulaunderscore1->setWidth( eulalink1->getDefaultWidth() );

    eulaunderscore2->setX(eulalink2->getX());
    eulaunderscore2->setY(eulalink2->getY() +  eulalink2->getDefaultHeight() - 7);
    eulaunderscore2->setWidth( eulalink2->getDefaultWidth() );
  /* eulaunderscore1->setX(eulalink1->getX());
  eulaunderscore1->setY(eulalink1->getY() +  eulalink1->getDefaultHeight() - 1);
  eulaunderscore1->setWidth( eulalink1->getDefaultWidth() );

  eulaunderscore2->setX(eulalink2->getX());
  eulaunderscore2->setY(eulalink2->getY() +  eulalink2->getDefaultHeight() - 7);
  eulaunderscore2->setWidth( eulalink2->getDefaultWidth() ); */

  //eulaunderscore1->setWidth();
  #endif

  main->show(PLACEMENT_SCREEN);

  setOnTop((Display*)ptrapp->getDisplay(), main->id());
  
  return application.run();
}
