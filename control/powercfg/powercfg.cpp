#include <fx.h>
#include <FXPNGIcon.h>
#include <ice2k/comctl32.h>
#include <ice2k/branding.h>
#include <ice2k/batmeter/I2KBatMeter.h>
#include "res/foxres.h"

FXSettings settings;
FXIcon* ico_powercfg_32;
FXIcon* ico_powercfg_16;

int bat_detailed = 0;

class PowerCfgWindow : public FXMainWindow {
	FXDECLARE(PowerCfgWindow);

protected:
	PowerCfgWindow() {}
private:
	FXVerticalFrame*        cont;
	FXTabBook*              tabbook;

	FXPacker*               schemecont;
	FXHorizontalFrame*      schemetopcont;
	FXPacker*               schemecont2;
	FXText*                 schemetext;

	FXGroupBox* schemegrp;
	FXGroupBox* settingsgrp;
	FXListBox* powerbox;

	FXPacker*               batmetercont;
	I2KBatMeter*            batmeter;

	FXPacker*               advanced_frm;
	FXHorizontalFrame*      advanced_top_frm;

	FXGroupBox* options_grp;
	FXGroupBox* powerbtn_grp;
	FXVerticalFrame* powerbtn_frm;
	FXListBox* powerbtn_box;
	FXCheckButton* showicon_chk;


	FXButton*               okbtn;
	FXButton*               cancelbtn;
	FXButton*               applybtn;
public:
	long onCmdPowerBox(FXObject*,FXSelector,void*);
	long onCmdShowIcon(FXObject*,FXSelector,void*);
	long onCmdBatMeter(FXObject*,FXSelector,void*);


	long onCmdDialogOK(FXObject*,FXSelector,void*);
	long onCmdDialogApply(FXObject*,FXSelector,void*);
	long onCmdDialogCancel(FXObject*,FXSelector,void*);

	
public:
	enum {
		ID_POWERBOX = FXMainWindow::ID_LAST,
		ID_SHOWICON,
		ID_BATMETER,
		ID_DLG_OK,
		ID_DLG_CANCEL,
		ID_DLG_APPLY,
	};

public:
	PowerCfgWindow(FXApp* a);

	virtual void create();
	void setFocus() {};
	virtual ~PowerCfgWindow();
};

FXDEFMAP(PowerCfgWindow) PowerCfgWindowMap[] = {
	FXMAPFUNC(SEL_COMMAND, PowerCfgWindow::ID_DLG_OK, PowerCfgWindow::onCmdDialogOK),
	FXMAPFUNC(SEL_COMMAND, PowerCfgWindow::ID_DLG_APPLY, PowerCfgWindow::onCmdDialogApply),
	FXMAPFUNC(SEL_COMMAND, PowerCfgWindow::ID_DLG_CANCEL, PowerCfgWindow::onCmdDialogCancel),
	FXMAPFUNC(SEL_COMMAND, PowerCfgWindow::ID_POWERBOX, PowerCfgWindow::onCmdPowerBox),

	FXMAPFUNC(SEL_COMMAND, PowerCfgWindow::ID_SHOWICON, PowerCfgWindow::onCmdShowIcon),
	FXMAPFUNC(SEL_COMMAND, PowerCfgWindow::ID_BATMETER, PowerCfgWindow::onCmdBatMeter),



};

FXIMPLEMENT(PowerCfgWindow, FXMainWindow, PowerCfgWindowMap, ARRAYNUMBER(PowerCfgWindowMap));


#define GOVERNOR_PERFORMANCE 1
#define GOVERNOR_POWERSAVE 2
#define GOVERNOR_SCHEDUTIL 3
#define GOVERNOR_ONDEMAND 4

long PowerCfgWindow::onCmdDialogOK(FXObject* obj,FXSelector sel, void* ptr) {
	onCmdDialogApply(obj, sel, ptr);
	getApp()->exit();
	return 1;
}


long PowerCfgWindow::onCmdDialogCancel(FXObject* obj,FXSelector sel, void* ptr) {
	getApp()->exit(1);
	return 1;
}

