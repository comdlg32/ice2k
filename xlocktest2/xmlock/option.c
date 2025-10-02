#if 0
static const char sccsid[] = "@(#)option.c	4.00 97/01/01 xlockmore";

#endif

/*-
 * option.c - option stuff for xmlock, the gui interface to xlock.
 *
 * Copyright (c) 1996 by Charles Vidal
 *
 * See xlock.c for copying information.
 *
 * Revision History:
 * Nov-96: written.
 *
 */

/*-
 * if you want add some options, so it's not really difficult
 * 1: Learn Motif ... but only if you want to :)
 * 2: add a name of widget after geometry,font,program
 * 3: add a string ( char * ) after *inval=NULL,*geo=NULL
 * 4: add a callback like f_username with your new string
 * 5: copy and paste after
   geometry = XtVaCreateManagedWidget("geometry",
		xmPushButtonGadgetClass,options, NULL);
   XtAddCallback(geometry,
		XmNactivateCallback, f_geometry, NULL);
 *    change geometry by your name of widget, and f_geometry by the name
 *    of your new callback
 * 6: go to the church , call god for a miracle :)
 * GOOD LUCK
 */

#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef VMS
#include <descrip.h>
#include <lib$routines.h>
#include <inttypes.h>
#else
#include <stdint.h>
#endif

#ifdef HAVE_MOTIF
/* #include <Xm/XmAll.h> Does not work on my version of Lesstif */
#include <Xm/RowColumn.h>
#include <Xm/SelectioB.h>
#include <Xm/CascadeB.h>
#include <Xm/PushBG.h>
#include <Xm/List.h>
#include <Xm/ToggleBG.h>
#else
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#ifdef HAVE_LIB_XAW3DXFT
#include <X11/Xaw3dxft/Form.h>
#include <X11/Xaw3dxft/Box.h>
#include <X11/Xaw3dxft/SimpleMenu.h>
#include <X11/Xaw3dxft/MenuButton.h>
#include <X11/Xaw3dxft/SmeBSB.h>
#include <X11/Xaw3dxft/Command.h>
#define HAVE_ATHENA 1
#elif defined(HAVE_LIB_XAW3D)
#include <X11/Xaw3d/Form.h>
#include <X11/Xaw3d/Box.h>
#include <X11/Xaw3d/SimpleMenu.h>
#include <X11/Xaw3d/MenuButton.h>
#include <X11/Xaw3d/SmeBSB.h>
#include <X11/Xaw3d/Command.h>
#define HAVE_ATHENA 1
#elif defined(HAVE_LIB_NEXTAW)
#include <X11/neXtaw/Form.h>
#include <X11/neXtaw/Box.h>
#include <X11/neXtaw/SimpleMenu.h>
#include <X11/neXtaw/MenuButton.h>
#include <X11/neXtaw/SmeBSB.h>
#include <X11/neXtaw/Command.h>
#define HAVE_ATHENA 1
#elif defined(HAVE_LIB_XAWPLUS)
#include <X11/XawPlus/Form.h>
#include <X11/XawPlus/Box.h>
#include <X11/XawPlus/SimpleMenu.h>
#include <X11/XawPlus/MenuButton.h>
#include <X11/XawPlus/SmeBSB.h>
#include <X11/XawPlus/Command.h>
#define HAVE_ATHENA 1
#elif defined(HAVE_ATHENA) /* HAVE_LIB_XAW */
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/Command.h>
#endif
#endif

#include "option.h"
#ifdef HAVE_ATHENA
#define check_width 16
#define check_height 16
static unsigned char check_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x38, 0x00, 0x1e, 0x08, 0x0f,
  0x8c, 0x07, 0xde, 0x03, 0xfe, 0x03, 0xfc, 0x01, 0xf8, 0x00, 0xf0, 0x00,
  0x70, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00};
int toggleState[numToggleNames];
#endif

/*#define OPTIONS 8*/
/*#define REGULAR_OPTIONS (OPTIONS-2)*/
#define NBPROGNAME 3
#define MAXNAMES 10000

/****************************************
 * This describe the options of user menu
 * but not the radio button
 * it associates the message , the command and the user data
 * this structure is describe in option.h
 ****************************************/

static char *fileTypes[] =
{
	(char *) "Exit"
};

#define numFileTypes (sizeof(fileType) / sizeof(fileType[0]))

static char *prognames[NBPROGNAME] =
{
	(char *) "fortune",
	(char *) "finger",
	(char *) "echo hello world"
};

Widget menuOption;
Widget toggles[numToggleNames];
static Widget options[numOptions];

/* extern variables */
extern Widget topLevel;
#ifdef HAVE_MOTIF
extern void setupOption(Widget menuBar);
void exitCallback(Widget w, XtPointer client_data, XtPointer call_data);



/* Widget */
static Widget PromptDialog, FontSelectionDialog, ProgramSelectionDialog;



/* string temp */
static char **whichone;


