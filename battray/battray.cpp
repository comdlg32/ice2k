#include <fx.h>
#include <ice2k/i2ktray/I2KTrayApp.h>
#include <ice2k/i2ktray/I2KTrayIcon.h>
#include "ice2k/batmeter/battery.h"

#include "res/foxres.h"
#include "ice2k/comctl32.h"

class BatTray : public FXObject {
	FXDECLARE(BatTray)

public:
    enum {
        ID_BATTERY = FXApp::ID_LAST,
		ID_POWERCFG,
		ID_TIMEOUT,
        ID_LAST
    };

    BatTray(FXApp* app);

    long onCmdBattery(FXObject* sender, FXSelector sel, void*);
    long onCmdPowerProperties(FXObject* sender, FXSelector sel, void*);

    long onTimeout(FXObject* sender, FXSelector sel, void*);

	void refreshInfo();


private:
    BatTray() {}

    FXApp* app;

    FXPopup* popup;
    I2KTrayIcon* tray;

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

	FXMenuCommand* powercmd;
	FXMenuCommand* metercmd;
};

FXDEFMAP(BatTray) BatTrayMap[] = {
	FXMAPFUNC(SEL_COMMAND,           BatTray::ID_BATTERY,  BatTray::onCmdBattery),
	FXMAPFUNC(SEL_COMMAND,           BatTray::ID_POWERCFG, BatTray::onCmdPowerProperties),

	FXMAPFUNC(SEL_TIMEOUT,           BatTray::ID_TIMEOUT,  BatTray::onTimeout),
};

FXIMPLEMENT(BatTray, FXObject, BatTrayMap, ARRAYNUMBER(BatTrayMap));




char batteries[POWER_PSU_NAME_MAX*8];


static void secToStr(char* str, unsigned int sec) {
	int h = sec / 3600;
	int m = ( sec % 3600) / 60;
	int s = sec % 60;

	if (h > 99) {
		sprintf(str, "99:59 hours");
	} else if (h) {
		sprintf(str, "%d:%02d hours", h, m);
	} else {
		sprintf(str, "%d:%02d min", m, s);
	}


	return;
}


void BatTray::refreshInfo() {
	int status = getOverallStatus();
	int powersource = getCurrentPowerSource();
	int percentage = 0;
	//char statusstr[256];
	//char percentagestr[64];
	//char percentage2str[64];
	
	int amount = getPowerSupplies(batteries, sizeof(batteries),
			POWER_PSU_TYPE_BATTERY);
	
	getOverallPercentage(&percentage);

	int duration = 0;
	char durationstr[32];

	if (powersource == POWER_PSU_TYPE_BATTERY) {
		if (percentage > 66) {
			tray->setIcon(ico_batfull);
		} else if (percentage > 33) {
			tray->setIcon(ico_bathalf);
		} else if (percentage > 9) {
			tray->setIcon(ico_batlow);
		} else {
			tray->setIcon(ico_batcrit);
		}
	}
	
	if (status == POWER_BAT_STATUS_CHARGING) {
		getOverallDuration(1, &duration);
		if (powersource != POWER_PSU_TYPE_BATTERY) {
			tray->setIcon(ico_chargeplug);
		}
	} else {
		getOverallDuration(0, &duration);
		if (powersource != POWER_PSU_TYPE_BATTERY) {
			tray->setIcon(ico_plug);
		}
	}

	char textbuf[1024] = {0};
	char* textptr = textbuf;
	char* textend = &textbuf[sizeof(textbuf)];

	if (amount != 0) {
		int i = 0;

		while (amount > i && 8 > i) {
			int present = getBatteryPresent(batteries+(i*POWER_PSU_NAME_MAX));
			if (!present) break;
			status = getBatteryStatus(batteries+(i*POWER_PSU_NAME_MAX));

			getBatteryCapacity(batteries+(i*POWER_PSU_NAME_MAX), &percentage);
			getBatteryDuration(batteries+(i*POWER_PSU_NAME_MAX),
					(status == POWER_BAT_STATUS_CHARGING), &duration);
			secToStr(durationstr, duration);

			if (textptr >= textend) break;

			if (i > 0) {
				textptr += snprintf(textptr, textend-textptr, "\n");

				if (textptr >= textend) break;
			}

			if (amount > 1) {
				textptr += snprintf(textptr, textend-textptr,
						"Battery %d: ", i+1);

				if (textptr >= textend) break;
			}
			
			if (duration > 0 && duration < 360000) {
				textptr += snprintf(textptr, textend-textptr,
						"%s (%d%%) remaining",
						durationstr, percentage);
			} else {
				textptr += snprintf(textptr, textend-textptr,
						"%d%% remaining",
						percentage);
			}

			if (textptr >= textend) break;

			if (status == POWER_BAT_STATUS_CHARGING) {
				textptr += snprintf(textptr, textend-textptr,
						" (charging)");
			}
			if (textptr >= textend) break;

			++i;
		}
	} else {
		strcpy(textbuf, "No batteries installed!");
	}

	//puts(textbuf);
	tray->setText(textbuf);
}

