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
* $Id: FXCalendarView.h,v 1.8 2007/02/07 20:21:52 fox Exp $                     *
********************************************************************************/
#ifndef FXCALENDARVIEW_H
#define FXCALENDARVIEW_H

#ifndef FXWINDOW_H
#include "FXWindow.h"
#endif

namespace FX {


/// Calendar styles
enum {
  CALENDAR_BROWSESELECT = 0x00000000,   /// Browse selection mode enforces one single item to be selected at all times
  CALENDAR_SINGLESELECT = 0x00100000,   /// Single selection mode allows up to one item to be selected
  CALENDAR_WEEKNUMBERS  = 0x00200000,   /// Show ISO8601 Week numbers
  CALENDAR_STATIC       = 0x00400000,   /// Disable navigation to prev/next month in display.
  CALENDAR_HIDEOTHER    = 0x00800000,   /// Do not show days of other months.
  };


class FXFont;


/**
* The Basic Calendar Widget. Renders the base calendar and keeps track of selection.
* Most usefull to widget developers. Use it if you need a calendar rendered in some component.
*/
class FXAPI FXCalendarView : public FXWindow {
FXDECLARE(FXCalendarView)
protected:
  FXFont  *font;                // Font
  FXColor  titlecolor;          // Title Color
  FXColor  titlebackcolor;      // Title Back Color
  FXColor  daycolor;            // Normal Day Color
  FXColor  daycolord;           // Normal Day Color Disabled
  FXColor  weekendcolor;        // Weekend Color
  FXColor  weekendcolord;       // Weekend Color Disabled
  FXColor  todaycolor;          // Today Color
  FXint    month;               // Which month is being viewed.
  FXint    firstday;            // Which day of the week we display in the first column
  FXint    ws;                  // First week number in Calendar
  FXDate   ds;                  // First Day in Calendar
  FXDate   current;             // Current Date
  FXDate   selected;            // Selected Date
  FXbool   has_selection;       // If any date is selected
  FXbool   state;               // State
protected:
  FXCalendarView();
  void moveFocus(FXDate);
  void markdirty(FXDate);
  void updateview(FXbool notify=true);
  FXint weeknumber(FXDate) const;
private:
  FXCalendarView(const FXCalendarView&);
  FXCalendarView &operator=(const FXCalendarView&);
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onLeftBtnPress(FXObject*,FXSelector,void*);
  long onLeftBtnRelease(FXObject*,FXSelector,void*);
  long onKeyPress(FXObject*,FXSelector,void*);
  long onKeyRelease(FXObject*,FXSelector,void*);
  long onFocusIn(FXObject*,FXSelector,void*);
  long onFocusOut(FXObject*,FXSelector,void*);
  long onClicked(FXObject*,FXSelector,void*);
  long onDoubleClicked(FXObject*,FXSelector,void*);
  long onTripleClicked(FXObject*,FXSelector,void*);
  long onCommand(FXObject*,FXSelector,void*);
  long onCmdSetValue(FXObject*,FXSelector,void*);
  long onCmdSetIntValue(FXObject*,FXSelector,void*);
  long onCmdGetIntValue(FXObject*,FXSelector,void*);
public:

  /// Construct a Calendar View
  FXCalendarView(FXComposite *p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=CALENDAR_BROWSESELECT,FXint x=0,FXint y=0,FXint w=0,FXint h=0);

  /// Create Calendar View
  virtual void create();

  /// Yes we can
  virtual bool canFocus() const;

  /// Set focus
  virtual void setFocus();

  /// Kill focus
  virtual void killFocus();

  /// Enable the window
  virtual void enable();

  /// Disable the window
  virtual void disable();

  /// Return the default width of this window
  virtual FXint getDefaultWidth();

  /// Return the default height of this window
  virtual FXint getDefaultHeight();

  /// Get date at x,y if any
  FXbool getDateAt(FXint x,FXint y,FXDate& date) const;

  /// Set date
  void setCurrentDate(FXDate date,FXbool notify=false);

  /// Get the current date
  FXDate getCurrentDate() const { return current; }

  /// Set the current month; current day will be properly updated for the choosen month
  void setCurrentMonth(FXint month,FXbool notify=false);

  /// Return the current month shown. The month may be different than the current
  /// date if a day in a sibling month is current.
  FXint getCurrentMonth() const { return month; }

  /// Select Date
  void selectDate(FXDate date,FXbool notify=false);

  /// Deselect Date
  void killSelection(FXbool notify=false);

  /// Get the selected date, if any
  FXbool getSelectedDate(FXDate& date) const;

  /// Set the first day of the week [0...6]
  void setFirstDay(FXint d);

  /// Get the first day of the week [0...6]
  FXint getFirstDay() const { return firstday; }

  /// Set the calendar style
  void setCalendarStyle(FXuint);

  /// Get the calendar style
  FXuint getCalendarStyle() const;

  /// Set the display color of titles
  void setTitleColor(FXColor c);

  /// Get the display color of titles
  FXColor getTitleColor() const { return titlecolor; }

  /// Set the display background color of titles
  void setTitleBackColor(FXColor c);

  /// Get the display background color of titles
  FXColor getTitleBackColor() const { return titlebackcolor; }

  /// Set the display color of non-weekend days
  void setDayColor(FXColor c);

  /// Get the display color of non-weekend days
  FXColor getDayColor() const { return daycolor; }

  /// Set the display color of non-weekend days not in the current month
  void setOtherDayColor(FXColor c);

  /// Get the display color of non-weekend days not in the current month
  FXColor getOtherDayColor() const { return daycolord; }

  /// Set the display color of today
  void setTodayColor(FXColor c);

  /// Get the display color of today
  FXColor getTodayColor() const { return todaycolor; }

  /// Set the display color of days in the weekend
  void setWeekendColor(FXColor c);

  /// Get the display color of days in the weekend
  FXColor getWeekendColor() const { return weekendcolor; }

  /// Set the display color of days in the weekend not in the current month
  void setOtherWeekendColor(FXColor c);

  /// Get the display color of days in the weekend not in the current month
  FXColor getOtherWeekendColor() const { return weekendcolord; }

  /// Destructor
  virtual ~FXCalendarView();
  };

}

#endif

