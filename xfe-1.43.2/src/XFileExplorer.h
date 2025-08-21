#ifndef XFILEEXPLORER_H
#define XFILEEXPLORER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "xfedefs.h"
#include "xfeutils.h"
#include "FileDict.h"
#include "FilePanel.h"
#include "InputDialog.h"
#include "HistInputDialog.h"
#include "BrowseInputDialog.h"
#include "Properties.h"
#include "DirPanel.h"
#include "Bookmarks.h"
#include "Preferences.h"
#include "TextWindow.h"
#include "SearchWindow.h"

// Typedef for the map between program string identifiers and integer indexes
typedef std::map<FXString, int>   progsmap;


// Application object
class XFileExplorer : public FXMainWindow
{
    FXDECLARE(XFileExplorer)
protected:
    enum
    {
        TREE_PANEL,
        ONE_PANEL,
        TWO_PANELS,
        TREE_TWO_PANELS,
        FILEPANEL_FOCUS,
        DIRPANEL_FOCUS,
    };
    int              panel_view;
    int              RunHistSize;
    int              throbymax;
    char             RunHistory[RUN_HIST_SIZE][MAX_COMMAND_SIZE];
    FXbool           vertpanels;
    FXSplitter*      panelsplit;
    FXMenuBar*       menubar;
    FXMenuPane*      toolsmenu;
    FXMenuPane*      filemenu;
    FXMenuPane*      trashmenu;
    FXMenuPane*      editmenu;
    FXMenuPane*      bookmarksmenu;
    FXMenuPane*      viewmenu;
    FXMenuButton*    viewbutton;
    FXMenuPane*      lpanelmenu;
    FXMenuPane*      rpanelmenu;
    FXMenuPane*      helpmenu;
    FXMenuTitle*     toolsmenutitle;
    FXMenuTitle*     filemenutitle;
    FXMenuTitle*     filemenutitle2;
    FXMenuCascade*   trashmenutitle;
    FXMenuTitle*     editmenutitle;
    FXMenuTitle*     bookmarksmenutitle;
    FXMenuTitle*     viewmenutitle;
    FXMenuCascade*   lpanelmenutitle;
    FXMenuCascade*   rpanelmenutitle;
    FXMenuTitle*     helpmenutitle;
    Bookmarks*       bookmarks;
    FXToolBar*       generaltoolbar;
    FXToolBar*       throbbertoolbar;
    FXToolBar*       toolstoolbar;
    FXToolBar*       paneltoolbar;
    FXToolBar*       locationbar;
    FXLabel*         addressicon;
    ComboBox*        address;
    DirPanel*        dirpanel;
    FilePanel*       lpanel;
    FilePanel*       rpanel;
    FXString         trashfileslocation;
    FXString         trashinfolocation;
    FXString         startlocation;
    FXuint           liststyle;
    FXColor          listbackcolor;
    FXColor          listforecolor;
    FXColor          highlightcolor;
    FXColor          pbarcolor;
    FXColor          attentioncolor;
    FXColor          scrollbarcolor;
    //FXArrowButton*   btnbackhist;
    FXButton*        btnbackhist;
    FXButton*        btnforwardhist;
    FXButton*        foldersbtn;
    HistInputDialog* rundialog;
    PreferencesBox*  prefsdialog;
    TextWindow*      helpwindow;
    FXString         message;
    FXuint           panelfocus;
    FXString         startdir1;
    FXString         startdir2;
    vector_FXString  startURIs;
    FXbool           starticonic;
    FXbool           startmaximized;
    FXbool           smoothscroll;
    double           twopanels_lpanel_pct;      // Panel sizes, relatively to the window width (in percent)
    double           treepanel_tree_pct;
    double           treetwopanels_tree_pct;
    double           treetwopanels_lpanel_pct;
    FXString         prevdir;
    int              prev_width;
    FXuint           search_xpos;
    FXuint           search_ypos;
    FXuint           search_width;
    FXuint           search_height;
    FXLabel*         throbber;
    FXLabel*         lpaneltext;
    SearchWindow*    searchwindow;
    progsmap         progs;                     // Map between program string identifiers and integer indexes
    FXbool           winshow;                   // If true, do not show the Xfe window
    FXbool           stop;                      // If true, stop Xfe immediately
    int              nbstartfiles;              // Number of files to open on startup
    int              go_lpad;
    int              go_rpad;
    int              go_tpad;
    int              go_bpad;

public:
    enum
    {
        ID_ABOUT=FXMainWindow::ID_LAST,
        ID_ANIMTHROB,
        ID_REALANIMTHROB,
        ID_ACTTHROB,
        ID_STOPANIMTHROB,
        ID_HELP,
        ID_REFRESH,
        ID_EMPTY_TRASH,
        ID_TRASH_SIZE,
        ID_XTERM,
        ID_DIR_UP,
        ID_DIR_BACK,
        ID_DIR_FORWARD,
        ID_DIR_BACK_HIST,
        ID_DIR_FORWARD_HIST,
        ID_FILE_PROPERTIES,
        ID_FILE_COPY,
        ID_FILE_RENAME,
        ID_FILE_MOVETO,
        ID_FILE_COPYTO,
        ID_FILE_CUT,
        ID_FILE_PASTE,
        ID_FILE_SYMLINK,
        ID_FILE_DELETE,
        ID_FILE_TRASH,
        ID_FILE_RESTORE,
        ID_FILE_ASSOC,
        ID_FILE_SEARCH,
        ID_CLEAR_LOCATION,
        ID_GOTO_LOCATION,
        ID_RUN,
        ID_SU,
        ID_PREFS,
        ID_DIR_BOX,
        ID_TOGGLE_STATUS,
        ID_SHOW_ONE_PANEL,
        ID_SHOW_TWO_PANELS,
        ID_SHOW_TREE_PANEL,
        ID_SHOW_TREE_TWO_PANELS,
        ID_SYNCHRONIZE_PANELS,
        ID_SWITCH_PANELS,
        ID_SWITCH_TREE,
        ID_RESTART,
        ID_NEW_WIN,
        ID_BOOKMARK,
        ID_ADD_BOOKMARK,
        ID_HARVEST,
        ID_QUIT,
        ID_FILE_ADDCOPY,
        ID_FILE_ADDCUT,
        ID_HORZ_PANELS,
        ID_VERT_PANELS,
        ID_KILL_FOCUS,
        ID_LAST
    };
protected:
    XFileExplorer() : panel_view(0), RunHistSize(0), throbymax(0), vertpanels(false), panelsplit(NULL), menubar(NULL), toolsmenu(NULL), filemenu(NULL),
                      trashmenu(NULL), editmenu(NULL), bookmarksmenu(NULL), viewmenu(NULL), lpanelmenu(NULL), rpanelmenu(NULL), helpmenu(NULL),
                      toolsmenutitle(NULL), filemenutitle(NULL), trashmenutitle(NULL), editmenutitle(NULL), bookmarksmenutitle(NULL), viewmenutitle(NULL),
                      lpanelmenutitle(NULL), rpanelmenutitle(NULL), helpmenutitle(NULL), bookmarks(NULL), generaltoolbar(NULL),
                      toolstoolbar(NULL), paneltoolbar(NULL), locationbar(NULL), address(NULL), dirpanel(NULL), lpanel(NULL),
                      rpanel(NULL), liststyle(0), listbackcolor(FXRGB(0, 0, 0)), listforecolor(FXRGB(0, 0, 0)),
                      highlightcolor(FXRGB(0, 0, 0)), pbarcolor(FXRGB(0, 0, 0)), attentioncolor(FXRGB(0, 0, 0)),
                      scrollbarcolor(FXRGB(0, 0, 0)), btnbackhist(NULL), btnforwardhist(NULL), rundialog(NULL), prefsdialog(NULL),
                      helpwindow(NULL), panelfocus(0), starticonic(false), startmaximized(false), smoothscroll(false),
                      twopanels_lpanel_pct(0.0), treepanel_tree_pct(0.0), treetwopanels_tree_pct(0.0), treetwopanels_lpanel_pct(0.0),
                      prev_width(0), search_xpos(0), search_ypos(0), search_width(0), search_height(0), searchwindow(NULL), winshow(false), stop(false), nbstartfiles(0)
    {}
public:
    XFileExplorer(FXApp* app, vector_FXString URIs, const FXbool iconic = false, const FXbool maximized = false, const char* title = "X File Explorer", FXIcon* bigicon = NULL, FXIcon* miniicon = NULL);
    virtual void create();

