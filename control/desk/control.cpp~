#include <fx.h>
#include <ice2k/comctl32.h>
#include "res/foxres.h"

FXIcon*                  ico_control;

class DesktopProperties : public FXMainWindow {

  // Macro for class hierarchy declarations
  FXDECLARE(DesktopProperties)

private:
  // gui
  FXDockSite*              topdock;
  
  FXHorizontalFrame*       statusbarcont;
  FXLabel*                 statuslbl;

  FXToolBarShell*          mbshell;
  FXMenuBar*               menubar;
  
  FXMenuPane*              actionmenu;
  FXMenuPane*              viewmenu;

  FXToolBarShell*          tbshell;
  FXToolBarShell*          tb2shell;
  
  FXToolBar*               toolbar;
  FXToolBar*               toolbar2;





protected:
  DesktopProperties(){}

public:

  // Message handlers
  long onItemChange(FXObject*,FXSelector,void*);
  long onStatus(FXObject*,FXSelector,void*);
  long onChangeText(FXObject*,FXSelector,void*);
  long addDevices(FXObject*,FXSelector,void*);


public:

  // Messages for our class
  enum {
    ID_MAINWIN=FXMainWindow::ID_LAST,
    ID_TREE,
    ID_ADDDEV,
  };

public:

  // CtrlAltDelWindow's constructor
  DesktopProperties(FXApp* a);

  // Initialize
  virtual void create();

  virtual ~DesktopProperties();
};

FXDEFMAP(DesktopProperties) DesktopPropertiesMap[] = {
  /* FXMAPFUNC(SEL_UPDATE, 0, DesktopProperties::onStatus),
  FXMAPFUNC(SEL_COMMAND, DesktopProperties::ID_SETSTRINGVALUE, DesktopProperties::onChangeText),
  FXMAPFUNC(SEL_COMMAND, DesktopProperties::ID_ADDDEV, DesktopProperties::addDevices),

  FXMAPFUNC(SEL_CHANGED, DesktopProperties::ID_TREE, DesktopProperties::onItemChange), */
};

FXIMPLEMENT(DesktopProperties,FXMainWindow,DesktopPropertiesMap,ARRAYNUMBER(DesktopPropertiesMap))

void DesktopProperties::create() {
	FXMainWindow::create();
}

DesktopProperties::~DesktopProperties() {
}

DesktopProperties::DesktopProperties(FXApp *app):FXMainWindow(app, "Control Panel", ico_control, NULL, DECOR_TITLE|DECOR_BORDER|DECOR_MENU|DECOR_CLOSE, 0,0,398,423,  0,0,0,0,  0,0) {
  FXIcon* monitorimage = new FXGIFIcon(app, resico_monitor);

  FXVerticalFrame* tabframe = new FXVerticalFrame(this,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_NONE, 0,0,0,0, 0,0,0,0, 0,0);
  FXTabBook* tabbook = new FXTabBook(tabframe,NULL,0,LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_RIGHT, 0,0,0,0, 6,6,7,6);

  new FXTabItem(tabbook,"Background",NULL,TAB_TOP_NORMAL,0,0,0,0,4,4,1,3);
  FXVerticalFrame* bgframe = new FXVerticalFrame(tabbook,FRAME_THICK|FRAME_RAISED, 0,0,0,0, 13,12,13,13, 0,0); 
  new FXLabel(bgframe, "", monitorimage, LABEL_NORMAL|LAYOUT_CENTER_X, 0,0,0,0,  0,0,0,30);

  new FXLabel(bgframe, "&Select a background picture or HTML document as Wallpaper:", NULL, LABEL_NORMAL|JUSTIFY_LEFT, 0,0,0,0,  0,0,0,2);

  FXHorizontalFrame* setbgframe = new FXHorizontalFrame(bgframe,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_NONE, 0,0,0,0, 0,0,0,0, 10,0); 

  FXFrame* bglist = new FXFrame(setbgframe, FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_Y|LAYOUT_FILL_X, 0,0,0,0,  0,0,0,0);
  bglist->setBackColor(app->getBackColor());


  FXVerticalFrame* setbgcontrols = new FXVerticalFrame(setbgframe,LAYOUT_FILL_Y|FRAME_NONE, 0,0,0,0, 0,0,2,0, 0,0); 
  FXButton* btn;
  new FXButton(setbgcontrols, "&Browse...", NULL, NULL, 0, BUTTON_DEFAULT|BUTTON_NORMAL|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0,0,75,23,  0,0,0,0);
  new FXSeparator(setbgcontrols, SEPARATOR_NONE|LAYOUT_FIX_HEIGHT, 0,0,0,13); // i could use a fxframe, but semantics r cute
  new FXLabel(setbgcontrols, "Picture &Display:", NULL, LABEL_NORMAL|JUSTIFY_LEFT, 0,0,0,0,  1,0,0,2);
  I2KListBox* picdisplay = new I2KListBox(setbgcontrols,NULL,NULL,COMBOBOX_INSERT_LAST|LAYOUT_FILL_X|COMBOBOX_STATIC|FRAME_SUNKEN|FRAME_THICK, 0, 0, 0, 0, 3, 0, 2, 1);
  new FXSeparator(setbgcontrols, SEPARATOR_NONE|LAYOUT_FIX_HEIGHT, 0,0,0,13);
  btn = new FXButton(setbgcontrols, "&Pattern...", NULL, NULL, 0, BUTTON_DEFAULT|BUTTON_NORMAL|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0,0,75,23,  0,0,0,0);
  btn->disable();

  picdisplay->insertItem(0, "Center");
  picdisplay->insertItem(1, "Tile");
  picdisplay->insertItem(2, "Stretch");

  picdisplay->setNumVisible(picdisplay->getNumItems());


  FXHorizontalFrame* btncont = new FXHorizontalFrame(tabframe, LAYOUT_RIGHT, 0, 0, 0, 0, 0, 6, 0, 7, 6, 0);

  FXButton* okbtn = new FXButton(btncont, "OK", NULL, NULL, 0, BUTTON_DEFAULT|BUTTON_NORMAL|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0, 0, 75, 23, 0, 0, 0, 0);
  FXButton* cancelbtn = new FXButton(btncont, "Cancel", NULL, NULL, 0, BUTTON_NORMAL|BUTTON_DEFAULT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0, 0, 75, 23, 0, 0, 0, 0);
  FXButton* applybtn = new FXButton(btncont, "&Apply", NULL, NULL, 0, BUTTON_NORMAL|BUTTON_DEFAULT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0, 0, 75, 23, 0, 0, 0, 0);
  applybtn->disable();
}

int main(int argc, char *argv[]) {

	FXApp application("Control", "Ice2KProj");
	application.init(argc, argv);

	FXApp* app = &application;


	ico_control = new FXGIFIcon(app, resico_control, IMAGE_NEAREST);

	FXMainWindow* controlwin = new DesktopProperties(app);

	application.create();
	controlwin->show(PLACEMENT_OWNER);
	return application.run();
}
