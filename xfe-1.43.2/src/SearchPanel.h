#ifndef SEARCHPANEL_H
#define SEARCHPANEL_H

#include <map>

#include "HistInputDialog.h"
#include "BrowseInputDialog.h"



// Typedef for the map between program string identifiers and integer indexes
typedef std::map<FXString, int>   progsmap;


// Search panel
class FXAPI SearchPanel : public FXVerticalFrame
{
    FXDECLARE(SearchPanel)
protected:
    FileDict*          associations;
    FileList*          list;                // File list
    ArchInputDialog*   archdialog;
    HistInputDialog*   opendialog;
    BrowseInputDialog* operationdialogsingle;
    InputDialog*       operationdialogrename;
    BrowseInputDialog* operationdialogmultiple;
    BrowseInputDialog* comparedialog;
    FXString           searchdir;
    FXbool             ctrlflag;        // Flag to select the right click control menu
    FXbool             shiftf10;        // Flag indicating that Shift-F10 was pressed
    FXPacker*          statusbar;
    FXLabel*           status;
    FXDragCorner*      corner;
    FXString           trashfileslocation;
    FXString           trashinfolocation;
    FXDragType         urilistType;     // Standard uri-list type
    FXDragType         xfelistType;     // Xfe, Gnome and XFCE list type
    FXDragType         kdelistType;     // KDE list type
    FXDragType         utf8Type;        // UTF-8 text type
    FXButton*          refreshbtn;
    FXButton*          gotodirbtn;
    FXButton*          copybtn;
    FXButton*          cutbtn;
    FXButton*          propbtn;
    FXButton*          trashbtn;
    FXButton*          delbtn;
    FXButton*          bigiconsbtn;
    FXButton*          smalliconsbtn;
    FXButton*          detailsbtn;
    FXToggleButton*    thumbbtn;
    progsmap           progs;           // Map between program string identifiers and integer indexes
protected:
    SearchPanel() : associations(NULL), list(NULL), archdialog(NULL), opendialog(NULL), operationdialogsingle(NULL),
                    operationdialogrename(NULL), operationdialogmultiple(NULL), comparedialog(NULL),
                    ctrlflag(false), shiftf10(false), statusbar(NULL), status(NULL), corner(NULL), urilistType(0), xfelistType(0),
                    kdelistType(0), utf8Type(0), refreshbtn(NULL), gotodirbtn(NULL), copybtn(NULL), cutbtn(NULL), propbtn(NULL),
                    trashbtn(NULL), delbtn(NULL), bigiconsbtn(NULL), smalliconsbtn(NULL), detailsbtn(NULL), thumbbtn(NULL)
    {}
public:
    enum
    {
        ID_CANCEL=FXTopWindow::ID_LAST,
        ID_FILELIST,
        ID_STATUS,
        ID_POPUP_MENU,
        ID_VIEW,
        ID_EDIT,
        ID_COMPARE,
        ID_OPEN,
        ID_OPEN_WITH,
        ID_SELECT_ALL,
        ID_DESELECT_ALL,
        ID_SELECT_INVERSE,
        ID_EXTRACT,
        ID_ADD_TO_ARCH,
        ID_DIR_USAGE,
#if defined(linux)
        ID_PKG_QUERY,
        ID_PKG_INSTALL,
        ID_PKG_UNINSTALL,
#endif
        ID_REFRESH,
        ID_PROPERTIES,
        ID_COPY_CLIPBOARD,
        ID_CUT_CLIPBOARD,
        ID_GO_SCRIPTDIR,
        ID_GOTO_PARENTDIR,
        ID_FILE_COPYTO,
        ID_FILE_MOVETO,
        ID_FILE_RENAME,
        ID_FILE_SYMLINK,
        ID_FILE_DELETE,
        ID_FILE_TRASH,
        ID_LAST
    };
    SearchPanel(FXComposite*, FXuint name_size = 200, FXuint dir_size = 150, FXuint size_size = 60, FXuint type_size = 100, FXuint ext_size = 100,
                FXuint modd_size = 150, FXuint user_size = 50, FXuint grou_size = 50, FXuint attr_size = 100,
                FXColor listbackcolor = FXRGB(255, 255, 255), FXColor listforecolor = FXRGB(0, 0, 0),
                FXuint opts = 0, int x = 0, int y = 0, int w = 0, int h = 0);

    virtual void create();

    virtual ~SearchPanel();
    void execFile(FXString);
    int  readScriptDir(FXMenuPane*, FXString);
    long appendItem(FXString&);

