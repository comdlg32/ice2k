// TAKEN FROM FOX SOURCE CODE
// SLIGHT MODIFICATIONS FOR ACCURACY

/********************************************************************************
*                                                                               *
*                       C o m b o   B o x   O b j e c t                         *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998,2006 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: I2KComboBox.cpp,v 1.66.2.2 2007/06/07 20:17:57 fox Exp $                      *
********************************************************************************/

#define _DO_NOT_OVERRIDE_FOX_CONTROLS

#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxkeys.h"
#include "FXHash.h"
#include "FXThread.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXObjectList.h"
#include "FXSettings.h"
#include "FXRegistry.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXDrawable.h"
#include "FXDC.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXLabel.h"
#include "FXTextField.h"
#include "FXButton.h"
#include "FXMenuButton.h"
#include "FXComposite.h"
#include "FXPacker.h"
#include "FXShell.h"
#include "FXPopup.h"
#include "FXScrollBar.h"
#include "FXScrollArea.h"
#include "FXList.h"
#include "I2KComboBox.h"

#ifdef _COMCTL32_BUILDING
#include "comctl32.h"
#else
#include "ice2k/comctl32.h"
#endif

/*
  Notes:
  - Handling typed text:
    a) Pass string to target only.
    b) Pass string to target & add to list [begin, after/before current, or end].
    c) Pass string to target & replace current item's label.
  - I2KComboBox is a text field which may be filled from an FXList.
  - I2KComboBox is a text field which in turn may fill an FXList also.
  - In most other respects, it behaves like a FXTextField.
  - Need to catch up/down arrow keys.
  - Combobox turns OFF GUI Updating while being manipulated.
  - Need some way to size the FXList automatically to the number of items.
  - If you leave the list then getCurrentItem() returns the last item under
    cursor which is not the same item shown in I2KComboBox.
  - I2KComboBox::getItem() and I2KComboBox::getItemText() are the same; this
    should probably change.
  - No reaction to up and down arrow while disabled.
*/

#define COMBOBOX_INS_MASK   (COMBOBOX_REPLACE|COMBOBOX_INSERT_BEFORE|COMBOBOX_INSERT_AFTER|COMBOBOX_INSERT_FIRST|COMBOBOX_INSERT_LAST)
#define COMBOBOX_MASK       (COMBOBOX_STATIC|COMBOBOX_INS_MASK)

using namespace FX;


/*******************************************************************************/

namespace FX {

// Map
FXDEFMAP(I2KComboBox) I2KComboBoxMap[]={
  FXMAPFUNC(SEL_FOCUS_UP,0,I2KComboBox::onFocusUp),
  FXMAPFUNC(SEL_FOCUS_DOWN,0,I2KComboBox::onFocusDown),
  FXMAPFUNC(SEL_FOCUS_SELF,0,I2KComboBox::onFocusSelf),
  FXMAPFUNC(SEL_UPDATE,I2KComboBox::ID_TEXT,I2KComboBox::onUpdFmText),
  FXMAPFUNC(SEL_CLICKED,I2KComboBox::ID_LIST,I2KComboBox::onListClicked),
  FXMAPFUNC(SEL_COMMAND,I2KComboBox::ID_LIST,I2KComboBox::onListClicked),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,I2KComboBox::ID_TEXT,I2KComboBox::onTextButton),
  FXMAPFUNC(SEL_MOUSEWHEEL,I2KComboBox::ID_TEXT,I2KComboBox::onMouseWheel),
  FXMAPFUNC(SEL_CHANGED,I2KComboBox::ID_TEXT,I2KComboBox::onTextChanged),
  FXMAPFUNC(SEL_COMMAND,I2KComboBox::ID_TEXT,I2KComboBox::onTextCommand),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETVALUE,I2KComboBox::onFwdToText),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETINTVALUE,I2KComboBox::onFwdToText),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETREALVALUE,I2KComboBox::onFwdToText),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETSTRINGVALUE,I2KComboBox::onFwdToText),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETINTVALUE,I2KComboBox::onFwdToText),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETREALVALUE,I2KComboBox::onFwdToText),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETSTRINGVALUE,I2KComboBox::onFwdToText),
  };


