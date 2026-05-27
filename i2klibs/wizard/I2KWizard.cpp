#include <ice2k/comctl32.h>
#include "ice2k/wizard/I2KWizard.h"

#define LAYOUT_FIX_SIZE LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT

FXDEFMAP(I2KWizard) I2KWizardMap[] = {
	FXMAPFUNC(SEL_COMMAND,I2KWizard::ID_BACK,   I2KWizard::onCmdBack),
	FXMAPFUNC(SEL_COMMAND,I2KWizard::ID_NEXT,   I2KWizard::onCmdNext),
	FXMAPFUNC(SEL_COMMAND,I2KWizard::ID_CANCEL, I2KWizard::onCmdCancel),
};

FXIMPLEMENT(I2KWizard, FXVerticalFrame, I2KWizardMap, ARRAYNUMBER(I2KWizardMap));

I2KWizard::I2KWizard(FXComposite* p,
	FXObject* tgt, FXSelector sel, FXuint opts,
	FXint x,  FXint y,  FXint w,  FXint h, 
	FXint pl, FXint pr, FXint pt, FXint pb)
	: FXVerticalFrame(p, opts, x, y, w, h, pl, pr, pt, pb, 0, 0)
{
	target = tgt;
	selector = sel;

	switcher = new FXSwitcher(this, LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_NONE, 0,0,0,0, 0,0,0,0);
	new FXHorizontalSeparator(this, LAYOUT_FILL_X|SEPARATOR_GROOVE, 0,0,0,0, 0,0,0,0);

	bottom = new FXHorizontalFrame(this, FRAME_NONE|LAYOUT_FILL_X,    0,0,0,0, 10,10,12,10, 0,0);
	rbuttons = new FXHorizontalFrame(bottom, FRAME_NONE|LAYOUT_RIGHT, 0,0,0,0, 0,0,0,0,     0,0);

	backbtn = new FXButton(rbuttons, "< &Back", NULL, this, ID_BACK, BUTTON_NORMAL|BUTTON_DEFAULT|LAYOUT_FIX_SIZE, 0,0,75,23, 0,0,0,0);
	backbtn->disable();

	nextbtn = new FXButton(rbuttons, "&Next >", NULL, this, ID_NEXT, BUTTON_NORMAL|BUTTON_INITIAL|BUTTON_DEFAULT|LAYOUT_FIX_SIZE, 0,0,75,23, 0,0,0,0);
	nextbtn->setFocus();

	new FXVerticalSeparator(rbuttons, LAYOUT_FILL_Y|LAYOUT_FIX_WIDTH, 0,0,10,0, 0,0,0,0);

	cancelbtn = new FXButton(rbuttons, "Cancel", NULL, this, ID_CANCEL, BUTTON_NORMAL|BUTTON_DEFAULT|LAYOUT_FIX_SIZE, 0,0,75,23, 0,0,0,0);
}


FXSwitcher* I2KWizard::getSwitcher() {
	return switcher;
}

FXint I2KWizard::getCurrent() {
	return switcher->getCurrent();
}

void I2KWizard::setCurrent(FXint index, FXbool notify) {
	switcher->setCurrent(index, notify);
}

FXButton* I2KWizard::getBackButton()   { return backbtn; }
FXButton* I2KWizard::getNextButton()   { return nextbtn; }
FXButton* I2KWizard::getCancelButton() { return cancelbtn; }

void I2KWizard::setFinish(FXbool finish) {
	// yes, the programmer can use getNextButton and set the text to Finish
	// manually, but you gotta keep translations in mind...
	//
	// there's no translation system in ice2k yet but i'll make one at one
	// point
	if (finish) nextbtn->setText("Finish");
	else nextbtn->setText("&Next >");
}

void I2KWizard::enable() {
	/*lbl->enable();
	sbalance->enable();
	svolume->enable();
	mutechk->enable();*/
}

void I2KWizard::disable() {
	/*lbl->disable();
	sbalance->disable();
	svolume->disable();
	mutechk->disable();*/
}


long I2KWizard::onCmdBack(FXObject* tgt,FXSelector,void* ptr) {
	if(target) target->tryHandle(this, FXSEL(SEL_COMMAND, selector), (void*)(FXuval)IWIZARD_ABACK);	

	return 1;
}

long I2KWizard::onCmdNext(FXObject* tgt,FXSelector,void* ptr) {
	if(target) target->tryHandle(this, FXSEL(SEL_COMMAND, selector), (void*)(FXuval)IWIZARD_ANEXT);	

	return 1;
}

long I2KWizard::onCmdCancel(FXObject* tgt,FXSelector,void* ptr) {
	if(target) target->tryHandle(this, FXSEL(SEL_COMMAND, selector), (void*)(FXuval)IWIZARD_ACANCEL);	

	return 1;
}


I2KWizard::~I2KWizard() {
}



