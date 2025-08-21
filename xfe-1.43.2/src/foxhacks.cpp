// This file contains some FOX functions redefinitions (FOX hacks for various purposes, except for Clearlooks controls)

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#ifdef HAVE_XFT_H
#include <X11/Xft/Xft.h>
#endif

#ifdef HAVE_XRANDR_H
#include <X11/extensions/Xrandr.h>
#endif

extern FXbool   file_tooltips;
extern FXString xdgconfighome;
extern FXbool   xim_used;


// Hack to fix issues with drag and drop within, from and to the dirList
#define SELECT_MASK    (TREELIST_SINGLESELECT|TREELIST_BROWSESELECT)

// Remove all siblings from [fm,to]
void FXTreeList::removeItems(FXTreeItem* fm, FXTreeItem* to, FXbool notify)
{
    register FXTreeItem* olditem = currentitem;
    register FXTreeItem* prv;
    register FXTreeItem* nxt;
    register FXTreeItem* par;

    if (fm && to)
    {
        if (fm->parent != to->parent)
        {
            fxerror("%s::removeItems: arguments have different parent.\n", getClassName());
        }

        // Delete items
        while (1)
        {
            // Scan till end
            while (to->last)
            {
                to = to->last;
            }
            do
            {
                // Notify item will be deleted
                if (notify && target)
                {
                    target->tryHandle(this, FXSEL(SEL_DELETED, message), (void*)to);
                }

                // Remember hookups
                nxt = to->next;
                prv = to->prev;
                par = to->parent;

                // !!! Hack to go back to the parent when an item disappeared

                // Adjust pointers; suggested by Alan Ott <ott@acusoft.com>
                anchoritem = par;
                currentitem = par;
                extentitem = par;
                viewableitem = par;

                // !!! End of hack

                // Remove item from list
                if (prv)
                {
                    prv->next = nxt;
                }
                else if (par)
                {
                    par->first = nxt;
                }
                else
                {
                    firstitem = nxt;
                }
                if (nxt)
                {
                    nxt->prev = prv;
                }
                else if (par)
                {
                    par->last = prv;
                }
                else
                {
                    lastitem = prv;
                }

                // Delete it
                delete to;

                // Was last one?
                if (to == fm)
                {
                    goto x;
                }
                to = par;
            } while (!prv);
            to = prv;
        }

        // Current item has changed
x:
        if (olditem != currentitem)
        {
            if (notify && target)
            {
                target->tryHandle(this, FXSEL(SEL_CHANGED, message), (void*)currentitem);
            }
        }

        // Deleted current item
        if (currentitem && (currentitem != olditem))
        {
            if (hasFocus())
            {
                currentitem->setFocus(true);
            }
            if (((options&SELECT_MASK) == TREELIST_BROWSESELECT) && currentitem->isEnabled())
            {
                selectItem(currentitem, notify);
            }
        }

        // Redo layout
        recalc();
    }
}


// Hack to display a tooltip with name, size, date, etc.
// We were asked about tip text
long FXTreeList::onQueryTip(FXObject* sender, FXSelector sel, void* ptr)
{
    if (FXWindow::onQueryTip(sender, sel, ptr))
    {
        return(1);
    }

    // File tooltips are optional
    if (file_tooltips)
    {
        if ((flags&FLAG_TIP) && !(options&TREELIST_AUTOSELECT)) // No tip when autoselect!
        {
            int    x, y;
            FXuint buttons;

            getCursorPosition(x, y, buttons);
            DirItem* item = (DirItem*)getItemAt(x, y);
            if (item)
            {
                // !!! Hack to display a tooltip with name, size, date, etc.
                FXString string;

                // Root folder
                if (item->getText() == ROOTDIR)
                {
                    string = _("Root folder");
                }

                // Other folders
                else
                {
                    // Get tooltip data
                    FXString str = item->getTooltipData();
                    if (str == "")
                    {
                        return(0);
                    }

                    // Add name, type, permissions, etc. to the tool tip
                    FXString name = str.section('\t', 0);
                    FXString type = str.section('\t', 1);
                    FXString date = str.section('\t', 2);
                    FXString user = str.section('\t', 3);
                    FXString group = str.section('\t', 4);
                    FXString perms = str.section('\t', 5);
                    FXString deldate = str.section('\t', 6);
                    FXString pathname = str.section('\t', 7);

                    // Compute root file size
                    FXulong dnsize;
                    char    dsize[64];
                    dnsize = ::dirsize(pathname.text());
#if __WORDSIZE == 64
                    snprintf(dsize, sizeof(dsize)-1, "%lu", dnsize);
#else
                    snprintf(dsize, sizeof(dsize)-1, "%llu", dnsize);
#endif
                    FXString size = ::hSize(dsize);
                    if (deldate.empty())
                    {
                        string = _("Name: ")+name+"\n"+_("Size in root: ")+size+"\n"+_("Type: ")+type
                                 +"\n"+_("Modified date: ")+date+"\n"+_("User: ")+user+" - "+_("Group: ")+group
                                 +"\n"+_("Permissions: ")+perms;
                    }
                    else
                    {
                        string = _("Name: ")+name+"\n"+_("Size in root: ")+size+"\n"+_("Type: ")+type
                                 +"\n"+_("Modified date: ")+date+"\n"+_("Deletion date: ")+deldate+"\n"+_("User: ")+user+" - "+_("Group: ")+group
                                 +"\n"+_("Permissions: ")+perms;
                    }
                }
                // !!! End of hack !!!

                sender->handle(this, FXSEL(SEL_COMMAND, ID_SETSTRINGVALUE), (void*)&string);
                return(1);
            }
        }
    }
    return(0);
}


//
// Hack of FXDCWindow
//

#define DISPLAY(app)    ((Display*)((app)->display))
#define FS    ((XFontStruct*)(font->font))

#ifndef HAVE_XFT_H
static int utf2db(XChar2b* dst, const char* src, int n)
{
    register int     len, p;
    register FXwchar w;

    for (p = len = 0; p < n; p += wclen(src+p), len++)
    {
        w = wc(src+p);
        dst[len].byte1 = (w>>8);
        dst[len].byte2 = (w&255);
    }
    return(len);
}


#endif


// Hack to take into account non UTF-8 strings
void FXDCWindow::drawText(int x, int y, const char* string, FXuint length)
{
    if (!surface)
    {
        fprintf(stderr, "FXDCWindow::drawText: DC not connected to drawable.\n");
        exit(EXIT_FAILURE);
    }
    if (!font)
    {
        fprintf(stderr, "FXDCWindow::drawText: no font selected.\n");
        exit(EXIT_FAILURE);
    }
    if (!string)
    {
        fprintf(stderr, "FXDCWindow::drawText: NULL string argument.\n");
        exit(EXIT_FAILURE);
    }

#ifdef HAVE_XFT_H
    XftColor color;
    color.pixel = devfg;
    color.color.red = FXREDVAL(fg)*257;
    color.color.green = FXGREENVAL(fg)*257;
    color.color.blue = FXBLUEVAL(fg)*257;
    color.color.alpha = FXALPHAVAL(fg)*257;

    // !!! Hack to draw string depending on its encoding !!!
    if (isUtf8(string, length))
    {
        XftDrawStringUtf8((XftDraw*)xftDraw, &color, (XftFont*)font->font, x, y, (const FcChar8*)string, length);
    }
    else
    {
        XftDrawString8((XftDraw*)xftDraw, &color, (XftFont*)font->font, x, y, (const FcChar8*)string, length);
    }
    // !!! End of hack !!!
#else
    register int     count, escapement, defwidth, ww, size, i;
    register double  ang, ux, uy;
    register FXuchar r, c;
    XChar2b          sbuffer[4096];
    count = utf2db(sbuffer, string, FXMIN(length, 4096));
    if (font->getAngle())
    {
        ang = font->getAngle()*0.00027270769562411399179;
        defwidth = FS->min_bounds.width;
        ux = cos(ang);
        uy = sin(ang);
        if (FS->per_char)
        {
            r = FS->default_char>>8;
            c = FS->default_char&255;
            size = (FS->max_char_or_byte2-FS->min_char_or_byte2+1);
            if ((FS->min_char_or_byte2 <= c) && (c <= FS->max_char_or_byte2) && (FS->min_byte1 <= r) && (r <= FS->max_byte1))
            {
                defwidth = FS->per_char[(r-FS->min_byte1)*size+(c-FS->min_char_or_byte2)].width;
            }
            for (i = escapement = 0; i < count; i++)
            {
                XDrawString16(DISPLAY(getApp()), surface->id(), (GC)ctx, (int)(x+escapement*ux), (int)(y-escapement*uy), &sbuffer[i], 1);
                r = sbuffer[i].byte1;
                c = sbuffer[i].byte2;
                escapement += defwidth;
                if ((FS->min_char_or_byte2 <= c) && (c <= FS->max_char_or_byte2) && (FS->min_byte1 <= r) && (r <= FS->max_byte1))
                {
                    if ((ww = FS->per_char[(r-FS->min_byte1)*size+(c-FS->min_char_or_byte2)].width) != 0)
                    {
                        escapement += ww-defwidth;
                    }
                }
            }
        }
        else
        {
            for (i = escapement = 0; i < count; i++)
            {
                XDrawString16(DISPLAY(getApp()), surface->id(), (GC)ctx, (int)(x+escapement*ux), (int)(y-escapement*uy), &sbuffer[i], 1);
                escapement += defwidth;
            }
        }
    }
    else
    {
        XDrawString16(DISPLAY(getApp()), surface->id(), (GC)ctx, x, y, sbuffer, count);
    }
#endif
}


//
// Hack of FXFont
//

// Hack to take into account non UTF-8 strings
int FXFont::getTextWidth(const char* string, FXuint length) const
{
    if (!string)
    {
        fprintf(stderr, "FXDCWindow::drawText: NULL string argument.\n");
        exit(EXIT_FAILURE);
    }

    if (font)
    {
#ifdef HAVE_XFT_H
        XGlyphInfo extents;
        // This returns rotated metrics; FOX likes to work with unrotated metrics, so if angle
        // is not 0, we calculate the unrotated baseline; note however that the calculation is
        // not 100% pixel exact when the angle is not a multiple of 90 degrees.

        // !!! Hack to evaluate string extent depending on its encoding !!!
        if (isUtf8(string, length))
        {
            XftTextExtentsUtf8(DISPLAY(getApp()), (XftFont*)font, (const FcChar8*)string, length, &extents);
        }
        else
        {
            XftTextExtents8(DISPLAY(getApp()), (XftFont*)font, (const FcChar8*)string, length, &extents);
        }
        // !!! End of hack !!!

        if (angle)
        {
            return((int)(0.5+sqrt(extents.xOff*extents.xOff+extents.yOff*extents.yOff)));
        }

        return(extents.xOff);

#else
        register const XFontStruct* fs = (XFontStruct*)font;
        register int                defwidth = fs->min_bounds.width;
        register int                width = 0, ww;
        register FXuint             p = 0;
        register FXuint             s;
        register FXuchar            r;
        register FXuchar            c;
        register FXwchar            w;
        if (fs->per_char)
        {
            r = fs->default_char>>8;
            c = fs->default_char&255;
            s = (fs->max_char_or_byte2-fs->min_char_or_byte2+1);
            if ((fs->min_char_or_byte2 <= c) && (c <= fs->max_char_or_byte2) && (fs->min_byte1 <= r) && (r <= fs->max_byte1))
            {
                defwidth = fs->per_char[(r-fs->min_byte1)*s+(c-fs->min_char_or_byte2)].width;
            }
            while (p < length)
            {
                w = wc(string+p);
                p += wclen(string+p);
                r = w>>8;
                c = w&255;
                if ((fs->min_char_or_byte2 <= c) && (c <= fs->max_char_or_byte2) && (fs->min_byte1 <= r) && (r <= fs->max_byte1))
                {
                    if ((ww = fs->per_char[(r-fs->min_byte1)*s+(c-fs->min_char_or_byte2)].width) != 0)
                    {
                        width += ww;
                        continue;
                    }
                }
                width += defwidth;
            }
        }
        else
        {
            while (p < length)
            {
                p += wclen(string+p);
                width += defwidth;
            }
        }
        return(width);
#endif
    }
    return(length);
}


//
// Hack of FXSplitter
//
// NB : - MIN_PANEL_WIDTH is defined in xfedefs.h
//      - Don't use LAYOUT_FIX_WIDTH with this hack because it won't work!
// This function is taken from the FXSplitter class
// and hacked to set a minimum splitter width when moving splitter to right
// It replaces the normal function...
void FXSplitter::moveHSplit(int pos)
{
    register int smin, smax;

    //register FXuint hints;
    FXASSERT(window);
    //hints=window->getLayoutHints();
    // !!! Hack to limit the width to a minimum value !!!
    if (options&SPLITTER_REVERSED)
    {
        smin = barsize;
        smax = window->getX()+window->getWidth();
    }
    else
    {
        smin = window->getX();
        smax = width-barsize;
    }
    smax = smax-MIN_PANEL_WIDTH;
    smin = smin+MIN_PANEL_WIDTH;
    split = pos;
    if (split < smin)
    {
        split = smin;
    }
    if (split > smax)
    {
        split = smax;
    }
    // !!! End of hack
}


void FXSplitter::moveVSplit(int pos)
{
    register int smin, smax;

    //register FXuint hints;
    FXASSERT(window);
    //hints=window->getLayoutHints();
    if (options&SPLITTER_REVERSED)
    {
        smin = barsize;
        smax = window->getY()+window->getHeight();
    }
    else
    {
        smin = window->getY();
        smax = height-barsize;
    }
    smax = smax-MIN_PANEL_WIDTH;
    smin = smin+MIN_PANEL_WIDTH;
    split = pos;
    if (split < smin)
    {
        split = smin;
    }
    if (split > smax)
    {
        split = smax;
    }
}


//
// Hack of FXRegistry
//

// Hack to change the defaults directories for config files and icons
// The vendor key is not used anymore

#define DESKTOP         "xferc"
#define REGISTRYPATH    "/etc:/usr/share:/usr/local/share"

// Read registry
bool FXRegistry::read()
{
    FXString      dirname;
    register bool ok = false;

    dirname = FXPath::search(REGISTRYPATH, "xfe");
    if (!dirname.empty())
    {
        ok = readFromDir(dirname, false);
    }

    // Try search along PATH if still not found
    if (!ok)
    {
        dirname = FXPath::search(FXSystem::getExecPath(), "xfe");
        if (!dirname.empty())
        {
            ok = readFromDir(dirname, false);
        }
    }

    // Get path to per-user settings directory
    dirname = xdgconfighome + PATHSEPSTRING XFECONFIGPATH;

    // Then read per-user settings; overriding system-wide ones
    if (readFromDir(dirname, true))
    {
        ok = true;
    }

    return(ok);
}


