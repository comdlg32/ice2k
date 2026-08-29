#include <X11/Xlib.h>
#include <X11/XKBlib.h>

#include <fx.h>
#define PSEP PATHSEPSTRING
#include <ice2k/comctl32.h>

#include "res/foxres.h"

Display* dpy = NULL;

FXIcon* mainIcon;

FXIcon* ico_rdelay;
FXIcon* ico_rrate;
FXuint blink_speed = 500;

FXSettings kbdsettings;

FXString kbdfile;


int blink_inv = 1;

class KeyboardProperties : public FXMainWindow {
	FXDECLARE(KeyboardProperties);

protected:
	KeyboardProperties() {}

private:
	FXVerticalFrame* cont;
	FXTabBook* tabbook;
	FXVerticalFrame* speed_cnt;

	FXGroupBox* repeat_grp;

	FXPacker* rdelay_cnt;
	FXHorizontalFrame* rdelay_sld_cnt;
	FXSlider* rdelay_sld;

	FXPacker* rrate_cnt;
	FXHorizontalFrame* rrate_sld_cnt;
	FXSlider* rrate_sld;

	FXPacker* test_cnt;
	FXTextField* test_txt;

	FXGroupBox* blink_grp;
	FXPacker* blink_cnt;
	FXCanvas* blink_cvs;
	FXHorizontalFrame* blink_sld_cnt;
	FXSlider* blink_sld;

	FXButton* ok_btn;
	FXButton* cancel_btn;
	FXButton* apply_btn;


public:
	long onChangeBlink(FXObject*, FXSelector, void*);
	long onChangeRepDelay(FXObject*, FXSelector, void*);
	long onChangeRepSpeed(FXObject*, FXSelector, void*);


	long onPaintBlinkAnim(FXObject*, FXSelector, void*);
	long onTimeoutBlinkAnim(FXObject*, FXSelector, void*);

	//long onCmdHello(FXObject*, FXSelector, void*);


	long onCmdDialogOK(FXObject*,FXSelector,void*);
	long onCmdDialogApply(FXObject*,FXSelector,void*);
	long onCmdDialogCancel(FXObject*,FXSelector,void*);


public:
	enum {
		ID_BLINK_ANIM = FXMainWindow::ID_LAST,
		
		ID_REPEAT_DELAY,
		ID_REPEAT_SPEED,
		ID_BLINK,
		
		ID_DLG_OK,
		ID_DLG_CANCEL,
		ID_DLG_APPLY,

		ID_LAST
	};

public:
	KeyboardProperties(FXApp* a);

	virtual void create();
	void setFocus() {};
	virtual ~KeyboardProperties();
};

FXDEFMAP(KeyboardProperties) KeyboardPropertiesMap[] = {
	FXMAPFUNC(SEL_COMMAND, KeyboardProperties::ID_DLG_OK, KeyboardProperties::onCmdDialogOK),
	FXMAPFUNC(SEL_COMMAND, KeyboardProperties::ID_DLG_APPLY, KeyboardProperties::onCmdDialogApply),
	FXMAPFUNC(SEL_COMMAND, KeyboardProperties::ID_DLG_CANCEL, KeyboardProperties::onCmdDialogCancel),

	FXMAPFUNC(SEL_CHANGED, KeyboardProperties::ID_REPEAT_DELAY, KeyboardProperties::onChangeRepDelay),

	FXMAPFUNC(SEL_COMMAND, KeyboardProperties::ID_REPEAT_SPEED, KeyboardProperties::onChangeRepSpeed),
	FXMAPFUNC(SEL_CHANGED, KeyboardProperties::ID_REPEAT_SPEED, KeyboardProperties::onChangeRepSpeed),

	FXMAPFUNC(SEL_CHANGED, KeyboardProperties::ID_BLINK, KeyboardProperties::onChangeBlink),

	FXMAPFUNC(SEL_PAINT,             KeyboardProperties::ID_BLINK_ANIM,  KeyboardProperties::onPaintBlinkAnim),
	FXMAPFUNC(SEL_TIMEOUT,           KeyboardProperties::ID_BLINK_ANIM,  KeyboardProperties::onTimeoutBlinkAnim),

	//FXMAPFUNC(SEL_COMMAND,           KeyboardProperties::ID_HELLO,  KeyboardProperties::onCmdHello),

};

FXIMPLEMENT(KeyboardProperties, FXMainWindow, KeyboardPropertiesMap, ARRAYNUMBER(KeyboardPropertiesMap));

