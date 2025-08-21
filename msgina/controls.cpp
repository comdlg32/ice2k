#include <fx.h>

static FXColor focuscolor;

void FXFrame::drawBorderRectangle(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h) {
  if (focuscolor == 0) {
    focuscolor = getApp()->reg().readColorEntry("SETTINGS", "focuscolor", FXRGB(0, 0, 0));
  }

  dc.setForeground(focuscolor);
  dc.drawRectangle(x,y,w-1,h-1);
}

// from fox src
// Handle repaint
long FXButton::onPaint(FXObject*,FXSelector,void* ptr){
  FXint tw=0,th=0,iw=0,ih=0,tx,ty,ix,iy;
  FXEvent *ev=(FXEvent*)ptr;

  // Start drawing
  FXDCWindow dc(this,ev);

  // Got a border at all?
  if(options&(FRAME_RAISED|FRAME_SUNKEN)){

    // Toolbar style
    if(options&BUTTON_TOOLBAR){

      // Enabled and cursor inside, and up
      if(isEnabled() && underCursor() && (state==STATE_UP)){
        dc.setForeground(backColor);
        dc.fillRectangle(border,border,width-border*2,height-border*2);
        if(options&FRAME_THICK) drawDoubleRaisedRectangle(dc,0,0,width,height);
        else drawRaisedRectangle(dc,0,0,width,height);
      }

      // Enabled and cursor inside and down
      else if(isEnabled() && underCursor() && (state==STATE_DOWN)){
        dc.setForeground(backColor);
        dc.fillRectangle(border,border,width-border*2,height-border*2);
        if(options&FRAME_THICK) drawDoubleSunkenRectangle(dc,0,0,width,height);
        else drawSunkenRectangle(dc,0,0,width,height);
      }

      // Enabled and checked
      else if(isEnabled() && (state==STATE_ENGAGED)){
        dc.setForeground(hiliteColor);
        dc.fillRectangle(border,border,width-border*2,height-border*2);
        if(options&FRAME_THICK) drawDoubleSunkenRectangle(dc,0,0,width,height);
        else drawSunkenRectangle(dc,0,0,width,height);
      }

      // Disabled or unchecked or not under cursor
      else{
        dc.setForeground(backColor);
        dc.fillRectangle(0,0,width,height);
      }
    }

    // Normal style
    else {

      // Default
      if(isDefault() && hasFocus()){

        // Draw in up state if disabled or up
        if(!isEnabled() || (state==STATE_UP)){
          //dc.setBackground(backColor);
          dc.setForeground(backColor);
          dc.fillRectangle(border+1,border+1,width-border*2-1,height-border*2-1);
          if(options&FRAME_THICK) drawDoubleRaisedRectangle(dc,1,1,width-2,height-2);
          else drawRaisedRectangle(dc,1,1,width-2,height-2);
          }

        // Draw sunken if enabled and either checked or pressed
        else {
          dc.setForeground(shadowColor);
          dc.drawRectangle(1, 1, width-3, height-3);
          if(state==STATE_ENGAGED) dc.setForeground(hiliteColor); else dc.setForeground(backColor);
          dc.fillRectangle(2, 2, width-4, height-4);

          /* dc.fillRectangle(border,border,width-border*2-1,height-border*2-1);
             if(options&FRAME_THICK) drawDoubleSunkenRectangle(dc, 0, 0, width-1, height-1);
             if(options&FRAME_THICK) drawDoubleSunkenRectangle(dc, 1, 1, width-2, height-2);
             else drawSunkenRectangle(dc, 0, 0, width-1, height-1); */
          }

        // Black default border
        drawBorderRectangle(dc, 0, 0, width, height);
      }

      // Non-Default
      else {

        // Draw in up state if disabled or up
        if (!isEnabled() || (state==STATE_UP)) {
          dc.setForeground(backColor);
          dc.fillRectangle(border, border, width-border*2, height-border*2);
          if(options&FRAME_THICK) drawDoubleRaisedRectangle(dc, 0, 0, width, height);
          else drawRaisedRectangle(dc, 0, 0, width, height);
          }

        // Draw sunken if enabled and either checked or pressed
        else{
          /* if(state==STATE_ENGAGED) dc.setForeground(hiliteColor); else dc.setForeground(backColor);
          dc.fillRectangle(border,border,width-border*2,height-border*2);
          if(options&FRAME_THICK) drawDoubleSunkenRectangle(dc,0,0,width,height);
          else drawSunkenRectangle(dc,0,0,width,height); */

          dc.setForeground(shadowColor);
          dc.drawRectangle(0,0,width-1,height-1);
          if(state==STATE_ENGAGED) dc.setForeground(hiliteColor); else dc.setForeground(backColor);
          dc.fillRectangle(1,1,width-2,height-2);

          }
        }
      }
    }

  // No borders
  else{
    if(isEnabled() && (state==STATE_ENGAGED)){
      dc.setForeground(hiliteColor);
      dc.fillRectangle(0,0,width,height);
      }
    else{
      dc.setForeground(backColor);
      dc.fillRectangle(0,0,width,height);
      }
    }

  // Place text & icon
  if(!label.empty()){
    tw=labelWidth(label);
    th=labelHeight(label);
    }
  if(icon){
    iw=icon->getWidth();
    ih=icon->getHeight();
    }

  just_x(tx,ix,tw,iw);
  just_y(ty,iy,th,ih);

  // Shift a bit when pressed
  if(state && (options&(FRAME_RAISED|FRAME_SUNKEN))){ ++tx; ++ty; ++ix; ++iy; }

  // Draw enabled state
  if(isEnabled()){
    if (icon) {
      dc.drawIcon(icon,ix,iy);
    }

    if (!label.empty()) {
      dc.setFont(font);
      dc.setForeground(textColor);
      drawLabel(dc,label,hotoff,tx,ty,tw,th);
    }

    if (hasFocus()) {
      //dc.drawFocusRectangle(border+1,border+1,width-2*border-2,height-2*border-2);
      dc.drawFocusRectangle(border+2,border+2,(width-2*border-2)-2,(height-2*border-2)-2);
    }
  }

  // Draw grayed-out state
  else {
    if (icon) {
      dc.drawIconSunken(icon,ix,iy);
    }
    if(!label.empty()){
      dc.setFont(font);
      dc.setForeground(hiliteColor);
      drawLabel(dc,label,hotoff,tx+1,ty+1,tw,th);
      dc.setForeground(shadowColor);
      drawLabel(dc,label,hotoff,tx,ty,tw,th);
    }
  }
  return 1;
}

