#include <fx.h>
#include <FXPNGIcon.h>
#include <ice2k/branding.h>
#include <ice2k/i2ktray/I2KTrayApp.h>
#include <ice2k/i2ktray/I2KTrayIcon.h>

#include "res/foxres.h"

FXIcon* ico_eject;

class EjectTray : public FXObject {
	FXDECLARE(EjectTray)

public:
    enum {
        ID_SAFELYREMOVE = FXApp::ID_LAST,
        ID_LAST
    };

    EjectTray(FXApp* app);

    long onCmdSafelyRemove(FXObject* sender, FXSelector sel, void*);

private:
    EjectTray() {}

    FXApp* app;

    FXPopup* popup;
    I2KTrayIcon* tray;
};

FXDEFMAP(EjectTray) EjectTrayMap[] = {
	FXMAPFUNC(SEL_COMMAND, EjectTray::ID_SAFELYREMOVE, EjectTray::onCmdSafelyRemove),
};

FXIMPLEMENT(EjectTray, FXObject, EjectTrayMap, ARRAYNUMBER(EjectTrayMap));


EjectTray::EjectTray(FXApp* a) :
	app(a)
{
    tray = new I2KTrayIcon(app, "Safely Remove Hardware", ico_eject, 0, this, ID_SAFELYREMOVE, TRAY_CMD_ON_LEFT|TRAY_MENU_ON_RIGHT);
    popup = new FXPopup(tray);
    new FXMenuCommand(popup, "Safely Remove Hardware", NULL, this, ID_SAFELYREMOVE);
	tray->setMenu(popup);
}

long EjectTray::onCmdSafelyRemove(FXObject*, FXSelector, void*) {
	system("i2khotplug &");
    return 1;
}


int main(int argc, char* argv[]) {
    I2KTrayApp application("EjectTray", "I2KProject");
   	application.init(argc, argv);

	if (i2kBGetWinVersionInt() < ICE2K_BRAND_WINXP) {
		ico_eject = new FXPNGIcon(&application, resico_2k_eject);
	} else {
		ico_eject = new FXPNGIcon(&application, resico_xp_eject);
		ico_eject->blend(application.getBaseColor());
	}

	ico_eject->create();

    EjectTray tray_app(&application);

    application.create();

    return application.run();
}


