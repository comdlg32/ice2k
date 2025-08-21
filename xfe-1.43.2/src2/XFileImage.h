#ifndef XFILEIMAGE_H
#define XFILEIMAGE_H


#include "InputDialog.h"
#include "PathLinker.h"

class XFileImage : public FXMainWindow
{
    FXDECLARE(XFileImage)
protected:
    FXbool             hiddenfiles;           // Show or hide hidden files
    FXbool             thumbnails;            // Show or hide image thumbnails
    FXuint             fileview;              // File list view
    FXuint             liststyle;             // Icon list style
    FXImageView*       imageview;             // Image viewer
    FXVerticalFrame*   imageviewcontainer;
    FXRecentFiles      mrufiles;              // Recent files
    FXString           filename;              // File being viewed
    FXMenuBar*         menubar;               // Menu bar
    FXToolBar*         toolbar;               // Tool bar
    FXVerticalFrame*   toolbarcont;           // Tool bar container
    FXToolBarShell*    dragshell1;            // Shell for floating menubar
    FXHorizontalFrame* statusbar;             // Status bar
    FXbool             filelistbefore;
    FXbool             vertpanels;
    int                zoomwin;
    FXDragCorner*      realcorner;
    FXPacker*          corner;
    FXSplitter*        splitter;              // Splitter
    FXVerticalFrame*   filebox;               // Box containing directories/files
    FileList*          filelist;              // File List
    FXLabel*           label;                 // Directory path
    FXMenuPane*        filemenu;              // File menu
    FXMenuPane*        viewmenu;              // View menu
    FXMenuPane*        helpmenu;              // Help menu
    FXMenuPane*        imagemenu;             // Image menu
    FXMenuPane*        prefsmenu;             // Preferences menu
    FXTextField*       filter;                // Filter for tree list
    FXImage*           img;                   // Image loaded
    FXImage*           tmpimg;                // Temporary image
    FXColor*           tmpdata;               // Temporary image data
    int                indZoom;               // Zoom index
    int                imgvwidth;
    int                imgvheight;
    double             zoomval;               // Actual zoom factor
    FXbool             fitwin;                // Fit window when opening an image
    FXbool             filterimgs;            // List only image files in file list
    InputDialog*       printdialog;
    FXbool             smoothscroll;
    double             filewidth_pct;
    double             fileheight_pct;
    FXArrowButton*     btnbackhist;           // Back history
    FXArrowButton*     btnforwardhist;        // Forward history
    PathLinker*        pathlink;
    TextLabel*         pathtext;
    int                prev_width;
    int                prev_height;
protected:
    XFileImage() : hiddenfiles(false), thumbnails(false), fileview(0), liststyle(0), imageview(NULL), menubar(NULL), toolbar(NULL), dragshell1(NULL),
                   statusbar(NULL), filelistbefore(false), vertpanels(false), splitter(NULL),
                   filebox(NULL), filelist(NULL), label(NULL), filemenu(NULL), viewmenu(NULL), helpmenu(NULL), imagemenu(NULL),
                   prefsmenu(NULL), filter(NULL), img(NULL), tmpimg(NULL), tmpdata(NULL), indZoom(0),
                   zoomval(0.0), fitwin(false), filterimgs(false), printdialog(NULL), smoothscroll(false), filewidth_pct(0.0), fileheight_pct(0.0),
                   btnbackhist(NULL), btnforwardhist(NULL), pathlink(NULL), pathtext(NULL), prev_width(0), prev_height(0)
    {}
public:
    long onCmdAbout(FXObject*, FXSelector, void*);
    long onCmdOpen(FXObject*, FXSelector, void*);
    long onCmdPrint(FXObject*, FXSelector, void*);
    long onCmdShowMini(FXObject*, FXSelector, void*);
    long onCmdShowBig(FXObject*, FXSelector, void*);
    long onCmdShowDetails(FXObject*, FXSelector, void*);
    long onCmdShowRows(FXObject*, FXSelector, void*);
    long onCmdShowCols(FXObject*, FXSelector, void*);
    long onCmdAutosize(FXObject*, FXSelector, void*);
    long onCmdSave(FXObject*, FXSelector, void*);
    long onSigHarvest(FXObject*, FXSelector, void*);
    long onCmdQuit(FXObject*, FXSelector, void*);
    long onCmdRestart(FXObject*, FXSelector, void*);
    long onUpdTitle(FXObject*, FXSelector, void*);
    long onCmdRecentFile(FXObject*, FXSelector, void*);
    long onCmdRotate(FXObject*, FXSelector, void*);
    long onCmdMirror(FXObject*, FXSelector, void*);
    long onCmdZoomIn(FXObject*, FXSelector, void*);
    long onCmdZoomOut(FXObject*, FXSelector, void*);
    long onCmdZoom100(FXObject*, FXSelector, void*);
    long onCmdZoomWin(FXObject*, FXSelector, void*);
    long onConfigureZoomWin(FXObject*, FXSelector, void*);
    long onLower(FXObject*, FXSelector, void*);
    long onUpdImage(FXObject*, FXSelector, void*);
    long onUpdFileView(FXObject*, FXSelector, void*);
    long onUpdIconView(FXObject*, FXSelector, void*);
    long onCmdToggleHidden(FXObject*, FXSelector, void*);
    long onUpdToggleHidden(FXObject*, FXSelector, void*);
    long onCmdToggleThumbnails(FXObject*, FXSelector, void*);
    long onUpdToggleThumbnails(FXObject*, FXSelector, void*);
    long onCmdItemDoubleClicked(FXObject*, FXSelector, void*);
    long onCmdItemClicked(FXObject*, FXSelector, void*);
    long onCmdToggleFitWin(FXObject*, FXSelector, void*);
    long onUpdToggleFitWin(FXObject*, FXSelector, void*);
    long onCmdPopupMenu(FXObject*, FXSelector, void*);
    long onCmdHome(FXObject*, FXSelector, void*);
    long onCmdWork(FXObject*, FXSelector, void*);
    long onCmdDirUp(FXObject*, FXSelector, void*);
    long onUpdDirUp(FXObject*, FXSelector, void*);
    long onCmdDirBack(FXObject*, FXSelector, void*);
    long onUpdDirBack(FXObject*, FXSelector, void*);
    long onCmdDirForward(FXObject*, FXSelector, void*);
    long onUpdDirForward(FXObject*, FXSelector, void*);
    long onCmdDirBackHist(FXObject*, FXSelector, void*);
    long onUpdDirBackHist(FXObject*, FXSelector, void*);
    long onCmdDirForwardHist(FXObject*, FXSelector, void*);
    long onUpdDirForwardHist(FXObject*, FXSelector, void*);
    long onCmdToggleFilterImages(FXObject*, FXSelector, void*);
    long onUpdToggleFilterImages(FXObject*, FXSelector, void*);
    long onCmdHorzVertPanels(FXObject*, FXSelector, void*);
    long onUpdHorzVertPanels(FXObject*, FXSelector, void*);
    long onCmdToggleFileListBefore(FXObject*, FXSelector, void*);
    long onUpdToggleFileListBefore(FXObject*, FXSelector, void*);
	long onKeyPress(FXObject*, FXSelector, void*);
	long onKeyRelease(FXObject*, FXSelector, void*);
	

public:
    enum
    {
        ID_ABOUT=FXMainWindow::ID_LAST,
        ID_OPEN,
        ID_POPUP_MENU,
        ID_TOGGLE_HIDDEN,
        ID_TOGGLE_THUMBNAILS,
        ID_SHOW_MINI_ICONS,
        ID_SHOW_BIG_ICONS,
        ID_SHOW_DETAILS,
        ID_COLS,
        ID_ROWS,
        ID_AUTO,
        ID_TITLE,
        ID_PRINT,
        ID_HARVEST,
        ID_QUIT,
        ID_RESTART,
        ID_FILELIST,
        ID_RECENTFILE,
        ID_ROTATE_90,
        ID_ROTATE_270,
        ID_MIRROR_HOR,
        ID_MIRROR_VER,
        ID_SCALE,
        ID_ZOOM_IN,
        ID_ZOOM_OUT,
        ID_ZOOM_100,
        ID_ZOOM_WIN,
        ID_TOGGLE_FIT_WIN,
        ID_TOGGLE_FILTER_IMAGES,
        ID_GO_HOME,
        ID_GO_WORK,
        ID_DIR_UP,
        ID_DIR_BACK,
        ID_DIR_FORWARD,
        ID_DIR_BACK_HIST,
        ID_DIR_FORWARD_HIST,
        ID_HORZ_PANELS,
        ID_VERT_PANELS,
        ID_TOGGLE_FILELIST_BEFORE,
        ID_LAST
    };
public:
    XFileImage(FXApp*, FXbool);
    virtual void create();
    FXbool loadimage(const FXString&);
    void saveConfig();

    void start(FXString);
    virtual ~XFileImage();
    void setSmoothScroll(FXbool smooth)
    {
        smoothscroll = smooth;
    }
};
#endif
