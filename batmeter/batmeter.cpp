#include <fx.h>
#include "ice2k/batmeter/I2KBatMeter.h"
#include "res/foxres.h"
FXSettings settings;

int bat_detailed = 0;
int bat_showicon = 0;

FXIcon* ico_baticon;

class BatMeterWindow : public FXMainWindow {
	FXDECLARE(BatMeterWindow);

protected:
	BatMeterWindow() {}

public:
	long onCmdBatMeter(FXObject*,FXSelector,void*);
public:
	enum {
		ID_BATMETER = FXMainWindow::ID_LAST,
	};

public:
	BatMeterWindow(FXApp* a);

	virtual void create();
	void setFocus() {};
	virtual ~BatMeterWindow();
};

FXDEFMAP(BatMeterWindow) BatMeterWindowMap[] = {
	FXMAPFUNC(SEL_COMMAND, BatMeterWindow::ID_BATMETER, BatMeterWindow::onCmdBatMeter),
};

FXIMPLEMENT(BatMeterWindow, FXMainWindow, BatMeterWindowMap, ARRAYNUMBER(BatMeterWindowMap));

BatMeterWindow::BatMeterWindow(FXApp *a) : FXMainWindow(a, "Power Meter", ico_baticon, NULL, DECOR_BORDER|DECOR_CLOSE|DECOR_TITLE|DECOR_MENU, 0,0,0,0, 7,7,9,9) {
	bat_showicon = getApp()->reg().readIntEntry("BatMeter", "AlwaysShow", FALSE);
	bat_detailed = getApp()->reg().readIntEntry("BatMeter", "Detailed", FALSE);

	I2KBatMeter* batmeter = new I2KBatMeter(this, LAYOUT_FILL);
	batmeter->setTarget(this);
	batmeter->setSelector(ID_BATMETER);
}

BatMeterWindow::~BatMeterWindow() {
	FXString desktopfile = FXSystem::getHomeDirectory()+PATHSEPSTRING+".foxrc"+PATHSEPSTRING+"Desktop";
	if (FXStat::exists(desktopfile)) {
		settings.parseFile(desktopfile, TRUE);
	}

	settings.writeIntEntry("BatMeter", "AlwaysShow", bat_showicon);
	settings.writeIntEntry("BatMeter", "Detailed", bat_detailed);


	settings.unparseFile(desktopfile);
	settings.clear();
}

void BatMeterWindow::create() {
	FXMainWindow::create();

	show(PLACEMENT_SCREEN);
}

long BatMeterWindow::onCmdBatMeter(FXObject* obj,FXSelector sel, void* ptr) {
	switch((unsigned)(FXuval)ptr) {
		case BATMETER_DETAILED_ON:
			bat_detailed = 1;
			break;
		case BATMETER_DETAILED_OFF:
			bat_detailed = 0;
			break;
		case BATMETER_SHOWICON_ON:
			bat_showicon = 1;
			break;
		case BATMETER_SHOWICON_OFF:
			bat_showicon = 0;
			break;
	}

	return 1;
}


int main(int argc, char *argv[]) {
	FXApp application("BatMeter", "I2KProject");
	ico_baticon = new FXGIFIcon(&application, resico_mainicon);

	application.init(argc, argv);

	new BatMeterWindow(&application);

	application.create();
	return application.run();
}
