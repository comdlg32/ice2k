#include "I2KWizHeader.h"

#define LAYOUT_FIX_SIZE LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT

FXDEFMAP(I2KWizHeader) I2KWizHeaderMap[] = {
};

FXIMPLEMENT(I2KWizHeader, FXVerticalFrame, I2KWizHeaderMap, ARRAYNUMBER(I2KWizHeaderMap));

I2KWizHeader::I2KWizHeader(FXComposite* p,
	FXImage* img, FXString& text, FXString& subtext, FXuint opts,
	FXint x,  FXint y,  FXint w,  FXint h, 
	FXint pl, FXint pr, FXint pt, FXint pb)
	: FXFrame(p, opts, x, y, w, h, pl, pr, pt, pb)
{
	txt = text;
	subtxt = subtext;
}


FXSwitcher* I2KWizHeader::getSwitcher() {
	return switcher;
}

FXint I2KWizHeader::getCurrent() {
	return switcher->getCurrent();
}

void I2KWizHeader::setCurrent(FXint index, FXbool notify) {
	switcher->setCurrent(index, notify);
}

FXButton* I2KWizHeader::getBackButton()   { return backbtn; }
FXButton* I2KWizHeader::getNextButton()   { return nextbtn; }
FXButton* I2KWizHeader::getCancelButton() { return cancelbtn; }

void I2KWizHeader::setFinish(FXbool finish) {
	// yes, the programmer can use getNextButton and set the text to Finish
	// manually, but you gotta keep translations in mind...
	//
	// there's no translation system in ice2k yet but i'll make one at one
	// point
	if (finish) nextbtn->setText("Finish");
	else nextbtn->setText("&Next >");
}

void I2KWizHeader::enable() {
	/*lbl->enable();
	sbalance->enable();
	svolume->enable();
	mutechk->enable();*/
}

void I2KWizHeader::disable() {
	/*lbl->disable();
	sbalance->disable();
	svolume->disable();
	mutechk->disable();*/
}


long I2KWizHeader::onCmdBack(FXObject* tgt,FXSelector,void* ptr) {
	if(target) target->tryHandle(this, FXSEL(SEL_COMMAND, selector), (void*)(FXuval)IWIZARD_ABACK);	

	return 1;
}

long I2KWizHeader::onCmdNext(FXObject* tgt,FXSelector,void* ptr) {
	if(target) target->tryHandle(this, FXSEL(SEL_COMMAND, selector), (void*)(FXuval)IWIZARD_ANEXT);	

	return 1;
}

long I2KWizHeader::onCmdCancel(FXObject* tgt,FXSelector,void* ptr) {
	if(target) target->tryHandle(this, FXSEL(SEL_COMMAND, selector), (void*)(FXuval)IWIZARD_ACANCEL);	

	return 1;
}


I2KWizHeader::~I2KWizHeader() {
}



