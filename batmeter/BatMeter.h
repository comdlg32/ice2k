#pragma once
// Based on scribble example


#include <fx.h>
#include "foxutils.h"

// Main Window
class BatMeter : public FXMainWindow {

  // Macro for class hierarchy declarations
  FXDECLARE(BatMeter)

private:
  // Controls
  FXTopWindow       *MainWindow;
  FXHorizontalFrame *checkboxes;

  FXHorizontalFrame *powerstatscont;
  FXGroupBox        *powerstatus;
  FXHorizontalFrame *powerinfo;
  FXMatrix          *powersrc;

  FXPacker          *pbarcont;
  FXProgressBar     *pbar;


protected:
  BatMeter(){}

public:

  // Message handlers
  long onCmdNothing(FXObject*,FXSelector,void*);
  long onCmdNew(FXObject*,FXSelector,void*);
  long onCmdAbout(FXObject*,FXSelector,void*);
  void show();
  void show(FXuint placement);

public:


  // Messages for our class
  enum {
    ID_MAINWIN=FXMainWindow::ID_LAST,
    ID_NOTHING,
    ID_NEW,
    ID_ABOUT,
    ID_HELP
  };

public:

  // BatMeter's constructor
  BatMeter(FXApp* a);

  // Initialize
  virtual void create();

  virtual ~BatMeter();
};

