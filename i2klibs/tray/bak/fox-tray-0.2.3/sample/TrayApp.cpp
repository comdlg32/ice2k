#include "fx.h"

#include "FXTrayIcon.h"

#include <memory>
using namespace std;

#include "tray_icon1.xpm"
#include "tray_icon2.xpm"

/*
 * Create a class derrived from FXObject to handle messages sent by
 * FXTrayIcon. You can use also derrive from FXTopWindow for example
 */
class TrayApp : public FXObject
{
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

    auto_ptr<FXIcon> mIcon1;
    auto_ptr<FXIcon> mIcon2;
    auto_ptr<FXPopup> mPup;
    FXTrayIcon* mTray;

    FXApp* mApp;
};

FXDEFMAP(TrayApp) TrayAppMap[] = {
    FXMAPFUNC(SEL_COMMAND, TrayApp::ID_HELLO, TrayApp::onHello)
};

FXIMPLEMENT(TrayApp, FXObject, TrayAppMap, ARRAYNUMBER(TrayAppMap))

TrayApp::TrayApp(FXApp* app) :
    mApp(app)
{
    // Create the icon
    mIcon1 = auto_ptr<FXXPMIcon> (new FXXPMIcon(app, 
            const_cast<const FXchar**>(tray_icon1_xpm)) );

    mIcon2 = auto_ptr<FXXPMIcon> (new FXXPMIcon(app,
            const_cast<const FXchar**>(tray_icon2_xpm)) );

    // create the tray icon
    // it will open a popup menu on right click and send SEL_COMMAND
    // on left click
    mTray = new FXTrayIcon(app, "Tray Test", mIcon1.get(),
            0, this, TrayApp::ID_HELLO, TRAY_CMD_ON_LEFT|TRAY_MENU_ON_RIGHT);

    // create the popup menu
    mPup = auto_ptr<FXPopup> (new FXPopup(mTray));
    new FXMenuCommand(mPup.get(), "Hello", 0, this, TrayApp::ID_HELLO);
    new FXMenuSeparator(mPup.get() );
    new FXMenuCommand(mPup.get(), "E&xit", 0, mApp, FXApp::ID_QUIT);
    mTray->setMenu(mPup.get());
}

long TrayApp::onHello(FXObject* sender, FXSelector sel, void*)
{
    static int clickCount = 0;
    clickCount++;

    // ensure the Icon is created
    mIcon2->create();
    mTray->setIcon(mIcon2.get() );

    FXMessageBox::information(mApp, MBOX_OK, "Tray Test", "Hello World!");

    mTray->setIcon(mIcon1.get() );

    // update the click count
    FXString text("Clicked: ");
    text += FXStringVal(clickCount) + " times";
    mTray->setText(text);

    return 1;
}

int main(int argc, char* argv[])
{
    FXTrayApp app;
    app.init(argc, argv);

    TrayApp tray_app(&app);

    app.create();

    return app.run();
}


