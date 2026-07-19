#include <fx.h>
#include "ice2k/batmeter/battery.h"
#include "ice2k/batmeter/I2KBatDialog.h"
#include "res/foxres.h"
#include "ice2k/comctl32.h"

FXDEFMAP(I2KBatDialog) I2KBatDialogMap[] = {
	FXMAPFUNC(SEL_COMMAND,           I2KBatDialog::ID_REFRESH,  I2KBatDialog::onCmdRefresh),
};

FXIMPLEMENT(I2KBatDialog, FXDialogBox, I2KBatDialogMap, ARRAYNUMBER(I2KBatDialogMap));

void I2KBatDialog::refreshInfo() {
	char buf[512];
	int lblsize;
	int onlinesize = sizeof("On Line, ")-1;
	int ret;
	
	buf[0] = '\0';

	strcpy(buf, "Battery name:   ");
	lblsize = sizeof("Battery name:   ")-1;
	ret = getPsuInfoStr(battery, "model_name", buf+lblsize, sizeof(buf)-lblsize);

	if (!ret) {
		batnamelbl->setText(" ");
	} else {
		buf[strcspn(buf, "\n")] = '\0';
		batnamelbl->setText(buf);
	}
	
	buf[0] = '\0';

	strcpy(buf, "Unique ID:   ");
	lblsize = sizeof("Unique ID:   ")-1;
	ret = getPsuInfoStr(battery, "serial_number", buf+lblsize, sizeof(buf)-lblsize);
	if (!ret) {
		seriallbl->setText(" ");
	} else {
		buf[strcspn(buf, "\n")] = '\0';
		seriallbl->setText(buf);
	}
	buf[0] = '\0';

	strcpy(buf, "Chemistry:   ");
	lblsize = sizeof("Chemistry:   ")-1;
	ret = getPsuInfoStr(battery, "technology", buf+lblsize, sizeof(buf)-lblsize);
	if (!ret) {
		chemlbl->setText(" ");
	} else {
		buf[strcspn(buf, "\n")] = '\0';
		chemlbl->setText(buf);
	}
	buf[0] = '\0';


	strcpy(buf, "Power state:   ");
	lblsize = sizeof("Power state:   ")-1;

	if (getBatteryPresent(battery)) {
		strcpy(buf+lblsize, "On Line, ");
		ret = getPsuInfoStr(battery, "status", buf+lblsize+onlinesize, sizeof(buf)-lblsize-onlinesize);
	} else {
		strcpy(buf+lblsize, "Off Line");
		ret = 1;
	}

	if (!ret) {
		statuslbl->setText(" ");
	} else {
		buf[strcspn(buf, "\n")] = '\0';
		statuslbl->setText(buf);
	}

	strcpy(buf, "Manufacturer:   ");
	lblsize = sizeof("Manufacturer:   ")-1;
	ret = getPsuInfoStr(battery, "manufacturer", buf+lblsize, sizeof(buf)-lblsize);
	if (!ret) {
		manufacturerlbl->setText(" ");
	} else {
		buf[strcspn(buf, "\n")] = '\0';
		manufacturerlbl->setText(buf);
	}
	buf[0] = '\0';
	//puts("Hi");
}

I2KBatDialog::I2KBatDialog(FXComposite* p, int num, const char* bat)
	: FXDialogBox(p, "Detailed Information", DECOR_TITLE|DECOR_BORDER|DECOR_CLOSE|DECOR_MENU, 0,0,0,0, 11,10,10,13, 12,12) {
	sprintf(battery, "%.*s", (int)sizeof(battery)-1, bat);
	grp = new FXGroupBox(this, "Battery status", FRAME_GROOVE|LAYOUT_FILL_X, 0,0,0,0, 11,11,5,12);

	batnamelbl = new FXLabel(grp, " ");
	seriallbl = new FXLabel(grp, " ");
	chemlbl = new FXLabel(grp, " ");
	statuslbl = new FXLabel(grp, " ");
	manufacturerlbl = new FXLabel(grp, " ");
	manudatelbl = new FXLabel(grp, " ");
	new FXLabel(grp,
			"                                                          "
			"                                                          ",
			NULL, LAYOUT_FIX_HEIGHT, 0,0,0,1, 0,0,0,0);

	new FXButton(grp, "&Refresh", NULL, this, ID_REFRESH,
			BUTTON_NORMAL|BUTTON_DEFAULT, 0,0,0,0, 17,16,2,3);

	FXButton* okbtn = new FXButton(this, "&OK", NULL, this, FXDialogBox::ID_ACCEPT,
			BUTTON_NORMAL|BUTTON_DEFAULT|LAYOUT_RIGHT, 0,0,0,0, 29,28,2,3);
	okbtn->setFocus();

	refreshInfo();
}

I2KBatDialog::~I2KBatDialog() {
}

void I2KBatDialog::create() {
	FXDialogBox::create();
}

long I2KBatDialog::onCmdRefresh(FXObject*, FXSelector, void*) {
	refreshInfo();
	return 1;
}

/*long I2KBatDialog::onTimeout(FXObject*, FXSelector, void*) {
	refreshInfo();
	getApp()->addTimeout(this, ID_TIMEOUT, 2000);
	return 1;
}*/