    long onClipboardGained(FXObject*, FXSelector, void*);
    long onClipboardLost(FXObject*, FXSelector, void*);
    long onClipboardRequest(FXObject*, FXSelector, void*);
    long onKeyPress(FXObject*, FXSelector, void*);
    long onCmdItemDoubleClicked(FXObject*, FXSelector, void*);
    long onCmdItemClicked(FXObject*, FXSelector, void*);
    long onCmdSelect(FXObject*, FXSelector, void*);
    long onCmdGotoParentdir(FXObject*, FXSelector, void*);
    long onCmdOpenWith(FXObject*, FXSelector, void*);
    long onCmdOpen(FXObject*, FXSelector, void*);
    long onCmdEdit(FXObject*, FXSelector, void*);
    long onCmdCompare(FXObject*, FXSelector, void*);
    long onCmdRefresh(FXObject*, FXSelector, void*);
    long onCmdProperties(FXObject* sender, FXSelector, void*);
    long onCmdPopupMenu(FXObject*, FXSelector, void*);
    long onCmdCopyCut(FXObject*, FXSelector, void*);
    long onCmdFileMan(FXObject*, FXSelector, void*);
    long onCmdAddToArch(FXObject*, FXSelector, void*);
    long onCmdExtract(FXObject*, FXSelector, void*);
    long onCmdFileTrash(FXObject*, FXSelector, void*);
    long onCmdFileDelete(FXObject*, FXSelector, void*);
    long onCmdGoScriptDir(FXObject*, FXSelector, void*);
    long onCmdDirUsage(FXObject*, FXSelector, void*);
    long onUpdStatus(FXObject*, FXSelector, void*);
    long onUpdSelMult(FXObject*, FXSelector, void*);
    long onUpdCompare(FXObject*, FXSelector, void*);
    long onUpdMenu(FXObject*, FXSelector, void*);
    long onUpdDirUsage(FXObject*, FXSelector, void*);
#if defined(linux)
    long onCmdPkgQuery(FXObject*, FXSelector, void*);
    long onUpdPkgQuery(FXObject*, FXSelector, void*);
#endif
public:
    // Get header size given its index
    int getHeaderSize(int index) const
    {
        return(list->getHeaderSize(index));
    }

    // Change show thumbnails mode
    void showThumbnails(FXbool display)
    {
        list->showThumbnails(display);
    }

    // Thumbnails shown?
    FXbool shownThumbnails(void) const
    {
        return(list->shownThumbnails());
    }

    // Enable toolbar and status bar buttons
    void enableButtons(void)
    {
        refreshbtn->enable();
        gotodirbtn->enable();
        bigiconsbtn->enable();
        smalliconsbtn->enable();
        detailsbtn->enable();
        thumbbtn->enable();
    }

    // Disable toolbar and status bar buttons
    void disableButtons(void)
    {
        refreshbtn->disable();
        gotodirbtn->disable();
        bigiconsbtn->disable();
        smalliconsbtn->disable();
        detailsbtn->disable();
        thumbbtn->disable();
    }

    // Change sort function
    void setSortFunc(IconListSortFunc func)
    {
        list->setSortFunc(func);
    }

    // Return sort function
    IconListSortFunc getSortFunc() const
    {
        return(list->getSortFunc());
    }

    // Set ignore case
    void setIgnoreCase(FXbool ignorecase)
    {
        list->setIgnoreCase(ignorecase);
    }

    // Get ignore case
    FXbool getIgnoreCase(void)
    {
        return(list->getIgnoreCase());
    }

    // Set directory first
    void setDirsFirst(FXbool dirsfirst)
    {
        list->setDirsFirst(dirsfirst);
    }

    // Set directory first
    FXbool getDirsFirst(void)
    {
        return(list->getDirsFirst());
    }

    // Get the current icon list style
    FXuint getListStyle(void) const
    {
        return(list->getListStyle());
    }

    // Get the current icon list style
    void setListStyle(FXuint style)
    {
        list->setListStyle(style);
    }

    // Return number of items
    int getNumItems() const
    {
        return(list->getNumItems());
    }

    // Get current item
    int getCurrentItem(void) const
    {
        return(list->getCurrentItem());
    }

    // Set current item
    void setCurrentItem(int item)
    {
        list->setCurrentItem(item);
        list->makeItemVisible(item);
    }

    // Set status text
    void setStatusText(FXString text)
    {
        status->setText(text);
    }

    // Clear list items and reset panel status
    void clearItems(void);

    // Set search path
    void setSearchPath(FXString);

    // Toggle file list refresh
    void setAllowRefresh(FXbool flag)
    {
        list->setAllowRefresh(flag);
    }

    // Refresh file list
    void forceRefresh(void)
    {
        list->onCmdRefresh(0, 0, 0);
    }

	// Deselect all items
	void deselectAll(void)
	{
		list->onCmdDeselectAll(0,0,0);
	}
};

#endif
