// Search panel
#include "config.h"
#include "i18n.h"

#include <fx.h>
#include <fxkeys.h>


#include <FXPNGIcon.h>
#include <FXJPGIcon.h>
#include <FXTIFIcon.h>


#include "xfedefs.h"
#include "startupnotification.h"
#include "icons.h"
#include "File.h"
#include "FileDict.h"
#include "FileDialog.h"
#include "FileList.h"
#include "MessageBox.h"
#include "ArchInputDialog.h"
#include "HistInputDialog.h"
#include "BrowseInputDialog.h"
#include "OverwriteBox.h"
#include "CommandWindow.h"
#include "ExecuteBox.h"
#include "XFileExplorer.h"
#include "SearchPanel.h"



#if defined(linux)
extern FXStringDict* fsdevices; // Devices from fstab
#endif

// Global Variables
extern FXMainWindow* mainWindow;
extern FXString      homedir;
extern FXString      xdgdatahome;
extern FXbool        allowPopupScroll;
extern FXuint        single_click;

// Clipboard
extern FXString clipboard;
extern FXuint   clipboard_type;

extern char OpenHistory[OPEN_HIST_SIZE][MAX_COMMAND_SIZE];
extern int  OpenNum;

#if defined(linux)
extern FXbool pkg_format;
#endif

// Button separator margins and height
#define SEP_SPACE     5
#define SEP_HEIGHT    20


// Map
FXDEFMAP(SearchPanel) SearchPanelMap[] =
{
    FXMAPFUNC(SEL_CLIPBOARD_LOST, 0, SearchPanel::onClipboardLost),
    FXMAPFUNC(SEL_CLIPBOARD_GAINED, 0, SearchPanel::onClipboardGained),
    FXMAPFUNC(SEL_CLIPBOARD_REQUEST, 0, SearchPanel::onClipboardRequest),
    FXMAPFUNC(SEL_CLICKED, SearchPanel::ID_FILELIST, SearchPanel::onCmdItemClicked),
    FXMAPFUNC(SEL_DOUBLECLICKED, SearchPanel::ID_FILELIST, SearchPanel::onCmdItemDoubleClicked),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_GOTO_PARENTDIR, SearchPanel::onCmdGotoParentdir),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_OPEN_WITH, SearchPanel::onCmdOpenWith),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_OPEN, SearchPanel::onCmdOpen),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_VIEW, SearchPanel::onCmdEdit),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_EDIT, SearchPanel::onCmdEdit),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_COMPARE, SearchPanel::onCmdCompare),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_REFRESH, SearchPanel::onCmdRefresh),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_PROPERTIES, SearchPanel::onCmdProperties),
    FXMAPFUNC(SEL_MIDDLEBUTTONPRESS, SearchPanel::ID_FILELIST, SearchPanel::onCmdEdit),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_SELECT_ALL, SearchPanel::onCmdSelect),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_DESELECT_ALL, SearchPanel::onCmdSelect),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_SELECT_INVERSE, SearchPanel::onCmdSelect),
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE, SearchPanel::ID_FILELIST, SearchPanel::onCmdPopupMenu),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_POPUP_MENU, SearchPanel::onCmdPopupMenu),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_COPY_CLIPBOARD, SearchPanel::onCmdCopyCut),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_CUT_CLIPBOARD, SearchPanel::onCmdCopyCut),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_FILE_COPYTO, SearchPanel::onCmdFileMan),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_FILE_MOVETO, SearchPanel::onCmdFileMan),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_FILE_RENAME, SearchPanel::onCmdFileMan),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_FILE_SYMLINK, SearchPanel::onCmdFileMan),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_ADD_TO_ARCH, SearchPanel::onCmdAddToArch),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_EXTRACT, SearchPanel::onCmdExtract),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_FILE_TRASH, SearchPanel::onCmdFileTrash),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_FILE_DELETE, SearchPanel::onCmdFileDelete),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_GO_SCRIPTDIR, SearchPanel::onCmdGoScriptDir),
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_DIR_USAGE, SearchPanel::onCmdDirUsage),
    FXMAPFUNC(SEL_UPDATE, SearchPanel::ID_STATUS, SearchPanel::onUpdStatus),
    FXMAPFUNC(SEL_UPDATE, SearchPanel::ID_FILE_RENAME, SearchPanel::onUpdSelMult),
    FXMAPFUNC(SEL_UPDATE, SearchPanel::ID_GOTO_PARENTDIR, SearchPanel::onUpdSelMult),
    FXMAPFUNC(SEL_UPDATE, SearchPanel::ID_COMPARE, SearchPanel::onUpdCompare),
    FXMAPFUNC(SEL_UPDATE, SearchPanel::ID_COPY_CLIPBOARD, SearchPanel::onUpdMenu),
    FXMAPFUNC(SEL_UPDATE, SearchPanel::ID_CUT_CLIPBOARD, SearchPanel::onUpdMenu),
    FXMAPFUNC(SEL_UPDATE, SearchPanel::ID_PROPERTIES, SearchPanel::onUpdMenu),
    FXMAPFUNC(SEL_UPDATE, SearchPanel::ID_FILE_TRASH, SearchPanel::onUpdMenu),
    FXMAPFUNC(SEL_UPDATE, SearchPanel::ID_FILE_DELETE, SearchPanel::onUpdMenu),
    FXMAPFUNC(SEL_UPDATE, SearchPanel::ID_DIR_USAGE, SearchPanel::onUpdDirUsage),
#if defined(linux)
    FXMAPFUNC(SEL_COMMAND, SearchPanel::ID_PKG_QUERY, SearchPanel::onCmdPkgQuery),
    FXMAPFUNC(SEL_UPDATE, SearchPanel::ID_PKG_QUERY, SearchPanel::onUpdPkgQuery),
#endif
};


// Object implementation
FXIMPLEMENT(SearchPanel, FXVerticalFrame, SearchPanelMap, ARRAYNUMBER(SearchPanelMap))



// Contruct Search Panel
SearchPanel::SearchPanel(FXComposite* p, FXuint name_size, FXuint dir_size, FXuint size_size, FXuint type_size, FXuint ext_size,
                         FXuint modd_size, FXuint user_size, FXuint grou_size, FXuint attr_size, FXColor listbackcolor, FXColor listforecolor,
                         FXuint opts, int x, int y, int w, int h) :
    FXVerticalFrame(p, opts, x, y, w, h, 0, 0, 0, 0)
{
    //mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_ANIMTHROB), NULL);
    //mainWindow->handle(this, FXSEL(SEL_COMMAND, XFileExplorer::ID_ANIMTHROB));

    // Global container
    FXVerticalFrame* cont = new FXVerticalFrame(this, LAYOUT_FILL_Y|LAYOUT_FILL_X|FRAME_NONE, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1);

    // Container for the action toolbar
    FXHorizontalFrame* toolbar = new FXHorizontalFrame(cont, LAYOUT_SIDE_TOP|LAYOUT_FILL_X|FRAME_NONE, 0, 0, 0, 0, DEFAULT_SPACING, DEFAULT_SPACING, DEFAULT_SPACING, DEFAULT_SPACING, 0, 0);

    // File list
    FXVerticalFrame* cont2 = new FXVerticalFrame(cont, LAYOUT_FILL_Y|LAYOUT_FILL_X|FRAME_SUNKEN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    list = new FileList(this, cont2, this, ID_FILELIST, 0, LAYOUT_FILL_X|LAYOUT_FILL_Y|_ICONLIST_DETAILED|_FILELIST_SEARCH);
    list->setHeaderSize(0, name_size);
    list->setHeaderSize(1, dir_size);
    list->setHeaderSize(2, size_size);
    list->setHeaderSize(3, type_size);
    list->setHeaderSize(4, ext_size);
    list->setHeaderSize(5, modd_size);
    list->setHeaderSize(6, user_size);
    list->setHeaderSize(7, grou_size);
    list->setHeaderSize(8, attr_size);
    list->setTextColor(listforecolor);
    list->setBackColor(listbackcolor);

	// Set list style
    FXuint liststyle = getApp()->reg().readUnsignedEntry("SEARCH PANEL", "liststyle", _ICONLIST_DETAILED);
    list->setListStyle(liststyle);
	
	// Set dirs first
    FXuint dirsfirst = getApp()->reg().readUnsignedEntry("SEARCH PANEL", "dirs_first", 1);
    list->setDirsFirst(dirsfirst);

	// Set ignore case
    FXuint ignorecase = getApp()->reg().readUnsignedEntry("SEARCH PANEL", "ignore_case", 1);
    list->setIgnoreCase(ignorecase);

    // Toolbar buttons
    FXHotKey hotkey;
    FXString key;

    // Refresh panel toolbar button
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "refresh", "Ctrl-R");
    refreshbtn = new FXButton(toolbar, TAB+_("Refresh panel")+PARS(key), reloadicon, this, SearchPanel::ID_REFRESH, BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT|ICON_BEFORE_TEXT);
    hotkey = _parseAccel(key);
    refreshbtn->addHotKey(hotkey);

    // Goto dir toolbar button
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "go_up", "Backspace");
    gotodirbtn = new FXButton(toolbar, TAB+_("Go to parent folder")+PARS(key), gotodiricon, this, SearchPanel::ID_GOTO_PARENTDIR, BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT|ICON_BEFORE_TEXT);
    hotkey = _parseAccel(key);
    gotodirbtn->addHotKey(hotkey);

    // Copy / cut / properties toolbar buttons
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "copy", "Ctrl-C");
    copybtn = new FXButton(toolbar, TAB+_("Copy selected files to clipboard")+PARS(key), copy_clpicon, this, SearchPanel::ID_COPY_CLIPBOARD, BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT|ICON_BEFORE_TEXT);
    hotkey = _parseAccel(key);
    copybtn->addHotKey(hotkey);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "cut", "Ctrl-X");
    cutbtn = new FXButton(toolbar, TAB+_("Cut selected files to clipboard")+PARS(key), cut_clpicon, this, SearchPanel::ID_CUT_CLIPBOARD, BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT|ICON_BEFORE_TEXT);
    hotkey = _parseAccel(key);
    cutbtn->addHotKey(hotkey);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "properties", "F9");
    propbtn = new FXButton(toolbar, TAB+_("Show properties of selected files")+PARS(key), attribicon, this, SearchPanel::ID_PROPERTIES, BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT|ICON_BEFORE_TEXT);
    hotkey = _parseAccel(key);
    propbtn->addHotKey(hotkey);

    // Separator
    new FXFrame(toolbar, LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0, 0, SEP_SPACE);
    new FXVerticalSeparator(toolbar, LAYOUT_SIDE_TOP|LAYOUT_CENTER_Y|SEPARATOR_GROOVE|LAYOUT_FIX_HEIGHT, 0, 0, 0, SEP_HEIGHT);
    new FXFrame(toolbar, LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0, 0, SEP_SPACE);

    // Move to trash / delete toolbar buttons
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "move_to_trash", "Del");
    trashbtn = new FXButton(toolbar, TAB+_("Move selected files to trash can")+PARS(key), filedeleteicon, this, SearchPanel::ID_FILE_TRASH, BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT|ICON_BEFORE_TEXT);
    hotkey = _parseAccel(key);
    trashbtn->addHotKey(hotkey);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "delete", "Shift-Del");
    delbtn = new FXButton(toolbar, TAB+_("Delete selected files")+PARS(key), filedelete_permicon, this, SearchPanel::ID_FILE_DELETE, BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT|ICON_BEFORE_TEXT);
    hotkey = _parseAccel(key);
    delbtn->addHotKey(hotkey);

    // Separator
    new FXFrame(toolbar, LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0, 0, SEP_SPACE);
    new FXVerticalSeparator(toolbar, LAYOUT_SIDE_TOP|LAYOUT_CENTER_Y|SEPARATOR_GROOVE|LAYOUT_FIX_HEIGHT, 0, 0, 0, SEP_HEIGHT);
    new FXFrame(toolbar, LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT, 0, 0, SEP_SPACE);

    // Icon view toolbar buttons
    key = getApp()->reg().readStringEntry("KEYBINDINGS", "big_icons", "F10");
    bigiconsbtn = new FXButton(toolbar, TAB+_("Big icon list")+PARS(key), bigiconsicon, list, IconList::ID_SHOW_BIG_ICONS, BUTTON_TOOLBAR|LAYOUT_TOP|LAYOUT_LEFT|ICON_BEFORE_TEXT|FRAME_RAISED);
    hotkey = _parseAccel(key);
    bigiconsbtn->addHotKey(hotkey);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "small_icons", "F11");
    smalliconsbtn = new FXButton(toolbar, TAB+_("Small icon list")+PARS(key), smalliconsicon, list, IconList::ID_SHOW_MINI_ICONS, BUTTON_TOOLBAR|LAYOUT_TOP|LAYOUT_LEFT|ICON_BEFORE_TEXT|FRAME_RAISED);
    hotkey = _parseAccel(key);
    smalliconsbtn->addHotKey(hotkey);

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "detailed_file_list", "F12");
    detailsbtn = new FXButton(toolbar, TAB+_("Detailed file list")+PARS(key), detailsicon, list, IconList::ID_SHOW_DETAILS, BUTTON_TOOLBAR|LAYOUT_TOP|LAYOUT_LEFT|ICON_BEFORE_TEXT|FRAME_RAISED);
    hotkey = _parseAccel(key);
    detailsbtn->addHotKey(hotkey);

    // Status bar
    statusbar = new FXHorizontalFrame(cont, LAYOUT_LEFT|JUSTIFY_LEFT|LAYOUT_FILL_X|FRAME_NONE, 0, 0, 0, 0, 3, 3, 3, 3);
    statusbar->setTarget(this);
    statusbar->setSelector(FXSEL(SEL_UPDATE, SearchPanel::ID_STATUS));

    key = getApp()->reg().readStringEntry("KEYBINDINGS", "thumbnails", "Ctrl-F7");
    thumbbtn = new FXToggleButton(statusbar, TAB+_("Show thumbnails")+PARS(key), TAB+_("Hide thumbnails")+PARS(key), showthumbicon, hidethumbicon, this->list,
                                  FileList::ID_TOGGLE_THUMBNAILS, TOGGLEBUTTON_TOOLBAR|LAYOUT_LEFT|ICON_BEFORE_TEXT|FRAME_RAISED);
    hotkey = _parseAccel(key);
    thumbbtn->addHotKey(hotkey);

    new FXHorizontalFrame(statusbar, LAYOUT_LEFT|JUSTIFY_LEFT|LAYOUT_FILL_X|FRAME_NONE, 0, 0, 0, 0, 0, 0, 0, 0);
    status = new FXLabel(statusbar, _("Status"), NULL, JUSTIFY_LEFT|LAYOUT_LEFT|LAYOUT_FILL_X);

    corner = new FXDragCorner(statusbar);

    // File associations
    associations = NULL;
    associations = new FileDict(getApp());

    // Dialogs
    archdialog = NULL;
    opendialog = NULL;
    operationdialogsingle = NULL;
    operationdialogrename = NULL;
    operationdialogmultiple = NULL;
    comparedialog = NULL;

    // Trahscan locations
    trashfileslocation = xdgdatahome + PATHSEPSTRING TRASHFILESPATH;
    trashinfolocation = xdgdatahome + PATHSEPSTRING TRASHINFOPATH;

    // Default programs identifiers
    progs["<txtviewer>"] = TXTVIEWER;
    progs["<txteditor>"] = TXTEDITOR;
    progs["<imgviewer>"] = IMGVIEWER;
    progs["<imgeditor>"] = IMGEDITOR;
    progs["<pdfviewer>"] = PDFVIEWER;
    progs["<audioplayer>"] = AUDIOPLAYER;
    progs["<videoplayer>"] = VIDEOPLAYER;
    progs["<archiver>"] = ARCHIVER;

    // Class variable initializations
    ctrlflag = false;
    shiftf10 = false;
}


// Create window
void SearchPanel::create()
{
    // Register standard uri-list type
    urilistType = getApp()->registerDragType("text/uri-list");

    // Register special uri-list type used for Gnome, XFCE and Xfe
    xfelistType = getApp()->registerDragType("x-special/gnome-copied-files");

    // Register special uri-list type used for KDE
    kdelistType = getApp()->registerDragType("application/x-kde-cutselection");

    // Register standard UTF-8 text type used for file dialogs
    utf8Type = getApp()->registerDragType("UTF8_STRING");

    FXVerticalFrame::create();

    // Single click navigation
    if (single_click == SINGLE_CLICK_DIR_FILE)
    {
        list->setDefaultCursor(getApp()->getDefaultCursor(DEF_HAND_CURSOR));
    }
    else
    {
        list->setDefaultCursor(getApp()->getDefaultCursor(DEF_ARROW_CURSOR));
    }
}