KeyboardProperties::KeyboardProperties(FXApp *a) : FXMainWindow(a, "Keyboard Properties", mainIcon, NULL, DECOR_CLOSE|DECOR_BORDER|DECOR_TITLE, 0,0,0,0, 0,0,2,3, 0,6) {
	cont = new FXVerticalFrame(this, LAYOUT_FILL_Y|LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0, 0,0);
	
	tabbook = new FXTabBook(cont, NULL, 0, TABBOOK_NORMAL|LAYOUT_FILL, 0,0,0,0, 6,6,5,5);
	new FXTabItem(tabbook, "Speed", NULL, TAB_TOP_NORMAL, 0,0,0,0, 5,5,1,2);
	speed_cnt = new FXVerticalFrame(tabbook, LAYOUT_FILL|FRAME_RAISED|FRAME_THICK, 0,0,0,0, 13,13,11,21, 6,6);

	repeat_grp = new FXGroupBox(speed_cnt, "Character repeat", FRAME_GROOVE|LAYOUT_FILL_X, 0,0,0,0, 17,17,9,18, 16,20);

	rdelay_cnt = new FXPacker(repeat_grp, LAYOUT_FILL_X, 0,0,0,0, 0,0,0,2, 16,6);
	new FXLabel(rdelay_cnt, "", ico_rdelay, LAYOUT_SIDE_LEFT);
	new FXLabel(rdelay_cnt, "Repeat &delay:", NULL, LABEL_NORMAL, 0,0,0,0, 2,2,0,2);

	rdelay_sld_cnt = new FXHorizontalFrame(rdelay_cnt,
			LAYOUT_FILL_X,
			0,0,0,0, 0,0,0,0, 10,10);
	new FXLabel(rdelay_sld_cnt, "Long");
	rdelay_sld = new FXSlider(rdelay_sld_cnt, this, ID_REPEAT_DELAY,
			LAYOUT_FIX_HEIGHT|LAYOUT_FIX_WIDTH|SLIDER_TICKS_BOTTOM|SLIDER_ARROW_DOWN,
			0,0,185,25+6, 0,0,6,0);
	rdelay_sld->setRange(1, 4);
	rdelay_sld->setSlotSize(4);
	rdelay_sld->setHeadSize(11);
	new FXLabel(rdelay_sld_cnt, "Short");

	rrate_cnt = new FXPacker(repeat_grp, LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0, 16,6);
	new FXLabel(rrate_cnt, "", ico_rrate, LAYOUT_SIDE_LEFT);
	new FXLabel(rrate_cnt, "&Repeat rate:", NULL, LABEL_NORMAL, 0,0,0,0, 2,2,0,2);

	rrate_sld_cnt = new FXHorizontalFrame(rrate_cnt,
			LAYOUT_FILL_X,
			0,0,0,0, 0,0,0,0, 10,10);
	new FXLabel(rrate_sld_cnt, "Slow");
	rrate_sld = new FXSlider(rrate_sld_cnt, this, ID_REPEAT_SPEED,
			LAYOUT_FIX_HEIGHT|LAYOUT_FIX_WIDTH|SLIDER_TICKS_BOTTOM|SLIDER_ARROW_DOWN,
			0,0,185,25+6, 0,0,6,0);
	rrate_sld->setRange(1, 32);
	rrate_sld->setSlotSize(4);
	rrate_sld->setHeadSize(11);
	new FXLabel(rrate_sld_cnt, "Fast");

	test_cnt = new FXVerticalFrame(repeat_grp, LAYOUT_FILL_X, 0,0,0,0, 2,2,1,2, 0,0);

	new FXLabel(test_cnt, "Click here and hold down a key to &test repeat rate:", NULL, LABEL_NORMAL, 0,0,0,0, 1,1,1,2);

	test_txt = new FXTextField(test_cnt, 10, NULL, 0, LAYOUT_FILL_X|TEXTFIELD_NORMAL, 0,0,0,0, 2,2,1,4);

	blink_grp = new FXGroupBox(speed_cnt, "Cursor blink rate", FRAME_GROOVE|LAYOUT_FILL_X, 0,0,0,0, 17,17,9,18-4, 16,20);

	blink_cnt = new FXPacker(blink_grp, LAYOUT_FILL_X, 0,0,0,0, 0,0,0,2, 16,6);

	blink_cvs = new FXCanvas(blink_cnt, this, ID_BLINK_ANIM, LAYOUT_SIDE_LEFT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0,0,32+4,32+4);

	//new FXLabel(blink_cnt, "", ico_rrate, LAYOUT_SIDE_LEFT);

	blink_sld_cnt = new FXHorizontalFrame(blink_cnt,
			LAYOUT_FILL_X,
			0,0,0,0, 0,0,0,0, 10,10);
	new FXLabel(blink_sld_cnt, "None");
	blink_sld = new FXSlider(blink_sld_cnt, this, ID_BLINK,
			LAYOUT_FIX_HEIGHT|LAYOUT_FIX_WIDTH|SLIDER_TICKS_BOTTOM|SLIDER_ARROW_DOWN,
			0,0,185,25+6, 0,0,6,0);
	blink_sld->setRange(1, 12);
	blink_sld->setSlotSize(4);
	blink_sld->setHeadSize(11);
	blink_sld->setValue( 12-(blink_speed/100)+2 );
	new FXLabel(blink_sld_cnt, "Fast");

	FXHorizontalFrame* btncont = new FXHorizontalFrame(cont, LAYOUT_RIGHT, 0,0,0,0, 0,6,1,4, 6,0);

	ok_btn = new FXButton(btncont, "OK", NULL, this, ID_DLG_OK, BUTTON_DEFAULT|BUTTON_NORMAL|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0,0,75,23, 0,0,0,0);
	cancel_btn = new FXButton(btncont, "Cancel", NULL, this, ID_DLG_CANCEL, BUTTON_NORMAL|BUTTON_DEFAULT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0,0,75,23, 0,0,0,0);
	apply_btn = new FXButton(btncont, "&Apply", NULL, this, ID_DLG_APPLY, BUTTON_NORMAL|BUTTON_DEFAULT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0,0,75,23, 0,0,0,0);

	apply_btn->disable();
	
	rrate_sld ->setValue(1+kbdsettings.readUnsignedEntry("Keyboard", "KeyboardSpeed", 20));
	rdelay_sld->setValue(4-kbdsettings.readUnsignedEntry("Keyboard", "KeyboardDelay",  1));


	getApp()->addTimeout(this, ID_BLINK_ANIM, blink_speed);

	//new FXSlider(repeatgrp);
	
}

