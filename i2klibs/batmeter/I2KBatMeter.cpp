#include <fx.h>
#include "ice2k/batmeter/battery.h"
#include "ice2k/batmeter/I2KBatMeter.h"
#include "ice2k/batmeter/I2KBatButton.h"
#include "ice2k/batmeter/I2KBatDialog.h"

#include "res/foxres.h"
#include "ice2k/comctl32.h"

FXDEFMAP(I2KBatMeter) I2KBatMeterMap[] = {
	FXMAPFUNC(SEL_COMMAND,           I2KBatMeter::ID_BATBUTTON,  I2KBatMeter::onCmdBattery),
	FXMAPFUNC(SEL_COMMAND,           I2KBatMeter::ID_DETAILED,  I2KBatMeter::onChkDetailed),
	FXMAPFUNC(SEL_TIMEOUT,           I2KBatMeter::ID_TIMEOUT,  I2KBatMeter::onTimeout),
};

FXIMPLEMENT(I2KBatMeter, FXVerticalFrame, I2KBatMeterMap, ARRAYNUMBER(I2KBatMeterMap));


char batteries[POWER_PSU_NAME_MAX*8];

static void secToStr(char* str, unsigned int sec) {
	int h = sec / 3600;
	int m = ( sec % 3600) / 60;
	int s = sec % 60;

	if (h > 99) {
		h = 99;
		m = 59;
		s = 59;
	}

	sprintf(str, "%02d:%02d:%02d", h, m, s);

	return;
}

void I2KBatMeter::hideAlwaysShowChk() {
	showiconchk->hide();
	topcont->setPadLeft(5);
}

void I2KBatMeter::showAlwaysShowChk() {
	showiconchk->show();
	topcont->setPadLeft(0);
}

void I2KBatMeter::refreshInfo() {
	int status = getOverallStatus();
	int powersource = getCurrentPowerSource();
	int percentage = 0;
	char percentagestr[64];
	char percentage2str[64];


	//char batteries[POWER_PSU_NAME_MAX*8];
	
	getOverallPercentage(&percentage);

	if (powersource == POWER_PSU_TYPE_BATTERY) {
		powersrc->setText("Batteries");

		if (percentage > 66) {
			iconlbl->setIcon(ico_batfull);
		} else if (percentage > 33) {
			iconlbl->setIcon(ico_bathalf);
		} else if (percentage > 9) {
			iconlbl->setIcon(ico_batlow);
		} else {
			iconlbl->setIcon(ico_batcrit);
		}
	} else {
		powersrc->setText("AC power");
	}
	
	int duration = 0;
	char durationstr[sizeof("99:59:59")];
	if (status == POWER_BAT_STATUS_CHARGING) {
		sprintf(percentagestr, "%d%%      (charging)", percentage);
		sprintf(percentage2str, "%d%%", percentage);

		getOverallDuration(1, &duration);
		if (powersource != POWER_PSU_TYPE_BATTERY) {
			iconlbl->setIcon(ico_chargeplug);
		}
	} else {
		sprintf(percentagestr, "%d%%", percentage);
		strcpy(percentage2str, percentagestr);
		getOverallDuration(0, &duration);
		if (powersource != POWER_PSU_TYPE_BATTERY) {
			iconlbl->setIcon(ico_plug);
		}
	}
	//puts(percentage2str);
	
	pbar->setProgress(percentage);
	caplbl->setText(percentage2str);
	secToStr(durationstr, duration);

	if ((duration > 0 && duration < 360000) &&
			status != POWER_BAT_STATUS_FULL) {
		powerduration->setText(durationstr);
		powerdurationleft->setText("Total time remaining:");
	} else {
		powerduration->setText(" ");
		powerdurationleft->setText(" ");
	}

	powerremain->setText(percentagestr);

	batbtn1->hideOnScreen();
	batbtn1->setUserData((void*)NULL);
	batbtn2->hideOnScreen();
	batbtn2->setUserData((void*)NULL);
	batbtn3->hideOnScreen();
	batbtn3->setUserData((void*)NULL);
	batbtn4->hideOnScreen();
	batbtn4->setUserData((void*)NULL);
	batbtn5->hideOnScreen();
	batbtn5->setUserData((void*)NULL);
	batbtn6->hideOnScreen();
	batbtn6->setUserData((void*)NULL);
	batbtn7->hideOnScreen();
	batbtn7->setUserData((void*)NULL);
	batbtn8->hideOnScreen();
	batbtn8->setUserData((void*)NULL);

	int amount = getPowerSupplies(batteries, sizeof(batteries),
			POWER_PSU_TYPE_BATTERY);

	if (amount != 0) {
		int i = 0;
		I2KBatButton* batbtn = batbtn1;

		while (amount > i && 8 > i) {
			//puts(batteries+(i*POWER_PSU_NAME_MAX));
			switch(i+1) {
				case 1: batbtn = batbtn1; break;
				case 2: batbtn = batbtn2; break;
				case 3: batbtn = batbtn3; break;
				case 4: batbtn = batbtn4; break;
				case 5: batbtn = batbtn5; break;
				case 6: batbtn = batbtn6; break;
				case 7: batbtn = batbtn7; break;
				case 8: batbtn = batbtn8; break;
			}

			int present = getBatteryPresent(batteries+(i*POWER_PSU_NAME_MAX));
			status = getBatteryStatus(batteries+(i*POWER_PSU_NAME_MAX));

			getBatteryCapacity(batteries+(i*POWER_PSU_NAME_MAX), &percentage);

			if (!present) {
				batbtn->setIcon(ico_notpres);
				batbtn->setPresent(0);
			} else if (percentage > 66) {
				batbtn->setIcon(ico_batfull);
				batbtn->setPresent(1);
			} else if (percentage > 33) {
				batbtn->setIcon(ico_bathalf);
				batbtn->setPresent(1);
			} else if (percentage > 9) {
				batbtn->setIcon(ico_batlow);
				batbtn->setPresent(1);
			} else {
				batbtn->setIcon(ico_batcrit);
				batbtn->setPresent(1);
			}

			batbtn->setPercentage(percentage);
			batbtn->showOnScreen();
			batbtn->setUserData((void*)(batteries+(i*POWER_PSU_NAME_MAX)));

			++i;
		}
	}
}