// Clean up
SearchPanel::~SearchPanel()
{
    delete list;
    if (opendialog != NULL)
    {
        delete opendialog;
    }
    if (archdialog != NULL)
    {
        delete archdialog;
    }
    if (operationdialogsingle != NULL)
    {
        delete operationdialogsingle;
    }
    if (operationdialogrename != NULL)
    {
        delete operationdialogrename;
    }
    if (operationdialogmultiple != NULL)
    {
        delete operationdialogmultiple;
    }
    if (comparedialog != NULL)
    {
        delete comparedialog;
    }
}


// Double Click on File Item
long SearchPanel::onCmdItemDoubleClicked(FXObject* sender, FXSelector sel, void* ptr)
{
    // Wait cursor
    getApp()->beginWaitCursor();
    mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_ANIMTHROB), NULL);

    FXlong item = (FXlong)ptr;
    if (item > -1)
    {
#ifdef STARTUP_NOTIFICATION
        // Startup notification option and exceptions (if any)
        FXbool   usesn = getApp()->reg().readUnsignedEntry("OPTIONS", "use_startup_notification", true);
        FXString snexcepts = getApp()->reg().readStringEntry("OPTIONS", "startup_notification_exceptions", "");
#endif

        // Default programs
        FXString txtviewer = getApp()->reg().readStringEntry("PROGS", "txtviewer", DEFAULT_TXTVIEWER);
        FXString txteditor = getApp()->reg().readStringEntry("PROGS", "txteditor", DEFAULT_TXTEDITOR);
        FXString imgviewer = getApp()->reg().readStringEntry("PROGS", "imgviewer", DEFAULT_IMGVIEWER);
        FXString imgeditor = getApp()->reg().readStringEntry("PROGS", "imgeditor", DEFAULT_IMGEDITOR);
        FXString pdfviewer = getApp()->reg().readStringEntry("PROGS", "pdfviewer", DEFAULT_PDFVIEWER);
        FXString audioplayer = getApp()->reg().readStringEntry("PROGS", "audioplayer", DEFAULT_AUDIOPLAYER);
        FXString videoplayer = getApp()->reg().readStringEntry("PROGS", "videoplayer", DEFAULT_VIDEOPLAYER);
        FXString archiver = getApp()->reg().readStringEntry("PROGS", "archiver", DEFAULT_ARCHIVER);

        FXString cmd, cmdname, filename, pathname, parentdir;

        // File name and path
        filename = list->getItemFilename(item);
        pathname = list->getItemFullPathname(item);

        // If directory, open the directory
        if (list->isItemDirectory(item))
        {
            // Does not have access
            if (!::isReadExecutable(pathname))
            {
                MessageBox::error(this, BOX_OK_SU, _("Error"), _(" Permission to: %s denied."), pathname.text());
                getApp()->endWaitCursor();
		mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
                return(0);
            }

            // Change directory in Xfe
            ((XFileExplorer*)mainWindow)->setDirectory(pathname);

            // Raise the Xfe window
            ((XFileExplorer*)mainWindow)->raise();
            ((XFileExplorer*)mainWindow)->setFocus();

            // Warning message when setting current folder in Xfe
            FXbool warn = getApp()->reg().readUnsignedEntry("OPTIONS", "folder_warn", true);
            if (warn)
            {
                MessageBox::information(((XFileExplorer*)mainWindow), BOX_OK, _("Information"), _("Current folder has been set to '%s'"), pathname.text());
            }
        }
        else if (list->isItemFile(item))
        {
            // Parent directory
            parentdir = FXPath::directory(pathname);

            // Update associations dictionary
            FileDict*  assocdict = new FileDict(getApp());
            FileAssoc* association = assocdict->findFileBinding(pathname.text());

            // If there is an association
            if (association)
            {
                // Use it to open the file
                if (association->command.section(',', 0) != "")
                {
                    cmdname = association->command.section(',', 0);

                    // Use a default program if possible
                    switch (progs[cmdname])
                    {
                    case TXTVIEWER:
                        cmdname = txtviewer;
                        break;

                    case TXTEDITOR:
                        cmdname = txteditor;
                        break;

                    case IMGVIEWER:
                        cmdname = imgviewer;
                        break;

                    case IMGEDITOR:
                        cmdname = imgeditor;
                        break;

                    case PDFVIEWER:
                        cmdname = pdfviewer;
                        break;

                    case AUDIOPLAYER:
                        cmdname = audioplayer;
                        break;

                    case VIDEOPLAYER:
                        cmdname = videoplayer;
                        break;

                    case ARCHIVER:
                        cmdname = archiver;
                        break;

                    case NONE: // No program found
                        ;
                        break;
                    }

                    // If command exists, run it
                    if (::existCommand(cmdname))
                    {
                        cmd = cmdname+" "+::quote(pathname);
#ifdef STARTUP_NOTIFICATION
                        runcmd(cmd, cmdname, parentdir, searchdir, usesn, snexcepts);
#else
                        runcmd(cmd, parentdir, searchdir);
#endif
                    }

                    // If command does not exist, call the "Open with..." dialog
                    else
                    {
			mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
                        getApp()->endWaitCursor();
                        this->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                    }
                }

                // Or execute the file
                else if (list->isItemExecutable(item))
                {
                    execFile(pathname);
                }

                // Or call the "Open with..." dialog
                else
                {
		    mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
                    getApp()->endWaitCursor();
                    this->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                }
            }

            // If no association but executable
            else if (list->isItemExecutable(item))
            {
                execFile(pathname);
            }

            // Other cases
            else
            {
		mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
                getApp()->endWaitCursor();
                this->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
            }
        }
    }

    mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
    getApp()->endWaitCursor();

    return(1);
}


// Execute file with an optional confirm dialog
void SearchPanel::execFile(FXString pathname)
{
    int      ret;
    FXString cmd, cmdname, parentdir;

#ifdef STARTUP_NOTIFICATION
    // Startup notification option and exceptions (if any)
    FXbool   usesn = getApp()->reg().readUnsignedEntry("OPTIONS", "use_startup_notification", true);
    FXString snexcepts = getApp()->reg().readStringEntry("OPTIONS", "startup_notification_exceptions", "");
#endif

    // Parent directory
    parentdir = FXPath::directory(pathname);

    // File is executable, but is it a text file?
    FXString str = mimetype(pathname);
    FXbool   isTextFile = true;
    if (strstr(str.text(), "charset=binary"))
    {
        isTextFile = false;
    }

    // With confirmation dialog
    FXbool confirm_execute = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_execute", true);
    if (isTextFile && (confirm_execute == 1))
    {
        FXString msg;
        msg.format(_("File %s is an executable text file, what do you want to do?"), pathname.text());
        ExecuteBox* dlg = new ExecuteBox(this, _("Confirm Execute"), msg);
        FXuint      answer = dlg->execute(PLACEMENT_CURSOR);
        delete dlg;

        // Execute
        if (answer == EXECBOX_CLICKED_EXECUTE)
        {
            cmdname = FXPath::name(pathname);
            cmd = ::quote(pathname);
#ifdef STARTUP_NOTIFICATION
            runcmd(cmd, cmdname, parentdir, searchdir, usesn, snexcepts);
#else
            runcmd(cmd, parentdir, searchdir);
#endif
        }

        // Execute in console mode
        if (answer == EXECBOX_CLICKED_CONSOLE)
        {
            ret = chdir(parentdir.text());
            if (ret < 0)
            {
                int errcode = errno;
                if (errcode)
                {
                    MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s: %s"), parentdir.text(), strerror(errcode));
                }
                else
                {
                    MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"), parentdir.text());
                }
            }

            cmdname = FXPath::name(pathname);
            cmd = ::quote(pathname);

            // Make and show command window
            // The CommandWindow object will delete itself when closed!
            CommandWindow* cmdwin = new CommandWindow(getApp(), _("Command log"), cmd, 30, 80);
            cmdwin->create();
            cmdwin->setIcon(runicon);
        }

        // Edit
        if (answer == EXECBOX_CLICKED_EDIT)
        {
            FXString txteditor = getApp()->reg().readStringEntry("PROGS", "txteditor", DEFAULT_TXTEDITOR);
            cmd = txteditor;
            cmdname = cmd;

            // If command exists, run it
            if (::existCommand(cmdname))
            {
                cmd = cmdname+" "+::quote(pathname);
#ifdef STARTUP_NOTIFICATION
                runcmd(cmd, cmdname, parentdir, searchdir, usesn, snexcepts);
#else
                runcmd(cmd, parentdir, searchdir);
#endif
            }

            // If command does not exist, call the "Open with..." dialog
            else
            {
                this->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
            }
        }
    }

    // Without confirmation dialog
    else
    {
        cmdname = FXPath::name(pathname);
        cmd = ::quote(pathname);
#ifdef STARTUP_NOTIFICATION
        runcmd(cmd, cmdname, parentdir, searchdir, usesn, snexcepts);
#else
        runcmd(cmd, parentdir, searchdir);
#endif
    }
}


// Single click on File Item
long SearchPanel::onCmdItemClicked(FXObject* sender, FXSelector sel, void* ptr)
{
    if (single_click != SINGLE_CLICK_NONE)
    {
        // Default programs
        FXString txtviewer = getApp()->reg().readStringEntry("PROGS", "txtviewer", DEFAULT_TXTVIEWER);
        FXString txteditor = getApp()->reg().readStringEntry("PROGS", "txteditor", DEFAULT_TXTEDITOR);
        FXString imgviewer = getApp()->reg().readStringEntry("PROGS", "imgviewer", DEFAULT_IMGVIEWER);
        FXString imgeditor = getApp()->reg().readStringEntry("PROGS", "imgeditor", DEFAULT_IMGEDITOR);
        FXString pdfviewer = getApp()->reg().readStringEntry("PROGS", "pdfviewer", DEFAULT_PDFVIEWER);
        FXString audioplayer = getApp()->reg().readStringEntry("PROGS", "audioplayer", DEFAULT_AUDIOPLAYER);
        FXString videoplayer = getApp()->reg().readStringEntry("PROGS", "videoplayer", DEFAULT_VIDEOPLAYER);
        FXString archiver = getApp()->reg().readStringEntry("PROGS", "archiver", DEFAULT_ARCHIVER);

        // In detailed mode, avoid single click when cursor is not over the first column
        int    x, y;
        FXuint state;
        getCursorPosition(x, y, state);
        FXbool allow = true;
        if (!(list->getListStyle()&(_ICONLIST_BIG_ICONS|_ICONLIST_MINI_ICONS)) && ((x-list->getXPosition()) > list->getHeaderSize(0)))
        {
            allow = false;
        }

        // Single click with control or shift
        if (state&(CONTROLMASK|SHIFTMASK))
        {
            return(1);
        }

        // Single click without control or shift
        else
        {
            FXString cmd, cmdname, filename, pathname, parentdir;

            // Wait cursor
	    mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_ANIMTHROB), NULL);
            getApp()->beginWaitCursor();

#ifdef STARTUP_NOTIFICATION
            // Startup notification option and exceptions (if any)
            FXbool   usesn = getApp()->reg().readUnsignedEntry("OPTIONS", "use_startup_notification", true);
            FXString snexcepts = getApp()->reg().readStringEntry("OPTIONS", "startup_notification_exceptions", "");
#endif

            FXlong item = (FXlong)ptr;
            if (item > -1)
            {
                // File name and path
                filename = list->getItemFilename(item);
                pathname = list->getItemFullPathname(item);

                // If directory, open the directory
                if ((single_click != SINGLE_CLICK_NONE) && list->isItemDirectory(item) && allow)
                {
                    // Does not have access
                    if (!::isReadExecutable(pathname))
                    {
                        MessageBox::error(this, BOX_OK_SU, _("Error"), _(" Permission to: %s denied."), pathname.text());
                        getApp()->endWaitCursor();
	    		mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
                        return(0);
                    }

                    // Change directory in Xfe
                    ((XFileExplorer*)mainWindow)->setDirectory(pathname);

                    // Raise the Xfe window
                    ((XFileExplorer*)mainWindow)->raise();
                    ((XFileExplorer*)mainWindow)->setFocus();

                    // Warning message when setting current folder in Xfe
                    FXbool warn = getApp()->reg().readUnsignedEntry("OPTIONS", "folder_warn", true);
                    if (warn)
                    {
                        MessageBox::information(((XFileExplorer*)mainWindow), BOX_OK, _("Information"), _("Current folder has been set to '%s'"), pathname.text());
                    }
                }

                // If file, use the association if any
                else if ((single_click == SINGLE_CLICK_DIR_FILE) && list->isItemFile(item) && allow)
                {
                    // Parent directory
                    parentdir = FXPath::directory(pathname);

                    // Update associations dictionary
                    FileDict*  assocdict = new FileDict(getApp());
                    FileAssoc* association = assocdict->findFileBinding(pathname.text());

                    // If there is an association
                    if (association)
                    {
                        // Use it to open the file
                        if (association->command.section(',', 0) != "")
                        {
                            cmdname = association->command.section(',', 0);

                            // Use a default program if possible
                            switch (progs[cmdname])
                            {
                            case TXTVIEWER:
                                cmdname = txtviewer;
                                break;

                            case TXTEDITOR:
                                cmdname = txteditor;
                                break;

                            case IMGVIEWER:
                                cmdname = imgviewer;
                                break;

                            case IMGEDITOR:
                                cmdname = imgeditor;
                                break;

                            case PDFVIEWER:
                                cmdname = pdfviewer;
                                break;

                            case AUDIOPLAYER:
                                cmdname = audioplayer;
                                break;

                            case VIDEOPLAYER:
                                cmdname = videoplayer;
                                break;

                            case ARCHIVER:
                                cmdname = archiver;
                                break;

                            case NONE: // No program found
                                ;
                                break;
                            }

                            // If command exists, run it
                            if (::existCommand(cmdname))
                            {
                                cmd = cmdname+" "+::quote(pathname);
#ifdef STARTUP_NOTIFICATION
                                runcmd(cmd, cmdname, parentdir, searchdir, usesn, snexcepts);
#else
                                runcmd(cmd, parentdir, searchdir);
#endif
                            }

                            // If command does not exist, call the "Open with..." dialog
                            else
                            {
                                getApp()->endWaitCursor();
			        mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
                                this->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                            }
                        }

                        // Or execute the file
                        else if (list->isItemExecutable(item))
                        {
                            execFile(pathname);
                        }

                        // Or call the "Open with..." dialog
                        else
                        {
                            getApp()->endWaitCursor();
			    mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
                            this->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                        }
                    }

                    // If no association but executable
                    else if (list->isItemExecutable(item))
                    {
                        execFile(pathname);
                    }

                    // Other cases
                    else
                    {
                        getApp()->endWaitCursor();
		        mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
                        this->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                    }
                }
            }
	    mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
            getApp()->endWaitCursor();
        }
    }
    return(1);
}


