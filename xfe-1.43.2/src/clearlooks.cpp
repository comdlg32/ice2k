#include "comboarrow.xbm"
#include "icons.h"
//#include <fx.h>
//#include "xfedefs.h"
// This file contains some FOX functions redefinitions (FOX hacks for the Clearlooks controls)
// The Clearlooks controls try to mimic the GTK Clearlooks theme
// They are optional and can be set within the Preferences dialog
// The hack is done mainly by redefining the onPaint() functions of the various widgets



//
// Some useful functions and macros
//


// Draw rectangle with gradient effect
// Default is vertical gradient
static void drawGradientRectangle(FXDC& dc, FXColor upper, FXColor lower, int x, int y, int w, int h, FXbool vert = true)
{
    register int rr, gg, bb, dr, dg, db, r1, g1, b1, r2, g2, b2, yl, yh, yy, dy, n, t, ww;
    const int    MAXSTEPS = 128;

    if ((0 < w) && (0 < h))
    {
        // Horizontal gradient : exchange w and h
        if (!vert)
        {
            ww = w;
            w = h;
            h = ww;
        }

        dc.setStipple(STIPPLE_NONE);
        dc.setFillStyle(FILL_SOLID);

        r1 = FXREDVAL(upper);
        r2 = FXREDVAL(lower);
        dr = r2-r1;
        g1 = FXGREENVAL(upper);
        g2 = FXGREENVAL(lower);
        dg = g2-g1;
        b1 = FXBLUEVAL(upper);
        b2 = FXBLUEVAL(lower);
        db = b2-b1;

        n = FXABS(dr);
        if ((t = FXABS(dg)) > n)
        {
            n = t;
        }
        if ((t = FXABS(db)) > n)
        {
            n = t;
        }
        n++;
        if (n > h)
        {
            n = h;
        }
        if (n > MAXSTEPS)
        {
            n = MAXSTEPS;
        }
        rr = (r1<<16)+32767;
        gg = (g1<<16)+32767;
        bb = (b1<<16)+32767;
        yy = 32767;

        dr = (dr<<16)/n;
        dg = (dg<<16)/n;
        db = (db<<16)/n;
        dy = (h<<16)/n;

        do
        {
            yl = yy>>16;
            yy += dy;
            yh = yy>>16;
            dc.setForeground(FXRGB(rr>>16, gg>>16, bb>>16));

            // Vertical gradient
            if (vert)
            {
                dc.fillRectangle(x, y+yl, w, yh-yl);
            }

            // Horizontal gradient
            else
            {
                dc.fillRectangle(x+yl, y, yh-yl, w);
            }

            rr += dr;
            gg += dg;
            bb += db;
        } while (yh < h);
    }
}


// These macros are used to simplify the code
// They draw a button in Standard or Clearlooks mode, in up or down state

#define DRAW_CLEARLOOKS_BUTTON_UP                                              \
    dc.setForeground(backColor);                                               \
    dc.drawPoints(basebackground, 4);                                          \
                                                                               \
    dc.setForeground(bordercolor);                                             \
    dc.drawRectangle(2, 0, width-5, 0);                                        \
    dc.drawRectangle(2, height-1, width-5, height-1);                          \
    dc.drawRectangle(0, 2, 0, height-5);                                       \
    dc.drawRectangle(width-1, 2, 0, height-5);                                 \
    dc.drawPoints(bordercorners, 4);                                           \
    dc.setForeground(shadecolor);                                              \
    dc.drawPoints(bordershade, 16);                                            \
                                                                               \
    drawGradientRectangle(dc, topcolor, bottomcolor, 2, 1, width-4, height-2); \
    dc.setForeground(topcolor);                                                \
    dc.drawRectangle(1, 3, 0, height-7);                                       \
    dc.setForeground(bottomcolor);                                             \
    dc.drawRectangle(width-2, 3, 0, height-7);


#define DRAW_CLEARLOOKS_BUTTON_DOWN                   \
    dc.setForeground(shadecolor);                     \
    dc.fillRectangle(0, 0, width, height);            \
                                                      \
    dc.setForeground(backColor);                      \
    dc.drawPoints(basebackground, 4);                 \
                                                      \
    dc.setForeground(bordercolor);                    \
    dc.drawRectangle(2, 0, width-5, 0);               \
    dc.drawRectangle(2, height-1, width-5, height-1); \
    dc.drawRectangle(0, 2, 0, height-5);              \
    dc.drawRectangle(width-1, 2, 0, height-5);        \
    dc.drawPoints(bordercorners, 4);                  \
    dc.setForeground(shadecolor);                     \
    dc.drawPoints(bordershade, 16);


#define DRAW_STANDARD_BUTTON_UP                                        \
    dc.setForeground(backColor);                                       \
    dc.fillRectangle(border, border, width-border*2, height-border*2); \
    if (options&FRAME_THICK) {                                         \
        drawDoubleRaisedRectangle(dc, 0, 0, width, height); }          \
    else{                                                              \
        drawRaisedRectangle(dc, 0, 0, width, height); }


#define DRAW_STANDARD_BUTTON_DOWN                                      \
    dc.setForeground(hiliteColor);                                     \
    dc.fillRectangle(border, border, width-border*2, height-border*2); \
    if (options&FRAME_THICK) {                                         \
        drawDoubleSunkenRectangle(dc, 0, 0, width, height); }          \
    else{                                                              \
        drawSunkenRectangle(dc, 0, 0, width, height); }


#define INIT_CLEARLOOKS                                                                          \
    static FXbool init = true;                                                                   \
    static FXbool  use_clearlooks = true;                                                        \
    static FXColor topcolor, bottomcolor, shadecolor, bordercolor;                               \
                                                                                                 \
    FXPoint basebackground[4] = { FXPoint(0, 0), FXPoint(width-1, 0), FXPoint(0, height-1),      \
                                  FXPoint(width-1, height-1) };                                  \
    FXPoint bordershade[16] = { FXPoint(0, 1), FXPoint(1, 0), FXPoint(1, 2), FXPoint(2, 1),      \
                                FXPoint(width-2, 0), FXPoint(width-1, 1), FXPoint(width-3, 1),   \
                                FXPoint(width-2, 2), FXPoint(0, height-2), FXPoint(1, height-1), \
                                FXPoint(1, height-3), FXPoint(2, height-2),                      \
                                FXPoint(width-1, height-2), FXPoint(width-2, height-1),          \
                                FXPoint(width-2, height-3), FXPoint(width-3, height-2)           \
    };                                                                                           \
    FXPoint bordercorners[4] = { FXPoint(1, 1), FXPoint(1, height-2), FXPoint(width-2, 1),       \
                                 FXPoint(width-2, height-2) };                                   \
                                                                                                 \
    if (init)                                                                                    \
    {                                                                                            \
        use_clearlooks = getApp()->reg().readUnsignedEntry("SETTINGS", "use_clearlooks", true);  \
                                                                                                 \
        if (use_clearlooks)                                                                      \
        {                                                                                        \
            FXuint r = FXREDVAL(baseColor);                                                      \
            FXuint g = FXGREENVAL(baseColor);                                                    \
            FXuint b = FXBLUEVAL(baseColor);                                                     \
                                                                                                 \
            topcolor = FXRGB(FXMIN(1.1*r, 255), FXMIN(1.1*g, 255), FXMIN(1.1*b, 255));           \
            (void)topcolor;      /* Hack to avoid unused variable compiler warning */            \
            bottomcolor = FXRGB(0.9*r, 0.9*g, 0.9*b);                                            \
            (void)bottomcolor;   /* Hack to avoid unused variable compiler warning */            \
            shadecolor = FXRGB(0.9*r, 0.9*g, 0.9*b);                                             \
            bordercolor = FXRGB(0.5*r, 0.5*g, 0.5*b);                                            \
        }                                                                                        \
        init = false;                                                                            \
    }



static FXColor focuscolor;

void FXFrame::drawBorderRectangle(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h) {
  if (focuscolor == 0) {
    focuscolor = getApp()->reg().readColorEntry("SETTINGS", "focuscolor", FXRGB(0, 0, 0));
  }

  dc.setForeground(focuscolor);
  dc.drawRectangle(x,y,w-1,h-1);
}


// Hack of FXButton
// Modified from FOX source code


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
        dc.setFillStyle(FILL_TILED);
        dc.setTile(imgstipple, 0, 0);

        dc.fillRectangle(border,border,width-border*2,height-border*2);


        dc.setFillStyle(FILL_SOLID);
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


//
// Hack of FXButton (button with gradient effect and rounded corners)
// Original author : Sander Jansen <sander@knology.net>
//


// Handle repaint
/* long FXButton::onPaint(FXObject*, FXSelector, void* ptr)
{
    // Initialize Clearlooks
    INIT_CLEARLOOKS

    FXEvent*   ev = (FXEvent*)ptr;
    FXDCWindow dc(this, ev);
    int        tw = 0, th = 0, iw = 0, ih = 0, tx, ty, ix, iy;

    // Button with nice gradient effect and rounded corners (Clearlooks)
    if (use_clearlooks)
    {
        // Toolbar style
        if (options&BUTTON_TOOLBAR)
        {
            // Enabled and cursor inside, and up
            if (isEnabled() && underCursor() && (state == STATE_UP))
            {
                DRAW_CLEARLOOKS_BUTTON_UP
            }

            // Enabled and cursor inside and down
            else if (isEnabled() && underCursor() && (state == STATE_DOWN))
            {
                DRAW_CLEARLOOKS_BUTTON_DOWN
            }

            // Enabled and checked
            else if (isEnabled() && (state == STATE_ENGAGED))
            {
                DRAW_CLEARLOOKS_BUTTON_UP
            }

            // Disabled or unchecked or not under cursor
            else
            {
                dc.setForeground(backColor);
                dc.fillRectangle(0, 0, width, height);
            }
        }

        // Normal style
        else
        {
            // Draw in up state if disabled or up
            if (!isEnabled() || (state == STATE_UP))
            {
                DRAW_CLEARLOOKS_BUTTON_UP
            }

            // Draw in down state if enabled and either checked or pressed
            else
            {
                DRAW_CLEARLOOKS_BUTTON_DOWN
            }
        }
    }   // End of gradient painting

    // Normal flat rectangular button
    else
    {
        // Got a border at all?
        if (options&(FRAME_RAISED|FRAME_SUNKEN))
        {
            // Toolbar style
            if (options&BUTTON_TOOLBAR)
            {
                // Enabled and cursor inside, and up
                if (isEnabled() && underCursor() && (state == STATE_UP))
                {
                    DRAW_STANDARD_BUTTON_UP
                }

                // Enabled and cursor inside and down
                else if (isEnabled() && underCursor() && (state == STATE_DOWN))
                {
                    DRAW_STANDARD_BUTTON_DOWN
                }

                // Enabled and checked
                else if (isEnabled() && (state == STATE_ENGAGED))
                {
                    DRAW_STANDARD_BUTTON_DOWN
                }

                // Disabled or unchecked or not under cursor
                else
                {
                    dc.setForeground(backColor);
                    dc.fillRectangle(0, 0, width, height);
                }
            }

            // Normal style
            else
            {
                // Draw in up state if disabled or up
                if (!isEnabled() || (state == STATE_UP))
                {
                    DRAW_STANDARD_BUTTON_UP
                }

                // Draw sunken if enabled and either checked or pressed
                // Caution! This one is different!
                else
                {
                    if (state == STATE_ENGAGED)
                    {
                        dc.setForeground(hiliteColor);
                    }
                    else
                    {
                        dc.setForeground(backColor);
                    }
                    dc.fillRectangle(border, border, width-border*2, height-border*2);
                    if (options&FRAME_THICK)
                    {
                        drawDoubleSunkenRectangle(dc, 0, 0, width, height);
                    }
                    else
                    {
                        drawSunkenRectangle(dc, 0, 0, width, height);
                    }
                }
            }
        }

        // No borders
        else
        {
            if (isEnabled() && (state == STATE_ENGAGED))
            {
                dc.setForeground(hiliteColor);
                dc.fillRectangle(0, 0, width, height);
            }
            else
            {
                dc.setForeground(backColor);
                dc.fillRectangle(0, 0, width, height);
            }
        }
    }   // End of normal painting

    // Place text & icon
    if (!label.empty())
    {
        tw = labelWidth(label);
        th = labelHeight(label);
    }
    if (icon)
    {
        iw = icon->getWidth();
        ih = icon->getHeight();
    }

    just_x(tx, ix, tw, iw);
    just_y(ty, iy, th, ih);

    // Shift a bit when pressed
    if (state && (options&(FRAME_RAISED|FRAME_SUNKEN)))
    {
        ++tx;
        ++ty;
        ++ix;
        ++iy;
    }

    // Draw enabled state
    if (isEnabled())
    {
        if (icon)
        {
            dc.drawIcon(icon, ix, iy);
        }
        if (!label.empty())
        {
            dc.setFont(font);
            dc.setForeground(textColor);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
        }
        if (hasFocus())
        {
            dc.drawFocusRectangle(border+1, border+1, width-2*border-2, height-2*border-2);
        }
    }

    // Draw grayed-out state
    else
    {
        if (icon)
        {
            dc.drawIconSunken(icon, ix, iy);
        }
        if (!label.empty())
        {
            dc.setFont(font);
            dc.setForeground(hiliteColor);
            drawLabel(dc, label, hotoff, tx+1, ty+1, tw, th);
            dc.setForeground(shadowColor);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
        }
    }

    return(1);
} */