static void
managePrompt(char *s)
{
	int ac;
	Arg args[3];
	XmString    label_str1, label_str2;

	ac = 0;
	label_str1 = XmStringCreateSimple(s);
	XtSetArg(args[ac], XmNselectionLabelString, label_str1);
	ac++;
	if (*whichone != NULL) {
		label_str2 = XmStringCreateSimple(*whichone);
		XtSetArg(args[ac], XmNtextString, label_str2);
		ac++;
	} else {
		label_str2 = XmStringCreateSimple((char *) "");
		XtSetArg(args[ac], XmNtextString, label_str2);
		ac++;
	}

	XtSetValues(PromptDialog, args, ac);
/* PURIFY 4.0.1 on Solaris 2 reports a 71 byte memory leak on the next line. */
	XtManageChild(PromptDialog);
	XmStringFree(label_str1);
	XmStringFree(label_str2);
}

/* CallBack */
static void
optionListener(Widget w, XtPointer client_data, XtPointer call_data)
{
	whichone = &Opt[(long) client_data].userdata;
	managePrompt(Opt[(long) client_data].description);
}

#if 0
static int  loadfont = 0;

/******************************
 * Callback for the font dialog
******************************/
static void
f_fontdia(Widget w, XtPointer client_data, XtPointer call_data)
{
	int         numdirnames, i;
	char      **dirnames;
	XmString    label_str;
	Widget      listtmp;
	Cursor      tmp;

	whichone = &Opt[OPTIONS].userdata;
	if (!loadfont) {
		tmp = XCreateFontCursor(XtDisplay(topLevel), XC_watch);
		XDefineCursor(XtDisplay(topLevel), XtWindow(topLevel), tmp);
		dirnames = XListFonts(XtDisplay(topLevel), "*", MAXNAMES, &numdirnames);
		listtmp = XmSelectionBoxGetChild(FontSelectionDialog, XmDIALOG_LIST);
		for (i = 0; i < numdirnames; i++) {
			label_str = XmStringCreateSimple(dirnames[i]);
			XmListAddItemUnselected(listtmp, label_str, 0);
			XmStringFree(label_str);
		}
		tmp = XCreateFontCursor(XtDisplay(topLevel), XC_left_ptr);
		XDefineCursor(XtDisplay(topLevel), XtWindow(topLevel), tmp);
		loadfont = 1;
		XFreeFontNames(dirnames);
	}
	XtManageChild(FontSelectionDialog);
}

/******************************
 * Callback for the program dialog
******************************/
static void f_programdia(Widget w, XtPointer client_data, XtPointer call_data) {
	/*whichone = &c_Options[REGULAR_OPTIONS + 1];*/
/* PURIFY 4.0.1 on Solaris 2 reports a 71 byte memory leak on the next line. */
	XtManageChild(ProgramSelectionDialog);
}
#endif

static void
f_Dialog(Widget w, XtPointer client_data, XtPointer call_data)
{
	static char *quoted_text;
	char       *nonquoted_text = (char *) NULL;
	XmSelectionBoxCallbackStruct *scb =
		(XmSelectionBoxCallbackStruct *) call_data;

	if (whichone != NULL)
		XtFree(*whichone);
	XmStringGetLtoR(scb->value, XmSTRING_DEFAULT_CHARSET, &nonquoted_text);
	if ((quoted_text = (char *) malloc(strlen(nonquoted_text) + 3)) ==
			NULL) {
		(void) fprintf(stderr, "Memory error\n");
		return;
	}
	(void) sprintf(quoted_text, "\"%s\"", nonquoted_text);
	free(nonquoted_text);
	*whichone = quoted_text;
}

#elif defined(HAVE_ATHENA)
static Pixmap checkPixmap = None;
void toggleCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
	long position = (long) client_data;

	toggleState[position] = (toggleState[position] != 0) ? 0: 1;
	XtVaSetValues(toggles[position],
		XtNrightBitmap, (toggleState[position]) ? checkPixmap : None,
		NULL);
}
#endif

