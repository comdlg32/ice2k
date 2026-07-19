#include <fx.h>
#include "ice2k/batmeter/I2KBatMeter.h"
#include "res/foxres.h"


FXIcon* ico_baticon;

class BatMeterWindow : public FXMainWindow {
	FXDECLARE(BatMeterWindow);

protected:
	BatMeterWindow() {}
private:
	FXVerticalFrame*        cont;
	FXTabBook*              tabbook;

	FXPacker*               batmetercont;
	I2KBatMeter*            batmeter;

	FXButton*               okbtn;
	FXButton*               cancelbtn;
	FXButton*               applybtn;
	

public:
	BatMeterWindow(FXApp* a);

	virtual void create();
	void setFocus() {return;};
	virtual ~BatMeterWindow();
};

FXIMPLEMENT(BatMeterWindow, FXMainWindow, NULL, 0);

BatMeterWindow::BatMeterWindow(FXApp *a) : FXMainWindow(a, "Power Options Properties", ico_baticon, NULL, DECOR_BORDER|DECOR_CLOSE|DECOR_TITLE|DECOR_MENU, 0,0,0,0, 0,0,3,3) {
	cont = new FXVerticalFrame(this, LAYOUT_FILL_Y|LAYOUT_FILL_X, 0,0,0,0, 6,6,6,6, 0,0);
	
	tabbook = new FXTabBook(cont, NULL, 0, TABBOOK_NORMAL|LAYOUT_FILL_Y|LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0);
	new FXTabItem(tabbook, "Power Meter", NULL, TAB_TOP_NORMAL, 0,0,0,0, 6,6,1,2);
	batmetercont = new FXVerticalFrame(tabbook, LAYOUT_FILL_Y|LAYOUT_FILL_X|FRAME_RAISED|FRAME_THICK, 0,0,0,0, 6,8,9,8, 0,0);
	batmeter = new I2KBatMeter(batmetercont, LAYOUT_FILL_X|LAYOUT_FILL_Y);
	batmeter->hideAlwaysShowChk();
}

BatMeterWindow::~BatMeterWindow() {}

void BatMeterWindow::create() {
	FXMainWindow::create();

	show(PLACEMENT_SCREEN);
}

int main(int argc, char *argv[]) {
	FXApp application("PowerCfg", "I2KProject");
	ico_baticon = new FXGIFIcon(&application, resico_mainicon);

	application.init(argc, argv);

	new BatMeterWindow(&application);

	application.create();
	return application.run();
}