// Open with
long SearchPanel::onCmdOpenWith(FXObject*, FXSelector, void*)
{
    char** str = NULL;

    if (list->getNumSelectedItems() == 0)
    {
        return(0);
    }

    FXString cmd = "", cmdname;
    if (opendialog == NULL)
    {
        opendialog = new HistInputDialog(this, "", _("Open selected file(s) with:"), _("Open With"), "", bigfileopenicon, HIST_INPUT_EXECUTABLE_FILE, true, _("A&ssociate"));
    }
    opendialog->setText(cmd);

    // Dialog with history list and associate checkbox
    opendialog->CursorEnd();
    opendialog->selectAll();
    opendialog->clearItems();
    for (int i = 0; i < OpenNum; i++)
    {
        opendialog->appendItem(OpenHistory[i]);
    }
    opendialog->sortItems();
    opendialog->setDirectory(ROOTDIR);
    if (opendialog->execute())
    {
        cmd = opendialog->getText();
        if (cmd == "")
        {
            MessageBox::warning(this, BOX_OK, _("Warning"), _("File name is empty, operation cancelled"));
            return(0);
        }

        for (int u = 0; u < list->getNumItems(); u++)
        {
            if (list->isItemSelected(u))
            {
                // Handles "associate" checkbox for "open with..." dialog
                if (opendialog->getOption())
                {
                    FXString filename = list->getItemFilename(u);
                    FXString ext = filename.rafter('.', 2).lower();

                    if ((ext == "tar.gz") || (ext == "tar.bz2") || (ext == "tar.xz") || (ext == "tar.z")) // Special cases
                    {
                    }
                    else
                    {
                        ext = FXPath::extension(filename).lower();
                    }

                    if (ext == "")
                    {
                        ext = FXPath::name(filename);
                    }

                    FileAssoc* association = list->getItemAssoc(u);

                    if (association)
                    {
                        // Update existing association
                        FXString oldfileassoc = getApp()->reg().readStringEntry("FILETYPES", ext.text(), "");
                        oldfileassoc.erase(0, oldfileassoc.section(';', 0).section(',', 0).length());
                        oldfileassoc.prepend(opendialog->getText());
                        getApp()->reg().writeStringEntry("FILETYPES", ext.text(), oldfileassoc.text());

                        // Handle file association
                        str = new char* [2];
                        str[0] = new char[strlen(ext.text())+1];
                        str[1] = new char[strlen(oldfileassoc.text())+1];
                        strlcpy(str[0], ext.text(), ext.length()+1);
                        strlcpy(str[1], oldfileassoc.text(), oldfileassoc.length()+1);
                        mainWindow->handle(this, FXSEL(SEL_COMMAND, XFileExplorer::ID_FILE_ASSOC), str);
                    }
                    else
                    {
                        // New association
                        FXString newcmd = opendialog->getText().append(";Document;;;;");
                        getApp()->reg().writeStringEntry("FILETYPES", ext.text(), newcmd.text());

                        // Handle file association
                        str = new char* [2];
                        str[0] = new char[strlen(ext.text())+1];
                        str[1] = new char[strlen(newcmd.text())+1];
                        strlcpy(str[0], ext.text(), ext.length()+1);
                        strlcpy(str[1], newcmd.text(), newcmd.length()+1);
                        mainWindow->handle(this, FXSEL(SEL_COMMAND, XFileExplorer::ID_FILE_ASSOC), str);
                    }
                }
                // End

                FXString pathname = list->getItemFullPathname(u);
                cmdname = cmd;
                cmd += " ";
                cmd = cmd+::quote(pathname);
            }
        }

        // Run command if it exists
        mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_ANIMTHROB), NULL);
        getApp()->beginWaitCursor();

#ifdef STARTUP_NOTIFICATION
        // Startup notification option and exceptions (if any)
        FXbool   usesn = getApp()->reg().readUnsignedEntry("OPTIONS", "use_startup_notification", true);
        FXString snexcepts = getApp()->reg().readStringEntry("OPTIONS", "startup_notification_exceptions", "");
#endif

        // If command exists, run it
        if (::existCommand(cmdname))
#ifdef STARTUP_NOTIFICATION
        {
            runcmd(cmd, cmdname, searchdir, searchdir, usesn, snexcepts);
        }
#else
        {
            runcmd(cmd, searchdir, searchdir);
        }
#endif
        // If command does not exist, call the "Open with..." dialog
        else
        {
            mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
            getApp()->endWaitCursor();
            this->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
            return(1);
        }

        // Update history list
        OpenNum = opendialog->getHistorySize();
        cmd = opendialog->getText();

        // Check if cmd is a new string, i.e. is not already in history
        FXbool newstr = true;
        for (int i = 0; i < OpenNum-1; i++)
        {
            if (streq(OpenHistory[i], cmd.text()))
            {
                newstr = false;
                break;
            }
        }

        // Restore original history order
        opendialog->clearItems();
        for (int i = 0; i < OpenNum; i++)
        {
            opendialog->appendItem(OpenHistory[i]);
        }

        // History limit reached
        if (OpenNum > OPEN_HIST_SIZE)
        {
            OpenNum--;
        }

        // New string
        if (newstr)
        {
            // FIFO
            strlcpy(OpenHistory[0], cmd.text(), cmd.length()+1);
            for (int i = 1; i < OpenNum; i++)
            {
                strlcpy(OpenHistory[i], opendialog->getHistoryItem(i-1).text(), opendialog->getHistoryItem(i-1).length()+1);
            }
        }
	mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
        getApp()->endWaitCursor();
    }

    return(1);
}


// Open single or multiple files
long SearchPanel::onCmdOpen(FXObject*, FXSelector, void*)
{
    // Wait cursor
    mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_ANIMTHROB), NULL);
    getApp()->beginWaitCursor();

    FXString   pathname, samecmd, cmd, cmdname, itemslist = " ";
    FileAssoc* association;
    FXbool     same = true;
    FXbool     first = true;

    if (list->getNumSelectedItems() == 0)
    {
        mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
        getApp()->endWaitCursor();
        return(0);
    }

    // Default programs
    FXString txtviewer = getApp()->reg().readStringEntry("PROGS", "txtviewer", DEFAULT_TXTVIEWER);
    FXString txteditor = getApp()->reg().readStringEntry("PROGS", "txteditor", DEFAULT_TXTEDITOR);
    FXString imgviewer = getApp()->reg().readStringEntry("PROGS", "imgviewer", DEFAULT_IMGVIEWER);
    FXString imgeditor = getApp()->reg().readStringEntry("PROGS", "imgeditor", DEFAULT_IMGEDITOR);
    FXString pdfviewer = getApp()->reg().readStringEntry("PROGS", "pdfviewer", DEFAULT_PDFVIEWER);
    FXString audioplayer = getApp()->reg().readStringEntry("PROGS", "audioplayer", DEFAULT_AUDIOPLAYER);
    FXString videoplayer = getApp()->reg().readStringEntry("PROGS", "videoplayer", DEFAULT_VIDEOPLAYER);
    FXString archiver = getApp()->reg().readStringEntry("PROGS", "archiver", DEFAULT_ARCHIVER);

    // Update associations dictionary
    FileDict* assocdict = new FileDict(getApp());

    // Check if all files have the same association
    for (int u = 0; u < list->getNumItems(); u++)
    {
        if (list->isItemSelected(u))
        {
            // Increment number of selected items
            pathname = list->getItemFullPathname(u);
            association = assocdict->findFileBinding(pathname.text());

            if (association)
            {
                cmd = association->command.section(',', 0);

                // Use a default program if possible
                switch (progs[cmd])
                {
                case TXTVIEWER:
                    cmd = txtviewer;
                    break;

                case TXTEDITOR:
                    cmd = txteditor;
                    break;

                case IMGVIEWER:
                    cmd = imgviewer;
                    break;

                case IMGEDITOR:
                    cmd = imgeditor;
                    break;

                case PDFVIEWER:
                    cmd = pdfviewer;
                    break;

                case AUDIOPLAYER:
                    cmd = audioplayer;
                    break;

                case VIDEOPLAYER:
                    cmd = videoplayer;
                    break;

                case ARCHIVER:
                    cmd = archiver;
                    break;

                case NONE: // No program found
                    ;
                    break;
                }

                if (cmd != "")
                {
                    // First selected item
                    if (first)
                    {
                        samecmd = cmd;
                        first = false;
                    }

                    if (samecmd != cmd)
                    {
                        same = false;
                        break;
                    }

                    // List of selected items
                    itemslist += ::quote(pathname) + " ";
                }
                else
                {
                    same = false;
                    break;
                }
            }
            else
            {
                same = false;
                break;
            }
        }
    }

#ifdef STARTUP_NOTIFICATION
    // Startup notification option and exceptions (if any)
    FXbool   usesn = getApp()->reg().readUnsignedEntry("OPTIONS", "use_startup_notification", true);
    FXString snexcepts = getApp()->reg().readStringEntry("OPTIONS", "startup_notification_exceptions", "");
#endif

    // Same command for all files : open them
    if (same)
    {
        cmdname = samecmd;

        // If command exists, run it
        if (::existCommand(cmdname))
        {
            cmd = samecmd+itemslist;
#ifdef STARTUP_NOTIFICATION
            runcmd(cmd, cmdname, searchdir, searchdir, usesn, snexcepts);
#else
            runcmd(cmd, searchdir, searchdir);
#endif
        }

        // If command does not exist, call the "Open with..." dialog
        else
        {
            getApp()->endWaitCursor();
            this->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
        }
    }

    // Files have different commands : handle them separately
    else
    {
        for (int u = 0; u < list->getNumItems(); u++)
        {
            if (list->isItemSelected(u))
            {
                pathname = list->getItemFullPathname(u);
                association = assocdict->findFileBinding(pathname.text());

                // If there is an association
                if (association)
                {
                    // Use it to open the file
                    cmd = association->command.section(',', 0);

                    // Use a default program if possible
                    switch (progs[cmd])
                    {
                    case TXTVIEWER:
                        cmd = txtviewer;
                        break;

                    case TXTEDITOR:
                        cmd = txteditor;
                        break;

                    case IMGVIEWER:
                        cmd = imgviewer;
                        break;

                    case IMGEDITOR:
                        cmd = imgeditor;
                        break;

                    case PDFVIEWER:
                        cmd = pdfviewer;
                        break;

                    case AUDIOPLAYER:
                        cmd = audioplayer;
                        break;

                    case VIDEOPLAYER:
                        cmd = videoplayer;
                        break;

                    case ARCHIVER:
                        cmd = archiver;
                        break;

                    case NONE: // No program found
                        ;
                        break;
                    }

                    if (cmd != "")
                    {
                        cmdname = cmd;

                        // If command exists, run it
                        if (::existCommand(cmdname))
                        {
                            cmd = cmdname+" "+::quote(pathname);
#ifdef STARTUP_NOTIFICATION
                            runcmd(cmd, cmdname, searchdir, searchdir, usesn, snexcepts);
#else
                            runcmd(cmd, searchdir, searchdir);
#endif
                        }

                        // If command does not exist, call the "Open with..." dialog
                        else
                        {
                            getApp()->endWaitCursor();
		            mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
                            this->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                        }
                    }

                    // or execute the file
                    else if (list->isItemExecutable(u))
                    {
                        execFile(pathname);
                    }

                    // or call the "Open with..." dialog
                    else
                    {
                        getApp()->endWaitCursor();
		        mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
                        this->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                    }
                }

                // If no association but executable
                else if (list->isItemExecutable(u))
                {
                    execFile(pathname);
                }

                // Other cases
                else
                {
                    getApp()->endWaitCursor();
	            mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
                    this->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                }
            }
        }
    }

    mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
    getApp()->endWaitCursor();

    return(1);
}


// View/Edit files
long SearchPanel::onCmdEdit(FXObject*, FXSelector s, void*)
{
    // Wait cursor
    getApp()->beginWaitCursor();
    mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_ANIMTHROB), NULL);

    FXString   pathname, samecmd, cmd, cmdname, itemslist = " ";
    FileAssoc* association;
    FXbool     same = true;
    FXbool     first = true;

    if (list->getNumSelectedItems() == 0)
    {
        getApp()->endWaitCursor();
        mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
        return(0);
    }

    FXString txtviewer = getApp()->reg().readStringEntry("PROGS", "txtviewer", DEFAULT_TXTVIEWER);
    FXString txteditor = getApp()->reg().readStringEntry("PROGS", "txteditor", DEFAULT_TXTEDITOR);
    FXString imgviewer = getApp()->reg().readStringEntry("PROGS", "imgviewer", DEFAULT_IMGVIEWER);
    FXString imgeditor = getApp()->reg().readStringEntry("PROGS", "imgeditor", DEFAULT_IMGEDITOR);
    FXString pdfviewer = getApp()->reg().readStringEntry("PROGS", "pdfviewer", DEFAULT_PDFVIEWER);
    FXString audioplayer = getApp()->reg().readStringEntry("PROGS", "audioplayer", DEFAULT_AUDIOPLAYER);
    FXString videoplayer = getApp()->reg().readStringEntry("PROGS", "videoplayer", DEFAULT_VIDEOPLAYER);
    FXString archiver = getApp()->reg().readStringEntry("PROGS", "archiver", DEFAULT_ARCHIVER);

    // Update associations dictionary
    FileDict* assocdict = new FileDict(getApp());

    // Check if all files have the same association
    for (int u = 0; u < list->getNumItems(); u++)
    {
        if (list->isItemSelected(u))
        {
            // Increment number of selected items
            pathname = list->getItemFullPathname(u);
            association = assocdict->findFileBinding(pathname.text());

            // If there is an association
            if (association)
            {
                // Use it to edit/view the files
                if (FXSELID(s) == ID_EDIT) // Edit
                {
                    cmd = association->command.section(',', 2);

                    // Use a default editor if possible
                    switch (progs[cmd])
                    {
                    case TXTEDITOR:
                        cmd = txteditor;
                        break;

                    case IMGEDITOR:
                        cmd = imgeditor;
                        break;

                    case ARCHIVER:
                        cmd = archiver;
                        break;

                    case NONE: // No default editor found
                        ;
                        break;
                    }

                    if (cmd.length() == 0)
                    {
                        cmd = txteditor;
                    }
                }
                else // Any other is View
                {
                    cmd = association->command.section(',', 1);

                    // Use a default viewer if possible
                    switch (progs[cmd])
                    {
                    case TXTVIEWER:
                        cmd = txtviewer;
                        break;

                    case IMGVIEWER:
                        cmd = imgviewer;
                        break;

                    case PDFVIEWER:
                        cmd = pdfviewer;
                        break;

                    case AUDIOPLAYER:
                        cmd = audioplayer;
                        break;

                    case VIDEOPLAYER:
                        cmd = videoplayer;
                        break;

                    case ARCHIVER:
                        cmd = archiver;
                        break;

                    case NONE: // No default viewer found
                        ;
                        break;
                    }

                    if (cmd.length() == 0)
                    {
                        cmd = txtviewer;
                    }
                }
                if (cmd.text() != NULL)
                {
                    // First selected item
                    if (first)
                    {
                        samecmd = cmd;
                        first = false;
                    }

                    if (samecmd != cmd)
                    {
                        same = false;
                        break;
                    }

                    // List of selected items
                    itemslist += ::quote(pathname) + " ";
                }
                else
                {
                    same = false;
                    break;
                }
            }

            // No association
            else
            {
                same = false;
                break;
            }
        }
    }

#ifdef STARTUP_NOTIFICATION
    // Startup notification option and exceptions (if any)
    FXbool   usesn = getApp()->reg().readUnsignedEntry("OPTIONS", "use_startup_notification", true);
    FXString snexcepts = getApp()->reg().readStringEntry("OPTIONS", "startup_notification_exceptions", "");
#endif

    // Same association for all files : execute the associated or default editor or viewer
    if (same)
    {
        cmdname = samecmd;

        // If command exists, run it
        if (::existCommand(cmdname))
        {
            cmd = cmdname+itemslist;
#ifdef STARTUP_NOTIFICATION
            runcmd(cmd, cmdname, searchdir, searchdir, usesn, snexcepts);
#else
            runcmd(cmd, searchdir, searchdir);
#endif
        }

        // If command does not exist, call the "Open with..." dialog
        else
        {
            getApp()->endWaitCursor();
	    mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
            this->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
        }
    }

    // Files have different associations : handle them separately
    else
    {
        for (int u = 0; u < list->getNumItems(); u++)
        {
            if (list->isItemSelected(u))
            {
                pathname = list->getItemFullPathname(u);

                // Only View / Edit regular files (not directories)
                if (::isFile(pathname))
                {
                    association = assocdict->findFileBinding(pathname.text());

                    // If there is an association
                    if (association)
                    {
                        // Use it to edit/view the file
                        if (FXSELID(s) == ID_EDIT) // Edit
                        {
                            cmd = association->command.section(',', 2);

                            // Use a default editor if possible
                            switch (progs[cmd])
                            {
                            case TXTEDITOR:
                                cmd = txteditor;
                                break;

                            case IMGEDITOR:
                                cmd = imgeditor;
                                break;

                            case ARCHIVER:
                                cmd = archiver;
                                break;
                            }

                            if (cmd.length() == 0)
                            {
                                cmd = txteditor;
                            }
                        }
                        else // Any other is View
                        {
                            cmd = association->command.section(',', 1);

                            // Use a default viewer if possible
                            switch (progs[cmd])
                            {
                            case TXTVIEWER:
                                cmd = txtviewer;
                                break;

                            case IMGVIEWER:
                                cmd = imgviewer;
                                break;

                            case PDFVIEWER:
                                cmd = pdfviewer;
                                break;

                            case AUDIOPLAYER:
                                cmd = audioplayer;
                                break;

                            case VIDEOPLAYER:
                                cmd = videoplayer;
                                break;

                            case ARCHIVER:
                                cmd = archiver;
                                break;

                            case NONE: // No default viewer found
                                ;
                                break;
                            }

                            if (cmd.length() == 0)
                            {
                                cmd = txtviewer;
                            }
                        }
                        if (cmd.text() != NULL)
                        {
                            cmdname = cmd;

                            // If command exists, run it
                            if (::existCommand(cmdname))
                            {
                                cmd = cmdname+" "+::quote(pathname);
#ifdef STARTUP_NOTIFICATION
                                runcmd(cmd, cmdname, searchdir, searchdir, usesn, snexcepts);
#else
                                runcmd(cmd, searchdir, searchdir);
#endif
                            }

                            // If command does not exist, call the "Open with..." dialog
                            else
                            {
                                getApp()->endWaitCursor();
			        mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
                                this->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                            }
                        }
                    }

                    // No association
                    else
                    {
                        if (FXSELID(s) == ID_EDIT)
                        {
                            cmd = txteditor;
                        }
                        else
                        {
                            cmd = txtviewer;
                        }

                        cmdname = cmd;

                        // If command exists, run it
                        if (::existCommand(cmdname))
                        {
                            cmd = cmdname+" "+::quote(pathname);
#ifdef STARTUP_NOTIFICATION
                            runcmd(cmd, cmdname, searchdir, searchdir, usesn, snexcepts);
#else
                            runcmd(cmd, searchdir, searchdir);
#endif
                        }

                        // If command does not exist, call the "Open with..." dialog
                        else
                        {
                            getApp()->endWaitCursor();
			    mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
                            this->handle(this, FXSEL(SEL_COMMAND, ID_OPEN_WITH), NULL);
                        }
                    }
                }
            }
        }
    }

    mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
    getApp()->endWaitCursor();

    return(1);
}


