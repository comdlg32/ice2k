#include <fx.h>
// #include <ice2k/comctl32.h>

#include "res/foxres.h"

FXIcon* mainIcon;

FXIcon* ico_rdelay;
FXIcon* ico_rrate;

FXuint blink_speed = 500;
int blink_inv = 1;

class HelloWindow : public FXMainWindow {
	FXDECLARE(HelloWindow);

protected:
	HelloWindow() {}

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

	FXGroupBox* blink_grp;
	FXPacker* blink_cnt;
	FXCanvas* blink_cvs;
	FXHorizontalFrame* blink_sld_cnt;
	FXSlider* blink_sld;

	FXButton* ok_btn;
	FXButton* cancel_btn;
	FXButton* apply_btn;


public:
	long onPaintBlink(FXObject*, FXSelector, void*);
	long onTimeoutBlink(FXObject*, FXSelector, void*);

	//long onCmdHello(FXObject*, FXSelector, void*);


	long onCmdDialogOK(FXObject*,FXSelector,void*);
	long onCmdDialogApply(FXObject*,FXSelector,void*);
	long onCmdDialogCancel(FXObject*,FXSelector,void*);


public:
	enum {
		ID_BLINK = FXMainWindow::ID_LAST,
		ID_DLG_OK,
		ID_DLG_CANCEL,
		ID_DLG_APPLY,
		ID_LAST
	};

public:
	HelloWindow(FXApp* a);

	virtual void create();
	virtual ~HelloWindow();
};

FXDEFMAP(HelloWindow) HelloWindowMap[] = {
	FXMAPFUNC(SEL_COMMAND, HelloWindow::ID_DLG_OK, HelloWindow::onCmdDialogOK),
	FXMAPFUNC(SEL_COMMAND, HelloWindow::ID_DLG_APPLY, HelloWindow::onCmdDialogApply),
	FXMAPFUNC(SEL_COMMAND, HelloWindow::ID_DLG_CANCEL, HelloWindow::onCmdDialogCancel),

	FXMAPFUNC(SEL_PAINT,             HelloWindow::ID_BLINK,  HelloWindow::onPaintBlink),
	FXMAPFUNC(SEL_TIMEOUT,           HelloWindow::ID_BLINK,  HelloWindow::onTimeoutBlink),

	//FXMAPFUNC(SEL_COMMAND,           HelloWindow::ID_HELLO,  HelloWindow::onCmdHello),

};

FXIMPLEMENT(HelloWindow, FXMainWindow, HelloWindowMap, ARRAYNUMBER(HelloWindowMap));