KeyboardProperties::~KeyboardProperties() {
	if (dpy != NULL) {
		kbdfile = FXSystem::getHomeDirectory()+PSEP+".icewm"+PSEP+"cfg"+PSEP+"keyboard.ini";
		kbdsettings.parseFile(kbdfile, 0);

		FXuint kbd_speed = kbdsettings.readUnsignedEntry("Keyboard", "KeyboardSpeed", 20);
		FXuint kbd_delay = kbdsettings.readUnsignedEntry("Keyboard", "KeyboardDelay",  1);

		if (kbd_delay > 3) {
			kbd_delay = 3;
		}

		if (kbd_speed > 31) {
			kbd_speed = 31;
		}

		kbd_delay = 250+250*kbd_delay;
		kbd_speed = 1000 / (2+(kbd_speed*30/32));
		
		XkbSetAutoRepeatRate(dpy, XkbUseCoreKbd, kbd_delay, kbd_speed);
	}

	//puts("Goodbye!");
}

void KeyboardProperties::create() {
	FXMainWindow::create();
	test_txt->setFocus();

	show(PLACEMENT_SCREEN);
}

long KeyboardProperties::onChangeBlink(FXObject* obj,FXSelector sel, void* ptr) {
	if ((FXint)(FXival)ptr == 1) {
		blink_speed = -1;
		getApp()->removeTimeout(this, ID_BLINK_ANIM);
		blink_inv = 1;
		blink_cvs->update();
	} else {
		blink_speed = 1400 - (FXint)(FXival)ptr*100;
		getApp()->removeTimeout(this, ID_BLINK_ANIM);
		//printf("%d\n", blink_speed);
		getApp()->addTimeout(this, ID_BLINK_ANIM, blink_speed);	

	}


	apply_btn->enable();
	return 1;
}
long KeyboardProperties::onChangeRepSpeed(FXObject* obj,FXSelector sel, void* ptr) {
	//printf("%d\n", 1000 / (2+(rrate_sld->getValue())*30/32));
	if (dpy != NULL) {
		XkbSetAutoRepeatRate(dpy, XkbUseCoreKbd,
				1250-250*(rdelay_sld->getValue()),
				1000 / (2+(rrate_sld->getValue())*30/32) );
	}


	apply_btn->enable();
	return 1;
}

long KeyboardProperties::onChangeRepDelay(FXObject* obj,FXSelector sel, void* ptr) {
	//printf("%d\n", 1250-250*(FXint)(FXival)ptr );

	if (dpy != NULL) {
		XkbSetAutoRepeatRate(dpy, XkbUseCoreKbd,
				1250-250*(rdelay_sld->getValue()),
				1000 / (2+(rrate_sld->getValue())*30/32) );
	}

	apply_btn->enable();
	return 1;
}



