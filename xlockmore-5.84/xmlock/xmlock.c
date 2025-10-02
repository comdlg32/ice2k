#if 0

static const char sccsid[] = "@(#)xmlock.c	4.08 98/02/18 xlockmore";

#endif

/*-
 * xmlock.c - main file for xmlock, the gui interface to xlock.
 *
 * Copyright (c) 1996 by Charles Vidal
 *
 * See xlock.c for copying information.
 *
 * Revision History:
 * Jul-22: Awt option added.  Also cleaned up variables so I could read
 *   easier.
 * Jun-03: Code added from Xlockup by Thad Phetteplace
 *     tdphette AT dexter.glaci.com
 *   Sun -- works (both timeouts seem connected to the same device)
 *   Linux -- mouse timeout works, keyboard timeout does not work
 *   Cygwin -- mouse timeout does not work, keyboard timeout does not work
 *   Alpha -- "does something"  ;)
 *   Since keyboard timeout does not seem to help, it was removed.
 * Nov-96: Continual minor improvements by Charles Vidal and David Bagley.
 * Oct-96: written.
 */

/*-
  XmLock Problems
  1. Allowing only one in -inroot.  Need a way to kill it.
  2. XLock resources need to be read and used to set initial values.
  3. Integer and floating point and string input.
 */

/*
 * 01/2001
 * I'm confused because the source is old and really not well programming
 * I put boolean option and option in the menubar
 * I have had the option.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef __FreeBSD__
#include <signal.h>
#endif
#include <sys/signal.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define HAVE_UNISTD_H 1
#endif /* HAVE_CONFIG_H */
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#ifdef VMS
#include <descrip.h>
#include <lib$routines.h>
#endif

#ifdef USE_MB
#include <X11/Xlocale.h>
#endif

#ifdef HAVE_MOTIF
#include <Xm/PanedW.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/List.h>
#include <Xm/PushB.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/Scale.h>
#else
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#ifdef HAVE_LIB_XAW3DXFT
#include <X11/Xaw3dxft/Form.h>
#include <X11/Xaw3dxft/Label.h>
#include <X11/Xaw3dxft/Toggle.h>
#include <X11/Xaw3dxft/Box.h>
#include <X11/Xaw3dxft/Scrollbar.h>
#include <X11/Xaw3dxft/SimpleMenu.h>
#include <X11/Xaw3dxft/MenuButton.h>
#include <X11/Xaw3dxft/SmeBSB.h>
#include <X11/Xaw3dxft/Command.h>
#include <X11/Xaw3dxft/Dialog.h>
#define HAVE_ATHENA 1
#elif defined(HAVE_LIB_XAW3D)
#include <X11/Xaw3d/Form.h>
#include <X11/Xaw3d/Label.h>
#include <X11/Xaw3d/Toggle.h>
#include <X11/Xaw3d/Box.h>
#include <X11/Xaw3d/Scrollbar.h>
#include <X11/Xaw3d/SimpleMenu.h>
#include <X11/Xaw3d/MenuButton.h>
#include <X11/Xaw3d/SmeBSB.h>
#include <X11/Xaw3d/Command.h>
#include <X11/Xaw3d/Dialog.h>
#define HAVE_ATHENA 1
#elif defined(HAVE_LIB_NEXTAW)
#include <X11/neXtaw/Form.h>
#include <X11/neXtaw/Label.h>
#include <X11/neXtaw/Toggle.h>
#include <X11/neXtaw/Box.h>
#include <X11/neXtaw/Scrollbar.h>
#undef XtCDelay
#include <X11/neXtaw/SimpleMenu.h>
#include <X11/neXtaw/MenuButton.h>
#include <X11/neXtaw/SmeBSB.h>
#include <X11/neXtaw/Command.h>
#include <X11/neXtaw/Dialog.h>
#define HAVE_ATHENA 1
#elif defined(HAVE_LIB_XAWPLUS)
#include <X11/XawPlus/Form.h>
#include <X11/XawPlus/Label.h>
#include <X11/XawPlus/Toggle.h>
#include <X11/XawPlus/Box.h>
#include <X11/XawPlus/Scrollbar.h>
#include <X11/XawPlus/SimpleMenu.h>
#include <X11/XawPlus/MenuButton.h>
#include <X11/XawPlus/SmeBSB.h>
#include <X11/XawPlus/Command.h>
#include <X11/XawPlus/Dialog.h>
#define HAVE_ATHENA 1
#elif defined(HAVE_ATHENA) /* HAVE_LIB_XAW */
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Scrollbar.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Dialog.h>
/*#include <X11/Xaw/AsciiText.h>*/
#endif
#endif