HelloWindow::HelloWindow(FXApp *a) : FXMainWindow(a, "Keyboard Properties", mainIcon, NULL, DECOR_CLOSE|DECOR_BORDER|DECOR_TITLE, 0,0,0,0, 0,0,2,3, 0,6) {
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
	rdelay_sld = new FXSlider(rdelay_sld_cnt, NULL, 0,
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
	rrate_sld = new FXSlider(rrate_sld_cnt, NULL, 0,
			LAYOUT_FIX_HEIGHT|LAYOUT_FIX_WIDTH|SLIDER_TICKS_BOTTOM|SLIDER_ARROW_DOWN,
			0,0,185,25+6, 0,0,6,0);
	rrate_sld->setRange(1, 25);
	rrate_sld->setSlotSize(4);
	rrate_sld->setHeadSize(11);
	new FXLabel(rrate_sld_cnt, "Fast");

	test_cnt = new FXVerticalFrame(repeat_grp, LAYOUT_FILL_X, 0,0,0,0, 2,2,1,2, 0,0);

	new FXLabel(test_cnt, "Click here and hold down a key to &test repeat rate:", NULL, LABEL_NORMAL, 0,0,0,0, 1,1,1,2);

	new FXTextField(test_cnt, 10, NULL, 0, LAYOUT_FILL_X|TEXTFIELD_NORMAL, 0,0,0,0, 2,2,1,4);

	blink_grp = new FXGroupBox(speed_cnt, "Cursor blink rate", FRAME_GROOVE|LAYOUT_FILL_X, 0,0,0,0, 17,17,9,18, 16,20);

	blink_cnt = new FXPacker(blink_grp, LAYOUT_FILL_X, 0,0,0,0, 0,0,0,2, 16,6);

	blink_cvs = new FXCanvas(blink_cnt, this, ID_BLINK, LAYOUT_SIDE_LEFT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0,0,32,32);

	//new FXLabel(blink_cnt, "", ico_rrate, LAYOUT_SIDE_LEFT);

	blink_sld_cnt = new FXHorizontalFrame(blink_cnt,
			LAYOUT_FILL_X,
			0,0,0,0, 0,0,0,0, 10,10);
	new FXLabel(blink_sld_cnt, "None");
	blink_sld = new FXSlider(blink_sld_cnt, NULL, 0,
			LAYOUT_FIX_HEIGHT|LAYOUT_FIX_WIDTH|SLIDER_TICKS_BOTTOM|SLIDER_ARROW_DOWN,
			0,0,185,25+6, 0,0,6,0);
	blink_sld->setRange(1, 10);
	blink_sld->setSlotSize(4);
	blink_sld->setHeadSize(11);
	new FXLabel(blink_sld_cnt, "Fast");

	FXHorizontalFrame* btncont = new FXHorizontalFrame(cont, LAYOUT_RIGHT, 0,0,0,0, 0,6,1,4, 6,0);

	ok_btn = new FXButton(btncont, "OK", NULL, this, ID_DLG_OK, BUTTON_DEFAULT|BUTTON_NORMAL|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0,0,75,23, 0,0,0,0);
	cancel_btn = new FXButton(btncont, "Cancel", NULL, this, ID_DLG_CANCEL, BUTTON_NORMAL|BUTTON_DEFAULT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0,0,75,23, 0,0,0,0);
	apply_btn = new FXButton(btncont, "&Apply", NULL, this, ID_DLG_APPLY, BUTTON_NORMAL|BUTTON_DEFAULT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0,0,75,23, 0,0,0,0);

	apply_btn->disable();



	getApp()->addTimeout(this, ID_BLINK, blink_speed);

	//new FXSlider(repeatgrp);
	
}

HelloWindow::~HelloWindow() {
}

void HelloWindow::create() {
	FXMainWindow::create();

	show(PLACEMENT_SCREEN);
}


long HelloWindow::onCmdDialogOK(FXObject* obj,FXSelector sel, void* ptr) {
	onCmdDialogApply(obj, sel, ptr);
	getApp()->exit();
	return 1;
}


long HelloWindow::onCmdDialogCancel(FXObject* obj,FXSelector sel, void* ptr) {
	getApp()->exit(1);
	return 1;
}


long HelloWindow::onCmdDialogApply(FXObject* obj,FXSelector sel, void* ptr) {
	apply_btn->disable();
	return 1;
}

long HelloWindow::onPaintBlink(FXObject* obj, FXSelector, void* ptr) {
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

long HelloWindow::onTimeoutBlink(FXObject*, FXSelector, void*) {
	getApp()->addTimeout(this, ID_BLINK, blink_speed);
	blink_inv =! blink_inv;
	blink_cvs->update();
	return 1;
}

int main(int argc, char *argv[]) {
	FXApp application("Hello", "I2KTest");
	
	application.init(argc, argv);

	ico_rrate = new FXGIFIcon(&application, resico_rrate);
	ico_rdelay = new FXGIFIcon(&application, resico_rdelay);
	mainIcon = new FXGIFIcon(&application, resico_mainicon, 0, IMAGE_OPAQUE);

	blink_speed = application.reg().readUnsignedEntry("SETTINGS", "blinkspeed", blink_speed);

	new HelloWindow(&application);

	application.create();
	return application.run();
}