// Compare two files
long SearchPanel::onCmdCompare(FXObject*, FXSelector s, void*)
{
    list->setFocus();
    int num = list->getNumSelectedItems();

    // Only one or two selected items can be handled
    if ((num != 1) && (num != 2))
    {
        mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
        getApp()->endWaitCursor();
        return(0);
    }

#ifdef STARTUP_NOTIFICATION
    // Startup notification option and exceptions (if any)
    FXbool   usesn = getApp()->reg().readUnsignedEntry("OPTIONS", "use_startup_notification", true);
    FXString snexcepts = getApp()->reg().readStringEntry("OPTIONS", "startup_notification_exceptions", "");
#endif

    FXString filecomparator = getApp()->reg().readStringEntry("PROGS", "filecomparator", DEFAULT_FILECOMPARATOR);
    FXString pathname, cmd, cmdname, itemslist = " ";

    // One selected item
    if (num == 1)
    {
        // Get the selected item
        for (int u = 0; u < list->getNumItems(); u++)
        {
            if (list->isItemSelected(u))
            {
                pathname = list->getItemFullPathname(u);
                itemslist += ::quote(pathname) + " ";
            }
        }

        // Open a dialog to select the other item to be compared
        if (comparedialog == NULL)
        {
            comparedialog = new BrowseInputDialog(this, "", "", _("Compare"), _("With:"), bigcompareicon, BROWSE_INPUT_FILE);
        }
        comparedialog->setIcon(bigcompareicon);
        comparedialog->setMessage(pathname);
        comparedialog->setText("");
        int rc = 1;
        rc = comparedialog->execute(PLACEMENT_CURSOR);

        // Get item path and add it to the list
        FXString str = comparedialog->getText();
        itemslist += ::quote(str);
        if (!rc || (str == ""))
        {
            return(0);
        }
    }

    // Two selected items
    else if (num == 2)
    {
        // Get the two selected items
        for (int u = 0; u < list->getNumItems(); u++)
        {
            if (list->isItemSelected(u))
            {
                pathname = list->getItemFullPathname(u);
                itemslist += ::quote(pathname) + " ";
            }
        }
    }

    // Wait cursor
    getApp()->beginWaitCursor();
    mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_ANIMTHROB), NULL);

    // If command exists, run it
    cmdname = filecomparator;
    if (::existCommand(cmdname))
    {
        cmd = cmdname+itemslist;
#ifdef STARTUP_NOTIFICATION
        runcmd(cmd, cmdname, searchdir, searchdir, usesn, snexcepts);
#else
        runcmd(cmd, searchdir, searchdir);
#endif
    }

    // If command does not exist, issue an error message
    else
    {
        mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
        getApp()->endWaitCursor();
        MessageBox::error(this, BOX_OK, _("Error"), _("Program %s not found. Please define a file comparator program in the Preferences dialog!"), cmdname.text());
    }

    mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
    getApp()->endWaitCursor();

    return(1);
}


// Force panel refresh
long SearchPanel::onCmdRefresh(FXObject* sender, FXSelector, void*)
{
    list->onCmdRefresh(0, 0, 0);

    return(1);
}


// Go to parent directory
long SearchPanel::onCmdGotoParentdir(FXObject*, FXSelector, void*)
{
    if (list->getNumSelectedItems() != 1)
    {
        return(0);
    }

    // Get selected item path name
    FXString pathname;
    for (int u = 0; u < list->getNumItems(); u++)
    {
        if (list->isItemSelected(u))
        {
            pathname = list->getItemFullPathname(u);
            break;
        }
    }

    // Parent directory name
    FXString parentdir = FXPath::directory(pathname);

    // Does not have access
    if (!::isReadExecutable(parentdir))
    {
        MessageBox::error(this, BOX_OK_SU, _("Error"), _(" Permission to: %s denied."), parentdir.text());
        mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
        getApp()->endWaitCursor();
        return(0);
    }

    // Change directory in Xfe
    ((XFileExplorer*)mainWindow)->setDirectory(parentdir);

    // Raise the Xfe window
    ((XFileExplorer*)mainWindow)->raise();
    ((XFileExplorer*)mainWindow)->setFocus();

    // Warning message when setting current folder in Xfe
    FXbool warn = getApp()->reg().readUnsignedEntry("OPTIONS", "folder_warn", true);
    if (warn)
    {
        MessageBox::information(((XFileExplorer*)mainWindow), BOX_OK, _("Information"), _("Current folder has been set to '%s'"), parentdir.text());
    }

    return(1);
}


// File or directory properties
long SearchPanel::onCmdProperties(FXObject* sender, FXSelector, void*)
{
    int num, itm;

    // There is one selected file in the file list
    num = list->getNumSelectedItems(&itm);
    if (num == 1)
    {
        FXString filename = list->getItemFilename(itm);
        FXString pathname = FXPath::directory(list->getItemFullPathname(itm));

        PropertiesBox* attrdlg = new PropertiesBox(this, filename, pathname);
        attrdlg->create();
        attrdlg->show(PLACEMENT_OWNER);
    }

    // There are multiple selected files in the file list
    else if (num > 1)
    {
        FXString* files = new FXString[num];
        FXString* paths = new FXString[num];

        int i = 0;
        for (int u = 0; u < list->getNumItems(); u++)
        {
            if (list->isItemSelected(u))
            {
                files[i] = list->getItemText(u).section('\t', 0);
                paths[i] = FXPath::directory(list->getItemFullPathname(u));
                i++;
            }
        }

        PropertiesBox* attrdlg = new PropertiesBox(this, files, num, paths);
        attrdlg->create();
        attrdlg->show(PLACEMENT_OWNER);
    }
    return(1);
}


// Handle item selection
long SearchPanel::onCmdSelect(FXObject* sender, FXSelector sel, void* ptr)
{
    switch (FXSELID(sel))
    {
    case ID_SELECT_ALL:
        list->handle(sender, FXSEL(SEL_COMMAND, FileList::ID_SELECT_ALL), ptr);
        return(1);

    case ID_DESELECT_ALL:
        list->handle(sender, FXSEL(SEL_COMMAND, FileList::ID_DESELECT_ALL), ptr);
        return(1);

    case ID_SELECT_INVERSE:
        list->handle(sender, FXSEL(SEL_COMMAND, FileList::ID_SELECT_INVERSE), ptr);
        return(1);
    }
    return(1);
}


// Set search root path
void SearchPanel::setSearchPath(FXString path)
{
    searchdir = path;
    list->setDirectory(path, false);
}


// Append an item to the file list
// Note that thumbnails are not displayed here
long SearchPanel::appendItem(FXString& pathname)
{
    FXString    filename, dirname;
    FXString    grpid, usrid, atts, mod, ext, del;
    FileAssoc*  fileassoc;
    FXString    filetype, lowext;
    FXIcon*     big, *mini;
    time_t      filemtime, filectime;
    struct stat info, linfo;
    FXbool      isLink, isBrokenLink, isDir;

    // Only process valid file paths and paths different from the search directory
    if (lstatrep(pathname.text(), &linfo) == 0)
    {
        filename = FXPath::name(pathname);
        dirname = FXPath::directory(pathname);

        // Get file/link info and indicate if it is a link
        isLink = S_ISLNK(linfo.st_mode);
        isBrokenLink = false;

        // Find if it is a broken link
        if (isLink && (statrep(pathname.text(), &info) != 0))
        {
            isBrokenLink = true;
        }

        // File times
        filemtime = linfo.st_mtime;
        filectime = linfo.st_ctime;

        // Find if it is a folder
        isDir = false;
        if (S_ISDIR(linfo.st_mode))
        {
            isDir = true;
        }

        // Extension
        ext = FXPath::extension(pathname);

        // User name
        usrid = FXSystem::userName(linfo.st_uid);

        // Group name
        grpid = FXSystem::groupName(linfo.st_gid);

        // Permissions (caution : we don't use the FXSystem::modeString() function because
        // it seems to be incompatible with the info.st_mode format)
        atts = ::permissions(linfo.st_mode);

        // Mod time
        mod = FXSystem::time("%x %X", linfo.st_mtime);
        del = "";
        ext = "";

        // Obtain the extension for files only
        if (!isDir)
        {
            ext = FXPath::extension(pathname);
        }

        // Obtain the stat info on the file itself
        if (statrep(pathname.text(), &info) != 0)
        {
            // Except in the case of a broken link
            if (isBrokenLink)
            {
                lstatrep(pathname.text(), &info);
            }
            else
            {
                goto end;
            }
        }

        // Assume no associations
        fileassoc = NULL;

        // Determine icons and type
        if (isDir)
        {
            if (!::isReadExecutable(pathname))
            {
                big = bigfolderlockedicon;
                mini = minifolderlockedicon;
                filetype = _("Folder");
            }
            else
            {
                big = bigfoldericon;
                mini = minifoldericon;
                filetype = _("Folder");
            }
        }
        else if (S_ISCHR(info.st_mode))
        {
            big = bigchardevicon;
            mini = minichardevicon;
            filetype = _("Character Device");
        }
        else if (S_ISBLK(info.st_mode))
        {
            big = bigblockdevicon;
            mini = miniblockdevicon;
            filetype = _("Block Device");
        }
        else if (S_ISFIFO(info.st_mode))
        {
            big = bigpipeicon;
            mini = minipipeicon;
            filetype = _("Named Pipe");
        }
        else if (S_ISSOCK(info.st_mode))
        {
            big = bigsocketicon;
            mini = minisocketicon;
            filetype = _("Socket");
        }
        else if ((info.st_mode&(S_IXUSR|S_IXGRP|S_IXOTH)) && !(S_ISDIR(info.st_mode) || S_ISCHR(info.st_mode) || S_ISBLK(info.st_mode) || S_ISFIFO(info.st_mode) || S_ISSOCK(info.st_mode)))
        {
            big = bigexecicon;
            mini = miniexecicon;
            filetype = _("Executable");
            if (associations)
            {
                fileassoc = associations->findFileBinding(pathname.text());
            }
        }
        else
        {
            big = bigdocicon;
            mini = minidocicon;
            filetype = _("Document");
            if (associations)
            {
                fileassoc = associations->findFileBinding(pathname.text());
            }
        }

        // If association is found, use it
        if (fileassoc)
        {
            filetype = fileassoc->extension.text();

            if (fileassoc->bigicon)
            {
                big = fileassoc->bigicon;
            }
            if (fileassoc->miniicon)
            {
                mini = fileassoc->miniicon;
            }
        }

        // Symbolic links have a specific type
        if (isBrokenLink)
        {
            filetype = _("Broken link");
        }

        else if (isLink)
        {
            if (associations)
            {
                // Don't forget to remove trailing '/' here!
                fileassoc = associations->findFileBinding(::cleanPath(::readLink(pathname)).text());
                if (fileassoc)
                {
                    filetype = _("Link to ")+fileassoc->extension;
                }
                else
                {
                    filetype = _("Link to ")+filetype;
                }
            }
        }

        // Don't display the file size for directories
        FXString hsize;
        if (isDir)
        {
            hsize = "";
        }
        else
        {
            char size[64];
#if __WORDSIZE == 64
            snprintf(size, sizeof(size)-1, "%lu", (FXulong)linfo.st_size);
#else
            snprintf(size, sizeof(size)-1, "%llu", (FXulong)linfo.st_size);
#endif
            hsize = ::hSize(size);
        }

#if defined(linux)
        // Devices have a specific icon
        if (fsdevices->find(pathname.text()))
        {
            filetype = _("Mount point");

            if (::streq(fsdevices->find(pathname.text()), "harddisk"))
            {
                big = bigharddiskicon;
                mini = harddiskicon;
            }
            else if (::streq(fsdevices->find(pathname.text()), "nfsdisk"))
            {
                big = bignfsdriveicon;
                mini = nfsdriveicon;
            }
            else if (::streq(fsdevices->find(pathname.text()), "smbdisk"))
            {
                big = bignfsdriveicon;
                mini = nfsdriveicon;
            }
            else if (::streq(fsdevices->find(pathname.text()), "floppy"))
            {
                big = bigfloppyicon;
                mini = floppyicon;
            }
            else if (::streq(fsdevices->find(pathname.text()), "cdrom"))
            {
                big = bigcdromicon;
                mini = cdromicon;
            }
            else if (::streq(fsdevices->find(pathname.text()), "zip"))
            {
                big = bigzipicon;
                mini = zipicon;
            }
        }
#endif

        // Symbolic links have a specific icon
        if (isLink)
        {
            // Broken link
            if (isBrokenLink)
            {
                big = bigbrokenlinkicon;
                mini = minibrokenlinkicon;
            }
            else
            {
                big = biglinkicon;
                mini = minilinkicon;
            }
        }

        // Add item to the file list
        FXString str = filename + "\t" + dirname + "\t" + hsize + "\t" + filetype + "\t" + ext + "\t" + mod + "\t" + usrid +"\t" + grpid + "\t" + atts + "\t" + del + "\t" + pathname;

        // Append item to the list
        list->appendItem(str, big, mini);

        // Get last item
        int       count = list->getNumItems();
        FileItem* item = (FileItem*)list->getItem(count-1);

        if (item == NULL)
        {
            fprintf(stderr, "%s::appendItem: NULL item specified.\n", getClassName());
            exit(EXIT_FAILURE);
        }

        // Set icons
        item->setBigIcon(big, false);
        item->setMiniIcon(mini, false);

        // Set item flags from the obtained info
        if (S_ISDIR(info.st_mode))
        {
            item->state |= FileItem::FOLDER;
        }
        else
        {
            item->state &= ~FileItem::FOLDER;
        }
        if (S_ISLNK(info.st_mode))
        {
            item->state |= FileItem::SYMLINK;
        }
        else
        {
            item->state &= ~FileItem::SYMLINK;
        }
        if (S_ISCHR(info.st_mode))
        {
            item->state |= FileItem::CHARDEV;
        }
        else
        {
            item->state &= ~FileItem::CHARDEV;
        }
        if (S_ISBLK(info.st_mode))
        {
            item->state |= FileItem::BLOCKDEV;
        }
        else
        {
            item->state &= ~FileItem::BLOCKDEV;
        }
        if (S_ISFIFO(info.st_mode))
        {
            item->state |= FileItem::FIFO;
        }
        else
        {
            item->state &= ~FileItem::FIFO;
        }
        if (S_ISSOCK(info.st_mode))
        {
            item->state |= FileItem::SOCK;
        }
        else
        {
            item->state &= ~FileItem::SOCK;
        }
        if ((info.st_mode&(S_IXUSR|S_IXGRP|S_IXOTH)) && !(S_ISDIR(info.st_mode) || S_ISCHR(info.st_mode) || S_ISBLK(info.st_mode) || S_ISFIFO(info.st_mode) || S_ISSOCK(info.st_mode)))
        {
            item->state |= FileItem::EXECUTABLE;
        }
        else
        {
            item->state &= ~FileItem::EXECUTABLE;
        }

        // We can drag items
        item->state |= FileItem::DRAGGABLE;

        // Set item attributes
        item->size = (FXulong)linfo.st_size;
        item->assoc = fileassoc;
        item->date = filemtime;
        item->cdate = filectime;

        // And finally, don't forget to create the appended item!
        item->create();
    }
    else
    {
        return(0);
    }
end:

    return(1);
}