//
// Hack of FXCheckButton
//

// Handle repaint
long FXCheckButton::onPaint(FXObject*, FXSelector, void* ptr)
{
    // Initialize Clearlooks (don't use the macro because here it's different)
    static FXbool  init = true;
    static FXbool  use_clearlooks = true;
    static FXColor shadecolor, bordercolor;

    if (init)
    {
        use_clearlooks = getApp()->reg().readUnsignedEntry("SETTINGS", "use_clearlooks", true);

        if (use_clearlooks)
        {
            FXuint r = FXREDVAL(baseColor);
            FXuint g = FXGREENVAL(baseColor);
            FXuint b = FXBLUEVAL(baseColor);

            shadecolor = FXRGB(0.9*r, 0.9*g, 0.9*b);
            bordercolor = FXRGB(0.5*r, 0.5*g, 0.5*b);
        }
        init = false;
    }

    FXEvent* ev = (FXEvent*)ptr;
    FXint    tw = 0, th = 0, tx, ty, ix, iy;

    FXDCWindow dc(this, ev);

    // Figure text size
    if (!label.empty())
    {
        tw = labelWidth(label);
        th = labelHeight(label);
    }

    // Placement
    just_x(tx, ix, tw, 13);
    just_y(ty, iy, th, 13);

    // Button with nice gradient effect and rounded corners (Clearlooks)
    if (use_clearlooks)
    {
        // Widget background
        dc.setForeground(backColor);
        dc.fillRectangle(ev->rect.x, ev->rect.y, ev->rect.w, ev->rect.h);

        // Check background
        if ((check == MAYBE) || !isEnabled())
        {
            dc.setForeground(baseColor);
        }
        else
        {
            dc.setForeground(boxColor);
        }

        // Check border for +
        if (options&CHECKBUTTON_PLUS)
        {
            dc.fillRectangle(ix+2, iy+2, 9, 9);
            dc.setForeground(bordercolor);
            dc.drawRectangle(ix+2, iy+2, 8, 8);
        }

        // Check border for v
        else
        {
            // Check background
            dc.fillRectangle(ix+2, iy+2, 9, 9);

            // Border
            dc.setForeground(bordercolor);
            dc.drawRectangle(ix+2, iy+1, 8, 0);
            dc.drawRectangle(ix+2, iy+11, 8, 0);
            dc.drawRectangle(ix+1, iy+2, 0, 8);
            dc.drawRectangle(ix+11, iy+2, 0, 8);

            // Border corners
            FXPoint checkcorners[4] =
            {
                FXPoint(ix+1, iy+1), FXPoint(ix+1, iy+11), \
                FXPoint(ix+11, iy+1), FXPoint(ix+11, iy+11)
            };
            dc.setForeground(shadecolor);
            dc.drawPoints(checkcorners, 4);
        }

        // Check color
        if ((check == MAYBE) || !isEnabled())
        {
            dc.setForeground(shadowColor);
        }
        else
        {
            dc.setForeground(checkColor);
        }
    }

    // Normal flat rectangular button
    else
    {
        // Widget background
        dc.setForeground(backColor);
        dc.fillRectangle(ev->rect.x, ev->rect.y, ev->rect.w, ev->rect.h);

        // Check background
        if ((check == MAYBE) || !isEnabled())
        {
            dc.setForeground(baseColor);
        }
        else
        {
            dc.setForeground(boxColor);
        }
        dc.fillRectangle(ix+2, iy+2, 9, 9);

        // Check border for +
        if (options&CHECKBUTTON_PLUS)
        {
            dc.setForeground(textColor);
            dc.drawRectangle(ix+2, iy+2, 8, 8);
        }

        // Check border for v
        else
        {
            dc.setForeground(shadowColor);
            dc.fillRectangle(ix, iy, 12, 1);
            dc.fillRectangle(ix, iy, 1, 12);
            dc.setForeground(borderColor);
            dc.fillRectangle(ix+1, iy+1, 10, 1);
            dc.fillRectangle(ix+1, iy+1, 1, 10);
            dc.setForeground(hiliteColor);
            dc.fillRectangle(ix, iy+12, 13, 1);
            dc.fillRectangle(ix+12, iy, 1, 13);
            dc.setForeground(baseColor);
            dc.fillRectangle(ix+1, iy+11, 11, 1);
            dc.fillRectangle(ix+11, iy+1, 1, 11);
        }

        // Check color
        if ((check == MAYBE) || !isEnabled())
        {
            dc.setForeground(shadowColor);
        }
        else
        {
            dc.setForeground(checkColor);
        }
    }

    // Show as +
    if (options&CHECKBUTTON_PLUS)
    {
        if (check != true)
        {
            dc.fillRectangle(ix+6, iy+4, 1, 5);
        }
        dc.fillRectangle(ix+4, iy+6, 5, 1);
    }

    // Show as v
    else
    {
        if (check != false)
        {
            FXSegment seg[6];
            seg[0].x1 = 3+ix;
            seg[0].y1 = 5+iy;
            seg[0].x2 = 5+ix;
            seg[0].y2 = 7+iy;
            seg[1].x1 = 3+ix;
            seg[1].y1 = 6+iy;
            seg[1].x2 = 5+ix;
            seg[1].y2 = 8+iy;
            seg[2].x1 = 3+ix;
            seg[2].y1 = 7+iy;
            seg[2].x2 = 5+ix;
            seg[2].y2 = 9+iy;
            seg[3].x1 = 5+ix;
            seg[3].y1 = 7+iy;
            seg[3].x2 = 9+ix;
            seg[3].y2 = 3+iy;
            seg[4].x1 = 5+ix;
            seg[4].y1 = 8+iy;
            seg[4].x2 = 9+ix;
            seg[4].y2 = 4+iy;
            seg[5].x1 = 5+ix;
            seg[5].y1 = 9+iy;
            seg[5].x2 = 9+ix;
            seg[5].y2 = 5+iy;
            dc.drawLineSegments(seg, 6);
        }
    }

    // Text
    if (!label.empty())
    {
        dc.setFont(font);
        if (isEnabled())
        {
            dc.setForeground(textColor);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
            if (hasFocus())
            {
                dc.drawFocusRectangle(tx-1, ty-1, tw+2, th+2);
            }
        }
        else
        {
            dc.setForeground(hiliteColor);
            drawLabel(dc, label, hotoff, tx+1, ty+1, tw, th);
            dc.setForeground(shadowColor);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
        }
    }

    // Frame
    drawFrame(dc, 0, 0, width, height);

    return(1);
}


//
// Hack of FXTextField
//

// Handle repaint
long FXTextField::onPaint(FXObject*, FXSelector, void* ptr)
{
    // Initialize Clearlooks
    INIT_CLEARLOOKS

    FXEvent*   ev = (FXEvent*)ptr;
    FXDCWindow dc(this, ev);

    // Draw frame
    drawFrame(dc, 0, 0, width, height);

    // Draw background
    dc.setForeground(backColor);
    dc.fillRectangle(border, border, width-(border<<1), height-(border<<1));

    // !!! Hack to get an optional rounded rectangle shape
    // only if _TEXTFIELD_NOFRAME is not specified !!!
    if ( (!(options&_TEXTFIELD_NOFRAME))  & use_clearlooks )
    {
        // Outside Background
        dc.setForeground(baseColor);
        dc.fillRectangle(0, 0, width, height);
        dc.drawPoints(basebackground, 4);

        // Border
        dc.setForeground(bordercolor);
        dc.drawRectangle(2, 0, width-5, 0);
        dc.drawRectangle(2, height-1, width-5, height-1);
        dc.drawRectangle(0, 2, 0, height-5);
        dc.drawRectangle(width-1, 2, 0, height-5);
        dc.drawPoints(bordercorners, 4);
        dc.setForeground(shadecolor);
        dc.drawPoints(bordershade, 16);
        dc.setForeground(backColor);
        dc.fillRectangle(2, 1, width-4, height-2);
    }
    // !!! End of hack

    // Draw text, clipped against frame interior
    dc.setClipRectangle(border, border, width-(border<<1), height-(border<<1));
    drawTextRange(dc, 0, contents.length());

    // Draw caret
    if (flags&FLAG_CARET)
    {
        int xx = coord(cursor)-1;
        dc.setForeground(cursorColor);
        dc.fillRectangle(xx+1, padtop+border, 1, height-padbottom-padtop-(border<<1));
        //dc.fillRectangle(xx-2, padtop+border, 5, 1);
        //dc.fillRectangle(xx-2, height-border-padbottom-1, 5, 1);
    }

    return(1);
}


void FXTextField::drawTextFragment(FXDCWindow& dc,FXint x,FXint y,FXint fm,FXint to){
  x+=font->getTextWidth(contents.text(),fm);
  y+=font->getFontAscent();
  dc.drawText(x,y+1,&contents[fm],to-fm);
}


//
// Hack of FXToggleButton
//