// Object implementation
FXIMPLEMENT(I2KComboBox,FXPacker,I2KComboBoxMap,ARRAYNUMBER(I2KComboBoxMap))


// Combo box
I2KComboBox::I2KComboBox(FXComposite *p,FXint cols,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb):
  FXPacker(p,opts,x,y,w,h, 0,0,0,0, 0,0){
  flags|=FLAG_ENABLED;
  target=tgt;
  message=sel;
  field=new FXTextField(this,cols,this,I2KComboBox::ID_TEXT,0, 0,0,0,0, pl,pr,pt,pb);
  if(options&COMBOBOX_STATIC) field->setEditable(FALSE);
  pane=new FXPopup(this,FRAME_LINE);
  list=new FXList(pane,this,I2KComboBox::ID_LIST,LIST_BROWSESELECT|LIST_AUTOSELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y|SCROLLERS_TRACK|HSCROLLER_NEVER);
  if(options&COMBOBOX_STATIC) list->setScrollStyle(SCROLLERS_TRACK|HSCROLLING_OFF);
  button=new I2KMenuButton(this,FXString::null,NULL,pane,FRAME_RAISED|FRAME_THICK|MENUBUTTON_DOWN|MENUBUTTON_ATTACH_RIGHT, 0,0,0,0, 0,0,0,0);
  button->setXOffset(border);
  button->setYOffset(border);
  flags&=~FLAG_UPDATE;  // Never GUI update
  }


// Create window
void I2KComboBox::create(){
  FXPacker::create();
  pane->create();
  }


// Detach window
void I2KComboBox::detach(){
  FXPacker::detach();
  pane->detach();
  }


// Destroy window
void I2KComboBox::destroy(){
  pane->destroy();
  FXPacker::destroy();
  }


// Enable the window
void I2KComboBox::enable(){
  if(!isEnabled()){
    FXPacker::enable();
    field->enable();
    button->enable();
    }
  }


// Disable the window
void I2KComboBox::disable(){
  if(isEnabled()){
    FXPacker::disable();
    field->disable();
    button->disable();
    }
  }


// Get default width
FXint I2KComboBox::getDefaultWidth(){
  FXint ww,pw;
  ww=field->getDefaultWidth()+button->getDefaultWidth()+(border<<1);
  pw=pane->getDefaultWidth();
  return FXMAX(ww,pw);
  }


// Get default height
FXint I2KComboBox::getDefaultHeight(){
  FXint th,bh;
  th=field->getDefaultHeight();
  bh=button->getDefaultHeight();
  return FXMAX(th,bh)+(border<<1);
  }


// Recalculate layout
void I2KComboBox::layout(){
  FXint buttonWidth,textWidth,itemHeight;
  itemHeight=height-(border<<1);
  buttonWidth=button->getDefaultWidth();
  textWidth=width-buttonWidth-(border<<1);
  field->position(border,border,textWidth,itemHeight);
  button->position(border+textWidth,border,buttonWidth,itemHeight);
  pane->resize(width,pane->getDefaultHeight());
  flags&=~FLAG_DIRTY;
  }


// Forward GUI update of text field to target; but only if pane is not popped
long I2KComboBox::onUpdFmText(FXObject*,FXSelector,void*){
  return target && !isPaneShown() && target->tryHandle(this,FXSEL(SEL_UPDATE,message),NULL);
  }


// Command handled in the text field
long I2KComboBox::onFwdToText(FXObject* sender,FXSelector sel,void* ptr){
  return field->handle(sender,sel,ptr);
  }


// Forward clicked message from list to target
long I2KComboBox::onListClicked(FXObject*,FXSelector sel,void* ptr){
  button->handle(this,FXSEL(SEL_COMMAND,ID_UNPOST),NULL);
  if(FXSELTYPE(sel)==SEL_COMMAND){
    field->setText(list->getItemText((FXint)(FXival)ptr));
    if(!(options&COMBOBOX_STATIC)) field->selectAll();          // Select if editable
    if(target) target->tryHandle(this,FXSEL(SEL_COMMAND,message),(void*)getText().text());
    }
  return 1;
  }