long PowerCfgWindow::onCmdDialogApply(FXObject* obj,FXSelector sel, void* ptr) {
	int index = powerbox->getCurrentItem();
	if (index != 0) {
		unsigned gov = (unsigned)(FXuval)powerbox->getItemData(index);
		switch (gov) {
			case GOVERNOR_PERFORMANCE:
				system("i2ksudo cpupower frequency-set -g performance > /dev/null &");
				break;
			case GOVERNOR_POWERSAVE:
				system("i2ksudo cpupower frequency-set -g powersave > /dev/null &");
				break;
			case GOVERNOR_SCHEDUTIL:
				system("i2ksudo cpupower frequency-set -g schedutil > /dev/null &");
				break;
			case GOVERNOR_ONDEMAND:
				system("i2ksudo cpupower frequency-set -g ondemand > /dev/null &");
				break;
		}
	}

	FXString desktopfile = FXSystem::getHomeDirectory()+PATHSEPSTRING+".foxrc"+PATHSEPSTRING+"Desktop";
	if (FXStat::exists(desktopfile)) {
		settings.parseFile(desktopfile, TRUE);
	}



	settings.writeIntEntry("BatMeter", "AlwaysShow", showicon_chk->getCheck());
	settings.writeIntEntry("BatMeter", "Detailed", bat_detailed);


	settings.unparseFile(desktopfile);
	settings.clear();


	applybtn->disable();
	return 1;
}

long PowerCfgWindow::onCmdBatMeter(FXObject* obj,FXSelector sel, void* ptr) {
	switch((unsigned)(FXuval)ptr) {
		case BATMETER_DETAILED_ON:
			bat_detailed = 1;
			break;
		case BATMETER_DETAILED_OFF:
			bat_detailed = 0;
			break;
	}

	applybtn->enable();
	return 1;
}

long PowerCfgWindow::onCmdPowerBox(FXObject* obj,FXSelector sel, void* ptr) {
	//obj->handle(this, FXSEL(SEL_COMMAND,ID_ENABLE), NULL);

	applybtn->enable();


	return 1;
}


long PowerCfgWindow::onCmdShowIcon(FXObject* obj,FXSelector sel, void* ptr) {
	//obj->handle(this, FXSEL(SEL_COMMAND,ID_ENABLE), NULL);

	applybtn->enable();


	return 1;
}

void addPowerSchemes(FXListBox* lb) {
	char buf[256];
	char curbuf[64];
	char* token;
	FILE* fp;
	FILE* fpcur;
	int curgov;

	int govperformance = 0;
	int govpowersave = 0;
	int govschedutil = 0;
	int govondemand = 0;

	int anyadded = 0;

	if (lb == NULL) goto fail;
	
	fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors", "r");
	if (fp == NULL) goto fail;
	fgets(buf, sizeof(buf), fp);
	fclose(fp);

	if (buf[0] == '\0') goto fail;

	fpcur = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "r");
	if (fpcur == NULL) goto fail;
	fgets(curbuf, sizeof(curbuf), fpcur);
	fclose(fpcur);

	curbuf[strcspn(curbuf, "\n")] = '\0';

	token = strtok(buf, " \n");
	
	while (token != NULL) {
		if (strcmp(token, "performance") == 0) {
			govperformance = 1;
			anyadded = 1;
		} else if (strcmp(token, "ondemand") == 0) {
			govondemand = 1;
			anyadded = 1;
		} else if (strcmp(token, "schedutil") == 0) {
			govschedutil = 1;
			anyadded = 1;
		} else if (strcmp(token, "powersave") == 0) {
			govpowersave = 1;
			anyadded = 1;
		}

		token = strtok(NULL, " \n");
	}

	if (curbuf[0] != '\0') {
		if (strcmp(curbuf, "performance") == 0) {
			curgov = GOVERNOR_PERFORMANCE;
		} else if (strcmp(curbuf, "ondemand") == 0) {
			curgov = GOVERNOR_ONDEMAND;
		} else if (strcmp(curbuf, "schedutil") == 0) {
			curgov = GOVERNOR_SCHEDUTIL;
		} else if (strcmp(curbuf, "powersave") == 0) {
			curgov = GOVERNOR_POWERSAVE;
		}
	}

	if (govperformance) {
		lb->appendItem("Performance", NULL, (void*)(FXuval)GOVERNOR_PERFORMANCE);
		if (curgov == GOVERNOR_PERFORMANCE) lb->setCurrentItem(lb->getNumItems()-1);
	}

	if (govschedutil) {
		lb->appendItem("Balanced", NULL, (void*)(FXuval)GOVERNOR_SCHEDUTIL);

		if (curgov == GOVERNOR_SCHEDUTIL || curgov == GOVERNOR_ONDEMAND) {
			lb->setCurrentItem(lb->getNumItems()-1);
		}
	} else if (govondemand) {
		lb->appendItem("Balanced", NULL, (void*)(FXuval)GOVERNOR_ONDEMAND);

		if (curgov == GOVERNOR_SCHEDUTIL || curgov == GOVERNOR_ONDEMAND) {
			lb->setCurrentItem(lb->getNumItems()-1);
		}
	}

	if (govpowersave) {
		lb->appendItem("Power Saving", NULL, (void*)(FXuval)GOVERNOR_POWERSAVE);
		if (curgov == GOVERNOR_POWERSAVE) lb->setCurrentItem(lb->getNumItems()-1);
	}

	lb->setNumVisible(lb->getNumItems());

	if (anyadded) return;