// Try read registry from directory
bool FXRegistry::readFromDir(const FXString& dirname, bool mark)
{
    bool ok = false;

    // Directory is empty?
    if (!dirname.empty())
    {
        // First try to load desktop registry
        if (parseFile(dirname+PATHSEPSTRING DESKTOP, false))
        {
            FXString nn = dirname+PATHSEPSTRING DESKTOP;
            ok = true;
        }

        // Have application key
        if (!applicationkey.empty())
        {
            if (parseFile(dirname+PATHSEPSTRING+applicationkey + "rc", mark))
            {
                ok = true;
            }
        }
    }
    return(ok);
}


// Write registry
bool FXRegistry::write()
{
    FXString pathname, tempname;

    // Settings have not changed
    if (!isModified())
    {
        return(true);
    }

    // We can not save if no application key given
    if (!applicationkey.empty())
    {
        // Changes written only in the per-user registry
        pathname = xdgconfighome + PATHSEPSTRING XFECONFIGPATH;

        // If this directory does not exist, make it
        if (!FXStat::exists(pathname))
        {
            if (!FXDir::create(pathname))
            {
                return(false);
            }
        }
        else
        {
            if (!FXStat::isDirectory(pathname))
            {
                return(false);
            }
        }

        // Add application key
        pathname.append(PATHSEPSTRING+applicationkey+"rc");

        // Construct temp name
        tempname.format("%s_%d", pathname.text(), fxgetpid());

        // Unparse settings into temp file first
        if (unparseFile(tempname))
        {
            // Rename ATOMICALLY to proper name
            if (!FXFile::rename(tempname, pathname))
            {
                return(false);
            }

            setModified(false);
            return(true);
        }
    }
    return(false);
}


//
// Hack of FXSettings
//

// Hack to allow writing any registry global registry key to user settings
// for the xfe application only


#define MAXVALUE    2000


// Write string
static bool writeString(FXFile& file, const FXchar* string)
{
    register FXint len = strlen(string);

    return(file.writeBlock(string, len) == len);
}


// Unparse registry file
bool FXSettings::unparseFile(const FXString& filename)
{
    // !!! Hack here !!!
    // Distinguish between xfe and other applications
    FXbool xfe = (filename.contains(DESKTOP) ? true : false);

    FXFile file(filename, FXIO::Writing);
    FXchar line[MAXVALUE];

    if (file.isOpen())
    {
        // Loop over all sections
        for (FXint s = first(); s < size(); s = next(s))
        {
            // Get group
            FXStringDict* group = data(s);
            bool          sec = false;

            // Loop over all entries
            for (FXint e = group->first(); e < group->size(); e = group->next(e))
            {
                // !!! Hack here !!!
                // Do this always for the xfe application
                // Is key-value pair marked?
                if (xfe | group->mark(e))
                {
                    // Write section name if not written yet
                    if (!sec)
                    {
                        if (!writeString(file, "["))
                        {
                            goto x;
                        }
                        if (!writeString(file, key(s)))
                        {
                            goto x;
                        }
                        if (!writeString(file, "]" ENDLINE))
                        {
                            goto x;
                        }
                        sec = true;
                    }

                    // Write marked key-value pairs only
                    if (!writeString(file, group->key(e)))
                    {
                        goto x;
                    }
                    if (!writeString(file, "="))
                    {
                        goto x;
                    }
                    if (!writeString(file, enquote(line, group->data(e))))
                    {
                        goto x;
                    }
                    if (!writeString(file, ENDLINE))
                    {
                        goto x;
                    }
                }
            }

            // Blank line after end
            if (sec)
            {
                if (!writeString(file, ENDLINE))
                {
                    goto x;
                }
            }
        }
        return(true);
    }
x:
    return(false);
}


//
// Hack of FXPopup
//

// The two functions below are taken from the FXPopup class
// and hacked to allow navigating using the keyboard on popup menus
// They replace the normal functions...

// !!! Global variable control keyboard scrolling on right click popup menus !!!
extern FXbool allowPopupScroll;

void FXPopup::setFocus()
{
    FXShell::setFocus();

    // !!! Hack to allow keyboard scroll on popup dialogs !!!
    if (allowPopupScroll)
    {
        grabKeyboard();
    }
}


void FXPopup::killFocus()
{
    FXShell::killFocus();

    // !!! Hack to allow keyboard scroll on popup dialogs !!!
    if (allowPopupScroll)
    {
        if (prevActive)
        {
            prevActive->setFocus();
        }
        else
        {
            ungrabKeyboard();
        }
    }
}


//
// Hack of FXStatusLine(translation hack)
//

// Status line construct and init
FXStatusLine::FXStatusLine(FXComposite* p, FXObject* tgt, FXSelector sel) :
    FXFrame(p, LAYOUT_LEFT|LAYOUT_FILL_Y|LAYOUT_FILL_X, 0, 0, 0, 0, 1, 1, 1, 1)
{
    flags |= FLAG_SHOWN;
    status = normal = _(" ");
    font = getApp()->getNormalFont();
    textColor = getApp()->getForeColor();
    textHighlightColor = getApp()->getForeColor();
    target = tgt;
    message = sel;
}


//
// Hack of FXReplaceDialog
//

// Taken from the FXReplaceDialog class
// - translation hack
// - small hack for the Clearlooks theme

// Padding for buttons
#define HORZ_PAD       12
#define VERT_PAD       2
#define SEARCH_MASK    (SEARCH_EXACT|SEARCH_IGNORECASE|SEARCH_REGEX)