// Pressed left button in text field
long I2KComboBox::onTextButton(FXObject*,FXSelector,void*){
  if(options&COMBOBOX_STATIC){
    button->handle(this,FXSEL(SEL_COMMAND,ID_POST),NULL);    // Post the list
    return 1;
    }
  return 0;
  }


// Text has changed
long I2KComboBox::onTextChanged(FXObject*,FXSelector,void* ptr){
  return target && target->tryHandle(this,FXSEL(SEL_CHANGED,message),ptr);
  }


// Text has changed
long I2KComboBox::onTextCommand(FXObject*,FXSelector,void* ptr){
  FXint index=list->getCurrentItem();
  if(!(options&COMBOBOX_STATIC)){
    switch(options&COMBOBOX_INS_MASK){
      case COMBOBOX_REPLACE:
        if(0<=index) setItem(index,(FXchar*)ptr,getItemData(index));
        break;
      case COMBOBOX_INSERT_BEFORE:
        if(0<=index) insertItem(index,(FXchar*)ptr);
        break;
      case COMBOBOX_INSERT_AFTER:
        if(0<=index) insertItem(index+1,(FXchar*)ptr);
        break;
      case COMBOBOX_INSERT_FIRST:
        insertItem(0,(FXchar*)ptr);
        break;
      case COMBOBOX_INSERT_LAST:
        appendItem((FXchar*)ptr);
        break;
      }
    }
  return target && target->tryHandle(this,FXSEL(SEL_COMMAND,message),ptr);
  }


// Bounce focus to the text field
long I2KComboBox::onFocusSelf(FXObject* sender,FXSelector,void* ptr){
  return field->handle(sender,FXSEL(SEL_FOCUS_SELF,0),ptr);
  }


// Select upper item
long I2KComboBox::onFocusUp(FXObject*,FXSelector,void*){
  if(isEnabled()){
    FXint index=getCurrentItem();
    if(index<0) index=getNumItems()-1;
    else if(0<index) index--;
    if(0<=index && index<getNumItems()){
      setCurrentItem(index,TRUE);
      }
    return 1;
    }
  return 0;
  }


// Select lower item
long I2KComboBox::onFocusDown(FXObject*,FXSelector,void*){
  if(isEnabled()){
    FXint index=getCurrentItem();
    if(index<0) index=0;
    else if(index<getNumItems()-1) index++;
    if(0<=index && index<getNumItems()){
      setCurrentItem(index,TRUE);
      }
    return 1;
    }
  return 0;
  }


// Mouse wheel
long I2KComboBox::onMouseWheel(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  if(isEnabled()){
    FXint index=getCurrentItem();
    if(event->code<0){
      if(index<0) index=0;
      else if(index<getNumItems()-1) index++;
      }
    else if(event->code>0){
      if(index<0) index=getNumItems()-1;
      else if(0<index) index--;
      }
    if(0<=index && index<getNumItems()){
      setCurrentItem(index,TRUE);
      }
    return 1;
    }
  return 0;
  }


// Return true if editable
FXbool I2KComboBox::isEditable() const {
  return field->isEditable();
  }


// Set widget is editable or not
void I2KComboBox::setEditable(FXbool edit){
  field->setEditable(edit);
  }


// Set text
void I2KComboBox::setText(const FXString& text){
  field->setText(text);
  }


// Obtain text
FXString I2KComboBox::getText() const {
  return field->getText();
  }


// Set number of text columns
void I2KComboBox::setNumColumns(FXint cols){
  field->setNumColumns(cols);
  }


// Get number of text columns
FXint I2KComboBox::getNumColumns() const {
  return field->getNumColumns();
  }


// Get number of items
FXint I2KComboBox::getNumItems() const {
  return list->getNumItems();
  }


// Get number of visible items
FXint I2KComboBox::getNumVisible() const {
  return list->getNumVisible();
  }


// Set number of visible items
void I2KComboBox::setNumVisible(FXint nvis){
  list->setNumVisible(nvis);
  }


// Is item current
FXbool I2KComboBox::isItemCurrent(FXint index) const {
  return list->isItemCurrent(index);
  }


// Change current item
void I2KComboBox::setCurrentItem(FXint index,FXbool notify){
  FXint current=list->getCurrentItem();
  if(current!=index){
    list->setCurrentItem(index);
    list->makeItemVisible(index);
    if(0<=index){
      setText(list->getItemText(index));
      }
    else{
      setText(FXString::null);
      }
    if(notify && target){target->tryHandle(this,FXSEL(SEL_COMMAND,message),(void*)getText().text());}
    }
  }