// Handle repaint
long FXToggleButton::onPaint(FXObject*, FXSelector, void* ptr)
{
    // Initialize Clearlooks
    INIT_CLEARLOOKS

    int        tw = 0, th = 0, iw = 0, ih = 0, tx, ty, ix, iy;
    FXEvent*   ev = (FXEvent*)ptr;
    FXDCWindow dc(this, ev);

    // Button with nice gradient effect and rounded corners (Clearlooks)
    if (use_clearlooks)
    {
        // Button style is toolbar
        if (options&TOGGLEBUTTON_TOOLBAR)
        {
            // Enabled and cursor inside and button down
            if (down || ((options&TOGGLEBUTTON_KEEPSTATE) && state))
            {
                DRAW_CLEARLOOKS_BUTTON_DOWN
            }
            // Enabled and cursor inside but button not down
            else if (isEnabled() && underCursor())
            {
                DRAW_CLEARLOOKS_BUTTON_UP
            }

            // Disabled or unchecked or not under cursor
            else
            {
                dc.setForeground(backColor);
                dc.fillRectangle(0, 0, width, height);
            }
        }

        // Button style is normal
        else
        {
            // Button down
            if (down || ((options&TOGGLEBUTTON_KEEPSTATE) && state))
            {
                DRAW_CLEARLOOKS_BUTTON_DOWN
            }

            // Button up
            else
            {
                DRAW_CLEARLOOKS_BUTTON_UP
            }
        }
    }   // End of gradient painting

    // Normal flat rectangular button
    else
    {
        // Got a border at all?
        if (options&(FRAME_RAISED|FRAME_SUNKEN))
        {
            // Button style is normal
            if (options&TOGGLEBUTTON_TOOLBAR)
            {
                // Enabled and cursor inside and down
                if (down || ((options&TOGGLEBUTTON_KEEPSTATE) && state))
                {
                    DRAW_STANDARD_BUTTON_DOWN
                }

                // Enabled and cursor inside, and up
                else if (isEnabled() && underCursor())
                {
                    DRAW_STANDARD_BUTTON_UP
                }

                // Disabled or unchecked or not under cursor
                else
                {
                    dc.setForeground(backColor);
                    dc.fillRectangle(0, 0, width, height);
                }
            }

            // Button style is normal
            else
            {
                // Draw sunken if pressed
                if (down || ((options&TOGGLEBUTTON_KEEPSTATE) && state))
                {
                    DRAW_STANDARD_BUTTON_DOWN
                }

                // Draw raised if not currently pressed down
                else
                {
                    DRAW_STANDARD_BUTTON_UP
                }
            }
        }

        // No borders
        else
        {
            dc.setForeground(backColor);
            dc.fillRectangle(0, 0, width, height);
        }
    }   // End of normal painting

    // Place text & icon
    if (state && !altlabel.empty())
    {
        tw = labelWidth(altlabel);
        th = labelHeight(altlabel);
    }
    else if (!label.empty())
    {
        tw = labelWidth(label);
        th = labelHeight(label);
    }
    if (state && alticon)
    {
        iw = alticon->getWidth();
        ih = alticon->getHeight();
    }
    else if (icon)
    {
        iw = icon->getWidth();
        ih = icon->getHeight();
    }

    just_x(tx, ix, tw, iw);
    just_y(ty, iy, th, ih);

    // Shift a bit when pressed
    if ((down || ((options&TOGGLEBUTTON_KEEPSTATE) && state)) && (options&(FRAME_RAISED|FRAME_SUNKEN)))
    {
        ++tx;
        ++ty;
        ++ix;
        ++iy;
    }

    // Draw enabled state
    if (isEnabled())
    {
        if (state && alticon)
        {
            dc.drawIcon(alticon, ix, iy);
        }
        else if (icon)
        {
            dc.drawIcon(icon, ix, iy);
        }
        if (state && !altlabel.empty())
        {
            dc.setFont(font);
            dc.setForeground(textColor);
            drawLabel(dc, altlabel, althotoff, tx, ty, tw, th);
        }
        else if (!label.empty())
        {
            dc.setFont(font);
            dc.setForeground(textColor);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
        }
        if (hasFocus())
        {
            dc.drawFocusRectangle(border+1, border+1, width-2*border-2, height-2*border-2);
        }
    }

    // Draw grayed-out state
    else
    {
        if (state && alticon)
        {
            dc.drawIconSunken(alticon, ix, iy);
        }
        else if (icon)
        {
            dc.drawIconSunken(icon, ix, iy);
        }
        if (state && !altlabel.empty())
        {
            dc.setFont(font);
            dc.setForeground(hiliteColor);
            drawLabel(dc, altlabel, althotoff, tx+1, ty+1, tw, th);
            dc.setForeground(shadowColor);
            drawLabel(dc, altlabel, althotoff, tx, ty, tw, th);
        }
        else if (!label.empty())
        {
            dc.setFont(font);
            dc.setForeground(hiliteColor);
            drawLabel(dc, label, hotoff, tx+1, ty+1, tw, th);
            dc.setForeground(shadowColor);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
        }
    }

    return(1);
}


//
// Hack of FXScrollBar
//


// Draw scrollbar button with gradient effect and nice grip
static void drawGradientScrollButton(FXDCWindow& dc, FXColor topcolor, FXColor bottomcolor, FXColor shadecolor, FXColor lightcolor,
                                     FXuint options, int x, int y, int w, int h)
{
    // Fill rectangle with gradient in the right direction (vertical or horizontal)
    FXbool vertical = ((options&SCROLLBAR_HORIZONTAL) ? true : false);

    drawGradientRectangle(dc, topcolor, bottomcolor, x, y, w, h, vertical);

    // Draw button borders
    dc.setForeground(lightcolor);
    dc.fillRectangle(x+1, y+1, w-1, 1);
    dc.fillRectangle(x+1, y+1, 1, h-2);
    dc.setForeground(shadecolor);
    dc.fillRectangle(x, y, w, 1);
    dc.fillRectangle(x, y, 1, h-1);
    dc.fillRectangle(x, y+h-1, w, 1);
    dc.fillRectangle(x+w-1, y, 1, h);

    // Draw grip lines for horizontal scrollbar
    if ((options&SCROLLBAR_HORIZONTAL))
    {
        dc.setForeground(shadecolor);
        dc.fillRectangle(x+w/2-3, y+4, 1, h-7);
        dc.fillRectangle(x+w/2, y+4, 1, h-7);
        dc.fillRectangle(x+w/2+3, y+4, 1, h-7);
        dc.setForeground(lightcolor);
        dc.fillRectangle(x+w/2-2, y+4, 1, h-7);
        dc.fillRectangle(x+w/2+1, y+4, 1, h-7);
        dc.fillRectangle(x+w/2+4, y+4, 1, h-7);
    }

    // Draw grip lines for vertical scrollbar
    else
    {
        dc.setForeground(shadecolor);
        dc.fillRectangle(x+4, y+h/2-3, w-7, 1);
        dc.fillRectangle(x+4, y+h/2, w-7, 1);
        dc.fillRectangle(x+4, y+h/2+3, w-7, 1);
        dc.setForeground(lightcolor);
        dc.fillRectangle(x+4, y+h/2-2, w-7, 1);
        dc.fillRectangle(x+4, y+h/2+1, w-7, 1);
        dc.fillRectangle(x+4, y+h/2+4, w-7, 1);
    }
}


// Small hack to set the minimum length of the scrollbar button to barsize*2 instead of barsize/2
void FXScrollBar::setPosition(int p)
{
    int total, travel, lo, hi, l, h;

    pos = p;
    if (pos < 0)
    {
        pos = 0;
    }
    if (pos > (range-page))
    {
        pos = range-page;
    }
    lo = thumbpos;
    hi = thumbpos+thumbsize;
    if (options&SCROLLBAR_HORIZONTAL)
    {
        total = width-height-height;
        thumbsize = (total*page)/range;
        // !!! Hack to change the minimum button size !!!
        if (thumbsize < (barsize<<1))
        {
            thumbsize = (barsize<<1);
        }
        // !!! End of hack !!!
        travel = total-thumbsize;
        if (range > page)
        {
            thumbpos = height+(int)((((double)pos)*travel)/(range-page));
        }
        else
        {
            thumbpos = height;
        }
        l = thumbpos;
        h = thumbpos+thumbsize;
        if ((l != lo) || (h != hi))
        {
            update(FXMIN(l, lo), 0, FXMAX(h, hi)-FXMIN(l, lo), height);
        }
    }
    else
    {
        total = height-width-width;
        thumbsize = (total*page)/range;
        // !!! Hack to change the minimum button size !!!
        if (thumbsize < (barsize<<1))
        {
            thumbsize = (barsize<<1);
        }
        // !!! End of hack !!!
        travel = total-thumbsize;
        if (range > page)
        {
            thumbpos = width+(int)((((double)pos)*travel)/(range-page));
        }
        else
        {
            thumbpos = width;
        }
        l = thumbpos;
        h = thumbpos+thumbsize;
        if ((l != lo) || (h != hi))
        {
            update(0, FXMIN(l, lo), width, FXMAX(h, hi)-FXMIN(l, lo));
        }
    }
}


// Arrow directions
enum
{
    _ARROW_LEFT,
    _ARROW_RIGHT,
    _ARROW_UP,
    _ARROW_DOWN
};


// Draw arrow button in scrollbar with gradient effect and rounded corners (Clearlooks)
static void drawGradientArrowButton(FXDCWindow& dc, FXColor backcolor, FXColor topcolor, FXColor bottomcolor, FXColor shadecolor,
                                    FXColor lightcolor, FXColor bordercolor, FXColor arrowcolor,
                                    FXuint options, int x, int y, int w, int h, FXbool down, FXuint direction)
{
    FXPoint arrowpoints[3];
    int     xx, yy, ah, ab;

    FXPoint basebackground[2];
    FXPoint bordershade[8];
    FXPoint bordercorners[2];

    // Rounded corner and arrow point coordinates depend on the button direction
    if (direction == _ARROW_UP)
    {
        // Rounded corners
        basebackground[0] = FXPoint(0, 0);
        basebackground[1] = FXPoint(w-1, 0);
        bordercorners[0] = FXPoint(1, 1);
        bordercorners[1] = FXPoint(w-2, 1);
        bordershade[0] = FXPoint(0, 1);
        bordershade[1] = FXPoint(1, 0);
        bordershade[2] = FXPoint(1, 2);
        bordershade[3] = FXPoint(2, 1);
        bordershade[4] = FXPoint(w-2, 0);
        bordershade[5] = FXPoint(w-1, 1);
        bordershade[6] = FXPoint(w-3, 1);
        bordershade[7] = FXPoint(w-2, 2);

        // Arrow points
        ab = (w-7)|1;
        ah = ab>>1;
        xx = x+((w-ab)>>1);
        yy = y+((h-ah)>>1);
        if (down)
        {
            ++xx;
            ++yy;
        }
        arrowpoints[0] = FXPoint(xx+(ab>>1), yy-1);
        arrowpoints[1] = FXPoint(xx, yy+ah);
        arrowpoints[2] = FXPoint(xx+ab, yy+ah);
    }
    else if (direction == _ARROW_DOWN)
    {
        // Rounded corners
        basebackground[0] = FXPoint(x, y+h-1);
        basebackground[1] = FXPoint(x+w-1, y+h-1);
        bordercorners[0] = FXPoint(x+1, y+h-2);
        bordercorners[1] = FXPoint(x+w-2, y+h-2);
        bordershade[0] = FXPoint(x, y+h-2);
        bordershade[1] = FXPoint(x+1, y+h-1);
        bordershade[2] = FXPoint(x+1, y+h-3);
        bordershade[3] = FXPoint(x+2, y+h-2);
        bordershade[4] = FXPoint(x+w-1, y+h-2);
        bordershade[5] = FXPoint(x+w-2, y+h-1);
        bordershade[6] = FXPoint(x+w-2, y+h-3);
        bordershade[7] = FXPoint(x+w-3, y+h-2);

        // Arrow points
        ab = (w-7)|1;
        ah = ab>>1;
        xx = x+((w-ab)>>1);
        yy = y+((h-ah)>>1);
        if (down)
        {
            ++xx;
            ++yy;
        }
        arrowpoints[0] = FXPoint(xx+1, yy);
        arrowpoints[1] = FXPoint(xx+ab-1, yy);
        arrowpoints[2] = FXPoint(xx+(ab>>1), yy+ah);
    }
    else if (direction == _ARROW_LEFT)
    {
        // Rounded corners
        basebackground[0] = FXPoint(0, 0);
        basebackground[1] = FXPoint(0, h-1);
        bordercorners[0] = FXPoint(1, 1);
        bordercorners[1] = FXPoint(1, h-2);
        bordershade[0] = FXPoint(0, 1);
        bordershade[1] = FXPoint(1, 0);
        bordershade[2] = FXPoint(1, 2);
        bordershade[3] = FXPoint(2, 1);
        bordershade[4] = FXPoint(0, h-2);
        bordershade[5] = FXPoint(1, h-1);
        bordershade[6] = FXPoint(1, h-3);
        bordershade[7] = FXPoint(2, h-2);

        // Arrow points
        ab = (h-7)|1;
        ah = ab>>1;
        xx = x+((w-ah)>>1);
        yy = y+((h-ab)>>1);
        if (down)
        {
            ++xx;
            ++yy;
        }
        arrowpoints[0] = FXPoint(xx+ah, yy);
        arrowpoints[1] = FXPoint(xx+ah, yy+ab-1);
        arrowpoints[2] = FXPoint(xx, yy+(ab>>1));
    }
    else // _ARROW_RIGHT
    {
        // Rounded corners
        basebackground[0] = FXPoint(x+w-1, y);
        basebackground[1] = FXPoint(x+w-1, y+h-1);
        bordercorners[0] = FXPoint(x+w-2, y+1);
        bordercorners[1] = FXPoint(x+w-2, y+h-2);
        bordershade[0] = FXPoint(x+w-2, y);
        bordershade[1] = FXPoint(x+w-1, y+1);
        bordershade[2] = FXPoint(x+w-3, y+1);
        bordershade[3] = FXPoint(x+w-2, y+2);
        bordershade[4] = FXPoint(x+w-1, y+h-2);
        bordershade[5] = FXPoint(x+w-2, y+h-1);
        bordershade[6] = FXPoint(x+w-2, y+h-3);
        bordershade[7] = FXPoint(x+w-3, y+h-2);

        // Arrow points
        ab = (h-7)|1;
        ah = ab>>1;
        xx = x+((w-ah)>>1);
        yy = y+((h-ab)>>1);
        if (down)
        {
            ++xx;
            ++yy;
        }
        arrowpoints[0] = FXPoint(xx, yy);
        arrowpoints[1] = FXPoint(xx, yy+ab-1);
        arrowpoints[2] = FXPoint(xx+ah, yy+(ab>>1));
    }

    // Draw button when up
    if (!down)
    {
        // Fill rectangle with gradient in the right direction (vertical or horizontal)
        FXbool vertical = ((options&SCROLLBAR_HORIZONTAL) ? true : false);
        drawGradientRectangle(dc, topcolor, bottomcolor, x, y, w, h, vertical);

        // Button borders
        dc.setForeground(lightcolor);
        dc.fillRectangle(x+1, y+1, w-1, 1);
        dc.fillRectangle(x+1, y+1, 1, h-2);
        dc.setForeground(shadecolor);
        dc.fillRectangle(x, y, w, 1);
        dc.fillRectangle(x, y, 1, h-1);
        dc.fillRectangle(x, y+h-1, w, 1);
        dc.fillRectangle(x+w-1, y, 1, h);

        // Rounded corners
        dc.setForeground(backcolor);
        dc.drawPoints(basebackground, 2);
        dc.setForeground(shadecolor);
        dc.drawPoints(bordercorners, 2);
        dc.setForeground(bordercolor);
        dc.drawPoints(bordershade, 8);

        // Arrow
        dc.setForeground(arrowcolor);
        dc.fillPolygon(arrowpoints, 3);
    }

    // Draw button when down (pressed)
    else
    {
        // Dark background
        dc.setForeground(bordercolor);
        dc.fillRectangle(x, y, w, h);

        // Button borders
        dc.setForeground(lightcolor);
        dc.fillRectangle(x+1, y+1, w-1, 1);
        dc.fillRectangle(x+1, y+1, 1, h-2);
        dc.setForeground(shadecolor);
        dc.fillRectangle(x, y, w, 1);
        dc.fillRectangle(x, y, 1, h-1);
        dc.fillRectangle(x, y+h-1, w, 1);
        dc.fillRectangle(x+w-1, y, 1, h);

        // Rounded corners
        dc.setForeground(backcolor);
        dc.drawPoints(basebackground, 2);
        dc.setForeground(shadecolor);
        dc.drawPoints(bordercorners, 2);
        dc.setForeground(bordercolor);
        dc.drawPoints(bordershade, 8);

        // Arrow
        dc.setForeground(arrowcolor);
        dc.fillPolygon(arrowpoints, 3);
    }
}


