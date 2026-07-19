/********************************************************************************
*                                                                               *
*                   B a s e   C a l e n d a r   W i d g e t                     *
*                                                                               *
*********************************************************************************
* Copyright (C) 2006,2007 by Sander Jansen.   All Rights Reserved.              *
*********************************************************************************
* This library is free software; you can redistribute it and/or                 *
* modify it under the terms of the GNU Lesser General Public                    *
* License as published by the Free Software Foundation; either                  *
* version 2.1 of the License, or (at your option) any later version.            *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU             *
* Lesser General Public License for more details.                               *
*                                                                               *
* You should have received a copy of the GNU Lesser General Public              *
* License along with this library; if not, write to the Free Software           *
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.    *
*********************************************************************************
* $Id: FXCalendar.cpp,v 1.7 2007/02/07 20:22:03 fox Exp $                       *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxascii.h"
#include "fxkeys.h"
#include "FXHash.h"
#include "FXThread.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXObjectList.h"
#include "FXRectangle.h"
#include "FXRegistry.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXDrawable.h"
#include "FXDC.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXLabel.h"
#include "FXComposite.h"
#include "FXPacker.h"
#include "FXButton.h"
#include "FXArrowButton.h"
#include "FXMenuButton.h"
#include "FXToolTip.h"
#include "ice2k/FXExtra/FXOptionMenu.h"
#include "FXHorizontalFrame.h"
#include "FXDCWindow.h"
#include "FXFont.h"
#include "FXDate.h"
#include "FXPopup.h"
#include "FXMenuCommand.h"
#include "ice2k/FXExtra/FXCalendarView.h"
#include "ice2k/FXExtra/FXCalendar.h"

/*
  Notes:
  - Most of calendar is implemented in FXCalendarView
*/

#define CALENDAR_MASK (CALENDAR_SINGLESELECT|CALENDAR_BROWSESELECT|CALENDAR_WEEKNUMBERS|CALENDAR_STATIC|CALENDAR_HIDEOTHER)


using namespace FX;

/*******************************************************************************/

namespace FX {

FXDEFMAP(FXCalendar) FXCalendarMap[]={
  FXMAPFUNC(SEL_REPLACED,FXCalendar::ID_CALENDAR,FXCalendar::onCmdDate),
  FXMAPFUNCS(SEL_COMMAND,FXCalendar::ID_MONTH_START,FXCalendar::ID_MONTH_END,FXCalendar::onCmdMonth),
  FXMAPFUNC(SEL_COMMAND,FXCalendar::ID_NEXTYEAR,FXCalendar::onCmdNextYear),
  FXMAPFUNC(SEL_COMMAND,FXCalendar::ID_PREVYEAR,FXCalendar::onCmdPrevYear),
  FXMAPFUNC(SEL_COMMAND,FXCalendar::ID_NEXTMONTH,FXCalendar::onCmdNextMonth),
  FXMAPFUNC(SEL_COMMAND,FXCalendar::ID_PREVMONTH,FXCalendar::onCmdPrevMonth),
  FXMAPFUNC(SEL_COMMAND,FXCalendar::ID_MONTH,FXCalendar::onCmdSelectMonth),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETVALUE,FXCalendar::onFwdToView),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETINTVALUE,FXCalendar::onFwdToView),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETINTVALUE,FXCalendar::onFwdToView),
  FXMAPFUNC(SEL_COMMAND,FXCalendar::ID_CALENDAR,FXCalendar::onFwdToTarget),
  FXMAPFUNC(SEL_CHANGED,FXCalendar::ID_CALENDAR,FXCalendar::onFwdToTarget),
  FXMAPFUNC(SEL_UPDATE,FXCalendar::ID_CALENDAR,FXCalendar::onFwdToTarget),
  };

FXIMPLEMENT(FXCalendar,FXPacker,FXCalendarMap,ARRAYNUMBER(FXCalendarMap));


FXCalendar::FXCalendar(){
  flags|=FLAG_ENABLED;
  }