#if USE_XMU
#include <X11/Xmu/Editres.h>
#endif

#include "option.h"

#ifdef SOLARIS2
extern int  kill(pid_t, int);
extern pid_t  wait(int *);
#endif

#include "bitmaps/m-xlock.xbm"	/* icon bitmap */

#define SKIPDELAY 60000 /* 1 minute */
#define MAXMIN 120

static char *labelPushButtons[] =
{
	(char *) "Launch",
	(char *) "In Root",
	(char *) "In Window",
};
#define numButtons (sizeof(labelPushButtons) / sizeof(labelPushButtons[0]))

enum ACTION {
	LAUNCH=0,
	ROOT,
	WINDOW
	};

/* extern variable */
extern Widget menuOption;
extern Widget toggles[];

static pid_t numberProcess = -1;	/* PID of xlock */

/*************************
 * To know the number of element
 * in the Opt Array
**************************/

/* Widget */
Widget topLevel;
static Widget labelXlock;
static Widget timeoutSlider;
#ifdef HAVE_ATHENA
static Widget modeList, modeListLabel;
static Widget menuBar, exitMenuButton;
extern int toggleState[];
#endif

static time_t timeNow;

/* If this worked on all systems it would be 30 min or 1800 */
static long timeout = 0;

static int mode = 0;

static Widget scrolledListModes, pushButtons[numButtons];

 /*Resource string */
#include "modes.h"

#if 0
static XmStringCharSet char_set = (XmStringCharSet) XmSTRING_DEFAULT_CHARSET;
#endif

/* some resources of buttons and toggles not really good programming :( */

static void checkTime(void);
/* CallBack */

void exitCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
	if (numberProcess != -1) {
		(void) kill(numberProcess, SIGKILL);
		/*(void) wait(&n);*/
	}
	exit(0);
}

static void
callLocker(int choice)
{
	int i;
	char command[500];

#ifdef VMS
	int mask = 17;
	struct dsc$descriptor_d vms_image;
#endif

	(void) sprintf(command, "%s ", XLOCK);

	/* booleans (+/-) options */

#ifdef HAVE_MOTIF
	for (i = 0; i < numToggleNames; i++) {
		if (XmToggleButtonGetState(toggles[i])) {
			(void) strcat(command, "-");
			(void) strcat(command, toggleNames[i]);
			(void) strcat(command, " ");
		}
	}
	for (i = 0; i < numOptions; i++)
		if (Opt[i].userdata != NULL) {
			(void) strcat(command, "-");
			(void) strcat(command, Opt[i].cmd);
			(void) strcat(command, " ");
			(void) strcat(command, Opt[i].userdata);
			(void) strcat(command, " ");
		}
#elif defined(HAVE_ATHENA)
	for (i = 0; i < numToggleNames; i++) {
		if (toggleState[i]) {
			(void) strcat(command, "-");
			(void) strcat(command, toggleNames[i]);
			(void) strcat(command, " ");
		}
	}
#endif

	switch (choice) {
		case LAUNCH:
			/* the default value then nothing to do */
			break;
		case WINDOW:
			(void) strcat(command, "-inwindow ");
			break;
		case ROOT:
			(void) strcat(command, "-inroot ");
			break;
	}
	(void) strcat(command, "-mode ");
	(void) strcat(command, LockProcs[mode].cmdline_arg);
#ifdef VMS
	vms_image.dsc$w_length = strlen(command);
	vms_image.dsc$a_pointer = command;
	vms_image.dsc$b_class = DSC$K_CLASS_S;
	vms_image.dsc$b_dtype = DSC$K_DTYPE_T;
	(void) printf("%s\n", command);
	(void) lib$spawn(&vms_image, 0, 0, &mask);
#else
	if (choice != LAUNCH)
		(void) strcat(command, " & ");
	(void) printf("%s\n", command);
	(void) system(command);
#endif
	if (choice == LAUNCH)
		(void) XtAppAddTimeOut(XtWidgetToApplicationContext(topLevel),
			SKIPDELAY, (XtTimerCallbackProc) checkTime, topLevel);
}

/*--------------------------------------------------------------*/
/* Code taken from Xlockup by Thad Phetteplace			*/
/*	tdphette@dexter.glaci.com used by permission		*/
/* CHECKTIME: This routine is called periodically by the	*/
/*	openwin event handler.  It checks the last access	*/
/*	time/date stamp on the mouse and keyboard devices	*/
/*	and compares it to the timeout value.			*/
/*--------------------------------------------------------------*/