// Draw flat scrollbar button with selected colors
static void drawFlatScrollButton(FXDCWindow& dc, FXint x, FXint y, FXint w, FXint h, FXbool down, FXColor hilitecolor, FXColor shadowcolor, FXColor bordercolor, FXColor scrollbarcolor)
{
    dc.setForeground(scrollbarcolor);
    dc.fillRectangle(x+2, y+2, w-4, h-4);
    if (!down)
    {
        dc.setForeground(scrollbarcolor);
        dc.fillRectangle(x, y, w-1, 1);
        dc.fillRectangle(x, y, 1, h-1);
        dc.setForeground(hilitecolor);
        dc.fillRectangle(x+1, y+1, w-2, 1);
        dc.fillRectangle(x+1, y+1, 1, h-2);
        dc.setForeground(shadowcolor);
        dc.fillRectangle(x+1, y+h-2, w-2, 1);
        dc.fillRectangle(x+w-2, y+1, 1, h-2);
        dc.setForeground(bordercolor);
        dc.fillRectangle(x, y+h-1, w, 1);
        dc.fillRectangle(x+w-1, y, 1, h);
    }
    else
    {
        dc.setForeground(bordercolor);
        dc.fillRectangle(x, y, w-2, 1);
        dc.fillRectangle(x, y, 1, h-2);
        dc.setForeground(shadowcolor);
        dc.fillRectangle(x+1, y+1, w-3, 1);
        dc.fillRectangle(x+1, y+1, 1, h-3);
        dc.setForeground(hilitecolor);
        dc.fillRectangle(x, y+h-1, w-1, 1);
        dc.fillRectangle(x+w-1, y+1, 1, h-1);
        dc.setForeground(scrollbarcolor);
        dc.fillRectangle(x+1, y+h-2, w-1, 1);
        dc.fillRectangle(x+w-2, y+2, 1, h-2);
    }
}


// Handle repaint
long FXScrollBar::onPaint(FXObject*, FXSelector, void* ptr)
{
    // Caution! Don't use the macro here because it's slightly different

    static FXbool  init = true;
    static FXbool  use_clearlooks = true;
    static FXColor bg_topcolor, bg_bottomcolor, bg_shadecolor, bg_bordercolor, bg_lightcolor;
    static FXColor sb_topcolor, sb_bottomcolor, sb_shadecolor, sb_bordercolor, sb_lightcolor, scrollbarcolor;

    register FXEvent* ev = (FXEvent*)ptr;
    register int      total;
    FXDCWindow        dc(this, ev);

    // At first run, select the scrollbar style and color
    if (init)
    {
        use_clearlooks = getApp()->reg().readUnsignedEntry("SETTINGS", "use_clearlooks", true);
        scrollbarcolor = getApp()->reg().readColorEntry("SETTINGS", "scrollbarcolor", FXRGB(237, 233, 227));

        // Compute gradient colors from the base color
        if (use_clearlooks)
        {
            // Decompose the base color
            FXuint r = FXREDVAL(backColor);
            FXuint g = FXGREENVAL(backColor);
            FXuint b = FXBLUEVAL(backColor);

            // Compute the gradient colors from the base color (background)
            bg_topcolor = FXRGB(FXMIN(1.1*r, 255), FXMIN(1.1*g, 255), FXMIN(1.1*b, 255));
            bg_bottomcolor = FXRGB(0.9*r, 0.9*g, 0.9*b);
            bg_shadecolor = FXRGB(0.8*r, 0.8*g, 0.8*b);
            bg_bordercolor = FXRGB(0.9*r, 0.9*g, 0.9*b);
            bg_lightcolor = FXRGB(FXMIN(1.3*r, 255), FXMIN(1.3*g, 255), FXMIN(1.3*b, 255));

            // Compute the gradient colors from the base color (scrollbar)
            r = FXREDVAL(scrollbarcolor);
            g = FXGREENVAL(scrollbarcolor);
            b = FXBLUEVAL(scrollbarcolor);
            sb_topcolor = FXRGB(FXMIN(1.1*r, 255), FXMIN(1.1*g, 255), FXMIN(1.1*b, 255));
            sb_bottomcolor = FXRGB(0.9*r, 0.9*g, 0.9*b);
            sb_shadecolor = FXRGB(0.8*r, 0.8*g, 0.8*b);
            sb_bordercolor = FXRGB(0.9*r, 0.9*g, 0.9*b);
            (void)sb_bordercolor; // Hack to avoid unused variable compiler warning
            sb_lightcolor = FXRGB(FXMIN(1.3*r, 255), FXMIN(1.3*g, 255), FXMIN(1.3*b, 255));
        }
        init = false;
    }

    // Nice scrollbar with gradient and rounded corners
    if (use_clearlooks)
    {
        if (options&SCROLLBAR_HORIZONTAL)
        {
            total = width-height-height;
            if (thumbsize < total)                                    // Scrollable
            {
                drawGradientScrollButton(dc, sb_topcolor, sb_bottomcolor, sb_shadecolor, sb_lightcolor, options, thumbpos, 0, thumbsize, height);
                dc.setForeground(bg_bordercolor);
                dc.setBackground(backColor);
                dc.fillRectangle(height, 0, thumbpos-height, height);
                dc.fillRectangle(thumbpos+thumbsize, 0, width-height-thumbpos-thumbsize, height);
            }
            else                                                    // Non-scrollable
            {
                dc.setForeground(bg_bordercolor);
                dc.setBackground(backColor);
                dc.fillRectangle(height, 0, total, height);
            }
            drawGradientArrowButton(dc, backColor, bg_topcolor, bg_bottomcolor, bg_shadecolor, bg_lightcolor, bg_bordercolor, arrowColor, options, width-height, 0, height, height, (mode == MODE_INC), _ARROW_RIGHT);
            drawGradientArrowButton(dc, backColor, bg_topcolor, bg_bottomcolor, bg_shadecolor, bg_lightcolor, bg_bordercolor, arrowColor, options, 0, 0, height, height, (mode == MODE_DEC), _ARROW_LEFT);
        }

        // Vertical
        else
        {
            total = height-width-width;
            if (thumbsize < total)                                    // Scrollable
            {
                drawGradientScrollButton(dc, sb_topcolor, sb_bottomcolor, sb_shadecolor, sb_lightcolor, options, 0, thumbpos, width, thumbsize);
                dc.setForeground(bg_bordercolor);
                dc.setBackground(backColor);
                dc.fillRectangle(0, width, width, thumbpos-width);
                dc.fillRectangle(0, thumbpos+thumbsize, width, height-width-thumbpos-thumbsize);
            }
            else                                                    // Non-scrollable
            {
                dc.setForeground(bg_bordercolor);
                dc.setBackground(backColor);
                dc.fillRectangle(0, width, width, total);
            }
            drawGradientArrowButton(dc, backColor, bg_topcolor, bg_bottomcolor, bg_shadecolor, bg_lightcolor, bg_bordercolor, arrowColor, options, 0, height-width, width, width, (mode == MODE_INC), _ARROW_DOWN);
            drawGradientArrowButton(dc, backColor, bg_topcolor, bg_bottomcolor, bg_shadecolor, bg_lightcolor, bg_bordercolor, arrowColor, options, 0, 0, width, width, (mode == MODE_DEC), _ARROW_UP);
        }
    }

    // Standard (flat) scrollbar with selected color
    else
    {
        if (options&SCROLLBAR_HORIZONTAL)
        {
            total = width-height-height;
            if (thumbsize < total)                                    // Scrollable
            {
                drawFlatScrollButton(dc, thumbpos, 0, thumbsize, height, 0, hiliteColor, shadowColor, borderColor, scrollbarcolor);
                dc.setStipple(STIPPLE_GRAY);
                dc.setFillStyle(FILL_OPAQUESTIPPLED);
                if (mode == MODE_PAGE_DEC)
                {
                    dc.setForeground(backColor);
                    dc.setBackground(shadowColor);
                }
                else
                {
                    dc.setForeground(hiliteColor);
                    dc.setBackground(backColor);
                }
                dc.fillRectangle(height, 0, thumbpos-height, height);
                if (mode == MODE_PAGE_INC)
                {
                    dc.setForeground(backColor);
                    dc.setBackground(shadowColor);
                }
                else
                {
                    dc.setForeground(hiliteColor);
                    dc.setBackground(backColor);
                }
                dc.fillRectangle(thumbpos+thumbsize, 0, width-height-thumbpos-thumbsize, height);
            }
            else                                                    // Non-scrollable
            {
                dc.setStipple(STIPPLE_GRAY);
                dc.setFillStyle(FILL_OPAQUESTIPPLED);
                dc.setForeground(hiliteColor);
                dc.setBackground(backColor);
                dc.fillRectangle(height, 0, total, height);
            }
            dc.setFillStyle(FILL_SOLID);
            drawButton(dc, width-height, 0, height, height, (mode == MODE_INC));
            drawRightArrow(dc, width-height, 0, height, height, (mode == MODE_INC));
            drawButton(dc, 0, 0, height, height, (mode == MODE_DEC));
            drawLeftArrow(dc, 0, 0, height, height, (mode == MODE_DEC));
        }
        else
        {
            total = height-width-width;
            if (thumbsize < total)                                    // Scrollable
            {
                drawFlatScrollButton(dc, 0, thumbpos, width, thumbsize, 0, hiliteColor, shadowColor, borderColor, scrollbarcolor);
                dc.setStipple(STIPPLE_GRAY);
                dc.setFillStyle(FILL_OPAQUESTIPPLED);
                if (mode == MODE_PAGE_DEC)
                {
                    dc.setForeground(backColor);
                    dc.setBackground(shadowColor);
                }
                else
                {
                    dc.setForeground(hiliteColor);
                    dc.setBackground(backColor);
                }
                dc.fillRectangle(0, width, width, thumbpos-width);
                if (mode == MODE_PAGE_INC)
                {
                    dc.setForeground(backColor);
                    dc.setBackground(shadowColor);
                }
                else
                {
                    dc.setForeground(hiliteColor);
                    dc.setBackground(backColor);
                }
                dc.fillRectangle(0, thumbpos+thumbsize, width, height-width-thumbpos-thumbsize);
            }
            else                                                    // Non-scrollable
            {
                dc.setStipple(STIPPLE_GRAY);
                dc.setFillStyle(FILL_OPAQUESTIPPLED);
                dc.setForeground(hiliteColor);
                dc.setBackground(backColor);
                dc.fillRectangle(0, width, width, total);
            }
            dc.setFillStyle(FILL_SOLID);
            drawButton(dc, 0, height-width, width, width, (mode == MODE_INC));
            drawDownArrow(dc, 0, height-width, width, width, (mode == MODE_INC));
            drawButton(dc, 0, 0, width, width, (mode == MODE_DEC));
            drawUpArrow(dc, 0, 0, width, width, (mode == MODE_DEC));
        }
    }
    return(1);
}


