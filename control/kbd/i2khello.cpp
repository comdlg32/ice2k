#include <fx.h>
// #include <ice2k/comctl32.h>

#include "res/foxres.h"

FXIcon* mainIcon;

FXIcon* ico_rdelay;
FXIcon* ico_rrate;

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
	FXPacker* rrate_cnt;

	FXSlider* rdelay_sld;


public:
	long onCmdHello(FXObject*, FXSelector, void*);

public:
	enum {
		ID_MAINWIN = FXMainWindow::ID_LAST,
		ID_HELLO,
		ID_LAST
	};

public:
	HelloWindow(FXApp* a);

	virtual void create();
	virtual ~HelloWindow();
};

FXDEFMAP(HelloWindow) HelloWindowMap[] = {
	FXMAPFUNC(SEL_COMMAND,           HelloWindow::ID_HELLO,  HelloWindow::onCmdHello),
};

FXIMPLEMENT(HelloWindow, FXMainWindow, HelloWindowMap, ARRAYNUMBER(HelloWindowMap));

HelloWindow::HelloWindow(FXApp *a) : FXMainWindow(a, "Keyboard Properties", mainIcon, NULL, DECOR_CLOSE|DECOR_BORDER|DECOR_TITLE|DECOR_RESIZE, 0,0,0,0, 0,0,2,3, 0,6) {
	cont = new FXVerticalFrame(this, LAYOUT_FILL_Y|LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0, 0,0);
	
	tabbook = new FXTabBook(cont, NULL, 0, TABBOOK_NORMAL|LAYOUT_FILL, 0,0,0,0, 6,6,5,5);
	new FXTabItem(tabbook, "Speed", NULL, TAB_TOP_NORMAL, 0,0,0,0, 4,4,1,2);
	speed_cnt = new FXVerticalFrame(tabbook, LAYOUT_FILL|FRAME_RAISED|FRAME_THICK, 0,0,0,0, 13,13,11,10, 8,8);

	repeat_grp = new FXGroupBox(speed_cnt, "Character repeat", FRAME_GROOVE|LAYOUT_FILL_X, 0,0,0,0, 17,17,9,9, 16,16);

	rdelay_cnt = new FXPacker(repeat_grp, LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0, 16,6);
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

	//new FXSlider(repeatgrp);
	
}

HelloWindow::~HelloWindow() {
}

void HelloWindow::create() {
	FXMainWindow::create();

	show(PLACEMENT_SCREEN);
}

long HelloWindow::onCmdHello(FXObject*, FXSelector, void*) {
	puts("Hello world!");
	return 1;
}

int main(int argc, char *argv[]) {
	FXApp application("Hello", "I2KTest");
	
	ico_rrate = new FXGIFIcon(&application, resico_rrate);
	ico_rdelay = new FXGIFIcon(&application, resico_rdelay);
	mainIcon = new FXGIFIcon(&application, resico_mainicon, 0, IMAGE_OPAQUE);

	application.init(argc, argv);

	new HelloWindow(&application);

	application.create();
	return application.run();
}