static void checkTime(void)
{
	struct stat statmouse;
#if 0
	struct stat statkeybd;
#endif

	(void) time( &timeNow );		/* get current time */
	/* next does not work on Cygwin */
	(void) stat("/dev/mouse", &statmouse );	/* get mouse status */
#if 0
	/* does not seem to help on Solaris and hinders on Linux */
	(void) stat("/dev/kbd", &statkeybd );	/* get keyboard status */
#endif

	/* check if last access time was larger than timeout */
	if (timeout > 0 && (timeNow-statmouse.st_atime >= timeout) /* &&
			(timeNow-statkeybd.st_atime >= timeout)*/) {
		callLocker(LAUNCH);
	}
#ifdef DEBUG
	printf("timeout %ld,  timeNow - amouse %ld\n",
		timeout, timeNow - statmouse.st_atime);
	printf("timeout %ld,  timeNow - akeybd %ld\n",
		timeout, timeNow - statkeybd.st_atime);
#endif
	(void) XtAppAddTimeOut(XtWidgetToApplicationContext(topLevel),
		SKIPDELAY, (XtTimerCallbackProc) checkTime, topLevel);
}

static void
listenerPushButtons(Widget w, XtPointer client_data, XtPointer call_data)
{
	callLocker((long) client_data);
}

static void
runInLabelWindow(int mode)
{
	char numberWidget[50];
	char str[50];
	int n;

	(void) sprintf(numberWidget, "%ld", XtWindow(labelXlock));
	(void) sprintf(str, "%s", LockProcs[mode].cmdline_arg);
	if (numberProcess != -1) {
		(void) kill(numberProcess, SIGKILL);
		(void) wait(&n);
	}
#ifdef VMS
#define FORK vfork
#else
#define FORK fork
#endif
	if ((numberProcess = FORK()) == -1)
		(void) fprintf(stderr, "Fork error\n");
	else if (numberProcess == 0) {
		(void) execlp(XLOCK, XLOCK, "-parent", numberWidget,
			"-mode", str, "-geometry", WINDOW_GEOMETRY,
			"-delay", "100000",
			"-nolock", "-inwindow", "+install", NULL);
	}
}

#ifdef HAVE_MOTIF
static void
listenerScrolledListModes(Widget w, XtPointer client_data, XtPointer call_data)
{
	mode = ((XmListCallbackStruct *) call_data)->item_position - 1;
	runInLabelWindow(mode);
}
#endif

#ifdef HAVE_ATHENA
static void
modeListener(Widget w, XtPointer clientData, XtPointer callData)
{
	mode = (size_t) clientData;
	XtVaSetValues(modeListLabel,
		XtNlabel, LockProcs[mode].cmdline_arg, NULL);
	runInLabelWindow(mode);
}

static void
createBlank(char **string, int size, char * defaultString, int side) {
	int i, defSize = strlen(defaultString) + 1, midSize;
	char * val;

	*string = (char *) malloc(size);
	val = *string;
	if (side == 0) /* middle */
		midSize = (size - defSize) >> 1;
	else /* right justified */
		midSize = size - defSize;
	for (i = 0; i < size - 1; i++) {
		if (i - midSize >= 0 &&
				(i - midSize < defSize - 1))
			val[i] = defaultString[i - midSize];
		else
			val[i] = ' ';
	}
	val[size - 1] = '\0';
}

static int
findMaxLength() {
	int mode, temp, max;

	max = 0;
	for (mode = 0; mode < numprocs; mode++) {
		temp = strlen(LockProcs[mode].cmdline_arg);
		if (temp > max)
			max = temp;
	}
	return ++max;
}

static void
createList(Widget form, int init)
{
	Widget w;
	int mode, max;
	char *defaultString;
	char string[160];

	max = findMaxLength();
	createBlank(&defaultString, max, LockProcs[init].cmdline_arg, 0);
	modeListLabel = XtVaCreateManagedWidget(defaultString,
		menuButtonWidgetClass, form,
		XtNfromVert, menuBar,
		/*XtNwidth, 202,*/
		NULL);
	free(defaultString);
	modeList = XtVaCreatePopupShell("menu",
		simpleMenuWidgetClass, modeListLabel, NULL);
	for (mode = 0; mode < numprocs; mode++) {
		(void) sprintf(string, "%-14s%s", LockProcs[mode].cmdline_arg,
			LockProcs[mode].desc);
		w = XtVaCreateManagedWidget(string,
			smeBSBObjectClass, modeList, NULL);
		XtAddCallback(w, XtNcallback,
			(XtCallbackProc) modeListener,
			(XtPointer) (size_t) mode);
	/*XtAddCallback(scrolledListModes, XtNcallback,
		listenerScrolledListModes, NULL);*/
	}
}