//
// Hack of FXComboBox
//


#define MENUBUTTONARROW_WIDTH     11
#define MENUBUTTONARROW_HEIGHT    5

// Small hack related to the Clearlooks theme
FXComboBox::FXComboBox(FXComposite* p, int cols, FXObject* tgt, FXSelector sel, FXuint opts, int x, int y, int w, int h, int pl, int pr, int pt, int pb) :
    FXPacker(p, FRAME_SUNKEN|FRAME_THICK|opts, x, y, w, h, 0, 0, 0, 0, 0, 0)
{
    flags |= FLAG_ENABLED;
    target = tgt;
    message = sel;

    // !!! Hack to set options to TEXTFIELD_NORMAL instead of 0 (used by the Clearlooks theme)
    //new FXButton(this, "a", bigfoldericon, this, FXApp::ID_QUIT, LAYOUT_RIGHT);
    if (options&0x01000000)
    {
        field = new FXTextField(this, cols, this, FXComboBox::ID_TEXT, FRAME_NONE, 0, 0, 0, 0, 21, pr, pt, pb);
    } else {
        field = new FXTextField(this, cols, this, FXComboBox::ID_TEXT, FRAME_NONE, 0, 0, 0, 0, pl, pr, pt, pb);
    }
    // !!! End of hack

    if (options&COMBOBOX_STATIC)
    {
        field->setEditable(false);
    }

    pane = new FXPopup(this, FRAME_LINE);
    list = new FXList(pane, this, FXComboBox::ID_LIST, LIST_BROWSESELECT|LIST_AUTOSELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y|SCROLLERS_TRACK|HSCROLLER_NEVER);
    if (options&COMBOBOX_STATIC)
    {
        list->setScrollStyle(SCROLLERS_TRACK|HSCROLLING_OFF);
    }

    // Load the XBM icon
    //FXBitmap* comboarrowbmp = NULL;
    //comboarrowbmp = new FXXBMIcon(getApp(), comboarrow_bits, comboarrow_bits, FXRGB(255,255,255), NULL, comboarrow_width, comboarrow_height);
    //convertBlackPixelsToColor(comboarrow, FXRGB(255, 0, 0));

    // Modify the icon color (for example, change all black pixels to red)
    //for(int y = 0; y < comboarrow_height; ++y) {
    //    for(int x = 0; x < comboarrow_width; ++x) {
    //        if(comboarrow_bits[y * (comboarrow_width / 8) + (x / 8)] & (0x80 >> (x % 8))) {
    //            // This pixel is set, change its color
    //            comboarrow->setPixel(x, y, FXRGB(255, 0, 0)); // Set it to red
    //        }
    //    }
    //}

    button = new FXMenuButton(this, FXString::null, comboarrowicon, pane, FRAME_RAISED|FRAME_THICK|MENUBUTTON_DOWN|MENUBUTTON_ATTACH_RIGHT|LAYOUT_FIX_WIDTH, 0, 0, 1, 0, 0, 0, 0, 0);
    button->setXOffset(border);
    button->setYOffset(border);
    button->setBaseColor(getApp()->getHiliteColor());
    button->setHiliteColor(getApp()->getBaseColor());

    flags &= ~FLAG_UPDATE;  // Never GUI update
}

//
// Hack of FXMenuTitle
//

// This hack adds an optional gradient with rounded corner theme to the menu title (Clearlooks)

// Handle repaint
long FXMenuTitle::onPaint(FXObject*, FXSelector, void* ptr)
{
    // Initialize Clearlooks
    FXColor baseColor = backColor;
    INIT_CLEARLOOKS

    FXEvent*   ev = (FXEvent*)ptr;
    FXDCWindow dc(this, ev);
    FXint      xx, yy;

    dc.setFont(font);
    xx = 6;
    yy = 0;

    if (isEnabled())
    {
        if (isActive())
        {
            // Button with nice gradient effect and rounded corners (Clearlooks)
            if (use_clearlooks)
            {
                dc.setForeground(selbackColor);
                dc.fillRectangle(0, 0, width, height);
                dc.setForeground(backColor);
                dc.drawPoints(basebackground, 4);
                dc.setForeground(bordercolor);
                dc.drawRectangle(2, 0, width-5, 0);
                dc.drawRectangle(2, height-1, width-5, height-1);
                dc.drawRectangle(0, 2, 0, height-5);
                dc.drawRectangle(width-1, 2, 0, height-5);
                dc.drawPoints(bordercorners, 4);
                dc.setForeground(selbackColor);
                dc.drawPoints(bordershade, 16);
            }

            // Normal flat rectangular button
            else
            {
                dc.setForeground(selbackColor);
                dc.fillRectangle(1, 1, width-2, height-2);
                dc.setForeground(shadowColor);
                dc.fillRectangle(0, 0, width, 1);
                dc.fillRectangle(0, 0, 1, height);
                dc.setForeground(hiliteColor);
                dc.fillRectangle(0, height-1, width, 1);
                dc.fillRectangle(width-1, 0, 1, height);
            }
            xx++;
            yy++;
        }
        else if (underCursor())
        {
            // Button with nice gradient effect and rounded corners (Clearlooks)
            if (use_clearlooks)
            {
                DRAW_CLEARLOOKS_BUTTON_UP
            }

            // Normal flat rectangular button
            else
            {
                dc.setForeground(backColor);
                dc.fillRectangle(1, 1, width-2, height-2);
                dc.setForeground(shadowColor);
                dc.fillRectangle(0, height-1, width, 1);
                dc.fillRectangle(width-1, 0, 1, height);
                dc.setForeground(hiliteColor);
                dc.fillRectangle(0, 0, width, 1);
                dc.fillRectangle(0, 0, 1, height);
            }
        }

        else
        {
            dc.setForeground(backColor);
            dc.fillRectangle(0, 0, width, height);
        }

        if (icon)
        {
            dc.drawIcon(icon, xx, yy+(height-icon->getHeight())/2);
            xx += 5+icon->getWidth();
        }

        if (!label.empty())
        {
            yy += font->getFontAscent()+(height-font->getFontHeight())/2;
            dc.setForeground(isActive() ? seltextColor : textColor);
            dc.drawText(xx, yy, label);
            if (0 <= hotoff)
            {
                dc.fillRectangle(xx+font->getTextWidth(&label[0], hotoff), yy+1, font->getTextWidth(&label[hotoff], wclen(&label[hotoff])), 1);
            }
        }
    }

    else
    {
        dc.setForeground(backColor);
        dc.fillRectangle(0, 0, width, height);
        if (icon)
        {
            dc.drawIconSunken(icon, xx, yy+(height-icon->getHeight())/2);
            xx += 5+icon->getWidth();
        }
        if (!label.empty())
        {
            yy += font->getFontAscent()+(height-font->getFontHeight())/2;
            dc.setForeground(hiliteColor);
            dc.drawText(xx+1, yy+1, label);
            if (0 <= hotoff)
            {
                dc.fillRectangle(xx+font->getTextWidth(&label[0], hotoff), yy+1, font->getTextWidth(&label[hotoff], wclen(&label[hotoff])), 1);
            }
            dc.setForeground(shadowColor);
            dc.drawText(xx, yy, label);
            if (0 <= hotoff)
            {
                dc.fillRectangle(xx+font->getTextWidth(&label[0], hotoff), yy+1, font->getTextWidth(&label[hotoff], wclen(&label[hotoff])), 1);
            }
        }
    }

    return(1);
}


//
// Hack of FXRadioButton
//