BatTray::BatTray(FXApp* a) :
	app(a)
{
	ico_batcrit = new FXGIFIcon(a, resico_batcrit);
	ico_batcrit->create();
	ico_batfull = new FXGIFIcon(a, resico_batfull);
	ico_batfull->create();
	ico_bathalf = new FXGIFIcon(a, resico_bathalf);
	ico_bathalf->create();
	ico_batlow = new FXGIFIcon(a, resico_batlow);
	ico_batlow->create();
	ico_battery = new FXGIFIcon(a, resico_battery);
	ico_battery->create();
	ico_battplug = new FXGIFIcon(a, resico_battplug);
	ico_battplug->create();
	ico_charging = new FXGIFIcon(a, resico_charging);
	ico_charging->create();
	ico_notpres = new FXGIFIcon(a, resico_notpres);
	ico_notpres->create();
	ico_plug = new FXGIFIcon(a, resico_plug);
	ico_plug->create();
	ico_unknown = new FXGIFIcon(a, resico_unknown);
	ico_unknown->create();

	ico_chargeplug = new FXIcon(a, NULL, FXRGB(192,192,192), 0, 16, 16);
	ico_chargeplug->create();

	FXDCWindow dc_chargeplug(ico_chargeplug);
	dc_chargeplug.setForeground(a->getBaseColor());
	dc_chargeplug.fillRectangle(0, 0, 16, 16);
	dc_chargeplug.drawIcon(ico_plug, 0,0);
	dc_chargeplug.drawIcon(ico_charging, 0,0);
	dc_chargeplug.end();

	ico_chargeplug->restore();
	ico_chargeplug->render();

	FXFont* normalfont = a->getNormalFont();

	FXString fname = normalfont->getFont();
	FXuint fsize = normalfont->getSize()/10;
	FXuint fslant = normalfont->getSlant();
	FXuint fencoding = normalfont->getEncoding();
	FXuint fsetwidth = normalfont->getSetWidth();
	FXuint fhints = normalfont->getHints();

	FXFont* boldfont = new FXFont(a, fname, fsize, FXFont::Bold, fslant, fencoding, fsetwidth, fhints);
	if (boldfont == NULL) {
		boldfont = normalfont;
	} else {
		boldfont->create();
	}

    tray = new I2KTrayIcon(app, "No batteries installed!", ico_plug, 0, this, ID_BATTERY, TRAY_CMD_ON_LEFT|TRAY_MENU_ON_RIGHT);
    popup = new FXPopup(tray);
	powercmd = new FXMenuCommand(popup, "&Adjust Power Properties", NULL, this, ID_POWERCFG);
	metercmd = new FXMenuCommand(popup, "&Open Power Meter", NULL, this, ID_BATTERY);
	metercmd->setFont(boldfont);

	tray->setMenu(popup);

	refreshInfo();

	a->addTimeout(this, ID_TIMEOUT, 2000);
}

/*BatTray::~BatTray() {
}*/


long BatTray::onCmdBattery(FXObject*, FXSelector, void*) {
	system("batmeter &");
    return 1;
}
long BatTray::onCmdPowerProperties(FXObject*, FXSelector, void*) {
	system("powercfg.cpi &");
    return 1;
}


long BatTray::onTimeout(FXObject*, FXSelector, void*) {
	refreshInfo();
	app->addTimeout(this, ID_TIMEOUT, 2000);
	return 1;
}

int main(int argc, char* argv[]) {
    I2KTrayApp application("BatTray", "I2KProject");
   	application.init(argc, argv);

    BatTray tray_app(&application);

    application.create();

    return application.run();
}