/* Setup Widget */
void
setupOption(Widget menuBar)
{
	int         i, ac = 0;
	Widget      listtmp, pulldownMenu, exitWidget;
	XtPointer   iptr = 0;
#ifdef HAVE_MOTIF
	Arg         args[15];
	XmString    label_str;

/**
 ** Popup Menu File
 **/
	pulldownMenu = XmCreatePulldownMenu(menuBar, (char *) "PopupFile",
		(Arg *) NULL, 0);
	label_str = XmStringCreateSimple((char *) "File");
	XtVaCreateManagedWidget("File",
		xmCascadeButtonWidgetClass, menuBar,
		XmNlabelString, label_str,
		XmNsubMenuId, pulldownMenu, NULL);
	XmStringFree(label_str);
	exitWidget = XtVaCreateManagedWidget("Exit",
		xmPushButtonGadgetClass, pulldownMenu, NULL);
	XtAddCallback(exitWidget,
		XmNactivateCallback, exitCallback, NULL);

	pulldownMenu = XmCreatePulldownMenu(menuBar, (char *) "PopupOptions",
		(Arg *) NULL, 0);
	label_str = XmStringCreateSimple((char *) "Options");
	XtVaCreateManagedWidget("Options",
		xmCascadeButtonWidgetClass, menuBar,
		XmNlabelString, label_str,
		XmNsubMenuId, pulldownMenu, NULL);
	XmStringFree(label_str);

	for (i = 0; i < (int) (sizeof(Opt)/sizeof(OptionStruct)); i++) {
		options[i] = XtVaCreateManagedWidget(Opt[i].description,
			/*xmToggleButtonGadgetClass, pulldownMenu, NULL);*/
			xmPushButtonGadgetClass, pulldownMenu, NULL);
		XtAddCallback(options[i],
			XmNactivateCallback, optionListener, iptr++);
	}
/* PURIFY 4.0.1 on Solaris 2 reports a 12 byte memory leak on the next line. */
	PromptDialog = XmCreatePromptDialog(topLevel, (char *) "PromptDialog",
		args, ac);
	XtAddCallback(PromptDialog, XmNokCallback, f_Dialog, NULL);

/* PURIFY 4.0.1 on Solaris 2 reports a 28 byte and a 12 byte memory leak on
   the next line. */
	FontSelectionDialog = XmCreateSelectionDialog(topLevel,
		(char *) "FontSelectionDialog", (Arg *) NULL, 0);
	XtAddCallback(FontSelectionDialog, XmNokCallback, f_Dialog, NULL);

/* PURIFY 4.0.1 on Solaris 2 reports a 38 byte memory leak on the next line. */
	ProgramSelectionDialog = XmCreateSelectionDialog(topLevel,
		(char *) "ProgramSelectionDialog", (Arg *) NULL, 0);
	XtAddCallback(ProgramSelectionDialog, XmNokCallback, f_Dialog, NULL);
	listtmp = XmSelectionBoxGetChild(ProgramSelectionDialog, XmDIALOG_LIST);
	for (i = 0; i < NBPROGNAME; i++) {
		label_str = XmStringCreateSimple(prognames[i]);
		XmListAddItemUnselected(listtmp, label_str, 0);
		XmStringFree(label_str);
	}

/**
 ** Make the menu with all options boolean
***/
	pulldownMenu = XmCreatePulldownMenu(menuBar, (char *) "PopupSwitches",
		(Arg *) NULL, 0);
	label_str = XmStringCreateSimple((char *) "Switches");
	XtVaCreateManagedWidget("Switches",
		xmCascadeButtonWidgetClass, menuBar,
		XmNlabelString, label_str,
		XmNsubMenuId, pulldownMenu, NULL);
	XmStringFree(label_str);

	for (i = 0; i < numToggleNames; i++) {
		toggles[i] = XtVaCreateManagedWidget(toggleNames[i],
			xmToggleButtonGadgetClass, pulldownMenu, NULL);
	}
#elif defined(HAVE_ATHENA)
	Widget w, fileMenu, fileLabel, switchesMenu, switchesLabel;
	Display *dpy = XtDisplay(topLevel);

	if (checkPixmap == None) {
		checkPixmap = XCreateBitmapFromData(dpy,
			RootWindow(dpy, DefaultScreen(dpy)),
			(char *)check_bits, check_width, check_height);
	}
	fileLabel = XtVaCreateManagedWidget("File",
		menuButtonWidgetClass, menuBar,
		XtNborderWidth, 0,
		/*XtNwidth, 202,*/
		NULL);
	fileMenu = XtVaCreatePopupShell("menu",
		simpleMenuWidgetClass, fileLabel, NULL);
	for (i = 0; i < 1; i++) {
		w = XtVaCreateManagedWidget(fileTypes[i],
			smeBSBObjectClass, fileMenu, NULL);
		XtAddCallback(w,
			XtNcallback, (XtCallbackProc) exitCallback,
			(XtPointer) (size_t) i);
	}
	switchesLabel = XtVaCreateManagedWidget("Switches",
		menuButtonWidgetClass, menuBar,
		XtNborderWidth, 0,
		XtNfromHoriz, fileLabel,
		/*XtNwidth, 202,*/
		NULL);
	switchesMenu = XtVaCreatePopupShell("menu",
		simpleMenuWidgetClass, switchesLabel, NULL);
	for (i = 0; i < numToggleNames; i++) {
		toggleState[i] = 0;
		toggles[i] = XtVaCreateManagedWidget(toggleNames[i],
			smeBSBObjectClass, switchesMenu, NULL);
		XtVaSetValues(toggles[i],
			XtNrightBitmap, (toggleState[i]) ? checkPixmap : None,
			XtNrightMargin, 24, NULL); /* 16 + 2 * 4 */
		XtAddCallback(toggles[i],
			XtNcallback, (XtCallbackProc) toggleCallback,
			(XtPointer) (size_t) i);
	}
	/*createList(menuBar, menu, label, "File", fileTypes, 1, NULL);*/
	/*XtAddCallback(menu,
		XtNcallback, exitCallback,
		(XtPointer) NULL);*/
	/*createList(menuBar, menu2, label2, "Switches", toggleNames, numToggleNames, menuForm);*/
#endif
}