// Handle repaint
long FXRadioButton::onPaint(FXObject*, FXSelector, void* ptr)
{
    // Initialize Clearlooks (don't use the macro because here it's different)
    static FXbool  init = true;
    static FXbool  use_clearlooks = true;
    static FXColor bordercolor;

    if (init)
    {
        use_clearlooks = getApp()->reg().readUnsignedEntry("SETTINGS", "use_clearlooks", true);

        if (use_clearlooks)
        {
            FXuint r = FXREDVAL(baseColor);
            FXuint g = FXGREENVAL(baseColor);
            FXuint b = FXBLUEVAL(baseColor);

            bordercolor = FXRGB(0.5*r, 0.5*g, 0.5*b);
        }
        init = false;
    }

    FXEvent*    ev = (FXEvent*)ptr;
    FXint       tw = 0, th = 0, tx, ty, ix, iy;
    FXRectangle recs[6];
    FXDCWindow  dc(this, ev);

    dc.setForeground(backColor);
    dc.fillRectangle(ev->rect.x, ev->rect.y, ev->rect.w, ev->rect.h);

    if (!label.empty())
    {
        tw = labelWidth(label);
        th = labelHeight(label);
    }

    just_x(tx, ix, tw, 13);
    just_y(ty, iy, th, 13);

    // Inside
    recs[0].x = ix+4;
    recs[0].y = iy+2;
    recs[0].w = 4;
    recs[0].h = 1;
    recs[1].x = ix+3;
    recs[1].y = iy+3;
    recs[1].w = 6;
    recs[1].h = 1;
    recs[2].x = ix+2;
    recs[2].y = iy+4;
    recs[2].w = 8;
    recs[2].h = 4;
    recs[3].x = ix+3;
    recs[3].y = iy+8;
    recs[3].w = 6;
    recs[3].h = 1;
    recs[4].x = ix+4;
    recs[4].y = iy+9;
    recs[4].w = 4;
    recs[4].h = 1;
    if (!isEnabled())                   // fix by Daniel Gehriger (gehriger@linkcad.com)
    {
        dc.setForeground(baseColor);
    }
    else
    {
        dc.setForeground(diskColor);
    }
    dc.fillRectangles(recs, 5);

    // Radio button with Clearlooks appearance
    if (use_clearlooks)
    {
        // Top left inside
        recs[0].x = ix+4;
        recs[0].y = iy+1;
        recs[0].w = 4;
        recs[0].h = 1;
        recs[1].x = ix+2;
        recs[1].y = iy+2;
        recs[1].w = 2;
        recs[1].h = 1;
        recs[2].x = ix+8;
        recs[2].y = iy+2;
        recs[2].w = 2;
        recs[2].h = 1;
        recs[3].x = ix+2;
        recs[3].y = iy+3;
        recs[3].w = 1;
        recs[3].h = 1;
        recs[4].x = ix+1;
        recs[4].y = iy+4;
        recs[4].w = 1;
        recs[4].h = 4;
        recs[5].x = ix+2;
        recs[5].y = iy+8;
        recs[5].w = 1;
        recs[5].h = 2;
        dc.setForeground(bordercolor);
        dc.fillRectangles(recs, 6);

        // Bottom right inside
        recs[0].x = ix+9;
        recs[0].y = iy+3;
        recs[0].w = 1;
        recs[0].h = 1;
        recs[1].x = ix+10;
        recs[1].y = iy+4;
        recs[1].w = 1;
        recs[1].h = 4;
        recs[2].x = ix+9;
        recs[2].y = iy+8;
        recs[2].w = 1;
        recs[2].h = 1;
        recs[3].x = ix+8;
        recs[3].y = iy+9;
        recs[3].w = 2;
        recs[3].h = 1;
        recs[4].x = ix+3;
        recs[4].y = iy+9;
        recs[4].w = 1;
        recs[4].h = 1;
        recs[5].x = ix+4;
        recs[5].y = iy+10;
        recs[5].w = 4;
        recs[5].h = 1;
        dc.setForeground(bordercolor);
        dc.fillRectangles(recs, 6);
    }

    // Standard radio button
    else
    {
        // Top left outside
        recs[0].x = ix+4;
        recs[0].y = iy+0;
        recs[0].w = 4;
        recs[0].h = 1;
        recs[1].x = ix+2;
        recs[1].y = iy+1;
        recs[1].w = 2;
        recs[1].h = 1;
        recs[2].x = ix+8;
        recs[2].y = iy+1;
        recs[2].w = 2;
        recs[2].h = 1;
        recs[3].x = ix+1;
        recs[3].y = iy+2;
        recs[3].w = 1;
        recs[3].h = 2;
        recs[4].x = ix+0;
        recs[4].y = iy+4;
        recs[4].w = 1;
        recs[4].h = 4;
        recs[5].x = ix+1;
        recs[5].y = iy+8;
        recs[5].w = 1;
        recs[5].h = 2;
        dc.setForeground(shadowColor);
        dc.fillRectangles(recs, 6);

        // Top left inside
        recs[0].x = ix+4;
        recs[0].y = iy+1;
        recs[0].w = 4;
        recs[0].h = 1;
        recs[1].x = ix+2;
        recs[1].y = iy+2;
        recs[1].w = 2;
        recs[1].h = 1;
        recs[2].x = ix+8;
        recs[2].y = iy+2;
        recs[2].w = 2;
        recs[2].h = 1;
        recs[3].x = ix+2;
        recs[3].y = iy+3;
        recs[3].w = 1;
        recs[3].h = 1;
        recs[4].x = ix+1;
        recs[4].y = iy+4;
        recs[4].w = 1;
        recs[4].h = 4;
        recs[5].x = ix+2;
        recs[5].y = iy+8;
        recs[5].w = 1;
        recs[5].h = 2;
        dc.setForeground(borderColor);
        dc.fillRectangles(recs, 6);

        // Bottom right outside
        recs[0].x = ix+10;
        recs[0].y = iy+2;
        recs[0].w = 1;
        recs[0].h = 2;
        recs[1].x = ix+11;
        recs[1].y = iy+4;
        recs[1].w = 1;
        recs[1].h = 4;
        recs[2].x = ix+10;
        recs[2].y = iy+8;
        recs[2].w = 1;
        recs[2].h = 2;
        recs[3].x = ix+8;
        recs[3].y = iy+10;
        recs[3].w = 2;
        recs[3].h = 1;
        recs[4].x = ix+2;
        recs[4].y = iy+10;
        recs[4].w = 2;
        recs[4].h = 1;
        recs[5].x = ix+4;
        recs[5].y = iy+11;
        recs[5].w = 4;
        recs[5].h = 1;
        dc.setForeground(hiliteColor);
        dc.fillRectangles(recs, 6);

        // Bottom right inside
        recs[0].x = ix+9;
        recs[0].y = iy+3;
        recs[0].w = 1;
        recs[0].h = 1;
        recs[1].x = ix+10;
        recs[1].y = iy+4;
        recs[1].w = 1;
        recs[1].h = 4;
        recs[2].x = ix+9;
        recs[2].y = iy+8;
        recs[2].w = 1;
        recs[2].h = 1;
        recs[3].x = ix+8;
        recs[3].y = iy+9;
        recs[3].w = 2;
        recs[3].h = 1;
        recs[4].x = ix+3;
        recs[4].y = iy+9;
        recs[4].w = 1;
        recs[4].h = 1;
        recs[5].x = ix+4;
        recs[5].y = iy+10;
        recs[5].w = 4;
        recs[5].h = 1;
        dc.setForeground(baseColor);
        dc.fillRectangles(recs, 6);
    }

    // Ball inside
    if (check != false)
    {
        recs[0].x = ix+5;
        recs[0].y = iy+4;
        recs[0].w = 2;
        recs[0].h = 1;
        recs[1].x = ix+4;
        recs[1].y = iy+5;
        recs[1].w = 4;
        recs[1].h = 2;
        recs[2].x = ix+5;
        recs[2].y = iy+7;
        recs[2].w = 2;
        recs[2].h = 1;
        if (isEnabled())
        {
            dc.setForeground(radioColor);
        }
        else
        {
            dc.setForeground(shadowColor);
        }
        dc.fillRectangles(recs, 3);
    }

    // Label
    if (!label.empty())
    {
        dc.setFont(font);
        if (isEnabled())
        {
            dc.setForeground(textColor);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
            if (hasFocus())
            {
                dc.drawFocusRectangle(tx-1, ty-1, tw+2, th+2);
            }
        }
        else
        {
            dc.setForeground(hiliteColor);
            drawLabel(dc, label, hotoff, tx+1, ty+1, tw, th);
            dc.setForeground(shadowColor);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
        }
    }
    drawFrame(dc, 0, 0, width, height);
    return(1);
}


//
// Hack of FXMenuButton
//



// Handle repaint
long FXMenuButton::onPaint(FXObject*, FXSelector, void* ptr)
{
    // Initialize Clearlooks
    INIT_CLEARLOOKS

    int        tw = 0, th = 0, iw = 0, ih = 0, tx, ty, ix, iy;
    FXEvent*   ev = (FXEvent*)ptr;
    FXPoint    points[3];
    FXDCWindow dc(this, ev);

    // Button with nice gradient effect and rounded corners (Clearlooks)
    if (use_clearlooks)
    {
        // Toolbar style
        if (options&MENUBUTTON_TOOLBAR)
        {
            // Enabled and cursor inside, and not popped up
            if (isEnabled() && underCursor() && !state)
            {
                DRAW_CLEARLOOKS_BUTTON_DOWN
            }

            // Enabled and popped up
            else if (isEnabled() && state)
            {
                DRAW_CLEARLOOKS_BUTTON_UP
            }

            // Disabled or unchecked or not under cursor
            else
            {
                dc.setForeground(backColor);
                dc.fillRectangle(0, 0, width, height);
            }
        }

        // Normal style
        else
        {
            // Draw in up state if disabled or up
            if (!isEnabled() || !state)
            {
                DRAW_CLEARLOOKS_BUTTON_UP
            }

            // If enabled and either checked or pressed
            else
            {
                DRAW_CLEARLOOKS_BUTTON_DOWN
            }
        }
    }   // End of gradient painting


    // Normal flat rectangular button
    else
    {
        // Got a border at all?
        if (options&(FRAME_RAISED|FRAME_SUNKEN))
        {
            // Toolbar style
            if (options&MENUBUTTON_TOOLBAR)
            {
                // Enabled and cursor inside, and not popped up
                if (isEnabled() && underCursor() && !state)
                {
                    DRAW_STANDARD_BUTTON_DOWN
                }

                // Enabled and popped up
                else if (isEnabled() && state)
                {
                    DRAW_STANDARD_BUTTON_UP
                }

                // Disabled or unchecked or not under cursor
                else
                {
                    dc.setForeground(backColor);
                    dc.fillRectangle(0, 0, width, height);
                }
            }

            // Normal style
            else
            {
                // Draw in up state if disabled or up
                if (!isEnabled() || !state)
                {
                    DRAW_STANDARD_BUTTON_UP
                }

                // Draw sunken if enabled and either checked or pressed
                else
                {
                    DRAW_STANDARD_BUTTON_DOWN
                }
            }
        }

        // No borders
        else
        {
            if (isEnabled() && state)
            {
                dc.setForeground(hiliteColor);
                dc.fillRectangle(0, 0, width, height);
            }
            else
            {
                dc.setForeground(backColor);
                dc.fillRectangle(0, 0, width, height);
            }
        }
    }   // End of normal painting

    // Position text & icon
    if (!label.empty())
    {
        tw = labelWidth(label);
        th = labelHeight(label);
    }

    // Icon?
    if (icon)
    {
        iw = icon->getWidth();
        ih = icon->getHeight();
    }

    // Arrows?
    else if (!(options&MENUBUTTON_NOARROWS))
    {
        if (options&MENUBUTTON_LEFT)
        {
            ih = MENUBUTTONARROW_WIDTH;
            iw = MENUBUTTONARROW_HEIGHT;
        }
        else
        {
            iw = MENUBUTTONARROW_WIDTH;
            ih = MENUBUTTONARROW_HEIGHT;
        }
    }

    // Keep some room for the arrow!
    just_x(tx, ix, tw, iw);
    just_y(ty, iy, th, ih);

    // Move a bit when pressed
    if (state)
    {
        ++tx;
        ++ty;
        ++ix;
        ++iy;
    }

    // Draw icon
    if (icon)
    {
        if (isEnabled())
        {
            dc.drawIcon(icon, ix, iy);
        }
        else
        {
            dc.drawIconSunken(icon, ix, iy);
        }
    }

    // Draw arrows
    else if (!(options&MENUBUTTON_NOARROWS))
    {
        // Right arrow
        if ((options&MENUBUTTON_RIGHT) == MENUBUTTON_RIGHT)
        {
            if (isEnabled())
            {
                dc.setForeground(textColor);
            }
            else
            {
                dc.setForeground(shadowColor);
            }
            points[0].x = ix;
            points[0].y = iy;
            points[1].x = ix;
            points[1].y = iy+MENUBUTTONARROW_WIDTH-1;
            points[2].x = ix+MENUBUTTONARROW_HEIGHT;
            points[2].y = (short)(iy+(MENUBUTTONARROW_WIDTH>>1));
            dc.fillPolygon(points, 3);
        }

        // Left arrow
        else if (options&MENUBUTTON_LEFT)
        {
            if (isEnabled())
            {
                dc.setForeground(textColor);
            }
            else
            {
                dc.setForeground(shadowColor);
            }
            points[0].x = ix+MENUBUTTONARROW_HEIGHT;
            points[0].y = iy;
            points[1].x = ix+MENUBUTTONARROW_HEIGHT;
            points[1].y = iy+MENUBUTTONARROW_WIDTH-1;
            points[2].x = ix;
            points[2].y = (short)(iy+(MENUBUTTONARROW_WIDTH>>1));
            dc.fillPolygon(points, 3);
        }

        // Up arrow
        else if (options&MENUBUTTON_UP)
        {
            if (isEnabled())
            {
                dc.setForeground(textColor);
            }
            else
            {
                dc.setForeground(shadowColor);
            }
            points[0].x = (short)(ix+(MENUBUTTONARROW_WIDTH>>1));
            points[0].y = iy-1;
            points[1].x = ix;
            points[1].y = iy+MENUBUTTONARROW_HEIGHT;
            points[2].x = ix+MENUBUTTONARROW_WIDTH;
            points[2].y = iy+MENUBUTTONARROW_HEIGHT;
            dc.fillPolygon(points, 3);
        }

        // Down arrow
        else
        {
            if (isEnabled())
            {
                dc.setForeground(textColor);
            }
            else
            {
                dc.setForeground(shadowColor);
            }
            points[0].x = ix+1;
            points[0].y = iy;
            points[2].x = ix+MENUBUTTONARROW_WIDTH-1;
            points[2].y = iy;
            points[1].x = (short)(ix+(MENUBUTTONARROW_WIDTH>>1));
            points[1].y = iy+MENUBUTTONARROW_HEIGHT;
            dc.fillPolygon(points, 3);
        }
    }

    // Draw text
    if (!label.empty())
    {
        dc.setFont(font);
        if (isEnabled())
        {
            dc.setForeground(textColor);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
        }
        else
        {
            dc.setForeground(hiliteColor);
            drawLabel(dc, label, hotoff, tx+1, ty+1, tw, th);
            dc.setForeground(shadowColor);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
        }
    }

    // Draw focus
    if (hasFocus())
    {
        if (isEnabled())
        {
            dc.drawFocusRectangle(border+1, border+1, width-2*border-2, height-2*border-2);
        }
    }
    return(1);
}


//
// Hack of FXArrowButton
//