long KeyboardProperties::onCmdDialogOK(FXObject* obj,FXSelector sel, void* ptr) {
	onCmdDialogApply(obj, sel, ptr);
	getApp()->exit();
	return 1;
}


long KeyboardProperties::onCmdDialogCancel(FXObject* obj,FXSelector sel, void* ptr) {
	getApp()->exit(1);
	return 1;
}


long KeyboardProperties::onCmdDialogApply(FXObject* obj,FXSelector sel, void* ptr) {
	//puts("Saving...");
	bool modified = getApp()->reg().isModified();
	FXSettings kbdsettings;

	FXString kbdfile = FXSystem::getHomeDirectory()+PSEP+".icewm"+PSEP+"cfg"+PSEP+"keyboard.ini";

	kbdsettings.writeUnsignedEntry("Keyboard", "KeyboardSpeed", rrate_sld->getValue()-1);
	kbdsettings.writeUnsignedEntry("Keyboard", "KeyboardDelay", 4-rdelay_sld->getValue());

	kbdsettings.unparseFile(kbdfile);
	kbdsettings.clear();

	getApp()->setBlinkSpeed(blink_speed);
	getApp()->reg().deleteEntry("SETTINGS", "blinkspeed");
	getApp()->reg().setModified(modified);

	apply_btn->disable();


	FXSettings settings;

	FXString desktopfile = FXSystem::getHomeDirectory()+PATHSEPSTRING+".foxrc"+PATHSEPSTRING+"Desktop";
	if (FXStat::exists(desktopfile)) {
		settings.parseFile(desktopfile, TRUE);
	}

	settings.writeUnsignedEntry("SETTINGS", "blinkspeed", blink_speed);

	settings.unparseFile(desktopfile);
	settings.clear();

	return 1;
}

long KeyboardProperties::onPaintBlinkAnim(FXObject* obj, FXSelector, void* ptr) {
	FXWindow* win = (FXWindow*)obj;
	FXEvent* ev = (FXEvent*)ptr;
	
	FXint winwidth = win->getWidth();
	FXint winheight = win->getHeight();

	FXDCWindow dc(win, ev);

	//dc.setForeground(FXRGB(255,0,255));
	dc.setForeground(backColor);
	dc.fillRectangle(0, 0, winwidth, winheight);


	if (blink_inv) {
		dc.setFunction(BLT_SRC_XOR_DST);
		dc.setForeground(FXRGB(255,255,255));
	}

	dc.fillRectangle( (winwidth-2)>>1, (winheight-13)>>1, 2, 13);
	dc.setFunction(BLT_SRC);

	dc.end();

	return 1;
}

long KeyboardProperties::onTimeoutBlinkAnim(FXObject*, FXSelector, void*) {
	getApp()->addTimeout(this, ID_BLINK_ANIM, blink_speed);
	blink_inv =! blink_inv;
	blink_cvs->update();
	return 1;
}

int main(int argc, char *argv[]) {
	FXApp application("KeyboardProperties", "I2KProject");
	
	application.init(argc, argv);

	if ((dpy = (Display*)application.getDisplay())) {
		kbdfile = FXSystem::getHomeDirectory()+PSEP+".icewm"+PSEP+"cfg"+PSEP+"keyboard.ini";
		kbdsettings.parseFile(kbdfile, 0);

		FXuint kbd_speed = kbdsettings.readUnsignedEntry("Keyboard", "KeyboardSpeed", 20);
		FXuint kbd_delay = kbdsettings.readUnsignedEntry("Keyboard", "KeyboardDelay", 1);

		if (kbd_delay > 3) {
			kbd_delay = 3;
		}

		if (kbd_speed > 31) {
			kbd_speed = 31;
		}

		kbd_delay = 250+250*kbd_delay;
		kbd_speed = 1000 / (2+(kbd_speed*30/32));
		
		XkbSetAutoRepeatRate(dpy, XkbUseCoreKbd, kbd_delay, kbd_speed);
	}

	if (argv[1] != NULL && (strcmp(argv[1], "-a") == 0)) {
		application.exit();
		return 0;
	}

	ico_rrate = new FXGIFIcon(&application, resico_rrate);
	ico_rdelay = new FXGIFIcon(&application, resico_rdelay);
	mainIcon = new FXGIFIcon(&application, resico_mainicon);

	blink_speed = application.getBlinkSpeed();
	//blink_speed = application.reg().readUnsignedEntry("SETTINGS", "blinkspeed", blink_speed);

	new KeyboardProperties(&application);

	application.create();
	return application.run();
}