#define FRAME_MASK           (FRAME_SUNKEN|FRAME_RAISED|FRAME_THICK)
#define GROUPBOX_TITLE_MASK  (GROUPBOX_TITLE_LEFT|GROUPBOX_TITLE_CENTER|GROUPBOX_TITLE_RIGHT)

long FXGroupBox::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  FXDCWindow dc(this,event);
  FXint tw,th,yy,xx;

  xx=0;
  yy=0;

  // Paint background
  dc.setForeground(backColor);
  dc.fillRectangle(event->rect.x,event->rect.y,event->rect.w,event->rect.h);

  // Draw label if there is one
  if(!label.empty()){
    yy=2+font->getFontAscent()/2;
  }

  // We should really just draw what's exposed!
  switch(options&FRAME_MASK) {
    case FRAME_LINE: drawBorderRectangle(dc,0,yy,width,height-yy); break;
    case FRAME_SUNKEN: drawSunkenRectangle(dc,0,yy,width,height-yy); break;
    case FRAME_RAISED: drawRaisedRectangle(dc,0,yy,width,height-yy); break;
    case FRAME_GROOVE: drawGrooveRectangle(dc,0,yy,width,height-yy); break;
    case FRAME_RIDGE: drawRidgeRectangle(dc,0,yy,width,height-yy); break;
    case FRAME_SUNKEN|FRAME_THICK: drawDoubleSunkenRectangle(dc,0,yy,width,height-yy); break;
    case FRAME_RAISED|FRAME_THICK: drawDoubleRaisedRectangle(dc,0,yy,width,height-yy); break;
  }

  // Draw label
  if(!label.empty()) {
    tw = font->getTextWidth(label);
    th = font->getFontHeight()+4;
    if (options&GROUPBOX_TITLE_RIGHT) xx=width-tw-12;
    else if(options&GROUPBOX_TITLE_CENTER) xx=(width-tw)/2-4;
    else xx=7;
    if(xx<5) xx=7;
    if(tw+16>width) tw=width-16;
    if(0<tw){
      dc.setForeground(backColor);
      dc.setFont(font);
      dc.fillRectangle(xx,yy,tw+4,2);
      dc.setClipRectangle(xx,0,tw+4,th);
      if(isEnabled()){
        dc.setForeground(textColor);
        dc.drawText(xx+2,2+font->getFontAscent(),label);
        }
      else{
        dc.setForeground(hiliteColor);
        dc.drawText(xx+3,3+font->getFontAscent(),label);
        dc.setForeground(shadowColor);
        dc.drawText(xx+2,2+font->getFontAscent(),label);
        }
      }
    }
  return 1;
}