// Get current item
FXint I2KComboBox::getCurrentItem() const {
  return list->getCurrentItem();
  }


// Retrieve item
FXString I2KComboBox::getItem(FXint index) const {
  return list->getItem(index)->getText();
  }


// Replace text of item at index
FXint I2KComboBox::setItem(FXint index,const FXString& text,void* ptr){
  if(index<0 || list->getNumItems()<=index){ fxerror("%s::setItem: index out of range.\n",getClassName()); }
  list->setItem(index,text,NULL,ptr);
  if(isItemCurrent(index)){
    field->setText(text);
    }
  recalc();
  return index;
  }


// Fill list by appending items from array of strings
FXint I2KComboBox::fillItems(const FXchar** strings){
  FXint numberofitems=list->getNumItems();
  FXint n=list->fillItems(strings);
  if(numberofitems<=list->getCurrentItem()){
    field->setText(list->getItemText(list->getCurrentItem()));
    }
  recalc();
  return n;
  }


// Fill list by appending items from newline separated strings
FXint I2KComboBox::fillItems(const FXString& strings){
  FXint numberofitems=list->getNumItems();
  FXint n=list->fillItems(strings);
  if(numberofitems<=list->getCurrentItem()){
    field->setText(list->getItemText(list->getCurrentItem()));
    }
  recalc();
  return n;
  }


// Insert item at index
FXint I2KComboBox::insertItem(FXint index,const FXString& text,void* ptr){
  if(index<0 || list->getNumItems()<index){ fxerror("%s::insertItem: index out of range.\n",getClassName()); }
  list->insertItem(index,text,NULL,ptr);
  if(isItemCurrent(index)){
    field->setText(text);
    }
  recalc();
  return index;
  }


// Append item
FXint I2KComboBox::appendItem(const FXString& text,void* ptr){
  FXint index=list->appendItem(text,NULL,ptr);
  if(isItemCurrent(getNumItems()-1)){
    field->setText(text);
    }
  recalc();
  return index;
  }


// Prepend item
FXint I2KComboBox::prependItem(const FXString& text,void* ptr){
  FXint index=list->prependItem(text,NULL,ptr);
  if(isItemCurrent(0)){
    field->setText(text);
    }
  recalc();
  return index;
  }


// Move item from oldindex to newindex
FXint I2KComboBox::moveItem(FXint newindex,FXint oldindex){
  if(newindex<0 || list->getNumItems()<=newindex || oldindex<0 || list->getNumItems()<=oldindex){ fxerror("%s::moveItem: index out of range.\n",getClassName()); }
  FXint current=list->getCurrentItem();
  list->moveItem(newindex,oldindex);
  if(current!=list->getCurrentItem()){
    current=list->getCurrentItem();
    if(0<=current){
      field->setText(list->getItemText(current));
      }
    else{
      field->setText(" ");
      }
    }
  recalc();
  return newindex;
  }


// Remove given item
void I2KComboBox::removeItem(FXint index){
  FXint current=list->getCurrentItem();
  list->removeItem(index);
  if(index==current){
    current=list->getCurrentItem();
    if(0<=current){
      field->setText(list->getItemText(current));
      }
    else{
      field->setText(FXString::null);
      }
    }
  recalc();
  }


// Remove all items
void I2KComboBox::clearItems(){
  field->setText(FXString::null);
  list->clearItems();
  recalc();
  }


// Get item by name
FXint I2KComboBox::findItem(const FXString& text,FXint start,FXuint flgs) const {
  return list->findItem(text,start,flgs);
  }


// Get item by data
FXint I2KComboBox::findItemByData(const void *ptr,FXint start,FXuint flgs) const {
  return list->findItemByData(ptr,start,flgs);
  }


// Set item text
void I2KComboBox::setItemText(FXint index,const FXString& txt){
  if(isItemCurrent(index)) setText(txt);
  list->setItemText(index,txt);
  recalc();
  }


// Get item text
FXString I2KComboBox::getItemText(FXint index) const {
  return list->getItemText(index);
  }


