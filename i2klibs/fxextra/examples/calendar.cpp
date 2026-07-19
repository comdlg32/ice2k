/*******************************************************************************
*                    Calendar Example and Test Program                         *
********************************************************************************
*              Copyright (C) 2006,2007 Sander Jansen. All Rights Reserved      *
*******************************************************************************/
#include <fx.h>
#include "ice2k/FXExtra/FXCalendar.h"
#include "ice2k/FXExtra/FXCalendarView.h"


// Main Window
class TestWindow : public FXMainWindow {
  FXDECLARE(TestWindow)
private:
  FXCalendar     *view;
  FXint           mode;
  FXbool          bstatic;
  FXbool          bsibling;
  FXbool          bweek;
  FXuint          framestyle;
  FXuint          date;
  FXDataTarget    dt_date;
  FXDataTarget    dt_static;
  FXDataTarget    dt_sibling;
  FXDataTarget    dt_week;
  FXDataTarget    dt_mode;
  FXDataTarget    dt_framestyle;
  FXCalendarView *cal[12];
protected:
  TestWindow(){}
public:
  enum {
    ID_FRAME_STYLE=FXMainWindow::ID_LAST,
    ID_SELECT_MODE,
    ID_STATIC_MODE,
    ID_SIBLING_MODE,
    ID_WEEK_MODE,
    ID_DAY_OF_WEEK,
    ID_COLOR_1,
    ID_COLOR_2,
    ID_COLOR_3,
    ID_COLOR_4,
    ID_COLOR_5,
    ID_COLOR_6,
    ID_COLOR_7,
    ID_COLOR_8,
    ID_COLOR_LAST,
    ID_CALENDAR,
    };
public:
  long onCmdFrameStyle(FXObject*,FXSelector,void*);
  long onCmdSelectMode(FXObject*,FXSelector,void*);
  long onCmdStaticMode(FXObject*,FXSelector,void*);
  long onCmdSiblingMode(FXObject*,FXSelector,void*);
  long onCmdWeekMode(FXObject*,FXSelector,void*);
  long onCmdDayOfWeek(FXObject*,FXSelector,void*);
  long onChgColor(FXObject*,FXSelector,void*);
  long onCmdSelected(FXObject*,FXSelector,void*);
public:
  TestWindow(FXApp *app);
  virtual void create();
  virtual ~TestWindow();
  };



// Message map
FXDEFMAP(TestWindow) TestWindowMap[]={
  FXMAPFUNC(SEL_COMMAND,TestWindow::ID_FRAME_STYLE,TestWindow::onCmdFrameStyle),
  FXMAPFUNC(SEL_COMMAND,TestWindow::ID_SELECT_MODE,TestWindow::onCmdSelectMode),
  FXMAPFUNC(SEL_COMMAND,TestWindow::ID_STATIC_MODE,TestWindow::onCmdStaticMode),
  FXMAPFUNC(SEL_COMMAND,TestWindow::ID_SIBLING_MODE,TestWindow::onCmdSiblingMode),
  FXMAPFUNC(SEL_COMMAND,TestWindow::ID_WEEK_MODE,TestWindow::onCmdWeekMode),
  FXMAPFUNC(SEL_COMMAND,TestWindow::ID_CALENDAR,TestWindow::onCmdSelected),
  FXMAPFUNC(SEL_COMMAND,TestWindow::ID_DAY_OF_WEEK,TestWindow::onCmdDayOfWeek),
  FXMAPFUNCS(SEL_CHANGED,TestWindow::ID_COLOR_1,TestWindow::ID_COLOR_LAST,TestWindow::onChgColor),
  FXMAPFUNCS(SEL_COMMAND,TestWindow::ID_COLOR_1,TestWindow::ID_COLOR_LAST,TestWindow::onChgColor),
  };

FXIMPLEMENT(TestWindow,FXMainWindow,TestWindowMap,ARRAYNUMBER(TestWindowMap));



