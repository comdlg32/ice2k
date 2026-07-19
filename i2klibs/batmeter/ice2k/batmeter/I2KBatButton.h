#pragma once
#include <fx.h>
#include "battery.h"
//#include <ice2k/comctl32.h>

class I2KBatButton : public FXSwitcher {
	FXDECLARE(I2KBatButton);

protected:
	I2KBatButton() {}
private:
	char battery[POWER_PSU_NAME_MAX+1];
private:
	FXObject* target;
	FXSelector selector;

	FXVerticalFrame* maincont;

	FXHorizontalFrame* horcont;
	FXVerticalFrame* vercont;

	FXLabel* caplbl;
	FXLabel* botlbl;
	FXButton* btn;
	FXLabel* numlbl;

public:
	void refreshInfo();

	void hideOnScreen();
	void showOnScreen();

	long onCmdButton(FXObject*, FXSelector, void*);

	void setPresent(int present);
	void setPercentage(int percentage);

	void setIcon(FXIcon* ic);
	void setText(const FXString& text);
	void setBattery(const char* bat);


public:
	enum {
		ID_COMMAND = FXMainWindow::ID_LAST,
		ID_LAST
	};

public:
	//I2KBatButton(FXComposite* p, FXuint opts=0, FXint x=0, FXint y=0, FXint w=0, FXint h=0);
	I2KBatButton(FXComposite* p, const FXString& text, FXIcon* ic=NULL, FXObject* tgt=NULL, FXSelector sel=0, FXuint opts=0, FXint x=0, FXint y=0, FXint w=0, FXint h=0);
	virtual void create();
	virtual ~I2KBatButton();
};