// Construct and initialize calendar widget
FXCalendar::FXCalendar(FXComposite *p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):FXPacker(p,opts,x,y,w,h,0,0,0,0,0,0){
  flags|=FLAG_ENABLED;
  target=tgt;
  message=sel;

  // Popup for months
  months=new FXPopup(this);
  /*for(int i=1; i<=12; i++){
    new FXOption(months,tr(FXDate::monthName(i)),NULL,this,ID_MONTH_START+(i-1),OPTIONMENU_NOGLYPH|LAYOUT_LEFT|JUSTIFY_CENTER_X|ICON_AFTER_TEXT);
    }*/
    for(int i=1; i<=12; i++){
	new FXMenuCommand(months, tr(FXDate::monthName(i)), NULL, this, ID_MONTH_START+(i-1));
    }


  // Caption above
  frame=new FXHorizontalFrame(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X,0,0,0,0,0,0,0,0,0,0);

  // Month selector
  arrows[0]=new FXArrowButton(frame,this,ID_PREVMONTH,ARROW_LEFT|ARROW_REPEAT|ARROW_TOOLBAR|FRAME_RAISED|LAYOUT_CENTER_Y);
  /*month=new FXOptionMenu(frame,months,OPTIONMENU_NOGLYPH|OPTIONMENU_TOOLBAR|FRAME_RAISED|JUSTIFY_CENTER_X|JUSTIFY_CENTER_Y|LAYOUT_CENTER_Y,0,0,0,0);*/
  month=new FXMenuButton(frame, "", NULL, months,MENUBUTTON_NOARROWS|MENUBUTTON_TOOLBAR|FRAME_RAISED|JUSTIFY_CENTER_X|JUSTIFY_CENTER_Y|LAYOUT_CENTER_Y,0,0,0,0);
  month->setTarget(this);
  month->setSelector(ID_MONTH);

  // Year selector
  arrows[1]=new FXArrowButton(frame,this,ID_NEXTMONTH,ARROW_RIGHT|ARROW_REPEAT|ARROW_TOOLBAR|FRAME_RAISED|LAYOUT_CENTER_Y);
  arrows[2]=new FXArrowButton(frame,this,ID_NEXTYEAR,ARROW_RIGHT|ARROW_REPEAT|ARROW_TOOLBAR|FRAME_RAISED|LAYOUT_CENTER_Y|LAYOUT_RIGHT);
  year=new FXLabel(frame,"0000",NULL,LAYOUT_RIGHT|LAYOUT_CENTER_Y);
  arrows[3]=new FXArrowButton(frame,this,ID_PREVYEAR,ARROW_LEFT|ARROW_REPEAT|ARROW_TOOLBAR|FRAME_RAISED|LAYOUT_CENTER_Y|LAYOUT_RIGHT);

  // Main widget
  view=new FXCalendarView(this,this,ID_CALENDAR,(options&CALENDAR_MASK)|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_SIDE_BOTTOM);

  // Fix the options
  if(options&FRAME_SUNKEN && !(options&FRAME_RAISED)){
    if(options&FRAME_THICK)
      frame->setFrameStyle(FRAME_RAISED|FRAME_THICK);
    else
      frame->setFrameStyle(FRAME_RAISED);
    }
  else{
    frame->setFrameStyle(FRAME_NONE);
    }
  }


void FXCalendar::create(){
  FXMenuCommand* mb = NULL;
  int width = 0;
  int curwidth = 0;

  FXPacker::create();
  FXDate date=view->getCurrentDate();
  year->setText(FXStringVal(date.year()));
  
  mb = (FXMenuCommand*)months->getFirst();
  while(mb != NULL) {
    month->setText(mb->getText());
    this->layout();
    curwidth = month->getWidth();
	
    if (curwidth > width) width = curwidth;

    mb = (FXMenuCommand*)mb->getNext();
    }

  month->setLayoutHints(month->getLayoutHints()|LAYOUT_FIX_WIDTH);
  month->setWidth(width);

  month->setText(((FXMenuCommand*)(months->childAtIndex(date.month()-1)))->getText());
  }

void FXCalendar::enable(){
  FXPacker::enable();
  view->enable();
  month->enable();
  year->enable();
  arrows[0]->enable();
  arrows[1]->enable();
  arrows[2]->enable();
  arrows[3]->enable();
  }

void FXCalendar::disable(){
  FXPacker::disable();
  view->disable();
  month->disable();
  year->disable();
  arrows[0]->disable();
  arrows[1]->disable();
  arrows[2]->disable();
  arrows[3]->disable();
  }



void FXCalendar::setCalendarStyle(FXuint style){
  view->setCalendarStyle(style);
  }


FXuint FXCalendar::getCalendarStyle() const {
  return view->getCalendarStyle();
  }


void FXCalendar::setFirstDay(FXint d){
  view->setFirstDay(d);
  view->update();
  }


FXint FXCalendar::getFirstDay() const{
  return view->getFirstDay();
  }


void FXCalendar::setFrameStyle(FXuint s){
  FXPacker::setFrameStyle(s);
  if(options&FRAME_SUNKEN && !(options&FRAME_RAISED)){
    if(options&FRAME_THICK)
      frame->setFrameStyle(FRAME_RAISED|FRAME_THICK);
    else
      frame->setFrameStyle(FRAME_RAISED);
    }
  else{
    frame->setFrameStyle(FRAME_NONE);
    }
  }


void FXCalendar::setBackColor(FXColor c){
  FXPacker::setBackColor(c);
  view->setBackColor(c);
  }


