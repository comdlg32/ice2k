#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
	Display* dpy;
	Window rootwin;
	Window win;
	Visual* visual;
	XEvent ev;
	int scr;

	XSetWindowAttributes attr;

	dpy = XOpenDisplay(NULL);

	if (dpy == NULL) {
		fputs("Could not open display!!!", stderr);
		return 1;
	}

	scr = DefaultScreen(dpy);
	rootwin = RootWindow(dpy, scr);
	visual = DefaultVisual(dpy, scr);

	attr.override_redirect = True;
	attr.background_pixmap = 0;

	win = XCreateWindow(dpy, rootwin, 0,0,
			DisplayWidth(dpy, scr), DisplayHeight(dpy,scr), 0,
			DefaultDepth(dpy, scr), InputOutput, visual, CWOverrideRedirect,
			&attr);

	XSelectInput(dpy, win, SubstructureNotifyMask|StructureNotifyMask);

	//XSetWindowBackgroundPixmap(dpy, win, 0);
	XMapWindow(dpy, win);
	XLowerWindow(dpy, win);

	while (1) {
		XNextEvent(dpy, &ev);
		switch (ev.type) {
			case ConfigureNotify:
			case MapNotify:
				XSelectInput(dpy, win, 0);
				XLowerWindow(dpy, win);
				XSelectInput(dpy, win, SubstructureNotifyMask|StructureNotifyMask);
				break;
		}
	}


	XCloseDisplay(dpy);
	return 0;
}
