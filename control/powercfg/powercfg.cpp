#include <fx.h>
#include "ice2k/batmeter/I2KBatMeter.h"
#include "res/foxres.h"


FXIcon* ico_baticon;

class PowerCfgWindow : public FXMainWindow {
	FXDECLARE(PowerCfgWindow);

protected:
	PowerCfgWindow() {}
private:
	FXVerticalFrame*        cont;
	FXTabBook*              tabbook;

	FXPacker*               schemecont;
	FXHorizontalFrame*      schemetopcont;
	FXText*                 schemetext;

	FXPacker*               batmetercont;
	I2KBatMeter*            batmeter;

	FXButton*               okbtn;
	FXButton*               cancelbtn;
	FXButton*               applybtn;

public:
	PowerCfgWindow(FXApp* a);

	virtual void create();
	void setFocus() {};
	virtual ~PowerCfgWindow();
};

FXIMPLEMENT(PowerCfgWindow, FXMainWindow, NULL, 0);

PowerCfgWindow::PowerCfgWindow(FXApp *a) : FXMainWindow(a, "Power Options Properties", ico_baticon, NULL, DECOR_BORDER|DECOR_CLOSE|DECOR_TITLE|DECOR_MENU, 0,0,0,0, 0,0,3,3) {
	cont = new FXVerticalFrame(this, LAYOUT_FILL_Y|LAYOUT_FILL_X, 0,0,0,0, 6,6,6,6, 0,0);
	
	tabbook = new FXTabBook(cont, NULL, 0, TABBOOK_NORMAL|LAYOUT_FILL, 0,0,0,0, 0,0,0,0);
	new FXTabItem(tabbook, "Power Schemes", NULL, TAB_TOP_NORMAL, 0,0,0,0, 6,6,1,2);
	schemecont = new FXVerticalFrame(tabbook, LAYOUT_FILL|FRAME_RAISED|FRAME_THICK, 0,0,0,0, 6,8,9,8, 0,0);
	schemetopcont = new FXVerticalFrame(tabbook, LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0, 8,8);

	schemetext = new FXText(schemetopcont, NULL, 0, LAYOUT_FILL_X|LAYOUT_FIX_HEIGHT|TEXT_WORDWRAP);
	schemetext->setText(
			"Select the CPU power scheme with the most appropriate settings for the current task.");
	schemetext->disable();
	schemetext->setMarginTop(0);
	schemetext->setMarginLeft(0);
	schemetext->setMarginBottom(0);
	schemetext->setMarginRight(0);
	schemetext->setBackColor(getApp()->getBaseColor());
	schemetext->setDefaultCursor(getApp()->getDefaultCursor(DEF_ARROW_CURSOR));


	new FXTabItem(tabbook, "Power Meter", NULL, TAB_TOP_NORMAL, 0,0,0,0, 6,6,1,2);
	batmetercont = new FXVerticalFrame(tabbook, LAYOUT_FILL|FRAME_RAISED|FRAME_THICK, 0,0,0,0, 6,8,9,8, 0,0);
	batmeter = new I2KBatMeter(batmetercont, LAYOUT_FILL);
	batmeter->hideAlwaysShowChk();
}

PowerCfgWindow::~PowerCfgWindow() {}

void PowerCfgWindow::create() {
	FXMainWindow::create();

	show(PLACEMENT_SCREEN);
}

int main(int argc, char *argv[]) {
	FXApp application("PowerCfg", "I2KProject");
	ico_baticon = new FXGIFIcon(&application, resico_mainicon);

	application.init(argc, argv);

	new PowerCfgWindow(&application);

	application.create();
	return application.run();
}
