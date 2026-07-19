#include <fx.h>
//#include "battery.h"
#include "ice2k/batmeter/I2KBatButton.h"
#include "res/foxres.h"

FXDEFMAP(I2KBatButton) I2KBatButtonMap[] = {
	FXMAPFUNC(SEL_COMMAND,           I2KBatButton::ID_COMMAND,  I2KBatButton::onCmdButton),
};

long I2KBatButton::onCmdButton(FXObject*, FXSelector, void* ptr) {
	if (target) return target->tryHandle(this, FXSEL(SEL_COMMAND, message), (void*)(FXuval)1);
	return 0;
}

FXIMPLEMENT(I2KBatButton, FXSwitcher, I2KBatButtonMap, ARRAYNUMBER(I2KBatButtonMap));

I2KBatButton::I2KBatButton(FXComposite* p, const FXString& text, FXIcon* ic, FXObject* tgt, FXSelector sel, FXuint opts,
		FXint x, FXint y, FXint w, FXint h) 
	: FXSwitcher(p, opts, x,y,w,h, 0,0,0,0) {
	target = tgt;
	message = sel;

	maincont = new FXVerticalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0);

	horcont = new FXHorizontalFrame(maincont, LAYOUT_FILL_X, 0,0,0,0, 10,0,0,0, 0,0);
	vercont = new FXVerticalFrame(horcont, LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0, 0,0);
	new FXLabel(vercont, text, NULL, LAYOUT_FILL_X, 0,0,0,0, 2,0,2,4);
	FXPacker* btncont = new FXPacker(vercont, FRAME_NONE, 0,0,0,0, 1,1,1,1, 0,0);
	btn = new FXButton(btncont, "", ic, this, ID_COMMAND, FRAME_LINE|BUTTON_DEFAULT, 0,0,0,0, -4,-5,-2,-1);
	btncont->setBackColor(FXRGB(0,0,0));
	btn->setShadowColor(btn->getHiliteColor());

	caplbl = new FXLabel(horcont, "100%", NULL, LAYOUT_FILL_Y|JUSTIFY_BOTTOM, 0,0,0,0, 2,0,0,4);
	//new FXButton(
	botlbl = new FXLabel(maincont, "Not Present", NULL, LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0);

	new FXFrame(this, FRAME_NONE, 0,0,0,0, 0,0,0,0);
}

I2KBatButton::~I2KBatButton() {
}

void I2KBatButton::create() {
	FXSwitcher::create();
}

void I2KBatButton::hideOnScreen() { setCurrent(1); }
void I2KBatButton::showOnScreen() { setCurrent(0); }

void I2KBatButton::setPresent(int present) {
	if(present) {
		botlbl->setTextColor(getBackColor());
		//botlbl->setText(" ");
		caplbl->setTextColor(getApp()->getForeColor());
	} else {
		//botlbl->setText("Not Present");
		botlbl->setTextColor(getApp()->getForeColor());
		caplbl->setTextColor(getBackColor());
	}
}
void I2KBatButton::setPercentage(int percentage) {
	char str[8];
	int percent;

	if (percentage > 100) percent = 100;
	else if (0 > percentage) percent = 0;
	else percent = percentage;

	sprintf(str, "%u%%", percent);
	caplbl->setText(str);
}

void I2KBatButton::setIcon(FXIcon* ic) { btn->setIcon(ic); }
void I2KBatButton::setText(const FXString& text) { numlbl->setText(text); }
void I2KBatButton::setBattery(const char* bat) { sprintf(battery, "%.*s", (int)sizeof(battery)-1, bat); }

/*long I2KBatButton::onCmdHello(FXObject*, FXSelector, void*) {
	puts("Hello world!");
	return 1;
}

long I2KBatButton::onTimeout(FXObject*, FXSelector, void*) {
	refreshInfo();
	getApp()->addTimeout(this, ID_TIMEOUT, 2000);
	return 1;
}*/
