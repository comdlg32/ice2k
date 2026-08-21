#include "fx.h"
#include "ice2k/i2ktray/I2KTrayApp.h"
#include "ice2k/i2ktray/I2KTrayIcon.h"

#include "res/foxres.h"

FXIcon* mainicon;

class TrayApp : public FXObject {
	FXDECLARE(TrayApp)

public:
    enum {
        ID_HELLO = FXApp::ID_LAST,
        ID_LAST
    };

    TrayApp(FXApp* app);

    long onHello(FXObject* sender, FXSelector sel, void*);
private:
    TrayApp() {}

    FXApp* app;

    FXPopup* popup;
    I2KTrayIcon* tray;
};

FXDEFMAP(TrayApp) TrayAppMap[] = {
	FXMAPFUNC(SEL_COMMAND, TrayApp::ID_HELLO, TrayApp::onHello)
};

FXIMPLEMENT(TrayApp, FXObject, TrayAppMap, ARRAYNUMBER(TrayAppMap));

TrayApp::TrayApp(FXApp* a) :
	app(a)
{
    tray = new I2KTrayIcon(app, "Tray Test", mainicon, 0, this, ID_HELLO, TRAY_CMD_ON_LEFT|TRAY_MENU_ON_RIGHT);

    popup = new FXPopup(tray);
    new FXMenuCommand(popup, "Hello", 0, this, TrayApp::ID_HELLO);
    new FXMenuSeparator(popup);
    new FXMenuCommand(popup, "E&xit", 0, a, FXApp::ID_QUIT);
	tray->setMenu(popup);
}

long TrayApp::onHello(FXObject*, FXSelector, void*) {
    FXMessageBox::information(app, MBOX_OK, "Tray Test", "Hello World!");

    return 1;
}

int main(int argc, char* argv[]) {
    I2KTrayApp application;
    application.init(argc, argv);
    mainicon = new FXGIFIcon(&application, resico_volume);
	
    TrayApp tray_app(&application);

    application.create();

    return application.run();
}


