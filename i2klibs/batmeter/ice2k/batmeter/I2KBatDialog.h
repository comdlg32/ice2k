#pragma once
#include <fx.h>
//#include <ice2k/comctl32.h>
//#include "battery.h"

class I2KBatDialog : public FXDialogBox {
	FXDECLARE(I2KBatDialog);

protected:
	I2KBatDialog() {}
private:
	char battery[POWER_PSU_NAME_MAX+1];
private:
	FXObject* target;
	FXSelector sel;
	FXGroupBox* grp;
	//FXVerticalFrame* vercont;

	FXLabel* batnamelbl;
	FXLabel* seriallbl;
	FXLabel* chemlbl;
	FXLabel* statuslbl;
	FXLabel* manufacturerlbl;
	FXLabel* manudatelbl;

	FXButton* refbtn;
	FXButton* okbtn;

public:
	void refreshInfo();
	void setFocus() {return;};


	long onCmdRefresh(FXObject*, FXSelector, void*);

	void setBattery(const char* bat);
	void setBatteryNum(const char* bat);

public:
	enum {
		ID_REFRESH = FXDialogBox::ID_LAST,
		ID_LAST
	};

public:
	//I2KBatDialog(FXComposite* p, FXuint opts=0, FXint x=0, FXint y=0, FXint w=0, FXint h=0);
	I2KBatDialog(FXComposite* p, int num, const char* bat);
	virtual void create();
	virtual ~I2KBatDialog();
};