// File list context menu
long SearchPanel::onCmdPopupMenu(FXObject* o, FXSelector s, void* p)
{
    // No item in list
    if (list->getNumItems() == 0)
    {
        return(0);
    }

    list->setAllowRefresh(false);

    // Check if control key was pressed
    ctrlflag = false;
    shiftf10 = false;
    if (p != NULL)
    {
        FXEvent* event = (FXEvent*)p;
        if (event->state&CONTROLMASK)
        {
            ctrlflag = true;
        }
        if (event->state&SHIFTMASK && (event->code == KEY_F10))
        {
            shiftf10 = true;
        }
    }

    // Use to select the item under cursor when right clicking
    // Only when Shift-F10 was not pressed
    if (!shiftf10 && (list->getNumSelectedItems() <= 1))
    {
        int    x, y;
        FXuint state;
        list->getCursorPosition(x, y, state);

        int item = list->getItemAt(x, y);

        if (list->getCurrentItem() >= 0)
        {
            list->deselectItem(list->getCurrentItem());
        }
        if (item >= 0)
        {
            list->setCurrentItem(item);
            list->selectItem(item);
        }
    }

    // If control flag is set, deselect all items
    if (ctrlflag)
    {
        list->handle(o, FXSEL(SEL_COMMAND, FileList::ID_DESELECT_ALL), p);
    }

    // Popup menu pane
    FXMenuPane* menu = new FXMenuPane(this);
    int         x, y;
    FXuint      state;
    getRoot()->getCursorPosition(x, y, state);

    int num, itm;
    num = list->getNumSelectedItems(&itm);

    // No selection or control key was pressed
    if ((num == 0) || ctrlflag)
    {
        // Reset the control flag
        ctrlflag = false;

        new FXMenuCheck(menu, _("Thum&bnails"), list, FileList::ID_TOGGLE_THUMBNAILS);
        new FXMenuSeparator(menu);
        new FXMenuRadio(menu, _("B&ig icons"), list, IconList::ID_SHOW_BIG_ICONS);
        new FXMenuRadio(menu, _("&Small icons"), list, IconList::ID_SHOW_MINI_ICONS);
        new FXMenuRadio(menu, _("F&ull file list"), list, IconList::ID_SHOW_DETAILS);
        new FXMenuSeparator(menu);
        new FXMenuRadio(menu, _("&Rows"), list, FileList::ID_ARRANGE_BY_ROWS);
        new FXMenuRadio(menu, _("&Columns"), list, FileList::ID_ARRANGE_BY_COLUMNS);
        new FXMenuCheck(menu, _("Autosize"), list, FileList::ID_AUTOSIZE);
        new FXMenuSeparator(menu);
        new FXMenuRadio(menu, _("&Name"), list, FileList::ID_SORT_BY_NAME);
        new FXMenuRadio(menu, _("Si&ze"), list, FileList::ID_SORT_BY_SIZE);
        new FXMenuRadio(menu, _("&Type"), list, FileList::ID_SORT_BY_TYPE);
        new FXMenuRadio(menu, _("E&xtension"), list, FileList::ID_SORT_BY_EXT);
        new FXMenuRadio(menu, _("&Date"), list, FileList::ID_SORT_BY_TIME);
        new FXMenuRadio(menu, _("&User"), list, FileList::ID_SORT_BY_USER);
        new FXMenuRadio(menu, _("&Group"), list, FileList::ID_SORT_BY_GROUP);
        new FXMenuRadio(menu, _("Per&missions"), list, FileList::ID_SORT_BY_PERM);
        new FXMenuSeparator(menu);
        new FXMenuCheck(menu, _("I&gnore case"), list, FileList::ID_SORT_CASE);
        new FXMenuCheck(menu, _("Fold&ers first"), list, FileList::ID_DIRS_FIRST);
        new FXMenuCheck(menu, _("Re&verse order"), list, FileList::ID_SORT_REVERSE);
    }
    // Non empty selection
    else
    {
        // Submenu items
        FXMenuPane* submenu = new FXMenuPane(this);
        new FXMenuCheck(submenu, _("Thum&bnails"), list, FileList::ID_TOGGLE_THUMBNAILS);
        new FXMenuSeparator(submenu);
        new FXMenuRadio(submenu, _("B&ig icons"), list, IconList::ID_SHOW_BIG_ICONS);
        new FXMenuRadio(submenu, _("&Small icons"), list, IconList::ID_SHOW_MINI_ICONS);
        new FXMenuRadio(submenu, _("&Full file list"), list, IconList::ID_SHOW_DETAILS);
        new FXMenuSeparator(submenu);
        new FXMenuRadio(submenu, _("&Rows"), list, FileList::ID_ARRANGE_BY_ROWS);
        new FXMenuRadio(submenu, _("&Columns"), list, FileList::ID_ARRANGE_BY_COLUMNS);
        new FXMenuCheck(submenu, _("&Autosize"), list, FileList::ID_AUTOSIZE);
        new FXMenuSeparator(submenu);
        new FXMenuRadio(submenu, _("&Name"), list, FileList::ID_SORT_BY_NAME);
        new FXMenuRadio(submenu, _("Si&ze"), list, FileList::ID_SORT_BY_SIZE);
        new FXMenuRadio(submenu, _("&Type"), list, FileList::ID_SORT_BY_TYPE);
        new FXMenuRadio(submenu, _("E&xtension"), list, FileList::ID_SORT_BY_EXT);
        new FXMenuRadio(submenu, _("&Date"), list, FileList::ID_SORT_BY_TIME);
        new FXMenuRadio(submenu, _("&User"), list, FileList::ID_SORT_BY_USER);
        new FXMenuRadio(submenu, _("&Group"), list, FileList::ID_SORT_BY_GROUP);
        new FXMenuRadio(submenu, _("Per&missions"), list, FileList::ID_SORT_BY_PERM);
        new FXMenuSeparator(submenu);
        new FXMenuCheck(submenu, _("Ignore c&ase"), list, FileList::ID_SORT_CASE);
        new FXMenuCheck(submenu, _("Fold&ers first"), list, FileList::ID_DIRS_FIRST);
        new FXMenuCheck(submenu, _("Re&verse order"), list, FileList::ID_SORT_REVERSE);
        new FXMenuCascade(menu, _("Pane&l"), NULL, submenu);
        new FXMenuSeparator(menu);


        FXbool ar = false;
        if (list->getItem(itm) && list->isItemFile(itm))
        {
            new FXMenuCommand(menu, _("Open &with..."), fileopenicon, this, SearchPanel::ID_OPEN_WITH);
            new FXMenuCommand(menu, _("&Open"), fileopenicon, this, SearchPanel::ID_OPEN);
            FXString name = this->list->getItemText(itm).section('\t', 0);

            // Last and before last file extensions
            FXString ext1 = name.rafter('.', 1).lower();
            FXString ext2 = name.rafter('.', 2).lower();

            // Display the extract and package menus according to the archive extensions
            if ((num == 1) && ((ext2 == "tar.gz") || (ext2 == "tar.bz2") || (ext2 == "tar.xz") || (ext2 == "tar.z")))
            {
                ar = true;
                new FXMenuCommand(menu, _("E&xtract to..."), archexticon, this, SearchPanel::ID_EXTRACT);
            }
            else if ((num == 1) && ((ext1 == "gz") || (ext1 == "bz2") || (ext1 == "xz") || (ext1 == "z")))
            {
                ar = true;
                new FXMenuCommand(menu, _("&Extract here"), archexticon, this, SearchPanel::ID_EXTRACT);
            }
            else if ((num == 1) && ((ext1 == "tar") || (ext1 == "tgz") || (ext1 == "tbz2") || (ext1 == "tbz") || (ext1 == "taz") || (ext1 == "txz") || (ext1 == "zip") || (ext1 == "7z") || (ext1 == "lzh") || (ext1 == "rar") || (ext1 == "ace") || (ext1 == "arj")))
            {
                ar = true;
                new FXMenuCommand(menu, _("E&xtract to..."), archexticon, this, SearchPanel::ID_EXTRACT);
            }
#if defined(linux)
            else if ((num == 1) && ((ext1 == "rpm") || (ext1 == "deb")))
            {
                ar = true;
                new FXMenuCommand(menu, _("&View"), packageicon, this, SearchPanel::ID_VIEW);
            }
#endif
            // Not archive nor package
            if (!ar)
            {
                new FXMenuCommand(menu, _("&View"), viewicon, this, SearchPanel::ID_VIEW);
                new FXMenuCommand(menu, _("&Edit"), editicon, this, SearchPanel::ID_EDIT);
                if (num == 1)
                {
                    new FXMenuCommand(menu, _("Com&pare..."), compareicon, this, SearchPanel::ID_COMPARE);
                }
                else
                {
                    new FXMenuCommand(menu, _("Com&pare"), compareicon, this, SearchPanel::ID_COMPARE);
                }
            }
        }
        if (!ar)
        {
            new FXMenuCommand(menu, _("&Add to archive..."), archaddicon, this, SearchPanel::ID_ADD_TO_ARCH);
        }
#if defined(linux)
        if ((num == 1) && !ar)
        {
            new FXMenuCommand(menu, _("&Packages query "), packageicon, this, SearchPanel::ID_PKG_QUERY);
        }
#endif

        // Build scripts menu
        new FXMenuSeparator(menu);
        FXString    scriptpath = homedir + PATHSEPSTRING CONFIGPATH PATHSEPSTRING XFECONFIGPATH PATHSEPSTRING SCRIPTPATH;
        FXMenuPane* scriptsmenu = new FXMenuPane(this);
        new FXMenuCascade(menu, _("Scripts"), runicon, scriptsmenu);
        readScriptDir(scriptsmenu, scriptpath);
        new FXMenuSeparator(scriptsmenu);
        new FXMenuCommand(scriptsmenu, _("&Go to script folder"), gotodiricon, this, SearchPanel::ID_GO_SCRIPTDIR);

        new FXMenuSeparator(menu);
        new FXMenuCommand(menu, _("&Go to parent folder"), gotodiricon, this, SearchPanel::ID_GOTO_PARENTDIR);
        new FXMenuCommand(menu, _("&Copy"), copy_clpicon, this, SearchPanel::ID_COPY_CLIPBOARD);
        new FXMenuCommand(menu, _("C&ut"), cut_clpicon, this, SearchPanel::ID_CUT_CLIPBOARD);
        new FXMenuSeparator(menu);
        new FXMenuCommand(menu, _("Re&name..."), renameiticon, this, SearchPanel::ID_FILE_RENAME);
        new FXMenuCommand(menu, _("Copy &to..."), copy_clpicon, this, SearchPanel::ID_FILE_COPYTO);
        new FXMenuCommand(menu, _("&Move to..."), moveiticon, this, SearchPanel::ID_FILE_MOVETO);
        new FXMenuCommand(menu, _("Symlin&k to..."), minilinkicon, this, SearchPanel::ID_FILE_SYMLINK);
        new FXMenuCommand(menu, _("M&ove to trash"), filedeleteicon, this, SearchPanel::ID_FILE_TRASH);
        new FXMenuCommand(menu, _("&Delete"), filedelete_permicon, this, SearchPanel::ID_FILE_DELETE);
        new FXMenuSeparator(menu);
        new FXMenuCommand(menu, _("Compare &sizes"), charticon, this, SearchPanel::ID_DIR_USAGE);
        new FXMenuCommand(menu, _("P&roperties"), attribicon, this, SearchPanel::ID_PROPERTIES);
    }
    menu->create();
    allowPopupScroll = true;  // Allow keyboard scrolling
    menu->popup(NULL, x, y);
    getApp()->runModalWhileShown(menu);
    allowPopupScroll = false;
    list->setAllowRefresh(true);

    return(1);
}


// Read all executable file names that are located into the script directory
// Sort entries alphabetically
int SearchPanel::readScriptDir(FXMenuPane* scriptsmenu, FXString dir)
{
    DIR*            dp;
    struct dirent** namelist;

    // Open directory
    if ((dp = opendir(dir.text())) == NULL)
    {
        return(0);
    }

    // Eventually add a / at the end of the directory name
    if (dir[dir.length()-1] != '/')
    {
        dir = dir+"/";
    }

    // Read directory and sort entries alphabetically
    int n = scandir(dir.text(), &namelist, NULL, alphasort);
    if (n < 0)
    {
        perror("scandir");
    }
    else
    {
        for (int k = 0; k < n; k++)
        {
            // Avoid hidden directories and '.' and '..'
            if (namelist[k]->d_name[0] != '.')
            {
                FXString pathname = dir + namelist[k]->d_name;

                // Recurse if non empty directory
                if (::isDirectory(pathname))
                {
                    if (!::isEmptyDir(pathname))
                    {
                        FXMenuPane* submenu = new FXMenuPane(this);
                        new FXMenuCascade(scriptsmenu, namelist[k]->d_name, NULL, submenu);
                        readScriptDir(submenu, pathname);
                    }
                }

                // Add only executable files to the list
                else if (isReadExecutable(pathname))
                {
                    new FXMenuCommand(scriptsmenu, namelist[k]->d_name + FXString("\t\t") + pathname, miniexecicon, this, FilePanel::ID_RUN_SCRIPT);
                }
            }
            free(namelist[k]);
        }
        free(namelist);
    }

    // Close directory
    (void)closedir(dp);

    return(1);
}


// Add files or directory to an archive
long SearchPanel::onCmdAddToArch(FXObject* o, FXSelector, void*)
{
    int      ret;
    FXString name, ext1, ext2, cmd, archive = "";
    File*    f;

    // Enter search directory
    ret = chdir(searchdir.text());
    if (ret < 0)
    {
        int errcode = errno;
        if (errcode)
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s: %s"), searchdir.text(), strerror(errcode));
        }
        else
        {
            MessageBox::error(this, BOX_OK, _("Error"), _("Can't enter folder %s"), searchdir.text());
        }

        return(0);
    }

    // If only one item is selected, use its name as a starting guess for the archive name
    if (list->getNumSelectedItems() == 1)
    {
        for (int u = 0; u < list->getNumItems(); u++)
        {
            if (list->isItemSelected(u))
            {
                name = list->getItemFilename(u);
                break;
            }
        }
        archive = name;
    }

    // Initial archive name with full path and default extension
    archive = homedir+PATHSEPSTRING+archive+".tar.gz";

    // Archive dialog
    if (archdialog == NULL)
    {
        archdialog = new ArchInputDialog(this, "");
    }
    archdialog->setText(archive);
    archdialog->CursorEnd();

    if (archdialog->execute())
    {
        if (archdialog->getText() == "")
        {
            MessageBox::warning(this, BOX_OK, _("Warning"), _("File name is empty, operation cancelled"));
            return(0);
        }

        // Get string and preserve escape characters
        archive = ::quote(archdialog->getText());

        // Get extensions of the archive name
        ext1 = archdialog->getText().rafter('.', 1).lower();
        ext2 = archdialog->getText().rafter('.', 2).lower();

        // Handle different archive formats
        if (ext2 == "tar.gz")
        {
            cmd = "tar -zcvf "+archive+" ";
        }
        else if (ext2 == "tar.bz2")
        {
            cmd = "tar -jcvf "+archive+" ";
        }
        else if (ext2 == "tar.xz")
        {
            cmd = "tar -Jcvf "+archive+" ";
        }
        else if (ext2 == "tar.z")
        {
            cmd = "tar -Zcvf "+archive+" ";
        }
        else if (ext1 == "tar")
        {
            cmd = "tar -cvf "+archive+" ";
        }
        else if (ext1 == "gz")
        {
            cmd = "gzip -v ";
        }
        else if (ext1 == "tgz")
        {
            cmd = "tar -zcvf "+archive+" ";
        }
        else if (ext1 == "taz")
        {
            cmd = "tar -Zcvf "+archive+" ";
        }
        else if (ext1 == "bz2")
        {
            cmd = "bzip2 -v ";
        }
        else if (ext1 == "xz")
        {
            cmd = "xz -v ";
        }
        else if ((ext1 == "tbz2") || (ext1 == "tbz"))
        {
            cmd = "tar -jcvf "+archive+" ";
        }
        else if (ext1 == "txz")
        {
            cmd = "tar -Jcvf "+archive+" ";
        }
        else if (ext1 == "z")
        {
            cmd = "compress -v ";
        }
        else if (ext1 == "zip")
        {
            cmd = "zip -r "+archive+" ";
        }
        else if (ext1 == "7z")
        {
            cmd = "7z a "+archive+" ";
        }

        // Default archive format
        else
        {
            archive += ".tar.gz";
            cmd = "tar -zcvf "+archive+" ";
        }

        for (int u = 0; u < list->getNumItems(); u++)
        {
            if (list->isItemSelected(u))
            {
                name = FXPath::relative(searchdir, list->getItemFullPathname(u));
                cmd += " ";
                cmd = cmd+::quote(name);
                cmd += " ";
            }
        }

        // Wait cursor
        mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_ANIMTHROB), NULL);
        getApp()->beginWaitCursor();

        // File object
        f = new File(this, _("Create archive"), ARCHIVE);
        f->create();

        // Create archive
        f->archive(archive, cmd);

        getApp()->endWaitCursor();
        mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
        delete f;
    }
    return(1);
}


