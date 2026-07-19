#include <fx.h>
#include "ice2k/batmeter/I2KBatMeter.h"
#include "res/foxres.h"


FXIcon* ico_baticon;

class BatMeterWindow : public FXMainWindow {
	FXDECLARE(BatMeterWindow);

protected:
	BatMeterWindow() {}

public:
	BatMeterWindow(FXApp* a);

	virtual void create();
	void setFocus() {return;};
	virtual ~BatMeterWindow();
};

FXIMPLEMENT(BatMeterWindow, FXMainWindow, NULL, 0);

BatMeterWindow::BatMeterWindow(FXApp *a) : FXMainWindow(a, "Power Meter", ico_baticon, NULL, DECOR_BORDER|DECOR_CLOSE|DECOR_TITLE|DECOR_MENU, 0,0,0,0, 7,7,9,9) {
	new I2KBatMeter(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);
}

BatMeterWindow::~BatMeterWindow() {}

void BatMeterWindow::create() {
	FXMainWindow::create();

	show(PLACEMENT_SCREEN);
}

int main(int argc, char *argv[]) {
	FXApp application("BatMeter", "I2KProject");
	ico_baticon = new FXGIFIcon(&application, resico_mainicon);

	application.init(argc, argv);

	new BatMeterWindow(&application);

	application.create();
	return application.run();
}