I2KBatMeter::I2KBatMeter(FXComposite* p, FXuint opts, FXint x, FXint y, FXint w, FXint h) 
	: FXVerticalFrame(p, opts, x,y,w,h, 0,0,0,0) {
	ico_batcrit = new FXGIFIcon(getApp(), resico_batcrit);
	ico_batcrit->create();
	ico_batfull = new FXGIFIcon(getApp(), resico_batfull);
	ico_batfull->create();
	ico_bathalf = new FXGIFIcon(getApp(), resico_bathalf);
	ico_bathalf->create();
	ico_batlow = new FXGIFIcon(getApp(), resico_batlow);
	ico_batlow->create();
	ico_battery = new FXGIFIcon(getApp(), resico_battery);
	ico_battery->create();
	ico_battplug = new FXGIFIcon(getApp(), resico_battplug);
	ico_battplug->create();
	ico_charging = new FXGIFIcon(getApp(), resico_charging);
	ico_charging->create();
	ico_notpres = new FXGIFIcon(getApp(), resico_notpres);
	ico_notpres->create();
	ico_plug = new FXGIFIcon(getApp(), resico_plug);
	ico_plug->create();
	ico_unknown = new FXGIFIcon(getApp(), resico_unknown);
	ico_unknown->create();

	ico_chargeplug = new FXIcon(getApp(), NULL, FXRGB(192,192,192), 0, 32, 32);
	ico_chargeplug->create();

	FXDCWindow dc_chargeplug(ico_chargeplug);
	dc_chargeplug.setForeground(getApp()->getBaseColor());
	dc_chargeplug.fillRectangle(0, 0, 32, 32);
	dc_chargeplug.drawIcon(ico_plug, 0,0);
	dc_chargeplug.drawIcon(ico_charging, 0,0);
	dc_chargeplug.end();

	ico_chargeplug->restore();
	ico_chargeplug->render();

	topcont = new FXHorizontalFrame(this, LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0, 12,12);
	showiconchk = new FXCheckButton(topcont, "&Always show icon on the taskbar.");
	showdetailschk = new FXCheckButton(topcont, "Show details for each &battery.", this, ID_DETAILED,
			CHECKBUTTON_NORMAL);

	powergrpcont = new FXPacker(this, LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 7,8,1,5, 0,0);
	powergrp = new FXGroupBox(powergrpcont, "Power status", FRAME_GROOVE|LAYOUT_FILL_X|LAYOUT_FILL_Y);

	powerinfocont = new FXHorizontalFrame(powergrp, LAYOUT_FILL_X, 0,0,0,0, 24,6,6,6, 14,14);
	iconlbl = new FXLabel(powerinfocont, "", ico_bathalf, LABEL_NORMAL, 0,0,0,0, 0,0,0,0);
	powerinfomtx = new FXMatrix(powerinfocont, 2, MATRIX_BY_COLUMNS|LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0, 13,2);

	new FXLabel(powerinfomtx, "Current power source:", NULL, LABEL_NORMAL, 0,0,0,0, 0,0,0,0);
	powersrc = new FXLabel(powerinfomtx, "POWERSOURCE", NULL, LABEL_NORMAL, 0,0,0,0, 0,0,0,0);

	new FXLabel(powerinfomtx, "Total battery power remaining:", NULL, LABEL_NORMAL, 0,0,0,0, 0,0,0,0);
	powerremain = new FXLabel(powerinfomtx, "POWERREMAIN", NULL, LABEL_NORMAL, 0,0,0,0, 0,0,0,0);
	powerdurationleft = new FXLabel(powerinfomtx, "Total time remaining:", NULL, LABEL_NORMAL, 0,0,0,0, 0,0,0,0);
	powerduration = new FXLabel(powerinfomtx, "POWERDURATION", NULL, LABEL_NORMAL, 0,0,0,0, 0,0,0,0);

	switcher = new FXSwitcher(powergrp, LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0);
	simplecont = new FXVerticalFrame(switcher, LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 2,10,14,0);

	pbarcont = new FXPacker(simplecont,FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FIX_HEIGHT, 0,0,0,23, 1,1,1,1);

	pbar = new FXProgressBar(pbarcont,NULL,0,LAYOUT_FILL_X|LAYOUT_FILL_Y);
	pbar->setBarBGColor(getApp()->getBaseColor());
	pbar->setBarColor(getApp()->reg().readColorEntry("SETTINGS", "selbackcolor", FXRGB(0, 0, 255)));

	pbar->setProgress(50);
	pbar->setTotal(100);

	caplbl = new FXLabel(simplecont, "50%", NULL, LAYOUT_CENTER_X, 0,0,0,0, 4,4,8,4);


	detailedcont = new FXVerticalFrame(switcher, LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0);
	switcher->setCurrent(0);

	batbtnmtx = new FXMatrix(detailedcont, 4, MATRIX_BY_COLUMNS|LAYOUT_FILL_X|PACK_UNIFORM_HEIGHT|PACK_UNIFORM_WIDTH, 0,0,0,0, 5,3,0,0, 8,8);

	batbtn1 = new I2KBatButton(batbtnmtx, "#1", ico_batcrit, this, ID_BATBUTTON, LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_CENTER_X);
	batbtn2 = new I2KBatButton(batbtnmtx, "#2", ico_batcrit, this, ID_BATBUTTON, LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_CENTER_X);
	batbtn3 = new I2KBatButton(batbtnmtx, "#3", ico_batcrit, this, ID_BATBUTTON, LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_CENTER_X);
	batbtn4 = new I2KBatButton(batbtnmtx, "#4", ico_batcrit, this, ID_BATBUTTON, LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_CENTER_X);

	batbtn5 = new I2KBatButton(batbtnmtx, "#5", ico_batcrit, this, ID_BATBUTTON, LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_CENTER_X);
	batbtn6 = new I2KBatButton(batbtnmtx, "#6", ico_batcrit, this, ID_BATBUTTON, LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_CENTER_X);
	batbtn7 = new I2KBatButton(batbtnmtx, "#7", ico_batcrit, this, ID_BATBUTTON, LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_CENTER_X);
	batbtn8 = new I2KBatButton(batbtnmtx, "#8", ico_batcrit, this, ID_BATBUTTON, LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_CENTER_X);

	new FXLabel(detailedcont, "Click an individual battery icon for more information.", NULL, LAYOUT_FILL_X, 0,0,0,0, 40,40,37,13);
	//batbtn8->hideOnScreen();

	//new FXFrame(batbtnmtx, FRAME_NONE, 0,0,0,0, 0,0,0,0);

	refreshInfo();

	getApp()->addTimeout(this, ID_TIMEOUT, 2000);
}

I2KBatMeter::~I2KBatMeter() {
}

void I2KBatMeter::create() {
	FXVerticalFrame::create();
}

long I2KBatMeter::onChkDetailed(FXObject*, FXSelector, void* ptr) {
	if (ptr == 0) switcher->setCurrent(0);
	else switcher->setCurrent(1);
	return 1;
}

long I2KBatMeter::onCmdBattery(FXObject* obj, FXSelector, void*) {
	I2KBatDialog dlg(this, 1, (char*)((I2KBatButton*)obj)->getUserData());
	dlg.execute(PLACEMENT_OWNER);
	return 1;
}

long I2KBatMeter::onTimeout(FXObject*, FXSelector, void*) {
	refreshInfo();
	getApp()->addTimeout(this, ID_TIMEOUT, 2000);
	return 1;
}