// Extract archive
long SearchPanel::onCmdExtract(FXObject*, FXSelector, void*)
{
    FXString name, ext1, ext2, cmd, dir;
    File*    f;

    // File selection dialog
    FileDialog  browse(this, _("Select a destination folder"));
    const char* patterns[] =
    {
        _("All Files"), "*", NULL
    };

    browse.setDirectory(homedir);
    browse.setPatternList(patterns);
    browse.setSelectMode(SELECT_FILE_DIRECTORY);

    int item;
    list->getNumSelectedItems(&item);
    if (list->getItem(item))
    {
        // Path
        FXString path = FXPath::directory(list->getItemFullPathname(item));

        // Archive name and extensions
        name = list->getItemText(item).text();
        ext1 = name.section('\t', 0).rafter('.', 1).lower();
        ext2 = name.section('\t', 0).rafter('.', 2).lower();
        name = ::quote(path + PATHSEPSTRING + name.section('\t', 0));

        // Handle different archive formats
        FXbool dialog = true;

        if (ext2 == "tar.gz")
        {
            cmd = "tar -zxvf ";
        }
        else if (ext2 == "tar.bz2")
        {
            cmd = "tar -jxvf ";
        }
        else if (ext2 == "tar.xz")
        {
            cmd = "tar -Jxvf ";
        }
        else if (ext2 == "tar.z")
        {
            cmd = "tar -Zxvf ";
        }
        else if (ext1 == "tar")
        {
            cmd = "tar -xvf ";
        }
        else if (ext1 == "gz")
        {
            cmd = "gunzip -v ";
            dialog = false;
        }
        else if (ext1 == "tgz")
        {
            cmd = "tar -zxvf ";
        }
        else if (ext1 == "taz")
        {
            cmd = "tar -Zxvf ";
        }
        else if (ext1 == "bz2")
        {
            cmd = "bunzip2 -v ";
            dialog = false;
        }
        else if (ext1 == "xz")
        {
            cmd = "unxz -v ";
            dialog = false;
        }
        else if ((ext1 == "tbz2") || (ext1 == "tbz"))
        {
            cmd = "tar -jxvf ";
        }
        else if (ext1 == "txz")
        {
            cmd = "tar -Jxvf ";
        }
        else if (ext1 == "z")
        {
            cmd = "uncompress -v ";
            dialog = false;
        }
        else if (ext1 == "zip")
        {
            cmd = "unzip -o ";
        }
        else if (ext1 == "7z")
        {
            cmd = "7z x -y ";
        }
        else if (ext1 == "rar")
        {
            cmd = "unrar x -o+ ";
        }
        else if (ext1 == "lzh")
        {
            cmd = "lha -xf ";
        }
        else if (ext1 == "ace")
        {
            cmd = "unace x ";
        }
        else if (ext1 == "arj")
        {
            cmd = "arj x -y ";
        }
        else
        {
            cmd = "tar -zxvf ";
        }

        // Final extract command
        cmd += name+" ";

        // Extract with file dialog
        if (dialog)
        {
            // Extract archive
            if (browse.execute())
            {
                dir = browse.getFilename();

                if (isWritable(dir))
                {
                    // Wait cursor
                    getApp()->beginWaitCursor();
		    mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_ANIMTHROB), NULL);

                    // File object
                    f = new File(this, _("Extract archive"), EXTRACT);
                    f->create();

                    // Extract archive
                    f->extract(name, dir, cmd);

		    mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
                    getApp()->endWaitCursor();
                    delete f;
                }
                else
                {
                    MessageBox::error(this, BOX_OK_SU, _("Error"), _("Can't write to %s: Permission denied"), dir.text());
                }
            }
        }

        // Extract here (without file dialog)
        else
        {
            if (isWritable(path))
            {
                // Wait cursor
	        mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_ANIMTHROB), NULL);
                getApp()->beginWaitCursor();

                // File object
                f = new File(this, _("Extract archive"), EXTRACT);
                f->create();

                // Extract archive
                f->extract(name, path, cmd);

                getApp()->endWaitCursor();
	        mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
                delete f;
            }
            else
            {
                MessageBox::error(this, BOX_OK_SU, _("Error"), _("Can't write to %s: Permission denied"), path.text());
            }
        }
    }

    return(1);
}


// Directory usage on file selection
long SearchPanel::onCmdDirUsage(FXObject* o, FXSelector, void*)
{
	FXString pathname, command, itemslist = " ";
	FXString cmd1 = "/usr/bin/du --apparent-size -k -s ";
	FXString cmd2 = " 2> /dev/null | /usr/bin/sort -rn | /usr/bin/cut -f2 | /usr/bin/xargs -d '\n' /usr/bin/du --apparent-size --total --si -s 2> /dev/null";

    // Construct selected files list
    for (int u = 0; u < list->getNumItems(); u++)
    {
        if (list->isItemSelected(u))
        {
 			pathname = list->getItemFullPathname(u);

            // List of selected items
            itemslist += ::quote(pathname) + " ";
        }
    }

	// Command to be executed
	command = cmd1 + itemslist + cmd2;

	// Make and show command window
	CommandWindow* cmdwin=new CommandWindow(getApp(),_("Sizes of Selected Items"),command,25,50);
	cmdwin->create();
	cmdwin->setIcon(charticon);

    return(1);
}


// Trash files from the file list
long SearchPanel::onCmdFileTrash(FXObject*, FXSelector, void*)
{
    int   firstitem = 0;
    File* f = NULL;

    FXbool confirm_trash = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_trash", true);

    // If we don't have permission to write to the trash directory
    if (!::isWritable(trashfileslocation))
    {
        MessageBox::error(this, BOX_OK_SU, _("Error"), _("Can't write to trash location %s: Permission denied"), trashfileslocation.text());
        return(0);
    }

    // Items number in the file list
    int num = list->getNumSelectedItems();
    if (num < 1)
    {
        return(0);
    }

    if (confirm_trash)
    {
        FXString message;
        if (num == 1)
        {
            FXString pathname;
            for (int u = 0; u < list->getNumItems(); u++)
            {
                if (list->isItemSelected(u))
                {
                    pathname = list->getItemFullPathname(u);
                }
            }
            if (::isDirectory(pathname))
            {
                message.format(_("Move folder %s to trash can?"), pathname.text());
            }
            else
            {
                message.format(_("Move file %s to trash can?"), pathname.text());
            }
        }
        else
        {
            message.format(_("Move %s selected items to trash can?"), FXStringVal(num).text());
        }

        MessageBox box(this, _("Confirm Trash"), message, delete_bigicon, BOX_OK_CANCEL|DECOR_TITLE|DECOR_BORDER);
        if (box.execute(PLACEMENT_CURSOR) != BOX_CLICKED_OK)
        {
            return(0);
        }
    }

    // Wait cursor
    getApp()->beginWaitCursor();
    mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_ANIMTHROB), NULL);

    // File object
    f = new File(this, _("Move to trash"), DELETE, num);
    f->create();
    list->setAllowRefresh(false);

    // Overwrite initialisations
    FXbool overwrite = false;
    FXbool overwrite_all = false;
    FXbool skip_all = false;

    // Delete selected files
    FXString filename, pathname;
    for (int u = 0; u < list->getNumItems(); u++)
    {
        if (list->isItemSelected(u))
        {
            // Get index of first selected item
            if (firstitem == 0)
            {
                firstitem = u;
            }

            // Get file name and path
            filename = list->getItemFilename(u);
            pathname = list->getItemFullPathname(u);

            // File could have already been trashed above in the tree
            if (!::exists(pathname))
            {
                continue;
            }

            // If we don't have permission to write to the file
            if (!::isWritable(pathname))
            {
                // Overwrite dialog if necessary
                if (!(overwrite_all | skip_all))
                {
                    f->hideProgressDialog();
                    FXString msg;
                    msg.format(_("File %s is write-protected, move it anyway to trash can?"), pathname.text());
  
					if (num ==1)
					{
						OverwriteBox* dlg = new OverwriteBox(this, _("Confirm Trash"), msg, OVWBOX_SINGLE_FILE);
						FXuint answer = dlg->execute(PLACEMENT_OWNER);
						delete dlg;	
						if (answer == 1)
						{
							overwrite = true;
						}
						else
						{
							goto end;
						}
					}  
					else
					{
						OverwriteBox* dlg = new OverwriteBox(this, _("Confirm Trash"), msg);
						FXuint        answer = dlg->execute(PLACEMENT_OWNER);
						delete dlg;
						switch (answer)
						{
						// Cancel
						case 0:
							goto end;
							break;

						// Overwrite
						case 1:
							overwrite = true;
							break;

						// Overwrite all
						case 2:
							overwrite_all = true;
							break;

						// Skip
						case 3:
							overwrite = false;
							break;

						// Skip all
						case 4:
							skip_all = true;
							break;
						}
					}
                }
                if ((overwrite | overwrite_all) & !skip_all)
                {
                    // Trash files path name
                    FXString trashpathname = createTrashpathname(pathname, trashfileslocation);

                    // Create trashinfo file
                    createTrashinfo(pathname, trashpathname, trashfileslocation, trashinfolocation);

                    // Move file to trash files location
                    int ret = f->move(pathname, trashpathname);

                    // An error has occurred
                    if ((ret == 0) && !f->isCancelled())
                    {
                        f->hideProgressDialog();
                        MessageBox::error(this, BOX_OK, _("Error"), _("An error has occurred during the move to trash operation!"));
                        break;
                    }
                }
                f->showProgressDialog();
            }

            // If we have permission to write
            else
            {
                // Trash files path name
                FXString trashpathname = createTrashpathname(pathname, trashfileslocation);

                // Create trashinfo file
                createTrashinfo(pathname, trashpathname, trashfileslocation, trashinfolocation);

                // Move file to trash files location
                int ret = f->move(pathname, trashpathname);

                // An error has occurred
                if ((ret == 0) && !f->isCancelled())
                {
                    f->hideProgressDialog();
                    MessageBox::error(this, BOX_OK, _("Error"), _("An error has occurred during the move to trash operation!"));
                    break;
                }

                // If action is cancelled in progress dialog
                if (f->isCancelled())
                {
                    f->hideProgressDialog();
                    MessageBox::error(this, BOX_OK, _("Warning"), _("Move to trash file operation cancelled!"));
                    break;
                }
            }
        }
    }
end:
    getApp()->endWaitCursor();
    mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
    delete f;

    list->setAllowRefresh(true);
    list->onCmdRefresh(0, 0, 0);

    return(1);
}


// Definitively delete files from the file list or the tree list (no trash can)
long SearchPanel::onCmdFileDelete(FXObject*, FXSelector, void*)
{
    int   firstitem = 0;
    File* f = NULL;

    FXbool confirm_del = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_delete", true);
    FXbool confirm_del_emptydir = getApp()->reg().readUnsignedEntry("OPTIONS", "confirm_delete_emptydir", true);


    // Items number in the file list
    int num = list->getNumSelectedItems();

    if (num == 0)
    {
        return(0);
    }

    // If exist selected files, use them
    if (num >= 1)
    {
        if (confirm_del)
        {
            FXString message;
            if (num == 1)
            {
                FXString pathname;
                for (int u = 0; u < list->getNumItems(); u++)
                {
                    if (list->isItemSelected(u))
                    {
                        pathname = list->getItemFullPathname(u);
                    }
                }
                if (::isDirectory(pathname))
                {
                    message.format(_("Definitively delete folder %s ?"), pathname.text());
                }
                else
                {
                    message.format(_("Definitively delete file %s ?"), pathname.text());
                }
            }
            else
            {
                message.format(_("Definitively delete %s selected items?"), FXStringVal(num).text());
            }
            MessageBox box(this, _("Confirm Delete"), message, delete_big_permicon, BOX_OK_CANCEL|DECOR_TITLE|DECOR_BORDER);
            if (box.execute(PLACEMENT_CURSOR) != BOX_CLICKED_OK)
            {
                return(0);
            }
        }
        // Wait cursor
        getApp()->beginWaitCursor();
	mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_ANIMTHROB), NULL);

        // File object
        f = new File(this, _("File delete"), DELETE, num);
        f->create();
        list->setAllowRefresh(false);

        // Overwrite initialisations
        FXbool overwrite = false;
        FXbool overwrite_all = false;
        FXbool skip_all = false;
        FXbool ask_del_empty = true;
        FXbool skip_all_del_emptydir = false;

        // Delete selected files
        FXString filename, pathname;
        for (int u = 0; u < list->getNumItems(); u++)
        {
            if (list->isItemSelected(u))
            {
                // Get index of first selected item
                if (firstitem == 0)
                {
                    firstitem = u;
                }

                // Get file name and path
                filename = list->getItemFilename(u);
                pathname = list->getItemFullPathname(u);

                // File could have already been deleted above in the tree
                if (!::exists(pathname))
                {
                    continue;
                }

                // Confirm empty directory deletion
                if (confirm_del & confirm_del_emptydir & ask_del_empty)
                {
                    if ((::isEmptyDir(pathname) == 0) && !::isLink(pathname))
                    {
                        if (skip_all_del_emptydir)
                        {
                            continue;
                        }

                        f->hideProgressDialog();
                        FXString msg;
                        msg.format(_("Folder %s is not empty, delete it anyway?"), pathname.text());
                        OverwriteBox* dlg = new OverwriteBox(this, _("Confirm Delete"), msg);
                        FXuint answer = dlg->execute(PLACEMENT_OWNER);
                        delete dlg;
                        switch (answer)
                        {
                        // Cancel
                        case 0:
                            goto end;
                            break;

                        // Yes
                        case 1:
                            break;

                        // Yes for all
                        case 2:
                            ask_del_empty = false;
                            break;

                        // Skip
                        case 3:
                            continue;
                            break;

                        // Skip all
                        case 4:
                            skip_all_del_emptydir = true;
                            continue;
                            break;
                        }
                        f->showProgressDialog();
                    }
                }

                // If we don't have permission to write to the file
                if (!::isWritable(pathname))
                {
                    // Overwrite dialog if necessary
                    if (!(overwrite_all | skip_all))
                    {
                        f->hideProgressDialog();
                        FXString msg;
                        msg.format(_("File %s is write-protected, delete it anyway?"), pathname.text());

                        if (num ==1)
                        {
                        	OverwriteBox* dlg = new OverwriteBox(this, _("Confirm Delete"), msg, OVWBOX_SINGLE_FILE);
							FXuint answer = dlg->execute(PLACEMENT_OWNER);
							delete dlg;
							if (answer == 1)
							{
								overwrite = true;
							}
							else
							{
								goto end;
							}							
						}
                        
                        else
                        {
							OverwriteBox* dlg = new OverwriteBox(this, _("Confirm Delete"), msg);
							FXuint answer = dlg->execute(PLACEMENT_OWNER);
							delete dlg;
							switch (answer)
							{
							// Cancel
							case 0:
								goto end;
								break;

							// Yes
							case 1:
								overwrite = true;
								break;

							// Yes for all
							case 2:
								overwrite_all = true;
								break;

							// Skip
							case 3:
								overwrite = false;
								break;

							// Skip all
							case 4:
								skip_all = true;
								break;
							}
						}
                    }
                    if ((overwrite | overwrite_all) & !skip_all)
                    {
                        // Definitively remove file or folder
                        f->remove(pathname);
                    }
                    f->showProgressDialog();
                }

                // If we have permission to write
                else
                {
                    // Definitively remove file or folder
                    f->remove(pathname);

                    // If is located at trash location, try to also remove the corresponding trashinfo file if it exists
                    // Do it silently and don't report any error if it fails
                    FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);
                    if (use_trash_can && (pathname.left(trashfileslocation.length()) == trashfileslocation))
                    {
                        FXString trashinfopathname = trashinfolocation+PATHSEPSTRING+filename+".trashinfo";
                        ::unlink(trashinfopathname.text());
                    }

                    // If action is cancelled in progress dialog
                    if (f->isCancelled())
                    {
                        f->hideProgressDialog();
                        MessageBox::error(this, BOX_OK, _("Warning"), _("Delete file operation cancelled!"));
                        break;
                    }
                }
            }
        }
end:
        getApp()->endWaitCursor();
        mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
        delete f;
    }

    list->setAllowRefresh(true);
    list->onCmdRefresh(0, 0, 0);

    return(1);
}


// We now really do have the clipboard, keep clipboard content
long SearchPanel::onClipboardGained(FXObject* sender, FXSelector sel, void* ptr)
{
    FXVerticalFrame::onClipboardGained(sender, sel, ptr);
    return(1);
}