static int
findMode(char * string)
{
	for (mode = 0; mode < numprocs; mode++)
		if (strcmp(LockProcs[mode].cmdline_arg, string) == 0)
			return mode;
	return 0;
}
#endif

#ifdef HAVE_MOTIF
static void
listenerTimeoutSlider(Widget w, XtPointer clientData, XmScaleCallbackStruct * cbs)
{
	timeout = cbs->value * 60;
}
#endif
/* Setup Form */

/**********************/
static void
setupForm(Widget father)
{
	Arg args[15];
	int i, ac;
	Widget pushButtonRow, exitButton;
	char string[160];
	XtPointer iptr = 0;
#ifdef HAVE_MOTIF
	XmString labelStr;
	XmString tabXmStr[numprocs];

	/* buttons in the bottom */
	ac = 0;
	XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
	ac++;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_FORM);
	ac++;

	menuOption = XmCreateMenuBar(father, (char *) "MenuBar", args,ac);
	XtManageChild(menuOption);

	ac = 0;
	XtSetArg(args[ac], XmNwidth, WINDOW_WIDTH);
	ac++;
	XtSetArg(args[ac], XmNheight, WINDOW_HEIGHT);
	ac++;

	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
	XtSetArg(args[ac], XmNtopWidget, menuOption); ac++;

#if 0
	int ModeLabel;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM); ac++;

	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM); ac++;

	XtSetArg(args[ac], XmNleftWidget, ModeLabel); ac++;
#endif

	labelXlock = XmCreateLabel(father, (char *) "Window", args, ac);
	XtManageChild(labelXlock);

	/* buttons in the bottom */
	ac = 0;
	XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
	ac++;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	pushButtonRow = XmCreateRowColumn(father,
		(char *) "pushButtonRow", args, ac);

	for (i = 0; i < numButtons; i++) {
		ac = 0;
#ifndef USE_MB
		labelStr = XmStringCreate(labelPushButtons[i],
				XmSTRING_DEFAULT_CHARSET);
		XtSetArg(args[ac], XmNlabelString, labelStr);
		ac++;
#endif
		pushButtons[i] = XmCreatePushButton(pushButtonRow,
			 labelPushButtons[i], args, ac);
#ifndef USE_MB
		XmStringFree(labelStr);
#endif
		XtAddCallback(pushButtons[i],
			XmNactivateCallback, listenerPushButtons, iptr++);
		XtManageChild(pushButtons[i]);
	}

	/*
	 *  For the exit button
	 */
	ac = 0;
#ifndef USE_MB
	labelStr = XmStringCreate((char *) "Exit",
		XmSTRING_DEFAULT_CHARSET);
	XtSetArg(args[ac], XmNlabelString, labelStr);
	ac++;
#endif
	exitButton = XmCreatePushButton(pushButtonRow,
			(char *) "Exit", args, ac);
#ifndef USE_MB
	XmStringFree(labelStr);
#endif
	XtAddCallback(exitButton, XmNactivateCallback, exitCallback,
			(XtPointer) NULL);
	XtManageChild(exitButton);

	timeoutSlider = XtVaCreateManagedWidget("timeout",
		xmScaleWidgetClass, pushButtonRow,
		XtVaTypedArg, XmNtitleString, XmRString, "Timeout", 8,
		XmNminimum, 0,
		XmNmaximum, MAXMIN,
		XmNvalue, timeout / 60,
		XmNshowValue, True,
		XmNorientation, XmHORIZONTAL, NULL);
	XtAddCallback(timeoutSlider, XmNvalueChangedCallback,
		(XtCallbackProc) listenerTimeoutSlider, (XtPointer) NULL);
	XtManageChild(pushButtonRow);

	/* list and toggles in row like that (row(list)(togglesRow(toggles...))) */
	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg(args[ac], XmNtopWidget, labelXlock);
	ac++;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg(args[ac], XmNbottomWidget, pushButtonRow);
	ac++;
	XtSetArg(args[ac], XmNitems, tabXmStr);
	ac++;
	XtSetArg(args[ac], XmNitemCount, numprocs);
	ac++;
	XtSetArg(args[ac], XmNvisibleItemCount, 10);
	ac++;

	for (i = 0; i < (int) numprocs; i++) {
		(void) sprintf(string, "%-14s%s", LockProcs[i].cmdline_arg,
			LockProcs[i].desc);
		tabXmStr[i] = XmStringCreate(string, XmSTRING_DEFAULT_CHARSET);
	}
	scrolledListModes = XmCreateScrolledList(father,
		(char *) "ScrolledListModes", args, ac);
	XtAddCallback(scrolledListModes, XmNbrowseSelectionCallback,
		listenerScrolledListModes, NULL);
	XtManageChild(scrolledListModes);