fail:
	lb->disable();
	lb->clearItems();
	lb->appendItem("(Unknown)", NULL, 0);
}

PowerCfgWindow::PowerCfgWindow(FXApp *a) : FXMainWindow(a, "Power Options Properties", ico_powercfg_16, NULL, DECOR_BORDER|DECOR_CLOSE|DECOR_TITLE, 0,0,0,0, 0,0,3,3, 0,6) {
	FXPacker* tempcont;
	FXButton* btn;

	cont = new FXVerticalFrame(this, LAYOUT_FILL_Y|LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0, 0,0);
	
	tabbook = new FXTabBook(cont, NULL, 0, TABBOOK_NORMAL|LAYOUT_FILL, 0,0,0,0, 6,6,5,5);
	new FXTabItem(tabbook, "Power Schemes", NULL, TAB_TOP_NORMAL, 0,0,0,0, 4,4,1,2);
	schemecont = new FXVerticalFrame(tabbook, LAYOUT_FILL|FRAME_RAISED|FRAME_THICK, 0,0,0,0, 13,2,11,10, 8,8);
	schemetopcont = new FXHorizontalFrame(schemecont, LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0, 16,16);

	new FXLabel(schemetopcont, "", ico_powercfg_32, FRAME_NONE, 0,0,0,0, 0,0,0,0);

	schemetext = new FXText(schemetopcont, NULL, 0, LAYOUT_FILL_X|LAYOUT_FIX_HEIGHT|TEXT_WORDWRAP);
	schemetext->setText(
			"Select the power scheme with the most appropriate settings "
			"for this computer. Note that changing the settings below "
			"will modify the selected scheme.");
	schemetext->disable();
	schemetext->setMarginTop(1);
	schemetext->setMarginLeft(0);
	schemetext->setMarginBottom(5);
	schemetext->setMarginRight(0);
	schemetext->setBackColor(getApp()->getBaseColor());
	schemetext->setDefaultCursor(getApp()->getDefaultCursor(DEF_ARROW_CURSOR));

	schemecont2 = new FXVerticalFrame(schemecont, LAYOUT_FILL, 0,0,0,0, 0,10,0,0, 8,8);
	schemegrp = new FXGroupBox(schemecont2, "Power schemes", GROUPBOX_NORMAL|FRAME_GROOVE|LAYOUT_FILL_X, 0,0,0,0, 8,9, 2,8, 8,8);
	FXListBox* schemesbox = new FXListBox(schemegrp, NULL, 0, FRAME_NORMAL|LISTBOX_NORMAL|LAYOUT_FILL_X, 0,0,0,0, 3,3,2,1);
	schemesbox->appendItem("(None)");
	schemesbox->disable();
	FXHorizontalFrame* schemebtns = new FXHorizontalFrame(schemegrp, PACK_UNIFORM_WIDTH|LAYOUT_RIGHT, 0,0,0,0, 0,0,0,0, 4,4);
	btn = new FXButton(schemebtns, "&Save As...", NULL, NULL, 0, BUTTON_NORMAL, 0,0,0,0, 10,10,2,3);
	btn->disable();
	btn = new FXButton(schemebtns, "&Delete", NULL, NULL, 0, BUTTON_NORMAL, 0,0,0,0, 10,10,2,3);
	btn->disable();

	settingsgrp = new FXGroupBox(schemecont2, "Settings", GROUPBOX_NORMAL|FRAME_GROOVE|LAYOUT_FILL_X, 0,0,0,0, 8,9, 2,10);
	tempcont = new FXMatrix(settingsgrp, 2, MATRIX_BY_COLUMNS|LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0, 8,8);
	new FXLabel(tempcont, "CPU governor: ", NULL, LAYOUT_CENTER_Y);
	powerbox = new FXListBox(tempcont, this, ID_POWERBOX, FRAME_NORMAL|LISTBOX_NORMAL|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN, 0,0,0,0, 3,3,2,1);
	addPowerSchemes(powerbox);


	/*new FXLabel(tempcont, "Turn off &monitor: ", NULL, LAYOUT_CENTER_Y);
	new FXListBox(tempcont, NULL, 0, FRAME_NORMAL|LISTBOX_NORMAL|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN, 0,0,0,0, 3,3,2,1);*/


	new FXTabItem(tabbook, "Power Meter", NULL, TAB_TOP_NORMAL, 0,0,0,0, 4,4,1,2);
	batmetercont = new FXVerticalFrame(tabbook, LAYOUT_FILL|FRAME_RAISED|FRAME_THICK, 0,0,0,0, 6,8,9,8, 0,0);
	batmeter = new I2KBatMeter(batmetercont, LAYOUT_FILL);
	batmeter->setTarget(this);
	batmeter->setSelector(ID_BATMETER);
	batmeter->hideAlwaysShowChk();

	new FXTabItem(tabbook, "Advanced", NULL, TAB_TOP_NORMAL, 0,0,0,0, 4,4,1,2);

	advanced_frm = new FXVerticalFrame(tabbook, LAYOUT_FILL|FRAME_RAISED|FRAME_THICK, 0,0,0,0, 13,15,11,30, 8,8);
	advanced_top_frm = new FXHorizontalFrame(advanced_frm, LAYOUT_FILL_X, 0,0,0,0, 0,0,0,4, 16,16);
	new FXLabel(advanced_top_frm, "", ico_powercfg_32, FRAME_NONE, 0,0,0,0, 0,0,0,0);
	new FXLabel(advanced_top_frm, "Select the power-saving settings you want to use.", NULL, LAYOUT_CENTER_Y, 0,0,0,0, 0,0,0,0);

	options_grp = new FXGroupBox(advanced_frm, "Options", GROUPBOX_NORMAL|FRAME_GROOVE|LAYOUT_FILL_X, 0,0,0,0, 6,7, 2,60);
	showicon_chk = new FXCheckButton(options_grp, "Alway&s show icon on the taskbar", this, ID_SHOWICON);
	showicon_chk->setCheck(getApp()->reg().readIntEntry("BatMeter", "AlwaysShow", FALSE));
	powerbtn_grp = new FXGroupBox(advanced_frm, "Power buttons", GROUPBOX_NORMAL|FRAME_GROOVE|LAYOUT_FILL, 0,0,0,0, 6,7, 2,20);
	powerbtn_frm = new FXVerticalFrame(powerbtn_grp, LAYOUT_CENTER_Y|LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0, 4,4);
	
	new FXLabel(powerbtn_frm, "Wh&en I press the power button on my computer:", NULL, LABEL_NORMAL, 0,0,0,0, 0,0,0,0);
	powerbtn_box = new FXListBox(powerbtn_frm, NULL, 0, FRAME_NORMAL|LISTBOX_NORMAL|LAYOUT_FILL_X, 0,0,0,0, 3,3,2,1);
	powerbtn_box->appendItem("Performance");
	powerbtn_box->appendItem("Power Saving");
	powerbtn_box->appendItem("Balanced");
	powerbtn_box->disable();


	FXHorizontalFrame* btncont = new FXHorizontalFrame(cont, LAYOUT_RIGHT|PACK_UNIFORM_WIDTH, 0,0,0,0, 0,6,1,4, 6,0);

	bat_detailed = getApp()->reg().readIntEntry("BatMeter", "Detailed", FALSE);
	
	okbtn = new FXButton(btncont, "OK", NULL, this, ID_DLG_OK, BUTTON_DEFAULT|BUTTON_NORMAL, 0,0,0,0, 19,20,2,3);
	cancelbtn = new FXButton(btncont, "Cancel", NULL, this, ID_DLG_CANCEL, BUTTON_NORMAL|BUTTON_DEFAULT, 0,0,0,0, 19,20,2,3);
	applybtn = new FXButton(btncont, "&Apply", NULL, this, ID_DLG_APPLY, BUTTON_NORMAL|BUTTON_DEFAULT, 0,0,0,0, 19,19,2,3);
	applybtn->disable();


}

PowerCfgWindow::~PowerCfgWindow() {}

void PowerCfgWindow::create() {
	FXMainWindow::create();
	schemetext->setHeight(schemetext->getContentHeight());

	show(PLACEMENT_SCREEN);
}

int main(int argc, char *argv[]) {
	FXApp application("PowerCfg", "I2KProject");
	application.init(argc, argv);

	if (i2kBGetWinVersionInt() > ICE2K_BRAND_WIN2K) {
		ico_powercfg_32 = new FXPNGIcon(&application, resico_xp_powercfg_32);
		ico_powercfg_32->blend(application.getBaseColor());
		ico_powercfg_16 = new FXPNGIcon(&application, resico_xp_powercfg_16);
	} else {
		ico_powercfg_32 = new FXPNGIcon(&application, resico_2k_powercfg_32);
		ico_powercfg_16 = new FXPNGIcon(&application, resico_2k_powercfg_16);
	}

	new PowerCfgWindow(&application);

	application.create();
	return application.run();
}