// We lost the clipboard
long SearchPanel::onClipboardLost(FXObject* sender, FXSelector sel, void* ptr)
{
    FXVerticalFrame::onClipboardLost(sender, sel, ptr);
    return(1);
}


// Somebody wants our clipboard content
long SearchPanel::onClipboardRequest(FXObject* sender, FXSelector sel, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;
    FXuchar* data;
    FXuint   len;

    // Perhaps the target wants to supply its own data for the clipboard
    if (FXVerticalFrame::onClipboardRequest(sender, sel, ptr))
    {
        return(1);
    }

    // Clipboard target is xfelistType (Xfe, Gnome or XFCE)
    if (event->target == xfelistType)
    {
        // Prepend "copy" or "cut" as in the Gnome way and avoid duplicating these strings
        if ((clipboard.find("copy\n") < 0) && (clipboard.find("cut\n") < 0))
        {
            if (clipboard_type == CUT_CLIPBOARD)
            {
                clipboard = "cut\n" + clipboard;
            }
            else
            {
                clipboard = "copy\n" + clipboard;
            }
        }

        // Return clipboard content
        if (event->target == xfelistType)
        {
            if (!clipboard.empty())
            {
                len = clipboard.length();
                FXMEMDUP(&data, clipboard.text(), FXuchar, len);
                setDNDData(FROM_CLIPBOARD, event->target, data, len);

                // Return because xfelistType is not compatible with other types
                return(1);
            }
        }
    }

    // Clipboard target is kdelisType (KDE)
    if (event->target == kdelistType)
    {
        // The only data to be passed in this case is "0" for copy and "1" for cut
        // The uri data are passed using the standard uri-list type
        FXString flag;
        if (clipboard_type == CUT_CLIPBOARD)
        {
            flag = "1";
        }
        else
        {
            flag = "0";
        }

        // Return clipboard content
        if (event->target == kdelistType)
        {
            FXMEMDUP(&data, flag.text(), FXuchar, 1);
            setDNDData(FROM_CLIPBOARD, event->target, data, 1);
        }
    }

    // Clipboard target is urilistType (KDE apps ; non Gnome, non XFCE and non Xfe apps)
    if (event->target == urilistType)
    {
        if (!clipboard.empty())
        {
            len = clipboard.length();
            FXMEMDUP(&data, clipboard.text(), FXuchar, len);
            setDNDData(FROM_CLIPBOARD, event->target, data, len);

            return(1);
        }
    }

    // Clipboard target is utf8Type (to paste file pathes as text to other applications)
    if (event->target == utf8Type)
    {
        if (!clipboard.empty())
        {
            int      beg = 0, end = 0;
            FXString str = "";
            FXString pathname, url;

            // Clipboard don't contain 'copy\n' or 'cut\n' as first line
            if ((clipboard.find("copy\n") < 0) && (clipboard.find("cut\n") < 0))
            {
                // Remove the 'file:' prefix for each file path
                while (1)
                {
                    end = clipboard.find('\n', end);
                    if (end < 0) // Last line
                    {
                        end = clipboard.length();
                        url = clipboard.mid(beg, end-beg+1);
                        pathname = FXURL::decode(FXURL::fileFromURL(url));
                        str += pathname;
                        break;
                    }
                    url = clipboard.mid(beg, end-beg+1);
                    pathname = FXURL::decode(FXURL::fileFromURL(url));
                    str += pathname;
                    end++;
                    beg = end;
                }
                end = str.length();
            }

            // Clipboard contains 'copy\n' or 'cut\n' as first line, thus skip it
            else
            {
                // Start after the 'copy\n' or 'cut\n' prefix
                end = clipboard.find('\n', 0);
                end++;
                beg = end;

                // Remove the 'file:' prefix for each file path
                while (1)
                {
                    end = clipboard.find('\n', end);
                    if (end < 0) // Last line
                    {
                        end = clipboard.length();
                        url = clipboard.mid(beg, end-beg+1);
                        pathname = FXURL::decode(FXURL::fileFromURL(url));
                        str += pathname;
                        break;
                    }
                    url = clipboard.mid(beg, end-beg+1);
                    pathname = FXURL::decode(FXURL::fileFromURL(url));
                    str += pathname;
                    end++;
                    beg = end;
                }
                end = str.length();
            }

            if (!str.empty())
            {
                len = str.length();
                FXMEMDUP(&data, str.text(), FXuchar, len);
                setDNDData(FROM_CLIPBOARD, event->target, data, len);

                return(1);
            }
        }
    }
    return(0);
}


// Copy or cut to clipboard
long SearchPanel::onCmdCopyCut(FXObject*, FXSelector sel, void*)
{
    // Clear clipboard
    clipboard.clear();

    // Clipboard type
    if (FXSELID(sel) == ID_CUT_CLIPBOARD)
    {
        clipboard_type = CUT_CLIPBOARD;
    }
    else
    {
        clipboard_type = COPY_CLIPBOARD;
    }

    // Items number in the file list
    int num = list->getNumSelectedItems();

    if (num == 0)
    {
        return(0);
    }

    // If exist selected files, use them
    if (num >= 1)
    {
        for (int u = 0; u < list->getNumItems(); u++)
        {
            if (list->isItemSelected(u))
            {
                FXString pathname = list->getItemFullPathname(u);
                clipboard += FXURL::encode(::fileToURI(pathname))+"\n";
            }
        }
    }

    // Remove the last \n of the list, for compatibility with some file managers (e.g. nautilus 2.30.1)
    clipboard.erase(clipboard.length()-1);

    // Acquire the clipboard
    FXDragType types[4];
    types[0] = xfelistType;
    types[1] = kdelistType;
    types[2] = urilistType;
    types[3] = utf8Type;

    if (acquireClipboard(types, 4))
    {
        return(0);
    }

    return(1);
}


// Copy/Move/Rename/Symlink file(s)
long SearchPanel::onCmdFileMan(FXObject* sender, FXSelector sel, void*)
{
    int      num;
    FXString src, targetdir, target, name, source;

    // Confirmation dialog?
    FXbool ask_before_copy = getApp()->reg().readUnsignedEntry("OPTIONS", "ask_before_copy", true);

    // Number of selected items
    num = list->getNumSelectedItems();

    // If no item, return
    if (num <= 0)
    {
        return(0);
    }

    // Obtain the list of source files
    for (int u = 0; u < list->getNumItems(); u++)
    {
        if (list->isItemSelected(u))
        {
            src += list->getItemFullPathname(u)+"\n";
        }
    }

    // Name and directory of the first source file
    source = src.section('\n', 0);
    name = FXPath::name(source);
    FXString dir = FXPath::directory(source);

    // Initialise target dir name
x:
    targetdir = homedir;
    if (targetdir != ROOTDIR)
    {
        target = targetdir+PATHSEPSTRING;
    }
    else
    {
        target = targetdir;
    }

    // Target dir for the rename command
    if (FXSELID(sel) == ID_FILE_RENAME)
    {
        targetdir = dir;
    }

    // Configure the command, title, message, etc.
    FXIcon*  icon = NULL;
    FXString command, title, message;
    if (FXSELID(sel) == ID_FILE_RENAME)
    {
        command = "rename";
        title = _("Rename");
        icon = move_bigicon;
        if (num == 1)
        {
            message = _("Rename ");
            message += name;
            target = name;
            title = _("Rename");
        }
        else
        {
            return(0);
        }
    }
    if (FXSELID(sel) == ID_FILE_COPYTO)
    {
        command = "copy";
        title = _("Copy");
        icon = copy_bigicon;
        if (num == 1)
        {
            message = _("Copy ");
            message += source;
        }
        else
        {
            message.format(_("Copy %s items"), FXStringVal(num).text());
        }
    }
    if (FXSELID(sel) == ID_FILE_MOVETO)
    {
        command = "move";
        title = _("Move");
        icon = move_bigicon;
        if (num == 1)
        {
            message = _("Move ");
            message += source;
            title = _("Move");
        }
        else
        {
            message.format(_("Move %s items"), FXStringVal(num).text());
        }
    }
    if (FXSELID(sel) == ID_FILE_SYMLINK)
    {
        command = "symlink";
        title = _("Symlink");
        icon = link_bigicon;
        if (num == 1)
        {
            message = _("Symlink ");
            message += source;
            target += name;
        }
        else
        {
            message.format(_("Symlink %s items"), FXStringVal(num).text());
        }
    }

    // File operation dialog, if needed
    if (ask_before_copy || (source == target) || (FXSELID(sel) == ID_FILE_COPYTO) || (FXSELID(sel) == ID_FILE_MOVETO) || (FXSELID(sel) == ID_FILE_RENAME) || (FXSELID(sel) == ID_FILE_SYMLINK))
    {
        if (num == 1)
        {
            if (FXSELID(sel) == ID_FILE_RENAME)
            {
                if (operationdialogrename == NULL)
                {
                    operationdialogrename = new InputDialog(this, "", "", title, _("To:"), icon);
                }
                operationdialogrename->setTitle(title);
                operationdialogrename->setIcon(icon);
				operationdialogrename->setMessage(message);
                operationdialogrename->setText(target);

                if (::isDirectory(source))  // directory
                {
                    operationdialogrename->selectAll();
                }
                else
                {
                    int pos = target.rfind('.');
                    if (pos <= 0)
                    {
                        operationdialogrename->selectAll(); // no extension or dot file
                    }
                    else
                    {
                        operationdialogrename->setSelection(0, pos);
                    }
                }

                int rc = 1;
                rc = operationdialogrename->execute(PLACEMENT_CURSOR);
                target = operationdialogrename->getText();

                // Target name contains '/'
                if (target.contains(PATHSEPCHAR))
                {
					MessageBox::error(this, BOX_OK, _("Error"), _("Character '/' is not allowed in file or folder names, operation cancelled"));
					return(0);
				}

                if (!rc)
                {
                    return(0);
                }
            }
            else
            {
                if (operationdialogsingle == NULL)
                {
                    operationdialogsingle = new BrowseInputDialog(this, "", "", title, _("To:"), icon, BROWSE_INPUT_MIXED);
                }
                operationdialogsingle->setTitle(title);
                operationdialogsingle->setIcon(icon);
                operationdialogsingle->setMessage(message);
                operationdialogsingle->setText(target);


                // Select file name without path
                if (FXSELID(sel) == ID_FILE_SYMLINK)
                {
                    int pos = target.rfind(PATHSEPSTRING);
                    if (pos >= 0)
                    {
                        operationdialogsingle->setSelection(pos+1, target.length());
                    }
                }

                operationdialogsingle->setDirectory(targetdir);
                int rc = 1;
                rc = operationdialogsingle->execute(PLACEMENT_CURSOR);
                target = operationdialogsingle->getText();
                if (!rc)
                {
                    return(0);
                }
            }
        }
        else
        {
            if (operationdialogmultiple == NULL)
            {
                operationdialogmultiple = new BrowseInputDialog(this, "", "", title, _("To folder:"), icon, BROWSE_INPUT_FOLDER);
            }
            operationdialogmultiple->setTitle(title);
            operationdialogmultiple->setIcon(icon);
            operationdialogmultiple->setMessage(message);
            operationdialogmultiple->setText(target);
            operationdialogmultiple->CursorEnd();
            operationdialogmultiple->setDirectory(targetdir);
            int rc = 1;
            rc = operationdialogmultiple->execute(PLACEMENT_CURSOR);
            target = operationdialogmultiple->getText();
            if (!rc)
            {
                return(0);
            }
        }
    }

    // Nothing entered
    if (target == "")
    {
        MessageBox::warning(this, BOX_OK, _("Warning"), _("File name is empty, operation cancelled"));
        return(0);
    }

    // Except for rename, an absolute path is required
    if ((FXSELID(sel) != ID_FILE_RENAME) && !ISPATHSEP(target[0]))
    {
        MessageBox::warning(this, BOX_OK, _("Warning"), _("You must enter an absolute path!"));
        goto x;
    }

    // Update target and target parent directory
	target=::filePath(target,targetdir);
 	if (::isDirectory(target))
	{
		targetdir = target;
	}
	else
	{
		targetdir = FXPath::directory(target);
	}
 
    // Target directory not writable
    if (!::isWritable(targetdir))
    {
        MessageBox::error(this, BOX_OK_SU, _("Error"), _("Can't write to %s: Permission denied"), targetdir.text());
        return(0);
    }

    // Multiple sources and non existent destination
    if ((num > 1) && !::exists(target))
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Folder %s doesn't exist"), target.text());
        return(0);
    }

    // Multiple sources and target is a file
    if ((num > 1) && ::isFile(target))
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("%s is not a folder"), target.text());
        return(0);
    }

    // Target is a directory and is not writable
    if (::isDirectory(target) && !::isWritable(target))
    {
        MessageBox::error(this, BOX_OK_SU, _("Error"), _("Can't write to %s: Permission denied"), target.text());
        return(0);
    }

    // Target is a file and its parent directory is not writable
    if (::isFile(target) && !::isWritable(targetdir))
    {
        MessageBox::error(this, BOX_OK_SU, _("Error"), _("Can't write to %s: Permission denied"), targetdir.text());
        return(0);
    }

    // Target parent directory doesn't exist
    if (!::exists(targetdir))
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Folder %s doesn't exist"), targetdir.text());
        return(0);
    }

    // Target parent directory is not a directory
    if (!::isDirectory(targetdir))
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("%s is not a folder"), targetdir.text());
        return(0);
    }

    // One source
    File* f = NULL;
    int   ret;
    if (num == 1)
    {
        // An empty source file name corresponds to the ".." file
        // Don't perform any file operation on it!
        if (source == "")
        {
            return(0);
        }

        // Wait cursor
        getApp()->beginWaitCursor();
        mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_ANIMTHROB), NULL);

        // File object
        if (command == "copy")
        {
            f = new File(this, _("File copy"), COPY, num);
            f->create();

            // If target file is located at trash location, also create the corresponding trashinfo file
            // Do it silently and don't report any error if it fails
            FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);
            if (use_trash_can && (target == trashfileslocation))
            {
                // Trash files path name
                FXString trashpathname = createTrashpathname(source, trashfileslocation);

                // Adjust target name to get the _N suffix if any
                FXString trashtarget = target+PATHSEPSTRING+FXPath::name(trashpathname);

                // Create trashinfo file
                createTrashinfo(source, trashpathname, trashfileslocation, trashinfolocation);

                // Copy source to trash target
                ret = f->copy(source, trashtarget);
            }

            // Copy source to target
            else
            {
                ret = f->copy(source, target);
            }

            // An error has occurred
            if ((ret == 0) && !f->isCancelled())
            {
                f->hideProgressDialog();
                MessageBox::error(this, BOX_OK, _("Error"), _("An error has occurred during the copy file operation!"));
            }

            // If action is cancelled in progress dialog
            if (f->isCancelled())
            {
                f->hideProgressDialog();
                MessageBox::error(this, BOX_OK, _("Warning"), _("Copy file operation cancelled!"));
            }
        }
        else if (command == "rename")
        {
            f = new File(this, _("File rename"), RENAME, num);
            f->create();
            ret = f->rename(source, target);

            // If source file is located at trash location, try to also remove the corresponding trashinfo file if it exists
            // Do it silently and don't report any error if it fails
            FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);
            if (use_trash_can && ret && (source.left(trashfileslocation.length()) == trashfileslocation))
            {
                FXString trashinfopathname = trashinfolocation+PATHSEPSTRING+FXPath::name(source)+".trashinfo";
                ::unlink(trashinfopathname.text());
            }
        }
        else if (command == "move")
        {
            f = new File(this, _("File move"), MOVE, num);
            f->create();

            // If target file is located at trash location, also create the corresponding trashinfo file
            // Do it silently and don't report any error if it fails
            FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);
            if (use_trash_can && (target == trashfileslocation))
            {
                // Trash files path name
                FXString trashpathname = createTrashpathname(source, trashfileslocation);

                // Adjust target name to get the _N suffix if any
                FXString trashtarget = target+PATHSEPSTRING+FXPath::name(trashpathname);

                // Create trashinfo file
                createTrashinfo(source, trashpathname, trashfileslocation, trashinfolocation);

                // Move source to trash target
                ret = f->move(source, trashtarget);
            }

            // Move source to target
            else
            {
                ret = f->move(source, target);
            }

            // If source file is located at trash location, try to also remove the corresponding trashinfo file if it exists
            // Do it silently and don't report any error if it fails
            if (use_trash_can && ret && (source.left(trashfileslocation.length()) == trashfileslocation))
            {
                FXString trashinfopathname = trashinfolocation+PATHSEPSTRING+FXPath::name(source)+".trashinfo";
                ::unlink(trashinfopathname.text());
            }

            // An error has occurred
            if ((ret == 0) && !f->isCancelled())
            {
                f->hideProgressDialog();
                MessageBox::error(this, BOX_OK, _("Error"), _("An error has occurred during the move file operation!"));
            }

            // If action is cancelled in progress dialog
            if (f->isCancelled())
            {
                f->hideProgressDialog();
                MessageBox::error(this, BOX_OK, _("Warning"), _("Move file operation cancelled!"));
            }
        }
        else if (command == "symlink")
        {
            f = new File(this, _("Symlink"), SYMLINK, num);
            f->create();
            f->symlink(source, target);
        }
        // Shouldn't happen
        else
        {
            exit(EXIT_FAILURE);
        }

        getApp()->endWaitCursor();
        mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
        delete f;
    }

    // Multiple sources
    // Note : rename cannot be used in this case!
    else if (num > 1)
    {
        // Wait cursor
        mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_ANIMTHROB), NULL);
        getApp()->beginWaitCursor();

        // File object
        if (command == "copy")
        {
            f = new File(this, _("File copy"), COPY, num);
        }
        else if (command == "move")
        {
            f = new File(this, _("File move"), MOVE, num);
        }
        else if (command == "symlink")
        {
            f = new File(this, _("Symlink"), SYMLINK, num);
        }
        // Shouldn't happen
        else
        {
            exit(EXIT_FAILURE);
        }
        f->create();

        list->setAllowRefresh(false);

        // Loop on the multiple files
        for (int i = 0; i < num; i++)
        {
            // Individual source file
            source = src.section('\n', i);

            // File could have already been moved above in the tree
            if (!::exists(source))
            {
                continue;
            }

            // An empty file name corresponds to the ".." file (why?)
            // Don't perform any file operation on it!
            if (source != "")
            {
                if (command == "copy")
                {
                    // If target file is located at trash location, also create the corresponding trashinfo file
                    // Do it silently and don't report any error if it fails
                    FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);
                    if (use_trash_can && (target == trashfileslocation))
                    {
                        // Trash files path name
                        FXString trashpathname = createTrashpathname(source, trashfileslocation);

                        // Adjust target name to get the _N suffix if any
                        FXString trashtarget = target+PATHSEPSTRING+FXPath::name(trashpathname);

                        // Create trashinfo file
                        createTrashinfo(source, trashpathname, trashfileslocation, trashinfolocation);

                        // Copy source to trash target
                        ret = f->copy(source, trashtarget);
                    }

                    // Copy source to target
                    else
                    {
                        ret = f->copy(source, target);
                    }

                    // An error has occurred
                    if ((ret == 0) && !f->isCancelled())
                    {
                        f->hideProgressDialog();
                        MessageBox::error(this, BOX_OK, _("Error"), _("An error has occurred during the copy file operation!"));
                        break;
                    }

                    // If action is cancelled in progress dialog
                    if (f->isCancelled())
                    {
                        f->hideProgressDialog();
                        MessageBox::error(this, BOX_OK, _("Warning"), _("Copy file operation cancelled!"));
                        break;
                    }
                }
                else if (command == "move")
                {
                    // If target file is located at trash location, also create the corresponding trashinfo file
                    // Do it silently and don't report any error if it fails
                    FXbool use_trash_can = getApp()->reg().readUnsignedEntry("OPTIONS", "use_trash_can", true);
                    if (use_trash_can && (target == trashfileslocation))
                    {
                        // Trash files path name
                        FXString trashpathname = createTrashpathname(source, trashfileslocation);

                        // Adjust target name to get the _N suffix if any
                        FXString trashtarget = target+PATHSEPSTRING+FXPath::name(trashpathname);

                        // Create trashinfo file
                        createTrashinfo(source, trashpathname, trashfileslocation, trashinfolocation);

                        // Move source to trash target
                        ret = f->move(source, trashtarget);
                    }

                    // Move source to target
                    else
                    {
                        ret = f->move(source, target);
                    }

                    // If source file is located at trash location, try to also remove the corresponding trashinfo file if it exists
                    // Do it silently and don't report any error if it fails
                    if (use_trash_can && ret && (source.left(trashfileslocation.length()) == trashfileslocation))
                    {
                        FXString trashinfopathname = trashinfolocation+PATHSEPSTRING+FXPath::name(source)+".trashinfo";
                        ::unlink(trashinfopathname.text());
                    }

                    // An error has occurred
                    if ((ret == 0) && !f->isCancelled())
                    {
                        f->hideProgressDialog();
                        MessageBox::error(this, BOX_OK, _("Error"), _("An error has occurred during the move file operation!"));
                        break;
                    }

                    // If action is cancelled in progress dialog
                    if (f->isCancelled())
                    {
                        f->hideProgressDialog();
                        MessageBox::error(this, BOX_OK, _("Warning"), _("Move file operation cancelled!"));
                        break;
                    }
                }
                else if (command == "symlink")
                {
                    ret = f->symlink(source, target);

                    // An error has occurred
                    if ((ret == 0) && !f->isCancelled())
                    {
                        f->hideProgressDialog();
                        MessageBox::error(this, BOX_OK, _("Error"), _("An error has occurred during the symlink operation!"));
                        break;
                    }

                    // If action is cancelled in progress dialog
                    if (f->isCancelled())
                    {
                        f->hideProgressDialog();
                        MessageBox::error(this, BOX_OK, _("Warning"), _("Symlink operation cancelled!"));
                        break;
                    }
                }
				// Shouldn't happen
				else
                {
                    exit(EXIT_FAILURE);
                }
            }
        }

        getApp()->endWaitCursor();
        mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
        delete f;
    }

    // Force list refresh
    list->setAllowRefresh(true);
    list->onCmdRefresh(0, 0, 0);

    return(1);
}