    ~XFileExplorer();
    void saveConfig();

    void openFiles(vector_FXString);
    long onSigHarvest(FXObject*, FXSelector, void*);
    long onQuit(FXObject*, FXSelector, void*);
    long onKeyPress(FXObject*, FXSelector, void*);
    long onKeyRelease(FXObject*, FXSelector, void*);
    long onCmdHelp(FXObject*, FXSelector, void*);
    long onCmdAnimateThrobber(FXObject*, FXSelector, void*);
    long onCmdActivateThrobber(FXObject*, FXSelector, void*);
    long onCmdStopAnimateThrobber(FXObject*, FXSelector, void*);
    long onCmdAbout(FXObject*, FXSelector, void*);
    long onCmdFileAssoc(FXObject*, FXSelector, void*);
    long onCmdRefresh(FXObject*, FXSelector, void*);
    long onCmdToggleStatus(FXObject*, FXSelector, void*);
    long onCmdPopupMenu(FXObject*, FXSelector, void*);
    long onCmdPrefs(FXObject*, FXSelector, void*);
    long onCmdRun(FXObject*, FXSelector, void*);
    long onCmdSu(FXObject*, FXSelector, void*);
    long onCmdXTerm(FXObject*, FXSelector, void*);
    long onCmdEmptyTrash(FXObject*, FXSelector, void*);
    long onCmdTrashSize(FXObject*, FXSelector, void*);
    long onCmdHorzVertPanels(FXObject*, FXSelector, void*);
    long onCmdShowPanels(FXObject*, FXSelector, void*);
    long onCmdSwitchTree(FXObject*, FXSelector, void*);
    long onCmdRestart(FXObject*, FXSelector, void*);
    long onCmdNewWindow(FXObject*, FXSelector, void*);
    long onCmdBookmark(FXObject*, FXSelector, void*);
    long onCmdGotoLocation(FXObject*, FXSelector, void*);
    long onCmdClearLocation(FXObject*, FXSelector, void*);
    long onUpdToggleStatus(FXObject*, FXSelector, void*);
    long onUpdHorzVertPanels(FXObject*, FXSelector, void*);
    long onUpdShowPanels(FXObject*, FXSelector, void*);
    long onUpdFileLocation(FXObject*, FXSelector, void*);
    long onUpdEmptyTrash(FXObject*, FXSelector, void*);
    long onUpdTrashSize(FXObject*, FXSelector, void*);
    long onCmdFileDelete(FXObject*, FXSelector, void*);
    long onCmdFileTrash(FXObject*, FXSelector, void*);
    long onCmdFileRestore(FXObject*, FXSelector, void*);
    long onUpdFileDelete(FXObject*, FXSelector, void*);
    long onUpdFileTrash(FXObject*, FXSelector, void*);
    long onUpdFileRestore(FXObject*, FXSelector, void*);
    long onCmdFileSearch(FXObject*, FXSelector sel, void*);
    long onCmdDirUp(FXObject*, FXSelector, void*);
    long onCmdDirBack(FXObject*, FXSelector, void*);
    long onUpdDirBack(FXObject*, FXSelector, void*);
    long onCmdDirForward(FXObject*, FXSelector, void*);
    long onUpdDirForward(FXObject*, FXSelector, void*);
    long onCmdDirBackHist(FXObject*, FXSelector, void*);
    long onUpdDirBackHist(FXObject*, FXSelector, void*);
    long onCmdDirForwardHist(FXObject*, FXSelector, void*);
    long onUpdDirForwardHist(FXObject*, FXSelector, void*);
    long onCmdFileCopyClp(FXObject*, FXSelector, void*);
    long onCmdFileCutClp(FXObject*, FXSelector, void*);
    long onCmdFileAddCopyClp(FXObject*, FXSelector, void*);
    long onCmdFileAddCutClp(FXObject*, FXSelector, void*);
    long onCmdFilePasteClp(FXObject*, FXSelector, void*);
    long onCmdFileRename(FXObject*, FXSelector, void*);
    long onCmdFileMoveto(FXObject*, FXSelector, void*);
    long onCmdFileCopyto(FXObject*, FXSelector, void*);
    long onCmdFileSymlink(FXObject*, FXSelector, void*);
    long onUpdFileMan(FXObject*, FXSelector, void*);
    long onUpdFilePaste(FXObject*, FXSelector, void*);
    long onCmdFileProperties(FXObject*, FXSelector, void*);
    long onUpdFileRename(FXObject*, FXSelector, void*);
    long onCmdSynchronizePanels(FXObject*, FXSelector, void*);
    long onUpdSynchronizePanels(FXObject*, FXSelector, void*);
    long onCmdSwitchPanels(FXObject*, FXSelector, void*);
    long onUpdSwitchPanels(FXObject*, FXSelector, void*);
    long onUpdSu(FXObject*, FXSelector, void*);
    long onUpdQuit(FXObject*, FXSelector, void*);
    long onUpdFileSearch(FXObject*, FXSelector, void*);
public:
    // Get associations
    FileDict* getAssociations()
    {
        return(lpanel->getCurrent()->getAssociations());
    }