FXColor FXCalendar::getBackColor() const{
  return view->getBackColor();
  }


void FXCalendar::setTitleColor(FXColor c){
  view->setTitleColor(c);
  }


FXColor FXCalendar::getTitleColor() const{
  return view->getTitleColor();
  }


void FXCalendar::setTitleBackColor(FXColor c){
  view->setTitleBackColor(c);
  }


FXColor FXCalendar::getTitleBackColor() const{
  return view->getTitleBackColor();
  }


void FXCalendar::setDayColor(FXColor c){
  view->setDayColor(c);
  }


FXColor FXCalendar::getDayColor() const{
  return view->getDayColor();
  }


void FXCalendar::setOtherDayColor(FXColor c){
  view->setOtherDayColor(c);
  }


FXColor FXCalendar::getOtherDayColor() const{
  return view->getOtherDayColor();
  }


void FXCalendar::setTodayColor(FXColor c){
  view->setTodayColor(c);
  }


FXColor FXCalendar::getTodayColor() const {
  return view->getTodayColor();
  }


void FXCalendar::setWeekendColor(FXColor c){
  view->setWeekendColor(c);
  }


FXColor FXCalendar::getWeekendColor() const{
  return view->getWeekendColor();
  }


void FXCalendar::setOtherWeekendColor(FXColor c){
  view->setOtherWeekendColor(c);
  }


FXColor FXCalendar::getOtherWeekendColor() const{
  return view->getOtherWeekendColor();
  }


long FXCalendar::onCmdDate(FXObject*,FXSelector,void* ptr){
  FXDate date;
  date.setJulian((FXuint)(FXival)(ptr));
  year->setText(FXStringVal(date.year()));
  /*month->setCurrentNo(view->getCurrentMonth()-1);*/
  /*month->setText(tr(FXDate::monthName(view->getCurrentMonth())));*/
  month->setText(((FXMenuCommand*)(months->childAtIndex(view->getCurrentMonth()-1)))->getText());
  if(target) target->tryHandle(this,FXSEL(SEL_CHANGED,message),ptr);
  return 1;
  }


long FXCalendar::onCmdMonth(FXObject*,FXSelector sel,void*){
  view->setCurrentMonth((FXSELID(sel)-ID_MONTH_START)+1,true);
  return 1;
  }


long FXCalendar::onCmdSelectMonth(FXObject*,FXSelector,void*ptr){
  view->setCurrentMonth(1+(FXint)(FXival)(ptr),true);
  return 1;
  }


long FXCalendar::onCmdNextYear(FXObject*,FXSelector,void*){
  FXDate date=view->getCurrentDate();
  date.setDate(date.year()+1,date.month(),date.day());
  view->setCurrentDate(date,true);
  return 1;
  }


long FXCalendar::onCmdPrevYear(FXObject*,FXSelector,void*){
  FXDate date=view->getCurrentDate();
  date.setDate(date.year()-1,date.month(),date.day());
  view->setCurrentDate(date,true);
  return 1;
  }


static FXDate incMonth(FXDate d){
  FXint day=d.day();
  FXint month=d.month();
  FXint year=d.year();
  FXDate temp;
  if(d.month()==12){
    month=1;
    year++;
    }
  else{
    month++;
    }
  temp.setDate(year,month,1);
  if(temp.daysInMonth()>day)
    return FXDate(year,month,day);
  else
    return FXDate(year,month,temp.daysInMonth());
  }


static FXDate decMonth(FXDate d){
  FXint day=d.day();
  FXint month=d.month();
  FXint year=d.year();
  FXDate temp;
  if(d.month()==1){
    month=12;
    year--;
    }
  else{
    month--;
    }
  temp.setDate(year,month,1);
  if(temp.daysInMonth()>day)
    return FXDate(year,month,day);
  else
    return FXDate(year,month,temp.daysInMonth());
  }


long FXCalendar::onCmdNextMonth(FXObject*,FXSelector,void*){
  FXDate date=view->getCurrentDate();
  view->setCurrentDate(incMonth(date),true);
  return 1;
  }


long FXCalendar::onCmdPrevMonth(FXObject*,FXSelector,void*){
  FXDate date=view->getCurrentDate();
  view->setCurrentDate(decMonth(date),true);
  return 1;
  }


long FXCalendar::onFwdToView(FXObject*sender,FXSelector sel,void*ptr){
  return view->handle(sender,sel,ptr);
  }


long FXCalendar::onFwdToTarget(FXObject*,FXSelector sel,void* ptr){
  return target && target->tryHandle(this,FXSEL(FXSELTYPE(sel),message),ptr);
  }


// Destroy
FXCalendar::~FXCalendar(){
  delete months;
  }


}