// Go to script directory
long SearchPanel::onCmdGoScriptDir(FXObject* o, FXSelector sel, void*)
{
    FXString scriptpath = homedir + PATHSEPSTRING CONFIGPATH PATHSEPSTRING XFECONFIGPATH PATHSEPSTRING SCRIPTPATH;

    if (!::exists(scriptpath))
    {
        // Create the script directory according to the umask
        int mask = umask(0);
        umask(mask);
        errno = 0;
        int ret = mkpath(scriptpath.text(), 511 & ~mask);
        int errcode = errno;
        if (ret == -1)
        {
            if (errcode)
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't create script folder %s: %s"), scriptpath.text(), strerror(errcode));
            }
            else
            {
                MessageBox::error(this, BOX_OK, _("Error"), _("Can't create script folder %s"), scriptpath.text());
            }

            return(0);
        }
    }

    // Change directory in Xfe
    ((XFileExplorer*)mainWindow)->setDirectory(scriptpath);

    // Raise the Xfe window
    ((XFileExplorer*)mainWindow)->raise();
    ((XFileExplorer*)mainWindow)->setFocus();

    return(1);
}


// Clear file list and reset panel status
void SearchPanel::clearItems(void)
{
    status->setText(_("0 item"));
    list->clearItems();
}


// Update the status bar
long SearchPanel::onUpdStatus(FXObject* sender, FXSelector, void*)
{
    // Update the status bar
    int      item = -1;
    FXString str, linkto;
    char     size[64];
    FXulong  sz = 0;

    FXString hsize = _("0 bytes");
    FXString path = list->getDirectory();
    int      num = list->getNumSelectedItems();

    item = list->getCurrentItem();

    if (num > 1)
    {
		int nbdirs = 0;
        for (int u = 0; u < list->getNumItems(); u++)
        {
            if (list->isItemSelected(u) && !list->isItemDirectory(u))
            {
                sz += list->getItemFileSize(u);
#if __WORDSIZE == 64
                snprintf(size, sizeof(size)-1, "%lu", sz);
#else
                snprintf(size, sizeof(size)-1, "%llu", sz);
#endif
                hsize = ::hSize(size);
            }
            
            if (list->isItemDirectory(u))
            {
				nbdirs++;
			}
        }

        // Don't count the '..' directory
        if (nbdirs >= 1)
        {
			nbdirs--;
		}
		
		int nbfiles = num - nbdirs;
        if (nbdirs <= 1 && nbfiles <= 1)
        {
	        str.format(_("%s in %s selected items (%s folder, %s file)"), hsize.text(), FXStringVal(num).text(), FXStringVal(nbdirs).text(), FXStringVal(nbfiles).text());		
		}
        else if (nbdirs <=1 && nbfiles > 1)
        {
	        str.format(_("%s in %s selected items (%s folder, %s files)"), hsize.text(), FXStringVal(num).text(), FXStringVal(nbdirs).text(), FXStringVal(nbfiles).text());		
		}
        else if (nbdirs > 1 && nbfiles <= 1)
        {
	        str.format(_("%s in %s selected items (%s folders, %s file)"), hsize.text(), FXStringVal(num).text(), FXStringVal(nbdirs).text(), FXStringVal(nbfiles).text());		
		}
		else
		{
	        str.format(_("%s in %s selected items (%s folders, %s files)"), hsize.text(), FXStringVal(num).text(), FXStringVal(nbdirs).text(), FXStringVal(nbfiles).text());		
		}
    }
    else
    {
        // Nothing selected
        if ((num == 0) || (item < 0))
        {
            num = list->getNumItems();
            if (num == 1)
            {
                str = _("1 item");
            }
            else
            {
				int nbdirs = 0;
				for (int u = 0; u < num; u++)
				{
					if (list->isItemDirectory(u))
					{
						nbdirs++;
					}
				}       

				int nbfiles = num - nbdirs;
		        str.format(_("%s items (%s folders, %s files)"), FXStringVal(num).text(), FXStringVal(nbdirs).text(), FXStringVal(nbfiles).text());
				if (nbdirs <= 1 && nbfiles <= 1)
				{
					str.format(_("%s items (%s folder, %s file)"), FXStringVal(num).text(), FXStringVal(nbdirs).text(), FXStringVal(nbfiles).text());		
				}
				else if (nbdirs <=1 && nbfiles > 1)
				{
					str.format(_("%s items (%s folder, %s files)"), FXStringVal(num).text(), FXStringVal(nbdirs).text(), FXStringVal(nbfiles).text());		
				}
				else if (nbdirs > 1 && nbfiles <= 1)
				{
					str.format(_("%s items (%s folders, %s file)"), FXStringVal(num).text(), FXStringVal(nbdirs).text(), FXStringVal(nbfiles).text());		
				}
				else
				{
					str.format(_("%s items (%s folders, %s files)"), FXStringVal(num).text(), FXStringVal(nbdirs).text(), FXStringVal(nbfiles).text());		
				}
            }
        }
        else // num=1
        {
            FXString string = list->getItemText(item);
            FXString name = string.section('\t', 0);
            FXString type = string.section('\t', 3);

            FXString date = string.section('\t', 5);
            FXString usr = string.section('\t', 6);
            FXString grp = string.section('\t', 7);
            FXString perm = string.section('\t', 8);

            if (type.contains(_("Broken link")))
            {
                linkto = ::readLink(path+PATHSEPSTRING+name);
                str = name + "->" + linkto.text() + " | " + type + " | " + date + " | " + usr + " | " + grp + " | " + perm;
            }
            else if (type.contains(_("Link")))
            {
                linkto = ::readLink(path+PATHSEPSTRING+name);
                str = name + "->" + linkto.text() + " | " + type + " | " + date + " | " + usr + " | " + grp + " | " + perm;
            }
            else
            {
                for (int u = 0; u < list->getNumItems(); u++)
                {
                    if (list->isItemSelected(u) && !list->isItemDirectory(u))
                    {
                        sz = list->getItemFileSize(u);
#if __WORDSIZE == 64
                        snprintf(size, sizeof(size)-1, "%lu", sz);
#else
                        snprintf(size, sizeof(size)-1, "%llu", sz);
#endif
                        hsize = ::hSize(size);
                        break;
                    }
                }
                str = hsize+ " | " + type + " | " + date + " | " + usr + " | " + grp + " | " + perm;
            }
        }
    }

    status->setText(str);

    return(1);
}


// Update the status of the menu items that should be disabled
// when the number of selected items is not one
long SearchPanel::onUpdSelMult(FXObject* o, FXSelector sel, void*)
{
    int num;

    num = list->getNumSelectedItems();

    if (num == 1)
    {
        o->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        o->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return(1);
}


// Update the file compare menu item
long SearchPanel::onUpdCompare(FXObject* o, FXSelector sel, void*)
{
    // Menu item is enabled only when two files are selected
    int num;

    num = list->getNumSelectedItems();

    if ((num == 1) || (num == 2))
    {
        o->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        o->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }

    return(1);
}


// Update menu items and toolbar buttons that are related to file operations
long SearchPanel::onUpdMenu(FXObject* o, FXSelector sel, void*)
{
    // Menu item is disabled when nothing is selected
    int num;

    num = list->getNumSelectedItems();

    if (num == 0)
    {
        o->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    else
    {
        o->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }

    return(1);
}


// Update directory usage menu item
long SearchPanel::onUpdDirUsage(FXObject* o, FXSelector, void*)
{
    // Menu item is enabled only when at least two items are selected
    int num, item;

    num = list->getNumSelectedItems(&item);
    if (num > 1)
    {
        o->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
    }
    else
    {
        o->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    return(1);
}


#if defined(linux)

// Query packages data base
long SearchPanel::onCmdPkgQuery(FXObject* o, FXSelector sel, void*)
{
    FXString cmd;

    // Name of the current selected file
    FXString file = list->getCurrentFile();

    // Command to perform
    if (pkg_format == DEB_PKG)
    {
        cmd = "dpkg -S " + ::quote(file);
    }
    else if (pkg_format == RPM_PKG)
    {
        cmd = "rpm -qf " + ::quote(file);
    }
    else
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("No compatible package manager (rpm or dpkg) found!"));
        return(0);
    }

    // Query command
    cmd += " 2>&1";

    // Wait cursor
    mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_ANIMTHROB), NULL);
    getApp()->beginWaitCursor();

    // Perform the command
    FILE* pcmd = popen(cmd.text(), "r");
    if (!pcmd)
    {
        MessageBox::error(this, BOX_OK, _("Error"), _("Failed command: %s"), cmd.text());
        return(0);
    }

    // Get command output
    char     text[10000] = { 0 };
    FXString buf;
    while (fgets(text, sizeof(text), pcmd))
    {
        buf += text;
    }
    snprintf(text, sizeof(text)-1, "%s", buf.text());

    // Close the stream and display error message if any
    if ((pclose(pcmd) == -1) && (errno != ECHILD))   // ECHILD can be set if the child was caught by sigHarvest
    {
        mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
        getApp()->endWaitCursor();
        MessageBox::error(this, BOX_OK, _("Error"), "%s", text);
        return(0);
    }
    mainWindow->handle(this, FXSEL(SEL_TIMEOUT,XFileExplorer::ID_STOPANIMTHROB), NULL);
    getApp()->endWaitCursor();

    // Get package name, or detect when the file isn't in a package
    FXString str = text;
    if (pkg_format == DEB_PKG)  // DEB based distribution
    {
        int idx = str.find(" ");               // Split output at first whitespace
        FXString pkgname = str.left(idx-1);    // Remove trailing colon
        FXString fname = str.right(str.length()-idx);
        fname.trim();                          // Remove leading space and trailing newline
        if (streq(fname.text(), file.text()))  // No other word than the file name
        {
            str = pkgname.text();
        }
        else
        {
            str = "";
        }
    }
    if (pkg_format == RPM_PKG)   // RPM based distribution
    {
        if (str.find(' ') != -1) // Space character exists in the string
        {
            str = "";
        }
    }

    // Display the related output message
    FXString message;
    if (str == "")
    {
        message.format(_("File %s does not belong to any package."), file.text());
        MessageBox::information(this, BOX_OK, _("Information"), "%s", message.text());
    }
    else
    {
        message.format(_("File %s belongs to the package: %s"), file.text(), str.text());
        MessageBox::information(this, BOX_OK, _("Information"), "%s", message.text());
    }

    return(1);
}


// Update the package query menu
long SearchPanel::onUpdPkgQuery(FXObject* o, FXSelector sel, void*)
{
    // Menu item is disabled when multiple selection
    // or when unique selection and the selected item is a directory

    int num;

    num = list->getNumSelectedItems();

    if (num > 1)
    {
        o->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
    }
    else // num=1
    {
        int item = list->getCurrentItem();
        if ((item >= 0) && list->isItemDirectory(item))
        {
            o->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_DISABLE), NULL);
        }
        else
        {
            o->handle(this, FXSEL(SEL_COMMAND, FXWindow::ID_ENABLE), NULL);
        }
    }

    return(1);
}


#endif