    // Change to selected directory
    void setDirectory(FXString pathname)
    {
        lpanel->getCurrent()->setDirectory(pathname, false);
        lpanel->getCurrent()->updatePath();
        dirpanel->setDirectory(pathname, true);
    }

    // Change default cursor for file and dir panels
    void setDefaultCursor(FXCursor* cur)
    {
        lpanel->setDefaultCursor(cur);
        rpanel->setDefaultCursor(cur);
        dirpanel->setDefaultCursor(cur);
    }

	// Deselect all items
    void deselectAll(void)
    {
        lpanel->deselectAll();
        rpanel->deselectAll();
        
        if (searchwindow)
        {
			searchwindow->deselectAll();
		}
    }
	 
    // Refresh file panels
    void refreshPanels(void)
    {
        lpanel->onCmdRefresh(0, 0, 0);
        rpanel->onCmdRefresh(0, 0, 0);
    }

    // Return a pointer on the current file panel
    FilePanel* getCurrentPanel(void)
    {
        return(lpanel->getCurrent());
    }

    // Return a pointer on the next file panel
    FilePanel* getNextPanel(void)
    {
        return(lpanel->getNext());
    }

    // Return the address box (location bar)
    FXComboBox* getAddressBox(void)
    {
        return(address);
    }

    // Return a pointer on the directory panel
    DirPanel* getDirPanel(void)
    {
        return(dirpanel);
    }

    FXLabel* getAddressIcon(void)
    {
        return(addressicon);
    }

    FXLabel* getThrobber(void)
    {
        return(throbber);
    }

};
#endif
