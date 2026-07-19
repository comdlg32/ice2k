#pragma once

#include <fx.h>
#include "I2KBatButton.h"

class I2KBatMeter : public FXVerticalFrame {
	FXDECLARE(I2KBatMeter);

protected:
	I2KBatMeter() {}

private:
	FXIcon* ico_batfull;
	FXIcon* ico_bathalf;
	FXIcon* ico_batlow;
	FXIcon* ico_batcrit;

	FXIcon* ico_charging;
	FXIcon* ico_plug;

	FXIcon* ico_notpres;
	FXIcon* ico_unknown;

	FXIcon* ico_battplug;
	FXIcon* ico_battery;
	FXIcon* ico_chargeplug;

	FXHorizontalFrame* topcont;
	
	FXCheckButton* showiconchk;
	FXCheckButton* showdetailschk;

	FXPacker* powergrpcont;
	FXGroupBox* powergrp;

	FXHorizontalFrame* powerinfocont;
	FXMatrix* powerinfomtx;

	FXLabel* powersrc;
	FXLabel* powerremain;
	FXLabel* powerdurationleft;
	FXLabel* powerduration;
	FXLabel* iconlbl;

	FXSwitcher* switcher;
	FXVerticalFrame* simplecont;
	FXVerticalFrame* detailedcont;

	FXMatrix* batbtnmtx;

	I2KBatButton* batbtn1;
	I2KBatButton* batbtn2;
	I2KBatButton* batbtn3;
	I2KBatButton* batbtn4;
	I2KBatButton* batbtn5;
	I2KBatButton* batbtn6;
	I2KBatButton* batbtn7;
	I2KBatButton* batbtn8;

	FXPacker* pbarcont;
	FXProgressBar* pbar;

	FXLabel* caplbl;

	//FXframe* batbtnspacer;
public:
	void refreshInfo();
	long onChkDetailed(FXObject*, FXSelector, void*);
	long onCmdBattery(FXObject*, FXSelector, void*);

	long onTimeout(FXObject*, FXSelector, void*);
	void hideAlwaysShowChk();
	void showAlwaysShowChk();

public:
	enum {
		ID_DETAILED = FXSwitcher::ID_LAST,
		ID_BATBUTTON,
		ID_TIMEOUT,
		ID_LAST
	};

public:
	I2KBatMeter(FXComposite* p, FXuint opts=0, FXint x=0, FXint y=0, FXint w=0, FXint h=0);

	virtual void create();
	virtual ~I2KBatMeter();
};