#if 0
	int togglesRow, row;
	togglesRow = XmCreateRowColumn(Row, (char *) "TogglesRow", NULL, 0);
	for (i = 0; i < numToggleNames; i++) {
#ifndef USE_MB
		ac = 0;
		labelStr = XmStringCreate(toggleNames[i],
				XmSTRING_DEFAULT_CHARSET);
		XtSetArg(args[ac], XmNlabelString, labelStr);
		ac++;
#endif
		toggles[i] = XmCreateToggleButton(togglesRow, toggleNames[i],
				args, ac);
#ifndef USE_MB
		XmStringFree(labelStr);
#endif
		XtManageChild(toggles[i]);
	}
	XtManageChild(togglesRow);

	XtManageChild(row);
#endif
	for (i = 0; i < (int) numprocs; i++) {
		XmStringFree(tabXmStr[i]);
	}
#elif defined(HAVE_ATHENA)
	labelXlock = XtVaCreateManagedWidget("Window",
		commandWidgetClass, father,
		XtNwidth, WINDOW_WIDTH,
		XtNheight, WINDOW_HEIGHT,
		XtNfromVert, modeListLabel, NULL);

	pushButtonRow = XtVaCreateManagedWidget("pushButtonRow",
		formWidgetClass, father,
		XtNborderWidth, 0,
		XtNfromVert, labelXlock, NULL);
	for (i = 0; i < numButtons; i++) {
		pushButtons[i] = XtVaCreateManagedWidget(labelPushButtons[i],
			commandWidgetClass, pushButtonRow, NULL);
		if (i > 0)
			XtVaSetValues(pushButtons[i],
				XtNfromHoriz, pushButtons[i - 1], NULL);
		XtAddCallback(pushButtons[i],
			XtNcallback, listenerPushButtons, iptr++);
	}
	exitButton = XtVaCreateManagedWidget("Exit",
		commandWidgetClass, pushButtonRow,
		XtNfromHoriz, pushButtons[numButtons - 1], NULL);
	XtAddCallback(exitButton,
		XtNcallback, exitCallback, (XtPointer) NULL);
#endif
}

extern void setupOption(Widget menuBar);

int
main(int argc, char **argv)
{
	Widget form;
	Arg args[15];
	int i;

#ifdef USE_MB
	setlocale(LC_ALL, "");
	XtSetLanguageProc(NULL, NULL, NULL);
#endif

/* PURIFY 4.0.1 on Solaris 2 reports an uninitialized memory read on the next
   line. */
	topLevel = XtInitialize(argv[0], "XmLock", (XrmOptionDescRec*) NULL, 0,
			&argc, argv);
	XtSetArg(args[0], XtNiconPixmap,
		 XCreateBitmapFromData(XtDisplay(topLevel),
			RootWindowOfScreen(XtScreen(topLevel)),
			(char *) image_bits, image_width, image_height));
	XtSetValues(topLevel, args, 1);
	/* creation Widget */
/* PURIFY 4.0.1 on Solaris 2 reports an uninitialized memory read on the next
   line. */
#ifdef HAVE_MOTIF
	form = XmCreateForm(topLevel, (char *) "Form", (Arg *) NULL, 0);
#elif defined(HAVE_ATHENA)
	form = XtVaCreateManagedWidget("Form",
		formWidgetClass, topLevel,
		XtNborderWidth, 0, NULL);
	menuBar = XtVaCreateManagedWidget("MenuBar",
		formWidgetClass, form,
		XtNborderWidth, 1, NULL);
	setupOption(menuBar);
	createList(form, findMode("random"));
#endif
	setupForm(form);
#ifdef HAVE_MOTIF
	XtManageChild(form);
	setupOption(menuOption);
#endif
	/*XtManageChild(menuBar);*/
	XtRealizeWidget(topLevel);
	(void) XtAppAddTimeOut(XtWidgetToApplicationContext(topLevel),
			SKIPDELAY, (XtTimerCallbackProc) checkTime, topLevel);
#if USE_XMU
	/* With this handler you can use editres */
	XtAddEventHandler(topLevel, (EventMask) 0, TRUE,
			(XtEventHandler) _XEditResCheckMessages, NULL);
#endif
	XtMainLoop();
#ifdef VMS
	return 1;
#else
	return 0;
#endif
}