// Handle repaint
long FXArrowButton::onPaint(FXObject*, FXSelector, void* ptr)
{
    // Initialize Clearlooks
    INIT_CLEARLOOKS

    FXEvent*   ev = (FXEvent*)ptr;
    FXDCWindow dc(this, ev);
    FXPoint    points[3];
    int        xx, yy, ww, hh, q;

    // Button with nice gradient effect and rounded corners (Clearlooks)
    if (use_clearlooks)
    {
        // Toolbar style
        if (options&ARROW_TOOLBAR)
        {
            // Enabled and cursor inside, and up
            if (isEnabled() && underCursor() && !state)
            {
                DRAW_CLEARLOOKS_BUTTON_UP
            }

            // Enabled and cursor inside and down
            else if (isEnabled() && state)
            {
                DRAW_CLEARLOOKS_BUTTON_DOWN
            }

            // Disabled or unchecked or not under cursor
            else
            {
                dc.setForeground(backColor);
                dc.fillRectangle(0, 0, width, height);
            }
        }

        // Normal style
        else
        {
            // Draw sunken if enabled and pressed
            if (isEnabled() && state)
            {
                DRAW_CLEARLOOKS_BUTTON_DOWN
            }

            // Draw in up state if disabled or up
            else
            {
                DRAW_CLEARLOOKS_BUTTON_UP
            }
        }
    }   // End of gradient painting

    // Normal flat rectangular button
    else
    {
        // With borders
        if (options&(FRAME_RAISED|FRAME_SUNKEN))
        {
            // Toolbar style
            if (options&ARROW_TOOLBAR)
            {
                // Enabled and cursor inside, and up
                if (isEnabled() && underCursor() && !state)
                {
                    DRAW_STANDARD_BUTTON_UP
                }

                // Enabled and cursor inside and down
                else if (isEnabled() && state)
                {
                    DRAW_STANDARD_BUTTON_DOWN
                }

                // Disabled or unchecked or not under cursor
                else
                {
                    dc.setForeground(backColor);
                    dc.fillRectangle(0, 0, width, height);
                }
            }

            // Normal style
            else
            {
                // Draw sunken if enabled and pressed
                if (isEnabled() && state)
                {
                    DRAW_STANDARD_BUTTON_DOWN
                }

                // Draw in up state if disabled or up
                else
                {
                    DRAW_STANDARD_BUTTON_UP
                }
            }
        }

        // No borders
        else
        {
            if (isEnabled() && state)
            {
                dc.setForeground(hiliteColor);
                dc.fillRectangle(0, 0, width, height);
            }
            else
            {
                dc.setForeground(backColor);
                dc.fillRectangle(0, 0, width, height);
            }
        }
    }   // End of normal painting

    // Compute size of the arrows....
    ww = width-padleft-padright-(border<<1);
    hh = height-padtop-padbottom-(border<<1);
    if (options&(ARROW_UP|ARROW_DOWN))
    {
        q = ww|1;
        if (q > (hh<<1))
        {
            q = (hh<<1)-1;
        }
        ww = q;
        hh = q>>1;
    }
    else
    {
        q = hh|1;
        if (q > (ww<<1))
        {
            q = (ww<<1)-1;
        }
        ww = q>>1;
        hh = q;
    }

    if (options&JUSTIFY_LEFT)
    {
        xx = padleft+border;
    }
    else if (options&JUSTIFY_RIGHT)
    {
        xx = width-ww-padright-border;
    }
    else
    {
        xx = (width-ww)/2;
    }

    if (options&JUSTIFY_TOP)
    {
        yy = padtop+border;
    }
    else if (options&JUSTIFY_BOTTOM)
    {
        yy = height-hh-padbottom-border;
    }
    else
    {
        yy = (height-hh)/2;
    }

    if (state)
    {
        ++xx;
        ++yy;
    }

    if (isEnabled())
    {
        dc.setForeground(arrowColor);
    }
    else
    {
        dc.setForeground(shadowColor);
    }

    // NB Size of arrow should stretch
    if (options&ARROW_UP)
    {
        points[0].x = xx+(ww>>1);
        points[0].y = yy-1;
        points[1].x = xx;
        points[1].y = yy+hh;
        points[2].x = xx+ww;
        points[2].y = yy+hh;
        dc.fillPolygon(points, 3);
    }
    else if (options&ARROW_DOWN)
    {
        points[0].x = xx+1;
        points[0].y = yy;
        points[1].x = xx+ww-1;
        points[1].y = yy;
        points[2].x = xx+(ww>>1);
        points[2].y = yy+hh;
        dc.fillPolygon(points, 3);
    }
    else if (options&ARROW_LEFT)
    {
        points[0].x = xx+ww;
        points[0].y = yy;
        points[1].x = xx+ww;
        points[1].y = yy+hh-1;
        points[2].x = xx;
        points[2].y = yy+(hh>>1);
        dc.fillPolygon(points, 3);
    }
    else if (options&ARROW_RIGHT)
    {
        points[0].x = xx;
        points[0].y = yy;
        points[1].x = xx;
        points[1].y = yy+hh-1;
        points[2].x = xx+ww;
        points[2].y = yy+(hh>>1);
        dc.fillPolygon(points, 3);
    }
    return(1);
}


//
// Hack of FXProgressBar
//

// Note : Not implemented for the dial and vertical progress bar!
//        This hacks assumes that border = 2


// Draw only the interior, i.e. the part that changes
void FXProgressBar::drawInterior(FXDCWindow& dc)
{
    static FXbool  init = true;
    static FXbool  use_clearlooks = true;
    static FXColor topcolor, bottomcolor, bordercolor;

    FXPoint bordercorners[4] = { FXPoint(1, 1), FXPoint(1, height-2), FXPoint(width-2, 1),      
                                 FXPoint(width-2, height-2) };                                  

    // Init Clearlooks (don't use the macro because here it's different)
    if (init)
    {
        use_clearlooks = getApp()->reg().readUnsignedEntry("SETTINGS", "use_clearlooks", true);

        if (use_clearlooks)
        {
            FXuint r = FXREDVAL(barColor);
            FXuint g = FXGREENVAL(barColor);
            FXuint b = FXBLUEVAL(barColor);

            topcolor = FXRGB(FXMIN(1.2*r, 255), FXMIN(1.2*g, 255), FXMIN(1.2*b, 255));
            bottomcolor = FXRGB(0.9*r, 0.9*g, 0.9*b);

            r = FXREDVAL(baseColor);
            g = FXGREENVAL(baseColor);
            b = FXBLUEVAL(baseColor);

            bordercolor = FXRGB(0.5*r, 0.5*g, 0.5*b);
        }
        init = false;
    }

    int  percent, barlength, barfilled, tx, ty, tw, th, n, d;
    char numtext[6];

    if (options&PROGRESSBAR_DIAL)
    {
        // If total is 0, it's 100%
        barfilled = 23040;
        percent = 100;
        if (total != 0)
        {
            barfilled = (FXuint)(((double)progress * (double)23040) / (double)total);
            percent = (FXuint)(((double)progress * 100.0) / (double)total);
        }

        tw = width-(border<<1)-padleft-padright;
        th = height-(border<<1)-padtop-padbottom;
        d = FXMIN(tw, th)-1;

        tx = border+padleft+((tw-d)/2);
        ty = border+padtop+((th-d)/2);

        if (barfilled != 23040)
        {
            dc.setForeground(barBGColor);
            dc.fillArc(tx, ty, d, d, 5760, 23040-barfilled);
        }
        if (barfilled != 0)
        {
            dc.setForeground(barColor);
            dc.fillArc(tx, ty, d, d, 5760, -barfilled);
        }

        // Draw outside circle
        dc.setForeground(borderColor);
        dc.drawArc(tx+1, ty, d, d, 90*64, 45*64);
        dc.drawArc(tx, ty+1, d, d, 135*64, 45*64);
        dc.setForeground(baseColor);
        dc.drawArc(tx-1, ty, d, d, 270*64, 45*64);
        dc.drawArc(tx, ty-1, d, d, 315*64, 45*64);

        dc.setForeground(shadowColor);
        dc.drawArc(tx, ty, d, d, 45*64, 180*64);
        dc.setForeground(hiliteColor);
        dc.drawArc(tx, ty, d, d, 225*64, 180*64);

        // Draw text
        if (options&PROGRESSBAR_PERCENTAGE)
        {
            dc.setFont(font);
            tw = font->getTextWidth("100%", 4);
            if (tw > (10*d)/16)
            {
                return;
            }
            th = font->getFontHeight();
            if (th > d/2)
            {
                return;
            }
            snprintf(numtext, sizeof(numtext)-1, "%d%%", percent);
            n = strlen(numtext);
            tw = font->getTextWidth(numtext, n);
            th = font->getFontHeight();
            tx = tx+d/2-tw/2;
            ty = ty+d/2+font->getFontAscent()+5;
            //dc.setForeground(textNumColor);
#ifdef HAVE_XFT_H
            dc.setForeground(barBGColor);             // Code for XFT until XFT can use BLT_SRC_XOR_DST
            dc.drawText(tx-1, ty, numtext, n);
            dc.drawText(tx+1, ty, numtext, n);
            dc.drawText(tx, ty-1, numtext, n);
            dc.drawText(tx, ty+1, numtext, n);
            dc.setForeground(textNumColor);
            dc.drawText(tx, ty, numtext, n);
#else
            dc.setForeground(FXRGB(255, 255, 255));     // Original code
            dc.setFunction(BLT_SRC_XOR_DST);
            dc.drawText(tx, ty, numtext, n);
#endif
        }
    }

    // Vertical bar
    else if (options&PROGRESSBAR_VERTICAL)
    {
        // If total is 0, it's 100%
        barlength = height-border-border;
        barfilled = barlength;
        percent = 100;
      
        if (total != 0)
        {
            barfilled = (FXuint)(((double)progress * (double)barlength) / (double)total);
            percent = (FXuint)(((double)progress * 100.0) / (double)total);
        }

        // Draw completed bar
        if (0 < barfilled)
        {
			dc.setForeground(barColor);
			dc.fillRectangle(border, height-border-barfilled, width-(border<<1), barfilled);
        }

        // Draw uncompleted bar
        if (barfilled < barlength)
        {
			dc.setForeground(barBGColor);
			dc.fillRectangle(border, border, width-(border<<1), barlength-barfilled);
        }

        // Draw text
        if (options&PROGRESSBAR_PERCENTAGE)
        {
            dc.setFont(font);
            snprintf(numtext, sizeof(numtext)-1, "%d%%", percent);
            n = strlen(numtext);
            tw = font->getTextWidth(numtext, n);
            th = font->getFontHeight();
            ty = (height-th)/2+font->getFontAscent();
            tx = (width-tw)/2;
            if (height-border-barfilled > ty)           // In upper side
            {
                dc.setForeground(textNumColor);
                dc.setClipRectangle(border, border, width-(border<<1), height-(border<<1));
                dc.drawText(tx, ty, numtext, n);
            }
            else if (ty-th > height-border-barfilled)   // In lower side
            {
                dc.setForeground(textAltColor);
                dc.setClipRectangle(border, border, width-(border<<1), height-(border<<1));
                dc.drawText(tx, ty, numtext, n);
            }
            else                                      // In between!
            {
                dc.setForeground(textAltColor);
                dc.setClipRectangle(border, height-border-barfilled, width-(border<<1), barfilled);
                dc.drawText(tx, ty, numtext, n);
                dc.setForeground(textNumColor);
                dc.setClipRectangle(border, border, width-(border<<1), barlength-barfilled);
                dc.drawText(tx, ty, numtext, n);
                dc.clearClipRectangle();
            }
        }
    }

    // Horizontal bar
    else
    {
        // If total is 0, it's 100%
        barlength = width-border-border;
        barfilled = barlength;
        percent = 100;
        if (total != 0)
        {
            barfilled = (FXuint)(((double)progress * (double)barlength) / (double)total);
            percent = (FXuint)(((double)progress * 100.0) / (double)total);
        }

        // Draw uncompleted bar
        if (barfilled < barlength)
        {
            // Clearlooks (gradient with rounded corners)
            if (use_clearlooks)
            {
	            dc.setForeground(barBGColor);
	            dc.fillRectangle(border+barfilled+(border>>1), border>>1, barlength-barfilled, height-border);
            }
            // Standard (flat)
            else
            {
	            dc.setForeground(barBGColor);
	            dc.fillRectangle(border+barfilled, border, barlength-barfilled, height-(border<<1));
            }
        }

        // Draw completed bar
        if (0 < barfilled)
        {
            // Clearlooks (gradient with rounded corners)
            if (use_clearlooks)
            {
                drawGradientRectangle(dc, topcolor, bottomcolor, border-1, border-1, barfilled+2, height-border, true);
				
				dc.setForeground(bordercolor);                                               
				dc.fillRectangle(barfilled+3, 2, 1, height-(border<<1));
				dc.drawPoints(bordercorners, 4);

    			FXPoint barcorners[2] = { FXPoint(barfilled+2, 1), FXPoint(barfilled+2, height-border) };                                  
				dc.drawPoints(barcorners, 2);
			}
            // Standard (flat)
            else
            {
                dc.setForeground(barColor);
                dc.fillRectangle(border, border, barfilled, height-(border<<1));
            }
        }
        // Draw text
        if (options&PROGRESSBAR_PERCENTAGE)
        {
            dc.setFont(font);
            snprintf(numtext, sizeof(numtext)-1, "%d%%", percent);
            n = strlen(numtext);
            tw = font->getTextWidth(numtext, n);
            th = font->getFontHeight();
            ty = (height-th)/2+font->getFontAscent();
            tx = (width-tw)/2;
            if (border+barfilled <= tx)           // In right side
            {
                dc.setForeground(textNumColor);
                dc.setClipRectangle(border, border, width-(border<<1), height-(border<<1));
                dc.drawText(tx, ty, numtext, n);
            }
            else if (tx+tw <= border+barfilled)   // In left side
            {
                dc.setForeground(textAltColor);
                dc.setClipRectangle(border, border, width-(border<<1), height-(border<<1));
                dc.drawText(tx, ty, numtext, n);
            }
            else                                // In between!
            {
                dc.setForeground(textAltColor);
                dc.setClipRectangle(border, border, barfilled, height);
                dc.drawText(tx, ty, numtext, n);
                dc.setForeground(textNumColor);
                dc.setClipRectangle(border+barfilled, border, barlength-barfilled, height);
                dc.drawText(tx, ty, numtext, n);
                dc.clearClipRectangle();
            }
        }
    }
}