// Set item data
void I2KComboBox::setItemData(FXint index,void* ptr) const {
  list->setItemData(index,ptr);
  }


// Get item data
void* I2KComboBox::getItemData(FXint index) const {
  return list->getItemData(index);
  }


// Is the pane shown
FXbool I2KComboBox::isPaneShown() const {
  return pane->shown();
  }


// Set font
void I2KComboBox::setFont(FXFont* fnt){
  if(!fnt){ fxerror("%s::setFont: NULL font specified.\n",getClassName()); }
  field->setFont(fnt);
  list->setFont(fnt);
  recalc();
  }


// Obtain font
FXFont* I2KComboBox::getFont() const {
  return field->getFont();
  }


// Change combobox style
void I2KComboBox::setComboStyle(FXuint mode){
  FXuint opts=(options&~COMBOBOX_MASK)|(mode&COMBOBOX_MASK);
  if(opts!=options){
    options=opts;
    if(options&COMBOBOX_STATIC){
      field->setEditable(FALSE);                                // Non-editable
      list->setScrollStyle(SCROLLERS_TRACK|HSCROLLING_OFF);     // No scrolling
      }
    else{
      field->setEditable(TRUE);                                 // Editable
      list->setScrollStyle(SCROLLERS_TRACK|HSCROLLER_NEVER);    // Scrollable, but no scrollbar
      }
    recalc();
    }
  }


// Get combobox style
FXuint I2KComboBox::getComboStyle() const {
  return (options&COMBOBOX_MASK);
  }


// Set text justify style
void I2KComboBox::setJustify(FXuint style){
  field->setJustify(style);
  }


// Get text justify style
FXuint I2KComboBox::getJustify() const {
  return field->getJustify();
  }


// Set window background color
void I2KComboBox::setBackColor(FXColor clr){
  field->setBackColor(clr);
  list->setBackColor(clr);
  }


// Get background color
FXColor I2KComboBox::getBackColor() const {
  return field->getBackColor();
  }


// Set text color
void I2KComboBox::setTextColor(FXColor clr){
  field->setTextColor(clr);
  list->setTextColor(clr);
  }


// Return text color
FXColor I2KComboBox::getTextColor() const {
  return field->getTextColor();
  }


// Set select background color
void I2KComboBox::setSelBackColor(FXColor clr){
  field->setSelBackColor(clr);
  list->setSelBackColor(clr);
  }


// Return selected background color
FXColor I2KComboBox::getSelBackColor() const {
  return field->getSelBackColor();
  }


// Set selected text color
void I2KComboBox::setSelTextColor(FXColor clr){
  field->setSelTextColor(clr);
  list->setSelTextColor(clr);
  }


// Return selected text color
FXColor I2KComboBox::getSelTextColor() const {
  return field->getSelTextColor();
  }


// Sort items using current sort function
void I2KComboBox::sortItems(){
  list->sortItems();
  }


// Return sort function
FXListSortFunc I2KComboBox::getSortFunc() const {
  return list->getSortFunc();
  }


// Change sort function
void I2KComboBox::setSortFunc(FXListSortFunc func){
  list->setSortFunc(func);
  }


// Set help text
void I2KComboBox::setHelpText(const FXString& txt){
  field->setHelpText(txt);
  }


// Get help text
const FXString& I2KComboBox::getHelpText() const {
  return field->getHelpText();
  }


// Set tip text
void I2KComboBox::setTipText(const FXString& txt){
  field->setTipText(txt);
  }


// Get tip text
const FXString& I2KComboBox::getTipText() const {
  return field->getTipText();
  }


// Save object to stream
void I2KComboBox::save(FXStream& store) const {
  FXPacker::save(store);
  store << field;
  store << button;
  store << list;
  store << pane;
  }


// Load object from stream
void I2KComboBox::load(FXStream& store){
  FXPacker::load(store);
  store >> field;
  store >> button;
  store >> list;
  store >> pane;
  }


// Delete it
I2KComboBox::~I2KComboBox(){
  delete pane;
  pane=(FXPopup*)-1L;
  field=(FXTextField*)-1L;
  button=(I2KMenuButton*)-1L;
  list=(FXList*)-1L;
  }

}