// File Open Dialog
FXReplaceDialog::FXReplaceDialog(FXWindow* owner, const FXString& caption, FXIcon* ic, FXuint opts, int x, int y, int w, int h) :
    FXDialogBox(owner, caption, opts|DECOR_TITLE|DECOR_BORDER|DECOR_RESIZE, x, y, w, h, 10, 10, 10, 10, 10, 10)
{
    FXHorizontalFrame* buttons = new FXHorizontalFrame(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH|PACK_UNIFORM_HEIGHT, 0, 0, 0, 0, 0, 0, 0, 0);

    accept = new FXButton(buttons, _("&Replace"), NULL, this, ID_ACCEPT, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_Y|LAYOUT_RIGHT, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
    every = new FXButton(buttons, _("Re&place All"), NULL, this, ID_ALL, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_RIGHT, 0, 0, 0, 0, 6, 6, VERT_PAD, VERT_PAD);
    cancel = new FXButton(buttons, _("&Cancel"), NULL, this, ID_CANCEL, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_Y|LAYOUT_RIGHT, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
    FXHorizontalFrame* pair = new FXHorizontalFrame(buttons, LAYOUT_FILL_Y|LAYOUT_RIGHT, 0, 0, 0, 0, 0, 0, 0, 0);
    FXArrowButton*     searchlast = new FXArrowButton(pair, this, ID_PREV, ARROW_LEFT|FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_Y, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
    FXArrowButton*     searchnext = new FXArrowButton(pair, this, ID_NEXT, ARROW_RIGHT|FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_Y, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
    FXHorizontalFrame* toppart = new FXHorizontalFrame(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|LAYOUT_CENTER_Y, 0, 0, 0, 0, 0, 0, 0, 0, 10, 10);
    new FXLabel(toppart, FXString::null, ic, ICON_BEFORE_TEXT|JUSTIFY_CENTER_X|JUSTIFY_CENTER_Y|LAYOUT_FILL_Y|LAYOUT_FILL_X);
    FXVerticalFrame* entry = new FXVerticalFrame(toppart, LAYOUT_FILL_X|LAYOUT_CENTER_Y, 0, 0, 0, 0, 0, 0, 0, 0);
    searchlabel = new FXLabel(entry, _("Search for:"), NULL, JUSTIFY_LEFT|ICON_BEFORE_TEXT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X);

    // !!! Hack to remove the FRAME_THICK and FRAME_SUNKEN options (required for the Clearlooks theme)
    searchbox = new FXHorizontalFrame(entry, LAYOUT_FILL_X|LAYOUT_CENTER_Y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    searchtext = new FXTextField(searchbox, 26, this, ID_SEARCH_TEXT, FRAME_SUNKEN|TEXTFIELD_ENTER_ONLY|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 0, 0, 0, 4, 4, 4, 4);
    // !!! End of hack

    FXVerticalFrame* searcharrows = new FXVerticalFrame(searchbox, LAYOUT_RIGHT|LAYOUT_FILL_Y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    FXArrowButton*   ar1 = new FXArrowButton(searcharrows, this, ID_SEARCH_UP, FRAME_RAISED|FRAME_THICK|ARROW_UP|ARROW_REPEAT|LAYOUT_FILL_Y|LAYOUT_FIX_WIDTH, 0, 0, 16, 0, 1, 1, 1, 1);
    FXArrowButton*   ar2 = new FXArrowButton(searcharrows, this, ID_SEARCH_DN, FRAME_RAISED|FRAME_THICK|ARROW_DOWN|ARROW_REPEAT|LAYOUT_FILL_Y|LAYOUT_FIX_WIDTH, 0, 0, 16, 0, 1, 1, 1, 1);
    ar1->setArrowSize(3);
    ar2->setArrowSize(3);
    replacelabel = new FXLabel(entry, _("Replace with:"), NULL, LAYOUT_LEFT);

    // !!! Hack to remove the FRAME_THICK and FRAME_SUNKEN options (required for the Clearlooks theme)
    replacebox = new FXHorizontalFrame(entry, LAYOUT_FILL_X|LAYOUT_CENTER_Y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    replacetext = new FXTextField(replacebox, 26, this, ID_REPLACE_TEXT, FRAME_SUNKEN|TEXTFIELD_ENTER_ONLY|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 0, 0, 0, 4, 4, 4, 4);
    // !!! End of hack

    FXVerticalFrame* replacearrows = new FXVerticalFrame(replacebox, LAYOUT_RIGHT|LAYOUT_FILL_Y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    FXArrowButton*   ar3 = new FXArrowButton(replacearrows, this, ID_REPLACE_UP, FRAME_RAISED|FRAME_THICK|ARROW_UP|ARROW_REPEAT|LAYOUT_FILL_Y|LAYOUT_FIX_WIDTH, 0, 0, 16, 0, 1, 1, 1, 1);
    FXArrowButton*   ar4 = new FXArrowButton(replacearrows, this, ID_REPLACE_DN, FRAME_RAISED|FRAME_THICK|ARROW_DOWN|ARROW_REPEAT|LAYOUT_FILL_Y|LAYOUT_FIX_WIDTH, 0, 0, 16, 0, 1, 1, 1, 1);
    ar3->setArrowSize(3);
    ar4->setArrowSize(3);
    FXHorizontalFrame* options1 = new FXHorizontalFrame(entry, LAYOUT_FILL_X, 0, 0, 0, 0, 0, 0, 0, 0);
    new FXRadioButton(options1, _("Ex&act"), this, ID_MODE+SEARCH_EXACT, ICON_BEFORE_TEXT|LAYOUT_CENTER_X);
    new FXRadioButton(options1, _("&Ignore Case"), this, ID_MODE+SEARCH_IGNORECASE, ICON_BEFORE_TEXT|LAYOUT_CENTER_X);
    new FXRadioButton(options1, _("E&xpression"), this, ID_MODE+SEARCH_REGEX, ICON_BEFORE_TEXT|LAYOUT_CENTER_X);
    new FXCheckButton(options1, _("&Backward"), this, ID_DIR, ICON_BEFORE_TEXT|LAYOUT_CENTER_X);
    searchlast->setTipText("Ctrl-B");
    searchnext->setTipText("Ctrl-F");
    searchlast->addHotKey(MKUINT(KEY_b, CONTROLMASK));
    searchnext->addHotKey(MKUINT(KEY_f, CONTROLMASK));
    searchmode = SEARCH_EXACT|SEARCH_FORWARD;
    current = 0;
}


//
// Hack of FXSearchDialog (translation hack)
//

// Taken from the FXSearchDialog class
FXSearchDialog::FXSearchDialog(FXWindow* owner, const FXString& caption, FXIcon* ic, FXuint opts, int x, int y, int w, int h) :
    FXReplaceDialog(owner, caption, ic, opts, x, y, w, h)
{
    accept->setText(_("&Search"));
    every->hide();
    replacelabel->hide();
    replacebox->hide();
}


//
// Hack of FXInputDialog (translation hack)
//

// Taken from the FXInputDialog class
void FXInputDialog::initialize(const FXString& label, FXIcon* icon)
{
    FXuint             textopts = TEXTFIELD_ENTER_ONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X;
    FXHorizontalFrame* buttons = new FXHorizontalFrame(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH, 0, 0, 0, 0, 0, 0, 0, 0);

    new FXButton(buttons, _("&OK"), NULL, this, ID_ACCEPT, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_RIGHT, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
    new FXButton(buttons, _("&Cancel"), NULL, this, ID_CANCEL, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_RIGHT, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
    FXHorizontalFrame* toppart = new FXHorizontalFrame(this, LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_CENTER_Y, 0, 0, 0, 0, 0, 0, 0, 0, 10, 10);
    new FXLabel(toppart, FXString::null, icon, ICON_BEFORE_TEXT|JUSTIFY_CENTER_X|JUSTIFY_CENTER_Y|LAYOUT_FILL_Y|LAYOUT_FILL_X);
    FXVerticalFrame* entry = new FXVerticalFrame(toppart, LAYOUT_FILL_X|LAYOUT_CENTER_Y, 0, 0, 0, 0, 0, 0, 0, 0);
    new FXLabel(entry, label, NULL, JUSTIFY_LEFT|ICON_BEFORE_TEXT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X);
    if (options&INPUTDIALOG_PASSWORD)
    {
        textopts |= TEXTFIELD_PASSWD;
    }
    if (options&INPUTDIALOG_INTEGER)
    {
        textopts |= TEXTFIELD_INTEGER|JUSTIFY_RIGHT;
    }
    if (options&INPUTDIALOG_REAL)
    {
        textopts |= TEXTFIELD_REAL|JUSTIFY_RIGHT;
    }
    input = new FXTextField(entry, 20, this, ID_ACCEPT, textopts, 0, 0, 0, 0, 8, 8, 4, 4);
    limlo = 1.0;
    limhi = 0.0;
}


//
// Hack of fxpriv (clipboard management)
//


// These two functions are hacked to reduce the timeout when the owner app of the clipboard has been closed

// Send request for selection info
Atom fxsendrequest(Display* display, Window window, Atom selection, Atom prop, Atom type, FXuint time)
{
    // !!! Hack here to reduce timeout !!!
    FXuint loops = 10;
    XEvent ev;

    XConvertSelection(display, selection, type, prop, window, time);
    while (!XCheckTypedWindowEvent(display, window, SelectionNotify, &ev))
    {
        if (loops == 0)
        {
            //fxwarning("fxsendrequest:timed out!\n");
            return(None);
        }
        FXThread::sleep(10000000);  // Don't burn too much CPU here:- the other guy needs it more....
        loops--;
    }
    return(ev.xselection.property);
}


// Wait for event of certain type
static FXbool fxwaitforevent(Display* display, Window window, int type, XEvent& event)
{
    // !!! Hack here to reduce timeout !!!
    FXuint loops = 10;

    while (!XCheckTypedWindowEvent(display, window, type, &event))
    {
        if (loops == 0)
        {
            //fxwarning("fxwaitforevent:timed out!\n");
            return(false);
        }
        FXThread::sleep(10000000);  // Don't burn too much CPU here:- the other guy needs it more....
        loops--;
    }
    return(true);
}


// The four following functions are not modified but are necessary here because the previous ones are not called directly

// Read property in chunks smaller than maximum transfer length,
// appending to data array; returns amount read from the property.
static FXuint fxrecvprop(Display* display, Window window, Atom prop, Atom& type, FXuchar*& data, FXuint& size)
{
    unsigned long maxtfrsize = XMaxRequestSize(display)*4;
    unsigned long tfroffset, tfrsize, tfrleft;
    FXuchar*      ptr;
    int           format;

    tfroffset = 0;

    // Read next chunk of data from property
    while (XGetWindowProperty(display, window, prop, tfroffset>>2, maxtfrsize>>2, False, AnyPropertyType, &type, &format, &tfrsize, &tfrleft, &ptr) == Success && type != None)
    {
        tfrsize *= (format>>3);

        // Grow the array to accomodate new data
        if (!FXRESIZE(&data, FXuchar, size+tfrsize+1))
        {
            XFree(ptr);
            break;
        }

        // Append new data at the end, plus the extra 0.
        memcpy(&data[size], ptr, tfrsize+1);
        size += tfrsize;
        tfroffset += tfrsize;
        XFree(ptr);
        if (tfrleft == 0)
        {
            break;
        }
    }

    // Delete property after we're done
    XDeleteProperty(display, window, prop);
    XFlush(display);
    return(tfroffset);
}


// Receive data via property
Atom fxrecvdata(Display* display, Window window, Atom prop, Atom incr, Atom& type, FXuchar*& data, FXuint& size)
{
    unsigned long tfrsize, tfrleft;
    FXuchar*      ptr;
    XEvent        ev;
    int           format;

    data = NULL;
    size = 0;
    if (prop)
    {
        // First, see what we've got
        if ((XGetWindowProperty(display, window, prop, 0, 0, False, AnyPropertyType, &type, &format, &tfrsize, &tfrleft, &ptr) == Success) && (type != None))
        {
            XFree(ptr);

            // Incremental transfer
            if (type == incr)
            {
                // Delete the INCR property
                XDeleteProperty(display, window, prop);
                XFlush(display);

                // Wait for the next batch of data
                while (fxwaitforevent(display, window, PropertyNotify, ev))
                {
                    // Wrong type of notify event; perhaps stale event
                    if ((ev.xproperty.atom != prop) || (ev.xproperty.state != PropertyNewValue))
                    {
                        continue;
                    }

                    // See what we've got
                    if ((XGetWindowProperty(display, window, prop, 0, 0, False, AnyPropertyType, &type, &format, &tfrsize, &tfrleft, &ptr) == Success) && (type != None))
                    {
                        XFree(ptr);

                        // if empty property, its the last one
                        if (tfrleft == 0)
                        {
                            // Delete property so the other side knows we've got the data
                            XDeleteProperty(display, window, prop);
                            XFlush(display);
                            break;
                        }

                        // Read and delete the property
                        fxrecvprop(display, window, prop, type, data, size);
                    }
                }
            }

            // All data in one shot
            else
            {
                // Read and delete the property
                fxrecvprop(display, window, prop, type, data, size);
            }
        }
        return(prop);
    }
    return(None);
}


// Retrieve CLIPBOARD selection data
void FXApp::clipboardGetData(const FXWindow* window, FXDragType type, FXuchar*& data, FXuint& size)
{
    FXID answer;

    data = NULL;
    size = 0;
    if (clipboardWindow)
    {
        event.type = SEL_CLIPBOARD_REQUEST;
        event.target = type;
        ddeData = NULL;
        ddeSize = 0;
        clipboardWindow->handle(this, FXSEL(SEL_CLIPBOARD_REQUEST, 0), &event);
        data = ddeData;
        size = ddeSize;
        ddeData = NULL;
        ddeSize = 0;
    }
    else
    {
        answer = fxsendrequest((Display*)display, window->id(), xcbSelection, ddeAtom, type, event.time);
        fxrecvdata((Display*)display, window->id(), answer, ddeIncr, type, data, size);
    }
}


// Get dropped data; called in response to DND enter or DND drop
bool FXWindow::getDNDData(FXDNDOrigin origin, FXDragType targettype, FXuchar*& data, FXuint& size) const
{
    if (xid == 0)
    {
        fxerror("%s::getDNDData: window has not yet been created.\n", getClassName());
    }

    switch (origin)
    {
    case FROM_DRAGNDROP:
        getApp()->dragdropGetData(this, targettype, data, size);
        break;

    case FROM_CLIPBOARD:
        getApp()->clipboardGetData(this, targettype, data, size);
        break;

    case FROM_SELECTION:
        getApp()->selectionGetData(this, targettype, data, size);
        break;
    }
    return(data != NULL);
}


//
// Hack of FXWindow
//

// This hack fixes a bug in FOX that prevent any character to be entered
// when FOX is compiled with the --with-xim option
// The bug is fixed in FOX 1.6.35 and above
// However, the hack is still here because the latest FOX is not necessarily present
// on the user's Linux distribution

#include "FXComposeContext.h"

// Create compose context
void FXWindow::createComposeContext()
{
    if (!composeContext)
    {
        composeContext = new FXComposeContext(getApp(), this, 0);

        // !!! This line was missing !!!
        composeContext->create();
    }
}


//
// Hack of FXTextField
//

// This hack fixes a bug in FOX that make some input fields crash the application
// when FOX was compiled with the --with-xim option
// The bug is not fixed yet in FOX 1.6.36

// Into focus chain
void FXTextField::setFocus()
{
    FXFrame::setFocus();
    setDefault(true);
    flags &= ~FLAG_UPDATE;
    if (getApp()->hasInputMethod() && this->id())
    {
        createComposeContext();
    }
}


// This hack allows to ignore caps lock when using Ctrl-A, Ctrl-C, Ctrl-V and Ctrl-X shortcuts

// Pressed a key
long FXTextField::onKeyPress(FXObject*, FXSelector, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;

    flags &= ~FLAG_TIP;
    if (isEnabled())
    {
        if (target && target->tryHandle(this, FXSEL(SEL_KEYPRESS, message), ptr))
        {
            return(1);
        }
        flags &= ~FLAG_UPDATE;
        switch (event->code)
        {
        case KEY_Right:
        case KEY_KP_Right:
            if (!(event->state&SHIFTMASK))
            {
                handle(this, FXSEL(SEL_COMMAND, ID_DESELECT_ALL), NULL);
            }
            if (event->state&CONTROLMASK)
            {
                handle(this, FXSEL(SEL_COMMAND, ID_CURSOR_WORD_RIGHT), NULL);
            }
            else
            {
                handle(this, FXSEL(SEL_COMMAND, ID_CURSOR_RIGHT), NULL);
            }
            if (event->state&SHIFTMASK)
            {
                handle(this, FXSEL(SEL_COMMAND, ID_EXTEND), NULL);
            }
            else
            {
                handle(this, FXSEL(SEL_COMMAND, ID_MARK), NULL);
            }
            return(1);

        case KEY_Left:
        case KEY_KP_Left:
            if (!(event->state&SHIFTMASK))
            {
                handle(this, FXSEL(SEL_COMMAND, ID_DESELECT_ALL), NULL);
            }
            if (event->state&CONTROLMASK)
            {
                handle(this, FXSEL(SEL_COMMAND, ID_CURSOR_WORD_LEFT), NULL);
            }
            else
            {
                handle(this, FXSEL(SEL_COMMAND, ID_CURSOR_LEFT), NULL);
            }
            if (event->state&SHIFTMASK)
            {
                handle(this, FXSEL(SEL_COMMAND, ID_EXTEND), NULL);
            }
            else
            {
                handle(this, FXSEL(SEL_COMMAND, ID_MARK), NULL);
            }
            return(1);

        case KEY_Home:
        case KEY_KP_Home:
            if (!(event->state&SHIFTMASK))
            {
                handle(this, FXSEL(SEL_COMMAND, ID_DESELECT_ALL), NULL);
            }
            handle(this, FXSEL(SEL_COMMAND, ID_CURSOR_HOME), NULL);
            if (event->state&SHIFTMASK)
            {
                handle(this, FXSEL(SEL_COMMAND, ID_EXTEND), NULL);
            }
            else
            {
                handle(this, FXSEL(SEL_COMMAND, ID_MARK), NULL);
            }
            return(1);

        case KEY_End:
        case KEY_KP_End:
            if (!(event->state&SHIFTMASK))
            {
                handle(this, FXSEL(SEL_COMMAND, ID_DESELECT_ALL), NULL);
            }
            handle(this, FXSEL(SEL_COMMAND, ID_CURSOR_END), NULL);
            if (event->state&SHIFTMASK)
            {
                handle(this, FXSEL(SEL_COMMAND, ID_EXTEND), NULL);
            }
            else
            {
                handle(this, FXSEL(SEL_COMMAND, ID_MARK), NULL);
            }
            return(1);

        case KEY_Insert:
        case KEY_KP_Insert:
            if (event->state&CONTROLMASK)
            {
                handle(this, FXSEL(SEL_COMMAND, ID_COPY_SEL), NULL);
                return(1);
            }
            else if (event->state&SHIFTMASK)
            {
                handle(this, FXSEL(SEL_COMMAND, ID_PASTE_SEL), NULL);
                return(1);
            }
            else
            {
                handle(this, FXSEL(SEL_COMMAND, ID_TOGGLE_OVERSTRIKE), NULL);
            }
            return(1);

        case KEY_Delete:
        case KEY_KP_Delete:
            if (hasSelection())
            {
                if (event->state&SHIFTMASK)
                {
                    handle(this, FXSEL(SEL_COMMAND, ID_CUT_SEL), NULL);
                }
                else
                {
                    handle(this, FXSEL(SEL_COMMAND, ID_DELETE_SEL), NULL);
                }
            }
            else
            {
                handle(this, FXSEL(SEL_COMMAND, ID_DELETE), NULL);
            }
            return(1);

        case KEY_BackSpace:
            if (hasSelection())
            {
                handle(this, FXSEL(SEL_COMMAND, ID_DELETE_SEL), NULL);
            }
            else
            {
                handle(this, FXSEL(SEL_COMMAND, ID_BACKSPACE), NULL);
            }
            return(1);

        case KEY_Return:
        case KEY_KP_Enter:
            if (isEditable())
            {
                flags |= FLAG_UPDATE;
                flags &= ~FLAG_CHANGED;
                if (target)
                {
                    target->tryHandle(this, FXSEL(SEL_COMMAND, message), (void*)contents.text());
                }
            }
            else
            {
                getApp()->beep();
            }
            return(1);

        case KEY_a:
        case KEY_A: // Hack
            if (!(event->state&CONTROLMASK))
            {
                goto ins;
            }
            handle(this, FXSEL(SEL_COMMAND, ID_SELECT_ALL), NULL);
            return(1);

        case KEY_x:
        case KEY_X: // Hack
            if (!(event->state&CONTROLMASK))
            {
                goto ins;
            }

        case KEY_F20:                             // Sun Cut key
            handle(this, FXSEL(SEL_COMMAND, ID_CUT_SEL), NULL);
            return(1);

        case KEY_c:
        case KEY_C: // Hack
            if (!(event->state&CONTROLMASK))
            {
                goto ins;
            }

        case KEY_F16:                             // Sun Copy key
            handle(this, FXSEL(SEL_COMMAND, ID_COPY_SEL), NULL);
            return(1);

        case KEY_v:
        case KEY_V: // Hack
            if (!(event->state&CONTROLMASK))
            {
                goto ins;
            }

        case KEY_F18:                             // Sun Paste key
            handle(this, FXSEL(SEL_COMMAND, ID_PASTE_SEL), NULL);
            return(1);

        default:
ins:
            if ((event->state&(CONTROLMASK|ALTMASK)) || ((FXuchar)event->text[0] < 32))
            {
                return(0);
            }
            if (isOverstrike())
            {
                handle(this, FXSEL(SEL_COMMAND, ID_OVERST_STRING), (void*)event->text.text());
            }
            else
            {
                handle(this, FXSEL(SEL_COMMAND, ID_INSERT_STRING), (void*)event->text.text());
            }
            return(1);
        }
    }
    return(0);
}


//
// Hack of FXApp
//
// This hack fixes a bug in FOX that prevent to enter composed characters when the mouse pointer
// lies outside the text field. The implementation differs if iBus is running or not
// The bug is not fixed yet in FOX 1.6.50


namespace FX
{
// Callback Record
struct FXCBSpec
{
    FXObject*  target;                // Receiver object
    FXSelector message;               // Message sent to receiver
};


// Timer record
struct FXTimer
{
    FXTimer*   next;                  // Next timeout in list
    FXObject*  target;                // Receiver object
    void*      data;                  // User data
    FXSelector message;               // Message sent to receiver
    FXlong     due;                   // When timer is due (ns)
};


// Signal record
struct FXSignal
{
    FXObject*  target;                // Receiver object
    FXSelector message;               // Message sent to receiver
    FXbool     handlerset;            // Handler was already set
    FXbool     notified;              // Signal has fired
};


// Idle record
struct FXChore
{
    FXChore*   next;                  // Next chore in list
    FXObject*  target;                // Receiver object
    void*      data;                  // User data
    FXSelector message;               // Message sent to receiver
};


// Input record
struct FXInput
{
    FXCBSpec read;                    // Callback spec for read
    FXCBSpec write;                   // Callback spec for write
    FXCBSpec excpt;                   // Callback spec for except
};


// A repaint event record
struct FXRepaint
{
    FXRepaint*  next;               // Next repaint in list
    FXID        window;             // Window ID of the dirty window
    FXRectangle rect;               // Dirty rectangle
    int         hint;               // Hint for compositing
    FXbool      synth;              // Synthetic expose event or real one?
};


// Recursive Event Loop Invocation
struct FXInvocation
{
    FXInvocation** invocation; // Pointer to variable holding pointer to current invocation
    FXInvocation*  upper;      // Invocation above this one
    FXWindow*      window;     // Modal window (if any)
    FXModality     modality;   // Modality mode
    int            code;       // Return code
    FXbool         done;       // True if breaking out

    // Enter modal loop
    FXInvocation(FXInvocation** inv, FXModality mode, FXWindow* win) : invocation(inv), upper(*inv), window(win), modality(mode), code(0), done(false)
    {
        *invocation = this;
    }

    // Exit modal loop
    ~FXInvocation()
    {
        *invocation = upper;
    }
};
} // namespace FX


// Largest number of signals on this system
#define MAXSIGNALS    64

// Regular define
#define SELECT(n, r, w, e, t)    select(n, r, w, e, t)

// DND protocol version
#define XDND_PROTOCOL_VERSION    5


// Send types via property
Atom fxsendtypes(Display* display, Window window, Atom prop, FXDragType* types, FXuint numtypes)
{
    if (types && numtypes)
    {
        XChangeProperty(display, window, prop, XA_ATOM, 32, PropModeReplace, (unsigned char*)types, numtypes);
        return(prop);
    }
    return(None);
}


// Send data via property
Atom fxsenddata(Display* display, Window window, Atom prop, Atom type, FXuchar* data, FXuint size)
{
    unsigned long maxtfrsize, tfrsize, tfroffset;
    int           mode;

    if (data && size)
    {
        maxtfrsize = 4*XMaxRequestSize(display);
        mode = PropModeReplace;
        tfroffset = 0;
        while (size)
        {
            tfrsize = size;
            if (tfrsize > maxtfrsize)
            {
                tfrsize = maxtfrsize;
            }
            XChangeProperty(display, window, prop, type, 8, mode, &data[tfroffset], tfrsize);
            mode = PropModeAppend;
            tfroffset += tfrsize;
            size -= tfrsize;
        }
        return(prop);
    }
    return(None);
}


// Reply to request for selection info
Atom fxsendreply(Display* display, Window window, Atom selection, Atom prop, Atom target, FXuint time)
{
    XEvent se;

    se.xselection.type = SelectionNotify;
    se.xselection.send_event = TRUE;
    se.xselection.display = display;
    se.xselection.requestor = window;
    se.xselection.selection = selection;
    se.xselection.target = target;
    se.xselection.property = prop;
    se.xselection.time = time;
    XSendEvent(display, window, True, NoEventMask, &se);
    XFlush(display);
    return(prop);
}


// Read type list from property
Atom fxrecvtypes(Display* display, Window window, Atom prop, FXDragType*& types, FXuint& numtypes, FXbool del)
{
    unsigned long  numitems, bytesleft;
    unsigned char* ptr;
    int            actualformat;
    Atom           actualtype;

    types = NULL;
    numtypes = 0;
    if (prop)
    {
        if (XGetWindowProperty(display, window, prop, 0, 1024, del, XA_ATOM, &actualtype, &actualformat, &numitems, &bytesleft, &ptr) == Success)
        {
            if ((actualtype == XA_ATOM) && (actualformat == 32) && (numitems > 0))
            {
                if (FXMALLOC(&types, Atom, numitems))
                {
                    memcpy(types, ptr, sizeof(Atom)*numitems);
                    numtypes = numitems;
                }
            }
            XFree(ptr);
        }
        return(prop);
    }
    return(None);
}


// Get an event
bool FXApp::getNextEvent(FXRawEvent& ev, bool blocking)
{
    XEvent e;

    // Set to no-op just in case
    ev.xany.type = 0;

    // Handle all past due timers
    if (timers)
    {
        handleTimeouts();
    }

    // Check non-immediate signals that may have fired
    if (nsignals)
    {
        for (int sig = 0; sig < MAXSIGNALS; sig++)
        {
            if (signals[sig].notified)
            {
                signals[sig].notified = false;
                if (signals[sig].target && signals[sig].target->tryHandle(this, FXSEL(SEL_SIGNAL, signals[sig].message), (void*)(FXival)sig))
                {
                    refresh();
                    return(false);
                }
            }
        }
    }

    // Are there no events already queued up?
    if (!initialized || !XEventsQueued((Display*)display, QueuedAfterFlush))
    {
        struct timeval delta;
        fd_set         readfds;
        fd_set         writefds;
        fd_set         exceptfds;
        int            maxfds;
        int            nfds;

        // Prepare fd's to check
        maxfds = maxinput;
        readfds = *((fd_set*)r_fds);
        writefds = *((fd_set*)w_fds);
        exceptfds = *((fd_set*)e_fds);

        // Add connection to display if its open
        if (initialized)
        {
            FD_SET(ConnectionNumber((Display*)display), &readfds);
            if (ConnectionNumber((Display*)display) > maxfds)
            {
                maxfds = ConnectionNumber((Display*)display);
            }
        }

        delta.tv_usec = 0;
        delta.tv_sec = 0;

        // Do a quick poll for any ready events or inputs
        nfds = SELECT(maxfds+1, &readfds, &writefds, &exceptfds, &delta);

        // Nothing to do, so perform idle processing
        if (nfds == 0)
        {
            // Release the expose events
            if (repaints)
            {
                register FXRepaint* r = repaints;
                ev.xany.type = Expose;
                ev.xexpose.window = r->window;
                ev.xexpose.send_event = r->synth;
                ev.xexpose.x = r->rect.x;
                ev.xexpose.y = r->rect.y;
                ev.xexpose.width = r->rect.w-r->rect.x;
                ev.xexpose.height = r->rect.h-r->rect.y;
                repaints = r->next;
                r->next = repaintrecs;
                repaintrecs = r;
                return(true);
            }

            // Do our chores :-)
            if (chores)
            {
                register FXChore* c = chores;
                chores = c->next;
                if (c->target && c->target->tryHandle(this, FXSEL(SEL_CHORE, c->message), c->data))
                {
                    refresh();
                }
                c->next = chorerecs;
                chorerecs = c;
            }

            // GUI updating:- walk the whole widget tree.
            if (refresher)
            {
                refresher->handle(this, FXSEL(SEL_UPDATE, 0), NULL);
                if (refresher->getFirst())
                {
                    refresher = refresher->getFirst();
                }
                else
                {
                    while (refresher->getParent())
                    {
                        if (refresher->getNext())
                        {
                            refresher = refresher->getNext();
                            break;
                        }
                        refresher = refresher->getParent();
                    }
                }
                FXASSERT(refresher);
                if (refresher != refresherstop)
                {
                    return(false);
                }
                refresher = refresherstop = NULL;
            }

            // There are more chores to do
            if (chores)
            {
                return(false);
            }

            // We're not blocking
            if (!blocking)
            {
                return(false);
            }

            // Now, block till timeout, i/o, or event
            maxfds = maxinput;
            readfds = *((fd_set*)r_fds);
            writefds = *((fd_set*)w_fds);
            exceptfds = *((fd_set*)e_fds);

            // Add connection to display if its open
            if (initialized)
            {
                FD_SET(ConnectionNumber((Display*)display), &readfds);
                if (ConnectionNumber((Display*)display) > maxfds)
                {
                    maxfds = ConnectionNumber((Display*)display);
                }
            }

            // If there are timers, we block only for a little while.
            if (timers)
            {
                // All that testing above may have taken some time...
                FXlong interval = timers->due-FXThread::time();

                // Some timers are already due; do them right away!
                if (interval <= 0)
                {
                    return(false);
                }

                // Compute how long to wait
                delta.tv_usec = (interval/1000)%1000000;
                delta.tv_sec = interval/1000000000;

                // Exit critical section
                appMutex.unlock();

                // Block till timer or event or interrupt
                nfds = SELECT(maxfds+1, &readfds, &writefds, &exceptfds, &delta);

                // Enter critical section
                appMutex.lock();
            }

            // If no timers, we block till event or interrupt
            else
            {
                // Exit critical section
                appMutex.unlock();

                // Block until something happens
                nfds = SELECT(maxfds+1, &readfds, &writefds, &exceptfds, NULL);

                // Enter critical section
                appMutex.lock();
            }
        }

        // Timed out or interrupted
        if (nfds <= 0)
        {
            if ((nfds < 0) && (errno != EAGAIN) && (errno != EINTR))
            {
                fxerror("Application terminated: interrupt or lost connection errno=%d\n", errno);
            }
            return(false);
        }

        // Any other file descriptors set?
        if (0 <= maxinput)
        {
            // Examine I/O file descriptors
            for (FXInputHandle fff = 0; fff <= maxinput; fff++)
            {
                // Copy the record as the callbacks may try to change things
                FXInput in = inputs[fff];

                // Skip the display connection, which is treated differently
                if (initialized && (fff == ConnectionNumber((Display*)display)))
                {
                    continue;
                }

                // Check file descriptors
                if (FD_ISSET(fff, &readfds))
                {
                    if (in.read.target && in.read.target->tryHandle(this, FXSEL(SEL_IO_READ, in.read.message), (void*)(FXival)fff))
                    {
                        refresh();
                    }
                }
                if (FD_ISSET(fff, &writefds))
                {
                    if (in.write.target && in.write.target->tryHandle(this, FXSEL(SEL_IO_WRITE, in.write.message), (void*)(FXival)fff))
                    {
                        refresh();
                    }
                }
                if (FD_ISSET(fff, &exceptfds))
                {
                    if (in.excpt.target && in.excpt.target->tryHandle(this, FXSEL(SEL_IO_EXCEPT, in.excpt.message), (void*)(FXival)fff))
                    {
                        refresh();
                    }
                }
            }
        }

        // If there is no event, we're done
        if (!initialized || !FD_ISSET(ConnectionNumber((Display*)display), &readfds) || !XEventsQueued((Display*)display, QueuedAfterReading))
        {
            return(false);
        }
    }

    // Get an event
    XNextEvent((Display*)display, &ev);

    // !!! Hack to fix the bug with composed characters !!!
    if (xim_used)
    {
        // Filter event through input method context, if any
        if (xim && XFilterEvent(&ev, None))
        {
            return(false);
        }
    }
    else
    {
        FXWindow* focuswin;
        focuswin = getFocusWindow();
        if (xim && focuswin && XFilterEvent(&ev, (Window)focuswin->id()))
        {
            return(false);
        }
    }
    // !!! End of hack !!!

    // Save expose events for later...
    if ((ev.xany.type == Expose) || (ev.xany.type == GraphicsExpose))
    {
        addRepaint((FXID)ev.xexpose.window, ev.xexpose.x, ev.xexpose.y, ev.xexpose.width, ev.xexpose.height, 0);
        return(false);
    }

    // Compress motion events
    if (ev.xany.type == MotionNotify)
    {
        while (XPending((Display*)display))
        {
            XPeekEvent((Display*)display, &e);
            if ((e.xany.type != MotionNotify) || (ev.xmotion.window != e.xmotion.window) || (ev.xmotion.state != e.xmotion.state))
            {
                break;
            }
            XNextEvent((Display*)display, &ev);
        }
    }

    // Compress wheel events
    else if ((ev.xany.type == ButtonPress) && ((ev.xbutton.button == Button4) || (ev.xbutton.button == Button5)))
    {
        int ticks = 1;
        while (XPending((Display*)display))
        {
            XPeekEvent((Display*)display, &e);
            if (((e.xany.type != ButtonPress) && (e.xany.type != ButtonRelease)) || (ev.xany.window != e.xany.window) || (ev.xbutton.button != e.xbutton.button))
            {
                break;
            }
            ticks += (e.xany.type == ButtonPress);
            XNextEvent((Display*)display, &ev);
        }
        ev.xbutton.subwindow = (Window)ticks;   // Stick it here for later
    }

    // Compress configure events
    else if (ev.xany.type == ConfigureNotify)
    {
        while (XCheckTypedWindowEvent((Display*)display, ev.xconfigure.window, ConfigureNotify, &e))
        {
            ev.xconfigure.width = e.xconfigure.width;
            ev.xconfigure.height = e.xconfigure.height;
            if (e.xconfigure.send_event)
            {
                ev.xconfigure.x = e.xconfigure.x;
                ev.xconfigure.y = e.xconfigure.y;
            }
        }
    }

    // Regular event
    return(true);
}


// Translate key code to utf8 text
FXString translateKeyEvent(FXRawEvent& event)
{
    char    buffer[40];
    KeySym  sym;
    FXwchar w;

    XLookupString(&event.xkey, buffer, sizeof(buffer), &sym, NULL);
    w = fxkeysym2ucs(sym);
    return(FXString(&w, 1));
}


// Get keysym; interprets the modifiers!
static FXuint keysym(FXRawEvent& event)
{
    KeySym sym = KEY_VoidSymbol;
    char   buffer[40];

    XLookupString(&event.xkey, buffer, sizeof(buffer), &sym, NULL);
    return(sym);
}


// Dispatch event to widget
bool FXApp::dispatchEvent(FXRawEvent& ev)
{
    FXWindow* window, *ancestor;
    FXint     tmp_x, tmp_y;
    Atom      answer;
    XEvent    se;
    Window    tmp;

    // Get window
    window = findWindowWithId(ev.xany.window);

    // Was one of our windows, so dispatch
    if (window)
    {
        switch (ev.xany.type)
        {
        // Repaint event
        case GraphicsExpose:
        case Expose:
            event.type = SEL_PAINT;
            event.rect.x = ev.xexpose.x;
            event.rect.y = ev.xexpose.y;
            event.rect.w = ev.xexpose.width;
            event.rect.h = ev.xexpose.height;
            event.synthetic = ev.xexpose.send_event;
            window->handle(this, FXSEL(SEL_PAINT, 0), &event);

        case NoExpose:
            return(true);

        // Keymap Notify
        case KeymapNotify:
            return(true);

        // Keyboard
        case KeyPress:

            // !!! Hack to fix the bug with composed characters !!!
            FXWindow* focuswin;
            focuswin = getFocusWindow();

            if ((ev.xkey.keycode != 0) && focuswin && focuswin->getComposeContext())
            {
                Window w;
                XGetICValues((XIC)focuswin->getComposeContext()->id(), XNFocusWindow, &w, NULL);

                // Mouse pointer is not over the text field
                if (!focuswin->underCursor())
                {
                    if ((focuswin->id() != w) && XFilterEvent(&ev, (Window)focuswin->id()))
                    {
                        return(true);
                    }
                }
            }
        // !!! End of hack !!!

        case KeyRelease:
            event.type = SEL_KEYPRESS+ev.xkey.type-KeyPress;
            event.time = ev.xkey.time;
            event.win_x = ev.xkey.x;
            event.win_y = ev.xkey.y;
            event.root_x = ev.xkey.x_root;
            event.root_y = ev.xkey.y_root;

            // Translate to keysym; must interpret modifiers!
            event.code = keysym(ev);

            // Translate to string on KeyPress
            if (ev.xkey.type == KeyPress)
            {
                if (getFocusWindow() && getFocusWindow()->getComposeContext())
                {
                    event.text = getFocusWindow()->getComposeContext()->translateEvent(ev);
                }
                else
                {
                    event.text = translateKeyEvent(ev);
                }
            }

            // Clear string on KeyRelease
            else
            {
                event.text.clear();
            }

            // Mouse buttons and modifiers but no wheel buttons
            event.state = ev.xkey.state&~(Button4Mask|Button5Mask);

            // Fix modifier state
            if (ev.xkey.type == KeyPress)
            {
                if (event.code == KEY_Shift_L)
                {
                    event.state |= SHIFTMASK;
                }
                else if (event.code == KEY_Shift_R)
                {
                    event.state |= SHIFTMASK;
                }
                else if (event.code == KEY_Control_L)
                {
                    event.state |= CONTROLMASK;
                }
                else if (event.code == KEY_Control_R)
                {
                    event.state |= CONTROLMASK;
                }
                else if (event.code == KEY_F13)
                {
                    event.state |= METAMASK;     // Key between Ctrl and Alt (on most keyboards)
                }
                else if (event.code == KEY_Alt_L)
                {
                    event.state |= ALTMASK;
                }
                else if (event.code == KEY_Alt_R)
                {
                    event.state |= ALTMASK;    // FIXME do we need ALTGR flag instead/in addition?
                }
                else if (event.code == KEY_Num_Lock)
                {
                    event.state |= NUMLOCKMASK;
                }
                else if (event.code == KEY_Caps_Lock)
                {
                    event.state |= CAPSLOCKMASK;
                }
                else if (event.code == KEY_Scroll_Lock)
                {
                    event.state |= SCROLLLOCKMASK;
                }
                else if (event.code == KEY_Super_L)
                {
                    event.state |= METAMASK;
                }
                else if (event.code == KEY_Super_R)
                {
                    event.state |= METAMASK;
                }
                else
                {
                    stickyMods = event.state&(SHIFTMASK|CONTROLMASK|METAMASK|ALTMASK);
                }
            }
            else
            {
                if (event.code == KEY_Shift_L)
                {
                    event.state &= ~SHIFTMASK;
                }
                else if (event.code == KEY_Shift_R)
                {
                    event.state &= ~SHIFTMASK;
                }
                else if (event.code == KEY_Control_L)
                {
                    event.state &= ~CONTROLMASK;
                }
                else if (event.code == KEY_Control_R)
                {
                    event.state &= ~CONTROLMASK;
                }
                else if (event.code == KEY_F13)
                {
                    event.state &= ~METAMASK;    // Key between Ctrl and Alt (on most keyboards)
                }
                else if (event.code == KEY_Alt_L)
                {
                    event.state &= ~ALTMASK;
                }
                else if (event.code == KEY_Alt_R)
                {
                    event.state &= ~ALTMASK;   // FIXME do we need ALTGR flag instead/in addition?
                }
                else if (event.code == KEY_Num_Lock)
                {
                    event.state &= ~NUMLOCKMASK;
                }
                else if (event.code == KEY_Caps_Lock)
                {
                    event.state &= ~CAPSLOCKMASK;
                }
                else if (event.code == KEY_Scroll_Lock)
                {
                    event.state &= ~SCROLLLOCKMASK;
                }
                else if (event.code == KEY_Super_L)
                {
                    event.state &= ~METAMASK;
                }
                else if (event.code == KEY_Super_R)
                {
                    event.state &= ~METAMASK;
                }
                else
                {
                    event.state |= stickyMods;
                    stickyMods = 0;
                }
            }

            // Keyboard grabbed by specific window
            if (keyboardGrabWindow)
            {
                if (keyboardGrabWindow->handle(this, FXSEL(event.type, 0), &event))
                {
                    refresh();
                }
                return(true);
            }

            // Remember window for later
            if (ev.xkey.type == KeyPress)
            {
                keyWindow = activeWindow;
            }

            // Dispatch to key window
            if (keyWindow)
            {
                // FIXME doesSaveUnder test should go away
                // Dispatch if not in a modal loop or in a modal loop for a window containing the focus window
                if (!invocation || (invocation->modality == MODAL_FOR_NONE) || (invocation->window && invocation->window->isOwnerOf(keyWindow)) || keyWindow->getShell()->doesSaveUnder())
                {
                    if (keyWindow->handle(this, FXSEL(event.type, 0), &event))
                    {
                        refresh();
                    }
                    return(TRUE);
                }

                // Beep if outside modal
                if (ev.xany.type == KeyPress)
                {
                    beep();
                }
            }
            return(true);

        // Motion
        case MotionNotify:
            event.type = SEL_MOTION;
            event.time = ev.xmotion.time;
            event.win_x = ev.xmotion.x;
            event.win_y = ev.xmotion.y;
            event.root_x = ev.xmotion.x_root;
            event.root_y = ev.xmotion.y_root;
            event.code = 0;

            // Mouse buttons and modifiers but no wheel buttons
            event.state = (ev.xmotion.state&~(Button4Mask|Button5Mask)) | stickyMods;

            // Moved more that delta
            if ((FXABS(event.root_x-event.rootclick_x) >= dragDelta) || (FXABS(event.root_y-event.rootclick_y) >= dragDelta))
            {
                event.moved = 1;
            }

            // Dispatch to grab window
            if (mouseGrabWindow)
            {
                window->translateCoordinatesTo(event.win_x, event.win_y, mouseGrabWindow, event.win_x, event.win_y);
                if (mouseGrabWindow->handle(this, FXSEL(SEL_MOTION, 0), &event))
                {
                    refresh();
                }
            }

            // FIXME doesSaveUnder test should go away
            // Dispatch if inside model window
            else if (!invocation || (invocation->modality == MODAL_FOR_NONE) || (invocation->window && invocation->window->isOwnerOf(window)) || window->getShell()->doesSaveUnder())
            {
                if (window->handle(this, FXSEL(SEL_MOTION, 0), &event))
                {
                    refresh();
                }
            }

            // Remember last mouse
            event.last_x = event.win_x;
            event.last_y = event.win_y;
            return(true);

        // Button
        case ButtonPress:
        case ButtonRelease:
            event.time = ev.xbutton.time;
            event.win_x = ev.xbutton.x;
            event.win_y = ev.xbutton.y;
            event.root_x = ev.xbutton.x_root;
            event.root_y = ev.xbutton.y_root;

            // Mouse buttons and modifiers but no wheel buttons
            event.state = (ev.xmotion.state&~(Button4Mask|Button5Mask)) | stickyMods;

            // Mouse Wheel
            if ((ev.xbutton.button == Button4) || (ev.xbutton.button == Button5))
            {
                event.type = SEL_MOUSEWHEEL;
                event.code = ((ev.xbutton.button == Button4) ? 120 : -120)*ev.xbutton.subwindow;

                // Dispatch to grab window
                if (mouseGrabWindow)
                {
                    window->translateCoordinatesTo(event.win_x, event.win_y, mouseGrabWindow, event.win_x, event.win_y);
                    if (mouseGrabWindow->handle(this, FXSEL(SEL_MOUSEWHEEL, 0), &event))
                    {
                        refresh();
                    }
                    return(true);
                }

                // Dispatch to window under cursor
                // FIXME doesSaveUnder test should go away
                while (window && (!invocation || invocation->modality == MODAL_FOR_NONE || (invocation->window && invocation->window->isOwnerOf(window)) || window->getShell()->doesSaveUnder()))
                {
                    if (window->handle(this, FXSEL(SEL_MOUSEWHEEL, 0), &event))
                    {
                        refresh();
                        break;
                    }
                    window = window->getParent();
                }
                return(true);
            }

            // Mouse Button
            event.code = ev.xbutton.button;
            if (ev.xbutton.type == ButtonPress)                                // Mouse button press
            {
                if (ev.xbutton.button == Button1)
                {
                    event.type = SEL_LEFTBUTTONPRESS;
                    event.state |= LEFTBUTTONMASK;
                }
                if (ev.xbutton.button == Button2)
                {
                    event.type = SEL_MIDDLEBUTTONPRESS;
                    event.state |= MIDDLEBUTTONMASK;
                }
                if (ev.xbutton.button == Button3)
                {
                    event.type = SEL_RIGHTBUTTONPRESS;
                    event.state |= RIGHTBUTTONMASK;
                }
                if (!event.moved && (event.time-event.click_time < clickSpeed) && (event.code == (FXint)event.click_button))
                {
                    event.click_count++;
                    event.click_time = event.time;
                }
                else
                {
                    event.click_count = 1;
                    event.click_x = event.win_x;
                    event.click_y = event.win_y;
                    event.rootclick_x = event.root_x;
                    event.rootclick_y = event.root_y;
                    event.click_button = event.code;
                    event.click_time = event.time;
                }
                if (!(ev.xbutton.state&(Button1Mask|Button2Mask|Button3Mask)))
                {
                    event.moved = 0;
                }
            }
            else                                                            // Mouse button release
            {
                if (ev.xbutton.button == Button1)
                {
                    event.type = SEL_LEFTBUTTONRELEASE;
                    event.state &= ~LEFTBUTTONMASK;
                }
                if (ev.xbutton.button == Button2)
                {
                    event.type = SEL_MIDDLEBUTTONRELEASE;
                    event.state &= ~MIDDLEBUTTONMASK;
                }
                if (ev.xbutton.button == Button3)
                {
                    event.type = SEL_RIGHTBUTTONRELEASE;
                    event.state &= ~RIGHTBUTTONMASK;
                }
            }

            // Dispatch to grab window
            if (mouseGrabWindow)
            {
                window->translateCoordinatesTo(event.win_x, event.win_y, mouseGrabWindow, event.win_x, event.win_y);
                if (mouseGrabWindow->handle(this, FXSEL(event.type, 0), &event))
                {
                    refresh();
                }
            }

            // Dispatch if inside model window
            // FIXME doesSaveUnder test should go away
            else if (!invocation || (invocation->modality == MODAL_FOR_NONE) || (invocation->window && invocation->window->isOwnerOf(window)) || window->getShell()->doesSaveUnder())
            {
                if (window->handle(this, FXSEL(event.type, 0), &event))
                {
                    refresh();
                }
            }

            // Beep if outside modal window
            else
            {
                if (ev.xany.type == ButtonPress)
                {
                    beep();
                }
            }

            // Remember last mouse
            event.last_x = event.win_x;
            event.last_y = event.win_y;
            return(true);

        // Crossing
        case EnterNotify:
            event.time = ev.xcrossing.time;
            if (cursorWindow != window)
            {
                if ((ev.xcrossing.mode == NotifyGrab) || (ev.xcrossing.mode == NotifyUngrab) || ((ev.xcrossing.mode == NotifyNormal) && (ev.xcrossing.detail != NotifyInferior)))
                {
                    ancestor = FXWindow::commonAncestor(window, cursorWindow);
                    event.root_x = ev.xcrossing.x_root;
                    event.root_y = ev.xcrossing.y_root;
                    event.code = ev.xcrossing.mode;
                    leaveWindow(cursorWindow, ancestor);
                    enterWindow(window, ancestor);
                }
            }
            return(true);

        // Crossing
        case LeaveNotify:
            event.time = ev.xcrossing.time;
            if (cursorWindow == window)
            {
                if ((ev.xcrossing.mode == NotifyGrab) || (ev.xcrossing.mode == NotifyUngrab) || ((ev.xcrossing.mode == NotifyNormal) && (ev.xcrossing.detail != NotifyInferior)))
                {
                    event.root_x = ev.xcrossing.x_root;
                    event.root_y = ev.xcrossing.y_root;
                    event.code = ev.xcrossing.mode;
                    FXASSERT(cursorWindow == window);
                    leaveWindow(window, window->getParent());
                }
            }
            return(true);

        // Focus change on shell window
        case FocusIn:
        case FocusOut:
            window = window->getShell();
            if ((ev.xfocus.type == FocusOut) && (activeWindow == window))
            {
                event.type = SEL_FOCUSOUT;
                if (window->handle(this, FXSEL(SEL_FOCUSOUT, 0), &event))
                {
                    refresh();
                }
                activeWindow = NULL;
            }
            if ((ev.xfocus.type == FocusIn) && (activeWindow != window))
            {
                event.type = SEL_FOCUSIN;
                if (window->handle(this, FXSEL(SEL_FOCUSIN, 0), &event))
                {
                    refresh();
                }
                activeWindow = window;
            }
            return(true);

        // Map
        case MapNotify:
            event.type = SEL_MAP;
            if (window->handle(this, FXSEL(SEL_MAP, 0), &event))
            {
                refresh();
            }
            return(true);

        // Unmap
        case UnmapNotify:
            event.type = SEL_UNMAP;
            if (window->handle(this, FXSEL(SEL_UNMAP, 0), &event))
            {
                refresh();
            }
            return(true);

        // Create
        case CreateNotify:
            event.type = SEL_CREATE;
            if (window->handle(this, FXSEL(SEL_CREATE, 0), &event))
            {
                refresh();
            }
            return(true);

        // Destroy
        case DestroyNotify:
            event.type = SEL_DESTROY;
            if (window->handle(this, FXSEL(SEL_DESTROY, 0), &event))
            {
                refresh();
            }
            return(true);

        // Configure
        case ConfigureNotify:
            event.type = SEL_CONFIGURE;
            // According to the ICCCM, if its synthetic, the coordinates are relative
            // to root window; otherwise, they're relative to the parent; so we use
            // the old coordinates if its not a synthetic configure notify
            if ((window->getShell() == window) && !ev.xconfigure.send_event)
            {
                ev.xconfigure.x = window->getX();
                ev.xconfigure.y = window->getY();
            }
            event.rect.x = ev.xconfigure.x;
            event.rect.y = ev.xconfigure.y;
            event.rect.w = ev.xconfigure.width;
            event.rect.h = ev.xconfigure.height;
            event.synthetic = ev.xconfigure.send_event;
            if (window->handle(this, FXSEL(SEL_CONFIGURE, 0), &event))
            {
                refresh();
            }
            return(true);

        // Circulate
        case CirculateNotify:
            event.type = SEL_RAISED+(ev.xcirculate.place&1);
            if (window->handle(this, FXSEL(event.type, 0), &event))
            {
                refresh();
            }
            return(true);

        // Selection Clear
        case SelectionClear:
            if (ev.xselectionclear.selection == XA_PRIMARY)
            {
                // We lost the primary selection if the new selection owner is different from selectionWindow
                if (selectionWindow && (selectionWindow->id() != XGetSelectionOwner((Display*)display, XA_PRIMARY)))
                {
                    event.type = SEL_SELECTION_LOST;
                    event.time = ev.xselectionclear.time;
                    if (selectionWindow->handle(this, FXSEL(SEL_SELECTION_LOST, 0), &event))
                    {
                        refresh();
                    }
                    selectionWindow = NULL;
                }
                FXFREE(&xselTypeList);
                xselNumTypes = 0;
            }
            else if (ev.xselectionclear.selection == xcbSelection)
            {
                // We lost the clipboard selection if the new clipboard owner is different from clipboardWindow
                if (clipboardWindow && (clipboardWindow->id() != XGetSelectionOwner((Display*)display, xcbSelection)))
                {
                    event.time = ev.xselectionclear.time;
                    event.type = SEL_CLIPBOARD_LOST;
                    if (clipboardWindow->handle(this, FXSEL(SEL_CLIPBOARD_LOST, 0), &event))
                    {
                        refresh();
                    }
                    clipboardWindow = NULL;
                }
                FXFREE(&xcbTypeList);
                xcbNumTypes = 0;
            }
            return(true);

        // Selection Request
        case SelectionRequest:
            answer = None;
            if (ev.xselectionrequest.selection == XA_PRIMARY)
            {
                if (selectionWindow)
                {
                    if (ev.xselectionrequest.target == ddeTargets)             // Request for TYPES
                    {
                        answer = fxsendtypes((Display*)display, ev.xselectionrequest.requestor, ev.xselectionrequest.property, xselTypeList, xselNumTypes);
                    }
                    else                                                    // Request for DATA
                    {
                        event.type = SEL_SELECTION_REQUEST;
                        event.time = ev.xselectionrequest.time;
                        event.target = ev.xselectionrequest.target;
                        ddeData = NULL;
                        ddeSize = 0;
                        selectionWindow->handle(this, FXSEL(SEL_SELECTION_REQUEST, 0), &event);
                        answer = fxsenddata((Display*)display, ev.xselectionrequest.requestor, ev.xselectionrequest.property, ev.xselectionrequest.target, ddeData, ddeSize);
                        FXFREE(&ddeData);
                        ddeData = NULL;
                        ddeSize = 0;
                    }
                }
            }
            else if (ev.xselectionrequest.selection == xcbSelection)
            {
                if (clipboardWindow)
                {
                    if (ev.xselectionrequest.target == ddeTargets)             // Request for TYPES
                    {
                        answer = fxsendtypes((Display*)display, ev.xselectionrequest.requestor, ev.xselectionrequest.property, xcbTypeList, xcbNumTypes);
                    }
                    else                                                    // Request for DATA
                    {
                        event.type = SEL_CLIPBOARD_REQUEST;
                        event.time = ev.xselectionrequest.time;
                        event.target = ev.xselectionrequest.target;
                        ddeData = NULL;
                        ddeSize = 0;
                        clipboardWindow->handle(this, FXSEL(SEL_CLIPBOARD_REQUEST, 0), &event);
                        answer = fxsenddata((Display*)display, ev.xselectionrequest.requestor, ev.xselectionrequest.property, ev.xselectionrequest.target, ddeData, ddeSize);
                        FXFREE(&ddeData);
                        ddeData = NULL;
                        ddeSize = 0;
                    }
                }
            }
            else if (ev.xselectionrequest.selection == xdndSelection)
            {
                if (dragWindow)
                {
                    if (ev.xselectionrequest.target == ddeTargets)             // Request for TYPES
                    {
                        answer = fxsendtypes((Display*)display, ev.xselectionrequest.requestor, ev.xselectionrequest.property, xdndTypeList, xdndNumTypes);
                    }
                    else                                                    // Request for DATA
                    {
                        event.type = SEL_DND_REQUEST;
                        event.time = ev.xselectionrequest.time;
                        event.target = ev.xselectionrequest.target;
                        ddeData = NULL;
                        ddeSize = 0;
                        dragWindow->handle(this, FXSEL(SEL_DND_REQUEST, 0), &event);
                        answer = fxsenddata((Display*)display, ev.xselectionrequest.requestor, ev.xselectionrequest.property, ev.xselectionrequest.target, ddeData, ddeSize);
                        FXFREE(&ddeData);
                        ddeData = NULL;
                        ddeSize = 0;
                    }
                }
            }
            fxsendreply((Display*)display, ev.xselectionrequest.requestor, ev.xselectionrequest.selection, answer, ev.xselectionrequest.target, ev.xselectionrequest.time);
            return(true);

        // Selection Notify
        case SelectionNotify:
            return(true);

        // Client message
        case ClientMessage:

            // WM_PROTOCOLS
            if (ev.xclient.message_type == wmProtocols)
            {
                if ((FXID)ev.xclient.data.l[0] == wmDeleteWindow)            // WM_DELETE_WINDOW
                {
                    event.type = SEL_CLOSE;
                    if (!invocation || (invocation->modality == MODAL_FOR_NONE) || (invocation->window && invocation->window->isOwnerOf(window)))
                    {
                        if (window->handle(this, FXSEL(SEL_CLOSE, 0), &event))
                        {
                            refresh();
                        }
                    }
                    else
                    {
                        beep();
                    }
                }
                else if ((FXID)ev.xclient.data.l[0] == wmQuitApp)            // WM_QUIT_APP
                {
                    event.type = SEL_CLOSE;
                    if (!invocation || (invocation->modality == MODAL_FOR_NONE) || (invocation->window && invocation->window->isOwnerOf(window)))
                    {
                        if (window->handle(this, FXSEL(SEL_CLOSE, 0), &event))
                        {
                            refresh();
                        }
                    }
                    else
                    {
                        beep();
                    }
                }
                else if ((FXID)ev.xclient.data.l[0] == wmTakeFocus)          // WM_TAKE_FOCUS
                {
                    if (invocation && invocation->window && invocation->window->id())
                    {
                        ev.xclient.window = invocation->window->id();
                    }
                    // Assign focus to innermost modal dialog, even when trying to focus
                    // on another window; these other windows are dead to inputs anyway.
                    // XSetInputFocus causes a spurious BadMatch error; we ignore this in xerrorhandler
                    XSetInputFocus((Display*)display, ev.xclient.window, RevertToParent, ev.xclient.data.l[1]);
                }
                else if ((FXID)ev.xclient.data.l[0] == wmNetPing)           // NET_WM_PING
                {
                    se.xclient.type = ClientMessage;
                    se.xclient.display = (Display*)display;                       // This lets a window manager know that
                    se.xclient.message_type = wmProtocols;                        // we're still alive after having received
                    se.xclient.format = 32;                                       // a WM_DELETE_WINDOW message
                    se.xclient.window = XDefaultRootWindow((Display*)display);
                    se.xclient.data.l[0] = ev.xclient.data.l[0];
                    se.xclient.data.l[1] = ev.xclient.data.l[1];
                    se.xclient.data.l[2] = ev.xclient.data.l[2];
                    se.xclient.data.l[3] = 0;
                    se.xclient.data.l[4] = 0;
                    XSendEvent((Display*)display, se.xclient.window, False, SubstructureRedirectMask|SubstructureNotifyMask, &se);
                }
            }

            // XDND Enter from source
            else if (ev.xclient.message_type == xdndEnter)
            {
                FXint ver = (ev.xclient.data.l[1]>>24)&255;
                if (ver > XDND_PROTOCOL_VERSION)
                {
                    return(true);
                }
                xdndSource = ev.xclient.data.l[0];                                  // Now we're talking to this guy
                if (ddeTypeList)
                {
                    FXFREE(&ddeTypeList);
                    ddeNumTypes = 0;
                }
                if (ev.xclient.data.l[1]&1)
                {
                    fxrecvtypes((Display*)display, xdndSource, xdndTypes, ddeTypeList, ddeNumTypes, FALSE);
                }
                else
                {
                    FXMALLOC(&ddeTypeList, FXDragType, 3);
                    ddeNumTypes = 0;
                    if (ev.xclient.data.l[2])
                    {
                        ddeTypeList[0] = ev.xclient.data.l[2];
                        ddeNumTypes++;
                    }
                    if (ev.xclient.data.l[3])
                    {
                        ddeTypeList[1] = ev.xclient.data.l[3];
                        ddeNumTypes++;
                    }
                    if (ev.xclient.data.l[4])
                    {
                        ddeTypeList[2] = ev.xclient.data.l[4];
                        ddeNumTypes++;
                    }
                }
            }

            // XDND Leave from source
            else if (ev.xclient.message_type == xdndLeave)
            {
                if (xdndSource != (FXID)ev.xclient.data.l[0])
                {
                    return(true);   // We're not talking to this guy
                }
                if (dropWindow)
                {
                    event.type = SEL_DND_LEAVE;
                    if (dropWindow->handle(this, FXSEL(SEL_DND_LEAVE, 0), &event))
                    {
                        refresh();
                    }
                    dropWindow = NULL;
                }
                if (ddeTypeList)
                {
                    FXFREE(&ddeTypeList);
                    ddeNumTypes = 0;
                }
                xdndSource = 0;
            }

            // XDND Position from source
            else if (ev.xclient.message_type == xdndPosition)
            {
                if (xdndSource != (FXID)ev.xclient.data.l[0])
                {
                    return(true);   // We're not talking to this guy
                }
                event.time = ev.xclient.data.l[3];
                event.root_x = ((FXuint)ev.xclient.data.l[2])>>16;
                event.root_y = ((FXuint)ev.xclient.data.l[2])&0xffff;
                // Search from target window down; there may be another window
                // (like e.g. the dragged shape window) right under the cursor.
                // Note this is the target window, not the proxy target....
                window = findWindowAt(event.root_x, event.root_y, ev.xclient.window);
                if ((FXID)ev.xclient.data.l[4] == xdndActionCopy)
                {
                    ddeAction = DRAG_COPY;
                }
                else if ((FXID)ev.xclient.data.l[4] == xdndActionMove)
                {
                    ddeAction = DRAG_MOVE;
                }
                else if ((FXID)ev.xclient.data.l[4] == xdndActionLink)
                {
                    ddeAction = DRAG_LINK;
                }
                else if ((FXID)ev.xclient.data.l[4] == xdndActionPrivate)
                {
                    ddeAction = DRAG_PRIVATE;
                }
                else
                {
                    ddeAction = DRAG_COPY;
                }
                ansAction = DRAG_REJECT;
                xdndWantUpdates = TRUE;
                xdndRect.x = event.root_x;
                xdndRect.y = event.root_y;
                xdndRect.w = 1;
                xdndRect.h = 1;
                if (window != dropWindow)
                {
                    if (dropWindow)
                    {
                        event.type = SEL_DND_LEAVE;
                        if (dropWindow->handle(this, FXSEL(SEL_DND_LEAVE, 0), &event))
                        {
                            refresh();
                        }
                    }
                    dropWindow = NULL;
                    if (window && window->isDropEnabled())
                    {
                        dropWindow = window;
                        event.type = SEL_DND_ENTER;
                        if (dropWindow->handle(this, FXSEL(SEL_DND_ENTER, 0), &event))
                        {
                            refresh();
                        }
                    }
                }
                if (dropWindow)
                {
                    event.type = SEL_DND_MOTION;
                    XTranslateCoordinates((Display*)display, XDefaultRootWindow((Display*)display), dropWindow->id(), event.root_x, event.root_y, &event.win_x, &event.win_y, &tmp);
                    if (dropWindow->handle(this, FXSEL(SEL_DND_MOTION, 0), &event))
                    {
                        refresh();
                    }
                    event.last_x = event.win_x;
                    event.last_y = event.win_y;
                }
                se.xclient.type = ClientMessage;
                se.xclient.display = (Display*)display;
                se.xclient.message_type = xdndStatus;
                se.xclient.format = 32;
                se.xclient.window = xdndSource;
                se.xclient.data.l[0] = ev.xclient.window;                   // Proxy Target window
                se.xclient.data.l[1] = 0;
                if (ansAction != DRAG_REJECT)
                {
                    se.xclient.data.l[1] |= 1;       // Target accepted
                }
                if (xdndWantUpdates)
                {
                    se.xclient.data.l[1] |= 2;              // Target wants continuous position updates
                }
                se.xclient.data.l[2] = MKUINT(xdndRect.y, xdndRect.x);
                se.xclient.data.l[3] = MKUINT(xdndRect.h, xdndRect.w);
                if (ansAction == DRAG_COPY)
                {
                    se.xclient.data.l[4] = xdndActionCopy; // Drag and Drop Action accepted
                }
                else if (ansAction == DRAG_MOVE)
                {
                    se.xclient.data.l[4] = xdndActionMove;
                }
                else if (ansAction == DRAG_LINK)
                {
                    se.xclient.data.l[4] = xdndActionLink;
                }
                else if (ansAction == DRAG_PRIVATE)
                {
                    se.xclient.data.l[4] = xdndActionPrivate;
                }
                else
                {
                    se.xclient.data.l[4] = None;
                }
                XSendEvent((Display*)display, xdndSource, True, NoEventMask, &se);
            }

            // XDND Drop from source
            else if (ev.xclient.message_type == xdndDrop)
            {
                if (xdndSource != (FXID)ev.xclient.data.l[0])
                {
                    return(true);   // We're not talking to this guy
                }
                xdndFinishSent = FALSE;
                event.type = SEL_DND_DROP;
                event.time = ev.xclient.data.l[2];
                if (!dropWindow || !dropWindow->handle(this, FXSEL(SEL_DND_DROP, 0), &event))
                {
                    ansAction = DRAG_REJECT;
                }
                if (!xdndFinishSent)
                {
                    se.xclient.type = ClientMessage;
                    se.xclient.display = (Display*)display;
                    se.xclient.message_type = xdndFinished;
                    se.xclient.format = 32;
                    se.xclient.window = xdndSource;
                    se.xclient.data.l[0] = ev.xclient.window;                     // Proxy Target window
                    se.xclient.data.l[1] = (ansAction == DRAG_REJECT) ? 0 : 1;    // Bit #0 means accepted
                    if (ansAction == DRAG_COPY)
                    {
                        se.xclient.data.l[2] = xdndActionCopy;
                    }
                    else if (ansAction == DRAG_MOVE)
                    {
                        se.xclient.data.l[2] = xdndActionMove;
                    }
                    else if (ansAction == DRAG_LINK)
                    {
                        se.xclient.data.l[2] = xdndActionLink;
                    }
                    else if (ansAction == DRAG_PRIVATE)
                    {
                        se.xclient.data.l[2] = xdndActionPrivate;
                    }
                    else
                    {
                        se.xclient.data.l[2] = None;
                    }
                    se.xclient.data.l[3] = 0;
                    se.xclient.data.l[4] = 0;
                    XSendEvent((Display*)display, xdndSource, True, NoEventMask, &se);
                }
                if (ddeTypeList)
                {
                    FXFREE(&ddeTypeList);
                    ddeNumTypes = 0;
                }
                dropWindow = NULL;
                xdndSource = 0;
                refresh();
            }

            // XDND Status from target
            else if (ev.xclient.message_type == xdndStatus)
            {
                // We ignore ev.xclient.data.l[0], because some other
                // toolkits, e.g. Qt, do not place the proper value there;
                // the proper value is xdndTarget, NOT xdndProxyTarget or None
                //if (xdndTarget!=(FXID)ev.xclient.data.l[0]) return true; // We're not talking to this guy
                ansAction = DRAG_REJECT;
                if (ev.xclient.data.l[1]&1)
                {
                    if ((FXID)ev.xclient.data.l[4] == xdndActionCopy)
                    {
                        ansAction = DRAG_COPY;
                    }
                    else if ((FXID)ev.xclient.data.l[4] == xdndActionMove)
                    {
                        ansAction = DRAG_MOVE;
                    }
                    else if ((FXID)ev.xclient.data.l[4] == xdndActionLink)
                    {
                        ansAction = DRAG_LINK;
                    }
                    else if ((FXID)ev.xclient.data.l[4] == xdndActionPrivate)
                    {
                        ansAction = DRAG_PRIVATE;
                    }
                }
                xdndWantUpdates = ev.xclient.data.l[1]&2;
                xdndRect.x = ((FXuint)ev.xclient.data.l[2])>>16;
                xdndRect.y = ((FXuint)ev.xclient.data.l[2])&0xffff;
                xdndRect.w = ((FXuint)ev.xclient.data.l[3])>>16;
                xdndRect.h = ((FXuint)ev.xclient.data.l[3])&0xffff;
                xdndStatusReceived = TRUE;
                xdndStatusPending = FALSE;
            }
            return(true);

        // Property change
        case PropertyNotify:

            event.time = ev.xproperty.time;

            // Update window position after minimize/maximize/restore whatever
            if ((ev.xproperty.atom == wmState) || (ev.xproperty.atom == wmNetState))
            {
                event.type = SEL_CONFIGURE;
                XTranslateCoordinates((Display*)display, ev.xproperty.window, XDefaultRootWindow((Display*)display), 0, 0, &tmp_x, &tmp_y, &tmp);
                event.rect.x = tmp_x;
                event.rect.y = tmp_y;
                event.rect.w = window->getWidth();
                event.rect.h = window->getHeight();
                event.synthetic = ev.xproperty.send_event;
                if (window->handle(this, FXSEL(SEL_CONFIGURE, 0), &event))
                {
                    refresh();
                }
            }
            return(true);

        // Keyboard mapping
        case MappingNotify:
            if (ev.xmapping.request != MappingPointer)
            {
                XRefreshKeyboardMapping(&ev.xmapping);
            }
            return(true);

        // Other events
        default:
#ifdef HAVE_XRANDR_H
            if (ev.type == xrreventbase+RRScreenChangeNotify)
            {
                XRRUpdateConfiguration(&ev);
                root->setWidth(root->getDefaultWidth());
                root->setHeight(root->getDefaultHeight());
            }
#endif
            return(true);
        }
    }
    return(false);
}


//
// Hack of FXScrollArea
//

// This hack allows to scroll in horizontal mode when we are in row and small/big icons mode of a FileList

// Mouse wheel used for vertical scrolling
long FXScrollArea::onVMouseWheel(FXObject* sender, FXSelector sel, void* ptr)
{
    // !!! Hack to scroll in horizontal mode !!!
    if (!(options&ICONLIST_COLUMNS) && options&(ICONLIST_BIG_ICONS|ICONLIST_MINI_ICONS) && streq(this->getClassName(), "FileList"))
    {
        horizontal->handle(sender, sel, ptr);
    }
    else
    {
        // !!! End of hack !!!
        vertical->handle(sender, sel, ptr);
    }

    return(1);
}


//
// Hack of FXButton
//

// This hack fixes a focus problem on the panels when activating a button which is already activated
// Now, the focus on the active panel is not lost anymore


// Pressed mouse button
long FXButton::onLeftBtnPress(FXObject*, FXSelector, void* ptr)
{
    handle(this, FXSEL(SEL_FOCUS_SELF, 0), ptr);
    flags &= ~FLAG_TIP;
    if (isEnabled() && !(flags&FLAG_PRESSED))
    {
        grab();
        if (target && target->tryHandle(this, FXSEL(SEL_LEFTBUTTONPRESS, message), ptr))
        {
            return(1);
        }
        //if (state!=STATE_ENGAGED) // !!! Hack here !!!
        setState(STATE_DOWN);
        flags |= FLAG_PRESSED;
        flags &= ~FLAG_UPDATE;
        return(1);
    }
    return(0);
}


// Hot key combination pressed
long FXButton::onHotKeyPress(FXObject*, FXSelector, void* ptr)
{
    flags &= ~FLAG_TIP;
    handle(this, FXSEL(SEL_FOCUS_SELF, 0), ptr);
    if (isEnabled() && !(flags&FLAG_PRESSED))
    {
        //if (state!=STATE_ENGAGED)  // !!! Hack here !!!
        setState(STATE_DOWN);
        flags &= ~FLAG_UPDATE;
        flags |= FLAG_PRESSED;
    }
    return(1);
}


//
// Hack of FXTopWindow
//

// This hack fixes a problem with some window managers like Icewm or Openbox
// These WMs do not deal with StaticGravity the same way as e.g. Metacity
// and then the window border can be invisible when launching the applications

// Request for toplevel window resize
void FXTopWindow::resize(int w, int h)
{
    if ((flags&FLAG_DIRTY) || (w != width) || (h != height))
    {
        width = FXMAX(w, 1);
        height = FXMAX(h, 1);
        if (xid)
        {
            XWindowChanges changes;
            XSizeHints     size;
            size.flags = USSize|PSize|PWinGravity|USPosition|PPosition;
            size.x = xpos;
            size.y = ypos;
            size.width = width;
            size.height = height;
            size.min_width = 0;
            size.min_height = 0;
            size.max_width = 0;
            size.max_height = 0;
            size.width_inc = 0;
            size.height_inc = 0;
            size.min_aspect.x = 0;
            size.min_aspect.y = 0;
            size.max_aspect.x = 0;
            size.max_aspect.y = 0;
            size.base_width = 0;
            size.base_height = 0;

            // !!! Hack here !!!
            size.win_gravity = NorthWestGravity;                      // Tim Alexeevsky <realtim@mail.ru>
            //size.win_gravity=StaticGravity;                       // Account for border (ICCCM)
            // !!! End of hack !!!

            if (!(options&DECOR_SHRINKABLE))
            {
                if (!(options&DECOR_STRETCHABLE))                       // Cannot change at all
                {
                    size.flags |= PMinSize|PMaxSize;
                    size.min_width = size.max_width = width;
                    size.min_height = size.max_height = height;
                }
                else                                                    // Cannot get smaller than default
                {
                    size.flags |= PMinSize;
                    size.min_width = getDefaultWidth();
                    size.min_height = getDefaultHeight();
                }
            }
            else if (!(options&DECOR_STRETCHABLE))                      // Cannot get larger than default
            {
                size.flags |= PMaxSize;
                size.max_width = getDefaultWidth();
                size.max_height = getDefaultHeight();
            }
            XSetWMNormalHints(DISPLAY(getApp()), xid, &size);
            changes.x = 0;
            changes.y = 0;
            changes.width = width;
            changes.height = height;
            changes.border_width = 0;
            changes.sibling = None;
            changes.stack_mode = Above;
            XReconfigureWMWindow(DISPLAY(getApp()), xid, DefaultScreen(DISPLAY(getApp())), CWWidth|CWHeight, &changes);
            layout();
        }
    }
}


// Request for toplevel window reposition
void FXTopWindow::position(int x, int y, int w, int h)
{
    if ((flags&FLAG_DIRTY) || (x != xpos) || (y != ypos) || (w != width) || (h != height))
    {
        xpos = x;
        ypos = y;
        width = FXMAX(w, 1);
        height = FXMAX(h, 1);
        if (xid)
        {
            XWindowChanges changes;
            XSizeHints     size;
            size.flags = USSize|PSize|PWinGravity|USPosition|PPosition;
            size.x = xpos;
            size.y = ypos;
            size.width = width;
            size.height = height;
            size.min_width = 0;
            size.min_height = 0;
            size.max_width = 0;
            size.max_height = 0;
            size.width_inc = 0;
            size.height_inc = 0;
            size.min_aspect.x = 0;
            size.min_aspect.y = 0;
            size.max_aspect.x = 0;
            size.max_aspect.y = 0;
            size.base_width = 0;
            size.base_height = 0;

            // !!! Hack here !!!
            size.win_gravity = NorthWestGravity;                      // Tim Alexeevsky <realtim@mail.ru>
            //size.win_gravity=StaticGravity;                       // Account for border (ICCCM)
            // !!! End of hack !!!

            if (!(options&DECOR_SHRINKABLE))
            {
                if (!(options&DECOR_STRETCHABLE))                         // Cannot change at all
                {
                    size.flags |= PMinSize|PMaxSize;
                    size.min_width = size.max_width = width;
                    size.min_height = size.max_height = height;
                }
                else                                                      // Cannot get smaller than default
                {
                    size.flags |= PMinSize;
                    size.min_width = getDefaultWidth();
                    size.min_height = getDefaultHeight();
                }
            }
            else if (!(options&DECOR_STRETCHABLE))                        // Cannot get larger than default
            {
                size.flags |= PMaxSize;
                size.max_width = getDefaultWidth();
                size.max_height = getDefaultHeight();
            }
            XSetWMNormalHints(DISPLAY(getApp()), xid, &size);
            changes.x = xpos;
            changes.y = ypos;
            changes.width = width;
            changes.height = height;
            changes.border_width = 0;
            changes.sibling = None;
            changes.stack_mode = Above;
            XReconfigureWMWindow(DISPLAY(getApp()), xid, DefaultScreen(DISPLAY(getApp())), CWX|CWY|CWWidth|CWHeight, &changes);
            layout();
        }
    }
}


// Position the window based on placement
void FXTopWindow::place(FXuint placement)
{
    int       rx, ry, rw, rh, ox, oy, ow, oh, wx, wy, ww, wh, x, y;
    FXuint    state;
    FXWindow* over;

    // Default placement:- leave it where it was
    wx = getX();
    wy = getY();
    ww = getWidth();
    wh = getHeight();

    // Get root window size
    rx = getRoot()->getX();
    ry = getRoot()->getY();
    rw = getRoot()->getWidth();
    rh = getRoot()->getHeight();

    // Placement policy
    switch (placement)
    {
    // Place such that it contains the cursor
    case PLACEMENT_CURSOR:

        // Get dialog location in root coordinates
        translateCoordinatesTo(wx, wy, getRoot(), 0, 0);

        // Where's the mouse?
        getRoot()->getCursorPosition(x, y, state);

        // Place such that mouse in the middle, placing it as
        // close as possible in the center of the owner window.
        // Don't move the window unless the mouse is not inside.

        // !!! Hack here !!!
        //if (!shown() || x<wx || y<wy || wx+ww<=x || wy+wh<=y)
        if ((x < wx) || (y < wy) || (wx+ww <= x) || (wy+wh <= y))
        // !!! End of hack !!!
        {
            // Get the owner
            over = getOwner() ? getOwner() : getRoot();

            // Get owner window size
            ow = over->getWidth();
            oh = over->getHeight();

            // Owner's coordinates to root coordinates
            over->translateCoordinatesTo(ox, oy, getRoot(), 0, 0);

            // Adjust position
            wx = ox+(ow-ww)/2;
            wy = oy+(oh-wh)/2;

            // Move by the minimal amount
            if (x < wx)
            {
                wx = x-20;
            }
            else if (wx+ww <= x)
            {
                wx = x-ww+20;
            }
            if (y < wy)
            {
                wy = y-20;
            }
            else if (wy+wh <= y)
            {
                wy = y-wh+20;
            }
        }

        // Adjust so dialog is fully visible
        if (wx < rx)
        {
            wx = rx+10;
        }
        if (wy < ry)
        {
            wy = ry+10;
        }
        if (wx+ww > rx+rw)
        {
            wx = rx+rw-ww-10;
        }
        if (wy+wh > ry+rh)
        {
            wy = ry+rh-wh-10;
        }
        break;

    // Place centered over the owner
    case PLACEMENT_OWNER:

        // Get the owner
        over = getOwner() ? getOwner() : getRoot();

        // Get owner window size
        ow = over->getWidth();
        oh = over->getHeight();

        // Owner's coordinates to root coordinates
        over->translateCoordinatesTo(ox, oy, getRoot(), 0, 0);

        // Adjust position
        wx = ox+(ow-ww)/2;
        wy = oy+(oh-wh)/2;

        // Adjust so dialog is fully visible
        if (wx < rx)
        {
            wx = rx+10;
        }
        if (wy < ry)
        {
            wy = ry+10;
        }
        if (wx+ww > rx+rw)
        {
            wx = rx+rw-ww-10;
        }
        if (wy+wh > ry+rh)
        {
            wy = ry+rh-wh-10;
        }
        break;

    // Place centered on the screen
    case PLACEMENT_SCREEN:

        // Adjust position
        wx = rx+(rw-ww)/2;
        wy = ry+(rh-wh)/2;
        break;

    // Place to make it fully visible
    case PLACEMENT_VISIBLE:

        // Adjust so dialog is fully visible
        if (wx < rx)
        {
            wx = rx+10;
        }
        if (wy < ry)
        {
            wy = ry+10;
        }
        if (wx+ww > rx+rw)
        {
            wx = rx+rw-ww-10;
        }
        if (wy+wh > ry+rh)
        {
            wy = ry+rh-wh-10;
        }
        break;

    // Place maximized
    case PLACEMENT_MAXIMIZED:
        wx = rx;
        wy = ry;
        ww = rw;                // Yes, I know:- we should substract the borders;
        wh = rh;                // trouble is, no way to know how big those are....
        break;

    // Default placement
    case PLACEMENT_DEFAULT:
    default:
        break;
    }

    // Place it
    position(wx, wy, ww, wh);
}


//
// Hack of FXAccelTable
//

// This hack allows to ignore caps lock when using keyboard shortcuts

#define EMPTYSLOT     0xfffffffe     // Previously used, now empty slot
#define UNUSEDSLOT    0xffffffff     // Unsused slot marker

// Keyboard press; forward to accelerator target
long FXAccelTable::onKeyPress(FXObject* sender, FXSelector, void* ptr)
{
    register FXEvent* event = (FXEvent*)ptr;

    // !!! Hack here !!!
    //register FXuint code=MKUINT(event->code,event->state&(SHIFTMASK|CONTROLMASK|ALTMASK|METAMASK));
    register FXuint code;

    if (event->state&CAPSLOCKMASK && (event->code >= KEY_A) && (event->code <= KEY_Z))
    {
        code = MKUINT(event->code+32, event->state&(SHIFTMASK|CONTROLMASK|ALTMASK|METAMASK));
    }
    else
    {
        code = MKUINT(event->code, event->state&(SHIFTMASK|CONTROLMASK|ALTMASK|METAMASK));
    }
    // !!! End of hack !!!

    register FXuint p = (code*13)&max;
    register FXuint c;
    FXASSERT(code != UNUSEDSLOT);
    FXASSERT(code != EMPTYSLOT);
    while ((c = key[p].code) != code)
    {
        if (c == UNUSEDSLOT)
        {
            return(0);
        }
        p = (p+1)&max;
    }
    if (key[p].target && key[p].messagedn)
    {
        key[p].target->tryHandle(sender, key[p].messagedn, ptr);
    }
    return(1);
}


// Keyboard release; forward to accelerator target
long FXAccelTable::onKeyRelease(FXObject* sender, FXSelector, void* ptr)
{
    register FXEvent* event = (FXEvent*)ptr;

    // !!! Hack here !!!
    //register FXuint code=MKUINT(event->code,event->state&(SHIFTMASK|CONTROLMASK|ALTMASK|METAMASK));
    register FXuint code;

    if (event->state&CAPSLOCKMASK && (event->code >= KEY_A) && (event->code <= KEY_Z))
    {
        code = MKUINT(event->code+32, event->state&(SHIFTMASK|CONTROLMASK|ALTMASK|METAMASK));
    }
    else
    {
        code = MKUINT(event->code, event->state&(SHIFTMASK|CONTROLMASK|ALTMASK|METAMASK));
    }
    // !!! End of hack !!!

    register FXuint p = (code*13)&max;
    register FXuint c;
    FXASSERT(code != UNUSEDSLOT);
    FXASSERT(code != EMPTYSLOT);
    while ((c = key[p].code) != code)
    {
        if (c == UNUSEDSLOT)
        {
            return(0);
        }
        p = (p+1)&max;
    }
    if (key[p].target && key[p].messageup)
    {
        key[p].target->tryHandle(sender, key[p].messageup, ptr);
    }
    return(1);
}


//
// Hack of FXURL
//

// Backport from Fox 1.7.37 to fix a bug when filenames contain '%' characters

// Hexadecimal digit of value
const FXchar value2Digit[36] =
{
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B',
    'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
};

// Encode control characters and characters from set using %-encoding
FXString FXURL::encode(const FXString& url)
{
    FXString result;

    if (!url.empty())
    {
        register FXint p, q, c;
        for (p = q = 0; p < url.length(); ++p)
        {
            c = (FXuchar)url[p];
            if ((c < 0x20) || (c == '%'))
            {
                q += 3;
                continue;
            }
            q++;
        }
        result.length(q);
        for (p = q = 0; p < url.length(); ++p)
        {
            c = (FXuchar)url[p];
            if ((c < 0x20) || (c == '%'))
            {
                result[q++] = '%';
                result[q++] = value2Digit[c>>4];
                result[q++] = value2Digit[c&15];
                continue;
            }
            result[q++] = c;
        }
    }
    return(result);
}


// Decode string containing %-encoded characters
FXString FXURL::decode(const FXString& url)
{
    FXString result;

    if (!url.empty())
    {
        register FXint p, q, c;
        for (p = q = 0; p < url.length(); ++p)
        {
            c = (FXuchar)url[p];
            if ((c == '%') && Ascii::isHexDigit(url[p+1]) && Ascii::isHexDigit(url[p+2]))
            {
                p += 2;
            }
            q++;
        }
        result.length(q);
        for (p = q = 0; p < url.length(); ++p)
        {
            c = (FXuchar)url[p];
            if ((c == '%') && Ascii::isHexDigit(url[p+1]) && Ascii::isHexDigit(url[p+2]))
            {
                c = (Ascii::digitValue(url[p+1])<<4)+Ascii::digitValue(url[p+2]);
                p += 2;
            }
            result[q++] = c;
        }
    }
    return(result);
}


//
// Hack of FXSpinner
// This hack fixes an issue with the appearance of the spinner textfield
//


#define INTMAX    2147483647
#define INTMIN    (-INTMAX-1)

// Construct spinner out of two buttons and a text field
FXSpinner::FXSpinner(FXComposite* p, FXint cols, FXObject* tgt, FXSelector sel, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb) :
    FXPacker(p, opts, x, y, w, h, 0, 0, 0, 0, 0, 0)
{
    flags |= FLAG_ENABLED;
    target = tgt;
    message = sel;

    // !!! Hack here !!!
    //textField=new FXTextField(this,cols,this,ID_ENTRY,TEXTFIELD_INTEGER|JUSTIFY_RIGHT,0,0,0,0,pl,pr,pt,pb);
    textField = new FXTextField(this, cols, this, ID_ENTRY, TEXTFIELD_INTEGER|JUSTIFY_RIGHT|FRAME_THICK|FRAME_SUNKEN, 0, 0, 0, 0, pl, pr, pt, pb);
    // !!! End of hack !!!

    upButton = new FXArrowButton(this, this, FXSpinner::ID_INCREMENT, FRAME_RAISED|FRAME_THICK|ARROW_UP|ARROW_REPEAT, 0, 0, 0, 0, 0, 0, 0, 0);
    downButton = new FXArrowButton(this, this, FXSpinner::ID_DECREMENT, FRAME_RAISED|FRAME_THICK|ARROW_DOWN|ARROW_REPEAT, 0, 0, 0, 0, 0, 0, 0, 0);
    range[0] = (options&SPIN_NOMIN) ? INTMIN : 0;
    range[1] = (options&SPIN_NOMAX) ? INTMAX : 100;
    textField->setText("0");
    incr = 1;
    pos = 0;
}


//
// Hack of FXMenuCommand, FXPopup, FXMenuCascade, FXMenuBar, FXMenuSeparator
// These hacks replace the reverse arrow cursor (DEF_RARROW_CURSOR) with the normal one (DEF_ARROW_CURSOR)
// when pointing on menu items
//


// Command menu item
FXMenuCommand::FXMenuCommand(FXComposite* p, const FXString& text, FXIcon* ic, FXObject* tgt, FXSelector sel, FXuint opts) :
    FXMenuCaption(p, text, ic, opts)
{
    FXAccelTable* table;
    FXWindow*     own;

    flags |= FLAG_ENABLED;
    defaultCursor = getApp()->getDefaultCursor(DEF_ARROW_CURSOR);
    target = tgt;
    message = sel;
    accel = text.section('\t', 1);
    acckey = parseAccel(accel);
    if (acckey)
    {
        own = getShell()->getOwner();
        if (own)
        {
            table = own->getAccelTable();
            if (table)
            {
                table->addAccel(acckey, this, FXSEL(SEL_COMMAND, ID_ACCEL));
            }
        }
    }
}


// Transient window used for popups
FXPopup::FXPopup(FXWindow* owner, FXuint opts, FXint x, FXint y, FXint w, FXint h) :
    FXShell(owner, opts, x, y, w, h), prevActive(NULL), nextActive(NULL)
{
    defaultCursor = getApp()->getDefaultCursor(DEF_ARROW_CURSOR);
    dragCursor = getApp()->getDefaultCursor(DEF_ARROW_CURSOR);
    flags |= FLAG_ENABLED;
    grabowner = NULL;
    baseColor = getApp()->getBaseColor();
    hiliteColor = getApp()->getHiliteColor();
    shadowColor = getApp()->getShadowColor();
    borderColor = getApp()->getBorderColor();
    border = (options&FRAME_THICK) ? 2 : (options&(FRAME_SUNKEN|FRAME_RAISED)) ? 1 : 0;
}


// Make cascade menu button
FXMenuCascade::FXMenuCascade(FXComposite* p, const FXString& text, FXIcon* ic, FXPopup* pup, FXuint opts) :
    FXMenuCaption(p, text, ic, opts)
{
    defaultCursor = getApp()->getDefaultCursor(DEF_ARROW_CURSOR);
    flags |= FLAG_ENABLED;
    pane = pup;
}


// Make a non-floatable menubar
FXMenuBar::FXMenuBar(FXComposite* p, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs) :
    FXToolBar(p, opts, x, y, w, h, pl, pr, pt, pb, hs, vs)
{
    flags |= FLAG_ENABLED;
    dragCursor = getApp()->getDefaultCursor(DEF_ARROW_CURSOR);
}


// Separator item
FXMenuSeparator::FXMenuSeparator(FXComposite* p, FXuint opts) :
    FXWindow(p, opts, 0, 0, 0, 0)
{
    flags |= FLAG_SHOWN;
    defaultCursor = getApp()->getDefaultCursor(DEF_ARROW_CURSOR);
    hiliteColor = getApp()->getHiliteColor();
    shadowColor = getApp()->getShadowColor();
}


//
// Hack of FxString
// This hack allocates a much longer string
// This is used in the SearchPanel when the path length is huge
//

// Print formatted string a-la vprintf
FXString& FXString::vformat(const FXchar* fmt, va_list args)
{
    register FXint len = 0;

    if (fmt && *fmt)
    {
        register FXint n = strlen(fmt);       // Result is longer than format string

        // !!! Hack here !!!
        //n+=1024;
        n += 8192;                            // Add a lot of slop
        // !!! End of hack

        length(n);
        len = vsprintf(str, fmt, args);

        FXASSERT(0 <= len && len <= n);
    }
    length(len);
    return(*this);
}



//
// Hack of FXIcon
// This hack fixes a Valgrind complaint about uninitialized values
// when allocating an icon
//


#define DARKCOLOR(r,g,b) (((r)+(g)+(b))<thresh)

// Determine threshold for etch mask
static FXshort guessthresh(const FXColor *data,FXint width,FXint height)
{
    register FXint med=(width*height)>>1;
    register FXint cum,i,j;
    register FXshort guess;
    FXint frequency[766];
    memset(frequency,0,sizeof(frequency));
    for(i=0; i<width*height; ++i)
    {
        frequency[((const FXuchar*)(data+i))[0]+((const FXuchar*)(data+i))[1]+((const FXuchar*)(data+i))[2]]++;
    }
    for(i=0,cum=0; i<766; ++i)
    {
        if((cum+=frequency[i])>=med) break;
    }
    for(j=765,cum=0; j>0; --j)
    {
        if((cum+=frequency[j])>=med) break;
    }
    guess=((i+j+1)>>1)+1; // Fanglin Zhu: raise threshold by one in case of single-color image
    return guess;
}

// Render icon X Windows
void FXIcon::render()
{
    if(xid)
    {
        register Visual *vis;
        register XImage *xim=NULL;
        register FXColor *img;
        register FXint x,y;
        register FXshort thresh; // Local variable in 1.6
        XGCValues values;
        GC gc;

        // Render the image pixels
        FXImage::render();

        // Fill with pixels if there is data
        if(data && 0<width && 0<height)
        {
            // Guess threshold
            thresh=guessthresh(data,width,height);

            // Get Visual
            vis=(Visual*)visual->visual;

            // Try create image
            xim=XCreateImage(DISPLAY(getApp()),vis,1,ZPixmap,0,NULL,width,height,32,0);
            if(!xim) fxerror("%s::render: unable to render icon.\n",getClassName());

            // Try create temp pixel store
            
            // !!! Hack here => replace FXMALLOC with FXCALLOC !!!
            if(!FXCALLOC(&xim->data,char,xim->bytes_per_line*height)){ fxerror("%s::render: unable to allocate memory.\n",getClassName()); }
            // !!! End of hack !!!

            // Make GC
            values.foreground=0xffffffff;
            values.background=0xffffffff;
            gc=XCreateGC(DISPLAY(getApp()),shape,GCForeground|GCBackground,&values);

            // Should have succeeded
            FXASSERT(xim);

            // Fill shape mask
            if(options&IMAGE_OPAQUE)                                  // Opaque image
            {
                FXTRACE((150,"Shape rectangle\n"));
                memset(xim->data,0xff,xim->bytes_per_line*height);
            }
            else if(options&(IMAGE_ALPHACOLOR|IMAGE_ALPHAGUESS))      // Transparent color
            {
                FXTRACE((150,"Shape from alpha-color\n"));
                img=data;
                for(y=0; y<height; y++)
                {
                    for(x=0; x<width; x++)
                    {
                        XPutPixel(xim,x,y,(img[x]!=transp));
                    }
                    img+=width;
                }
            }
            else                                                      // Transparency channel
            {
                FXTRACE((150,"Shape from alpha-channel\n"));
                img=data;
                for(y=0; y<height; y++)
                {
                    for(x=0; x<width; x++)
                    {
                        XPutPixel(xim,x,y,(((FXuchar*)(img+x))[3]!=0));
                    }
                    img+=width;
                }
            }

            XPutImage(DISPLAY(getApp()),shape,gc,xim,0,0,0,0,width,height);

            // Fill etch image
            if(options&IMAGE_OPAQUE)                                  // Opaque image
            {
                img=data;
                for(y=0; y<height; y++)
                {
                    for(x=0; x<width; x++)
                    {
                        XPutPixel(xim,x,y,DARKCOLOR(((FXuchar*)(img+x))[0],((FXuchar*)(img+x))[1],((FXuchar*)(img+x))[2]));
                    }
                    img+=width;
                }
            }
            else if(options&(IMAGE_ALPHACOLOR|IMAGE_ALPHAGUESS))      // Transparent color
            {
                img=data;
                for(y=0; y<height; y++)
                {
                    for(x=0; x<width; x++)
                    {
                        XPutPixel(xim,x,y,(img[x]!=transp) && DARKCOLOR(((FXuchar*)(img+x))[0],((FXuchar*)(img+x))[1],((FXuchar*)(img+x))[2]));
                    }
                    img+=width;
                }
            }
            else                                                      // Transparency channel
            {
                img=data;
                for(y=0; y<height; y++)
                {
                    for(x=0; x<width; x++)
                    {
                        XPutPixel(xim,x,y,(((FXuchar*)(img+x))[3]!=0) && DARKCOLOR(((FXuchar*)(img+x))[0],((FXuchar*)(img+x))[1],((FXuchar*)(img+x))[2]));
                    }
                    img+=width;
                }
            }

            // Transfer image
            XPutImage(DISPLAY(getApp()),etch,gc,xim,0,0,0,0,width,height);

            // Clean up
            FXFREE(&xim->data);
            XDestroyImage(xim);
            XFreeGC(DISPLAY(getApp()),gc);
        }
    }
}