// Main window constructor
TestWindow::TestWindow(FXApp *a):FXMainWindow(a,"Calendar Test Program",NULL,NULL,DECOR_ALL,0,0,0,0){

  // Switcher
  FXTabBook * tabbook=new FXTabBook(this,NULL,0,LAYOUT_FILL_X|LAYOUT_FILL_Y);

  // First item is a list
  new FXTabItem(tabbook,"One Calendar",NULL);

  FXHorizontalFrame * hframe = new FXHorizontalFrame(tabbook,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_RAISED|FRAME_THICK,0,0,0,0,0,0,0,0,0);
  FXVerticalFrame * vframe1 = new FXVerticalFrame(hframe,LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
  FXVerticalFrame * vframe2 = new FXVerticalFrame(hframe,LAYOUT_FILL_Y|LAYOUT_FILL_X);
  FXVerticalFrame * vframe3 = new FXVerticalFrame(hframe,LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);

  FXGroupBox * box = new FXGroupBox(vframe2,"Calendar View",FRAME_RIDGE|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  view = new FXCalendar(box,/*&dt_date,FXDataTarget::ID_VALUE,*/NULL,0,/*LAYOUT_FILL_X|LAYOUT_FILL_Y|*/FRAME_SUNKEN|FRAME_THICK);
  //view->disable();

  hframe = new FXHorizontalFrame(box);
  new FXLabel(hframe,"Julian Day Number");
  new FXTextField(hframe,9,&dt_date,FXDataTarget::ID_VALUE,TEXTFIELD_NORMAL|TEXTFIELD_READONLY);

  date=FXDate::localDate().getJulian();
  dt_date.connect(date);


  framestyle = 6;
  dt_framestyle.connect(framestyle,this,ID_FRAME_STYLE);

  box = new FXGroupBox(vframe1,"Frame Style",FRAME_RIDGE);
  new FXRadioButton(box,"None",&dt_framestyle,FXDataTarget::ID_OPTION+0);
  new FXRadioButton(box,"Line",&dt_framestyle,FXDataTarget::ID_OPTION+1);
  new FXRadioButton(box,"Ridge",&dt_framestyle,FXDataTarget::ID_OPTION+2);
  new FXRadioButton(box,"Groove",&dt_framestyle,FXDataTarget::ID_OPTION+3);
  new FXRadioButton(box,"Raised",&dt_framestyle,FXDataTarget::ID_OPTION+4);
  new FXRadioButton(box,"Raised Thick",&dt_framestyle,FXDataTarget::ID_OPTION+5);
  new FXRadioButton(box,"Sunken",&dt_framestyle,FXDataTarget::ID_OPTION+6);
  new FXRadioButton(box,"Sunken Thick",&dt_framestyle,FXDataTarget::ID_OPTION+7);

  mode=1;
  bstatic=false;
  bsibling=false;
  bweek=false;

  dt_mode.connect(mode,this,ID_SELECT_MODE);
  dt_static.connect(bstatic,this,ID_STATIC_MODE);
  dt_sibling.connect(bsibling,this,ID_SIBLING_MODE);
  dt_week.connect(bweek,this,ID_WEEK_MODE);

  box = new FXGroupBox(vframe1,"Options",FRAME_RIDGE);
  new FXRadioButton(box,"Browse Select",&dt_mode,FXDataTarget::ID_OPTION+1);
  new FXRadioButton(box,"Single Select",&dt_mode,FXDataTarget::ID_OPTION+0);
  new FXCheckButton(box,"Static",&dt_static,FXDataTarget::ID_VALUE);
  new FXCheckButton(box,"Hide other days",&dt_sibling,FXDataTarget::ID_VALUE);
  new FXCheckButton(box,"Week Numbers",&dt_week,FXDataTarget::ID_VALUE);
  new FXCheckButton(box,"Enabled",view,FXWindow::ID_TOGGLEENABLED);

  box = new FXGroupBox(vframe1,"First Day of the Week",FRAME_RIDGE|LAYOUT_FILL_Y);
  FXSpinner * spinner = new FXSpinner(box,1,this,ID_DAY_OF_WEEK,FRAME_SUNKEN|FRAME_THICK);
  spinner->setRange(0,6);

  box = new FXGroupBox(vframe3,"Keys",FRAME_RIDGE);

  FXMatrix * mat = new FXMatrix(box,2,LAYOUT_FILL_X|MATRIX_BY_COLUMNS,0,0,0,0,0,0,0,0,0,0);
  new FXLabel(mat,"Page Up",NULL,LAYOUT_CENTER_Y|JUSTIFY_LEFT);
  new FXLabel(mat,"Previous Month",NULL,LAYOUT_CENTER_Y|JUSTIFY_LEFT);

  new FXLabel(mat,"Page Down",NULL,LAYOUT_CENTER_Y|JUSTIFY_LEFT);
  new FXLabel(mat,"Next Month",NULL,LAYOUT_CENTER_Y|JUSTIFY_LEFT);

  new FXLabel(mat,"Home",NULL,LAYOUT_CENTER_Y|JUSTIFY_LEFT);
  new FXLabel(mat,"Select Today",NULL,LAYOUT_CENTER_Y|JUSTIFY_LEFT);

  new FXLabel(mat,"End",NULL,LAYOUT_CENTER_Y|JUSTIFY_LEFT);
  new FXLabel(mat,"Show Selected",NULL,LAYOUT_CENTER_Y|JUSTIFY_LEFT);

  box = new FXGroupBox(vframe3,"Colors",FRAME_RIDGE|LAYOUT_FILL_Y);



  mat = new FXMatrix(box,2,LAYOUT_FILL_X|MATRIX_BY_COLUMNS,0,0,0,0,0,0,0,0,0,0);
  FXColorWell * cw;

  cw = new FXColorWell(mat,0,this,ID_COLOR_1,LAYOUT_CENTER_Y);
  cw->setRGBA(view->getBackColor());
  new FXLabel(mat,"Background",NULL,LAYOUT_CENTER_Y|JUSTIFY_LEFT);

  cw = new FXColorWell(mat,0,this,ID_COLOR_2,LAYOUT_CENTER_Y);
  cw->setRGBA(view->getTitleBackColor());
  new FXLabel(mat,"Title Background",NULL,LAYOUT_CENTER_Y|JUSTIFY_LEFT);

  cw = new FXColorWell(mat,0,this,ID_COLOR_3,LAYOUT_CENTER_Y);
  cw->setRGBA(view->getTitleColor());
  new FXLabel(mat,"Title Foreground",NULL,LAYOUT_CENTER_Y|JUSTIFY_LEFT);

  cw = new FXColorWell(mat,0,this,ID_COLOR_4,LAYOUT_CENTER_Y);
  cw->setRGBA(view->getWeekendColor());
  new FXLabel(mat,"Weekend",NULL,LAYOUT_CENTER_Y|JUSTIFY_LEFT);

  cw = new FXColorWell(mat,0,this,ID_COLOR_5,LAYOUT_CENTER_Y);
  cw->setRGBA(view->getOtherWeekendColor());
  new FXLabel(mat,"Weekend (disabled)",NULL,LAYOUT_CENTER_Y|JUSTIFY_LEFT);


  cw = new FXColorWell(mat,0,this,ID_COLOR_6,LAYOUT_CENTER_Y);
  cw->setRGBA(view->getDayColor());
  new FXLabel(mat,"Day",NULL,LAYOUT_CENTER_Y|JUSTIFY_LEFT);

  cw = new FXColorWell(mat,0,this,ID_COLOR_7,LAYOUT_CENTER_Y);
  cw->setRGBA(view->getOtherDayColor());
  new FXLabel(mat,"Day (disabled)",NULL,LAYOUT_CENTER_Y|JUSTIFY_LEFT);

  cw = new FXColorWell(mat,0,this,ID_COLOR_8,LAYOUT_CENTER_Y);
  cw->setRGBA(view->getTodayColor());
  new FXLabel(mat,"Today",NULL,LAYOUT_CENTER_Y|JUSTIFY_LEFT);


  spinner->setValue(view->getFirstDay());

  FXLabel * label;
  // First item is a list
  new FXTabItem(tabbook,"Multiple Calendars",NULL);

  hframe = new FXHorizontalFrame(tabbook,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_RAISED|FRAME_THICK,0,0,0,0);
  hframe = new FXHorizontalFrame(hframe,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0,0,0,0,0);

  FXScrollWindow * scrollframe = new FXScrollWindow(hframe,LAYOUT_FILL_X|LAYOUT_FILL_Y);

  mat = new FXMatrix(scrollframe,3,LAYOUT_FILL_X|LAYOUT_FILL_Y|MATRIX_BY_COLUMNS,0,0,0,0);
  mat->setBackColor(FXRGB(255,255,255));

  label = new FXLabel(mat,"January",NULL,LAYOUT_CENTER_X|LAYOUT_CENTER_Y);
  label->setBackColor(FXRGB(255,255,255));
  label = new FXLabel(mat,"February",NULL,LAYOUT_CENTER_X|LAYOUT_CENTER_Y);
  label->setBackColor(FXRGB(255,255,255));
  label = new FXLabel(mat,"March",NULL,LAYOUT_CENTER_X|LAYOUT_CENTER_Y);
  label->setBackColor(FXRGB(255,255,255));

  cal[0] = new FXCalendarView(mat,this,ID_CALENDAR,CALENDAR_STATIC|CALENDAR_HIDEOTHER|CALENDAR_SINGLESELECT);
  cal[1] = new FXCalendarView(mat,this,ID_CALENDAR,CALENDAR_STATIC|CALENDAR_HIDEOTHER|CALENDAR_SINGLESELECT);
  cal[2] = new FXCalendarView(mat,this,ID_CALENDAR,CALENDAR_STATIC|CALENDAR_HIDEOTHER|CALENDAR_SINGLESELECT);

  label = new FXLabel(mat,"April",NULL,LAYOUT_CENTER_X|LAYOUT_CENTER_Y);
  label->setBackColor(FXRGB(255,255,255));
  label = new FXLabel(mat,"May",NULL,LAYOUT_CENTER_X|LAYOUT_CENTER_Y);
  label->setBackColor(FXRGB(255,255,255));
  label = new FXLabel(mat,"June",NULL,LAYOUT_CENTER_X|LAYOUT_CENTER_Y);
  label->setBackColor(FXRGB(255,255,255));

  cal[3] = new FXCalendarView(mat,this,ID_CALENDAR,CALENDAR_STATIC|CALENDAR_HIDEOTHER|CALENDAR_SINGLESELECT);
  cal[4] = new FXCalendarView(mat,this,ID_CALENDAR,CALENDAR_STATIC|CALENDAR_HIDEOTHER|CALENDAR_SINGLESELECT);
  cal[5] = new FXCalendarView(mat,this,ID_CALENDAR,CALENDAR_STATIC|CALENDAR_HIDEOTHER|CALENDAR_SINGLESELECT);

  label = new FXLabel(mat,"July",NULL,LAYOUT_CENTER_X|LAYOUT_CENTER_Y);
  label->setBackColor(FXRGB(255,255,255));
  label = new FXLabel(mat,"August",NULL,LAYOUT_CENTER_X|LAYOUT_CENTER_Y);
  label->setBackColor(FXRGB(255,255,255));
  label = new FXLabel(mat,"September",NULL,LAYOUT_CENTER_X|LAYOUT_CENTER_Y);
  label->setBackColor(FXRGB(255,255,255));

  cal[6] = new FXCalendarView(mat,this,ID_CALENDAR,CALENDAR_STATIC|CALENDAR_HIDEOTHER|CALENDAR_SINGLESELECT);
  cal[7] = new FXCalendarView(mat,this,ID_CALENDAR,CALENDAR_STATIC|CALENDAR_HIDEOTHER|CALENDAR_SINGLESELECT);
  cal[8] = new FXCalendarView(mat,this,ID_CALENDAR,CALENDAR_STATIC|CALENDAR_HIDEOTHER|CALENDAR_SINGLESELECT);

  label = new FXLabel(mat,"October",NULL,LAYOUT_CENTER_X|LAYOUT_CENTER_Y);
  label->setBackColor(FXRGB(255,255,255));
  label = new FXLabel(mat,"November",NULL,LAYOUT_CENTER_X|LAYOUT_CENTER_Y);
  label->setBackColor(FXRGB(255,255,255));
  label = new FXLabel(mat,"December",NULL,LAYOUT_CENTER_X|LAYOUT_CENTER_Y);
  label->setBackColor(FXRGB(255,255,255));

  cal[9] = new FXCalendarView(mat,this,ID_CALENDAR,CALENDAR_STATIC|CALENDAR_HIDEOTHER|CALENDAR_SINGLESELECT);
  cal[10] = new FXCalendarView(mat,this,ID_CALENDAR,CALENDAR_STATIC|CALENDAR_HIDEOTHER|CALENDAR_SINGLESELECT);
  cal[11] = new FXCalendarView(mat,this,ID_CALENDAR,CALENDAR_STATIC|CALENDAR_HIDEOTHER|CALENDAR_SINGLESELECT);

  for (int i=0;i<12;i++){
    cal[i]->setCurrentMonth(i+1);
    cal[i]->killSelection();
    }
  }


// Place in the center when created
void TestWindow::create(){
  FXMainWindow::create();
  show(PLACEMENT_SCREEN);
  }


long TestWindow::onCmdSelected(FXObject*sender,FXSelector,void*){
  for (int i=0;i<12;i++){
    if (sender!=cal[i])
      cal[i]->killSelection();
    }
  return 1;
  }


long TestWindow::onCmdFrameStyle(FXObject*,FXSelector,void*){
  FXuint style=0;
  switch(framestyle){
    case 0: style = FRAME_NONE; break;
    case 1: style = FRAME_LINE;  break;
    case 2: style = FRAME_RIDGE; break;
    case 3: style = FRAME_GROOVE; break;
    case 4: style = FRAME_RAISED; break;
    case 5: style = FRAME_RAISED|FRAME_THICK; break;
    case 6: style = FRAME_SUNKEN; break;
    case 7: style = FRAME_SUNKEN|FRAME_THICK; break;
    }
  view->setFrameStyle(style);
  return 1;
  }


long TestWindow::onCmdSelectMode(FXObject*,FXSelector,void*){
  FXuint style = view->getCalendarStyle();
  if (mode)
    view->setCalendarStyle(style&=~CALENDAR_SINGLESELECT);
  else
    view->setCalendarStyle(style|=CALENDAR_SINGLESELECT);
  return 1;
  }


long TestWindow::onCmdStaticMode(FXObject*,FXSelector,void*){
  FXuint style = view->getCalendarStyle();
  if (bstatic)
    view->setCalendarStyle(style|CALENDAR_STATIC);
  else
    view->setCalendarStyle(style&=~CALENDAR_STATIC);
  return 1;
  }


long TestWindow::onCmdSiblingMode(FXObject*,FXSelector,void*){
  FXuint style = view->getCalendarStyle();
  if (bsibling)
    view->setCalendarStyle(style|CALENDAR_HIDEOTHER);
  else
    view->setCalendarStyle(style&=~CALENDAR_HIDEOTHER);
  return 1;
  }


long TestWindow::onCmdWeekMode(FXObject*,FXSelector,void*){
  FXuint style = view->getCalendarStyle();
  if (bweek)
    view->setCalendarStyle(style|CALENDAR_WEEKNUMBERS);
  else
    view->setCalendarStyle(style&=~CALENDAR_WEEKNUMBERS);
  return 1;
  }


long TestWindow::onCmdDayOfWeek(FXObject*,FXSelector,void*ptr){
  FXint value = (FXint)(FXival)(ptr);
  view->setFirstDay(value);
  return 1;
  }


long TestWindow::onChgColor(FXObject*,FXSelector sel,void*ptr){
  FXColor value = (FXuint)(FXuval)(ptr);
  switch(FXSELID(sel)){
    case ID_COLOR_1: view->setBackColor(value); break;
    case ID_COLOR_2: view->setTitleBackColor(value); break;
    case ID_COLOR_3: view->setTitleColor(value); break;
    case ID_COLOR_4: view->setWeekendColor(value); break;
    case ID_COLOR_5: view->setOtherWeekendColor(value); break;
    case ID_COLOR_6: view->setDayColor(value); break;
    case ID_COLOR_7: view->setOtherDayColor(value); break;
    case ID_COLOR_8: view->setTodayColor(value); break;
    }
  return 1;
  }


TestWindow::~TestWindow(){
  }



// Start the program
int main(int argc,char *argv[]){

  // Make application
  FXApp application("CalendarTest","FoxTest");

  // Open display
  application.init(argc,argv);

  // Make the main window
  new TestWindow(&application);

  // Realize the tree
  application.create();

  // Run
  return application.run();
  }