// Draw the progress bar
long FXProgressBar::onPaint(FXObject*,FXSelector,void *ptr)
{
    // Initialize Clearlooks
    INIT_CLEARLOOKS

	FXEvent *event=(FXEvent*)ptr;
	FXDCWindow dc(this,event);

	// Draw borders if any
	drawFrame(dc,0,0,width,height);

	// Background
	dc.setForeground(getBaseColor());
	dc.fillRectangle(border,border,width-(border<<1),height-(border<<1));

    // !!! Hack to get an optional rounded rectangle shape
    // only if _TEXTFIELD_NOFRAME is not specified !!!
    if ( (!(options&_TEXTFIELD_NOFRAME))  & use_clearlooks )
    {
        // Outside Background
        dc.setForeground(baseColor);
        dc.fillRectangle(0, 0, width, height);
        dc.drawPoints(basebackground, 4);

        // Border
        dc.setForeground(bordercolor);
        dc.drawRectangle(2, 0, width-5, 0);
        dc.drawRectangle(2, height-1, width-5, height-1);
        dc.drawRectangle(0, 2, 0, height-5);
        dc.drawRectangle(width-1, 2, 0, height-5);
        dc.drawPoints(bordercorners, 4);
        dc.setForeground(shadecolor);
        dc.drawPoints(bordershade, 16);
        dc.setForeground(backColor);
        dc.fillRectangle(2, 1, width-4, height-2);
    }
    // !!! End of hack

	// Interior
	drawInterior(dc);
	return 1;
}


//
// Hack of FXPacker
//

// This hack optionally draws a rectangle with rounded corners (Clearlooks)
void FXPacker::drawGrooveRectangle(FXDCWindow& dc, FXint x, FXint y, FXint w, FXint h)
{
    static FXbool  init = true;
    static FXbool  use_clearlooks = true;
    static FXColor shadecolor, bordercolor;

    FXPoint bordershade[16] =
    {
        FXPoint(x, y+1), FXPoint(x+1, y), FXPoint(x+1, y+2), FXPoint(x+2, y+1),
        FXPoint(x+w-2, y), FXPoint(x+w-1, y+1), FXPoint(x+w-3, y+1),
        FXPoint(x+w-2, y+2), FXPoint(x, y+h-2), FXPoint(x+1, y+h-1),
        FXPoint(x+1, y+h-3), FXPoint(x+2, y+h-2),
        FXPoint(x+w-1, y+h-2), FXPoint(x+w-2, y+h-1),
        FXPoint(x+w-2, y+h-3), FXPoint(x+w-3, y+h-2)
    };
    FXPoint bordercorners[4] =
    {
        FXPoint(x+1, y+1), FXPoint(x+1, y+h-2), FXPoint(x+w-2, y+1),
        FXPoint(x+w-2, y+h-2)
    };

    if (init)
    {
        use_clearlooks = getApp()->reg().readUnsignedEntry("SETTINGS", "use_clearlooks", true);

        if (use_clearlooks)
        {
            FXuint r = FXREDVAL(backColor);
            FXuint g = FXGREENVAL(backColor);
            FXuint b = FXBLUEVAL(backColor);

            shadecolor = FXRGB(0.9*r, 0.9*g, 0.9*b);
            bordercolor = FXRGB(0.5*r, 0.5*g, 0.5*b);
            (void)bordercolor; // Hack to avoid unused variable compiler warning
        }
        init = false;
    }

    if ((0 < w) && (0 < h))
    {
        // Rectangle with rounded corners (Clearlooks)
        if (use_clearlooks)
        {
            // Draw the 4 edges
            dc.setForeground(shadowColor);
            dc.drawRectangle(x+w-1, y+2, 0, h-5); // right
            dc.drawRectangle(x, y+2, 0, h-5);     // left
            dc.drawRectangle(x+2, y, w-5, 0);     // up
            dc.drawRectangle(x+2, y+h-1, w-5, 0); // down

            // Draw the 4 rounded corners (with shade)
            dc.setForeground(shadowColor);
            dc.drawPoints(bordercorners, 4);
            dc.setForeground(shadecolor);
            dc.drawPoints(bordershade, 16);
        }

        // Standard rectangle
        else
        {
            dc.setForeground(shadowColor);
            dc.fillRectangle(x, y, w, 1);
            dc.fillRectangle(x, y, 1, h);
            dc.setForeground(hiliteColor);
            dc.fillRectangle(x, y+h-1, w, 1);
            dc.fillRectangle(x+w-1, y, 1, h);
            if ((1 < w) && (1 < h))
            {
                dc.setForeground(shadowColor);
                dc.fillRectangle(x+1, y+h-2, w-2, 1);
                dc.fillRectangle(x+w-2, y+1, 1, h-2);
                dc.setForeground(hiliteColor);
                dc.fillRectangle(x+1, y+1, w-3, 1);
                dc.fillRectangle(x+1, y+1, 1, h-3);
            }
        }
    }
}

//
// Hack of FXDragCorner
//

#define CORNERSIZE    13


long FXDragCorner::onPaint(FXObject*,FXSelector,void* ptr){
    FXEvent*   ev = (FXEvent*)ptr;
    FXDCWindow dc(this, ev);
/*  dc.setForeground(backColor);
  dc.fillRectangle(ev->rect.x,ev->rect.y,ev->rect.w,ev->rect.h);
  dc.setForeground(shadowColor);
  dc.drawLine(width-2,height-1,width,height-3);
  dc.drawLine(width-8,height-1,width,height-9);
  dc.drawLine(width-14,height-1,width,height-15);
  dc.setForeground(hiliteColor);
  dc.drawLine(width-5,height-1,width,height-6);
  dc.drawLine(width-11,height-1,width,height-12);
  dc.drawLine(width-17,height-1,width,height-18);*/
     dc.setForeground(backColor);
     dc.fillRectangle(ev->rect.x,ev->rect.y,ev->rect.w,ev->rect.h);
     dc.drawIcon(resizecorner, height-13, width-13);
  return 1;
}

//long FXDragCorner::onMap(FXObject*,FXSelector,void* ptr){


// Get default width
FXint FXDragCorner::getDefaultWidth(){
  return CORNERSIZE;
}


// Get default height
FXint FXDragCorner::getDefaultHeight(){
  return CORNERSIZE;
}


// Everything below including FXText and FXTextField is modified from FOX
// Draw the cursor
void FXText::drawCursor(FXuint state){
  register FXint xx,yt,yb,xlo,xhi,fh;
  if((state^flags)&FLAG_CARET){
    if(xid){
      FXASSERT(0<=cursorpos && cursorpos<=length);
      FXASSERT(0<=cursorrow && cursorrow<=nrows);
      if(toprow<=cursorrow && cursorrow<toprow+nvisrows){
        xx=pos_x+marginleft+barwidth+lineWidth(cursorstart,cursorpos-cursorstart)-1;
        if(barwidth<=xx+3 && xx-2<viewport_w){
          FXDCWindow dc(this);
          fh=font->getFontHeight();
          yt=pos_y+margintop+cursorrow*fh;
          yb=yt+fh-1;

          // Cursor can overhang margins but not line number bar
          dc.setClipRectangle(barwidth,0,viewport_w-barwidth,viewport_h);

          // Draw I beam
          if(state&FLAG_CARET){

            // Draw I-beam
            dc.setForeground(cursorColor);
            dc.fillRectangle(xx+2,yt+1,1,yb-yt-1);
            // dc.fillRectangle(xx-2,yt,6,1);
            // dc.fillRectangle(xx-2,yb,6,1);
            }

          // Erase I-beam
          else{

            // Erase I-beam, plus the text immediately surrounding it
            dc.setForeground(backColor);
            dc.fillRectangle(xx-2,yt,6,yb-yt+1);

            // Clip the text to the margins AND the rectangle that was
            // just erased.  We don't want to overdraw any existing
            // characters, because of ClearType.
            xlo=FXMAX(xx-2,marginleft+barwidth);
            xhi=FXMIN(xx+4,viewport_w-marginright);
            dc.setClipRectangle(xlo,margintop,xhi-xlo,viewport_h-margintop-marginbottom);

            // Restore text
            dc.setFont(font);
            drawTextRow(dc,cursorrow-toprow,xx-3,xx+4);
            }
          }
        }
      }
    flags^=FLAG_CARET;
    }
  }


void FXTextField::drawCursor(FXuint state){
  FXint cl,ch,xx,xlo,xhi;
  if((state^flags)&FLAG_CARET){
    if(xid){
      FXDCWindow dc(this);
      FXASSERT(0<=cursor && cursor<=contents.length());
      FXASSERT(0<=anchor && anchor<=contents.length());
      xx=coord(cursor)-1;

      // Clip rectangle around cursor
      xlo=FXMAX(xx-2,border);
      xhi=FXMIN(xx+3,width-border);

      // Cursor can overhang padding but not borders
      dc.setClipRectangle(xlo,border,xhi-xlo,height-(border<<1));

      // Draw I beam
      if(state&FLAG_CARET){

        // Draw I-beam
        dc.setForeground(cursorColor);
        dc.fillRectangle(xx+1,padtop+border+1,1,height-1-padbottom-padtop-(border<<1));
        //dc.fillRectangle(xx-2,padtop+border,5,1);
        //dc.fillRectangle(xx-2,height-border-padbottom-1,5,1);
        }

      // Erase I-beam
      else{

        // Erase I-beam, plus the text immediately surrounding it
        dc.setForeground(backColor);
        dc.fillRectangle(xx-2,border+1,5,height-1-(border<<1));

        // Draw two characters before and after cursor
        cl=ch=cursor;
        if(0<cl){
          cl=contents.dec(cl);
          if(0<cl){
            cl=contents.dec(cl);
            }
          }
        if(ch<contents.length()){
          ch=contents.inc(ch);
          if(ch<contents.length()){
            ch=contents.inc(ch);
            }
          }
        drawTextRange(dc,cl,ch);
        }
      }
    flags^=FLAG_CARET;
    }
  }

// Handle repaint
/* long FXTextField::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent *ev=(FXEvent*)ptr;
  FXDCWindow dc(this,ev);

  // Draw frame
  drawFrame(dc,0,0,width,height);

  // Gray background if disabled
  if(isEnabled())
    dc.setForeground(backColor);
  else
    dc.setForeground(baseColor);

  // Draw background
  dc.fillRectangle(border,border,width-(border<<1),height-(border<<1));

  // Draw text, clipped against frame interior
  dc.setClipRectangle(border,border,width-(border<<1),height-(border<<1));
  drawTextRange(dc,0,contents.length());

  // Draw caret
  if(flags&FLAG_CARET){
    int xx=coord(cursor)-1;
    dc.setForeground(cursorColor);
    dc.fillRectangle(xx+1,padtop+border,1,height-padbottom-padtop-(border<<1));
    //dc.fillRectangle(xx-2,padtop+border,5,1);
    //dc.fillRectangle(xx-2,height-border-padbottom-1,5,1);
    }
  return 1;
  } */
