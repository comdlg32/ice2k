#ifndef SEARCHWINDOW_H
#define SEARCHWINDOW_H

#include "SearchPanel.h"

// Search window
class FXAPI SearchWindow : public FXTopWindow
{
    FXDECLARE(SearchWindow)
public:
    enum
    {
        ID_CANCEL=FXTopWindow::ID_LAST,
        ID_START,
        ID_STOP,
        ID_BROWSE_PATH,
        ID_READ_DATA,
        ID_CLOSE,
        ID_MORE_OPTIONS,
        ID_SIZE,
        ID_PERM,
        ID_RESET_OPTIONS,
        ID_LAST
    };
    SearchWindow(FXApp* app, const FXString& name, FXuint opts = DECOR_TITLE|DECOR_BORDER,
                 int x = 0, int y = 0, int w = 0, int h = 0, int pl = 10, int pr = 10, int pt = 10, int pb = 10, int hs = 4, int vs = 4);
    virtual void show(FXuint placement = PLACEMENT_CURSOR);
    virtual void create();

    virtual ~SearchWindow();
protected:
    FXApp*           application;
    FXLabel*         searchresults;
    FXTextField*     findfile;
    FXTextField*     wheredir;
    FXTextField*     greptext;
    FXButton*        dirbutton;
    FXButton*        startbutton;
    FXButton*        stopbutton;
    SearchPanel*     searchpanel;
    TextWindow*      warnwindow;
    int              in[2];              // Input and output pipes
    int              out[2];
    int              pid;                // Proccess ID of child (valid if busy).
    FXuint           count;
    FXbool           running;
    FXString         strprev;
    FXString         searchcommand;
    FXString         uid;
    FXString         gid;
    FXGroupBox*      moregroup;
    FXVerticalFrame* searchframe;
    FXCheckButton*   grepigncase;
    FXCheckButton*   findigncase;
    FXCheckButton*   findhidden;
    FXCheckButton*   moreoptions;
    FXSpinner*       minsize;
    FXSpinner*       maxsize;
    FXSpinner*       mindays;
    FXSpinner*       maxdays;
    FXComboBox*      user;
    FXComboBox*      grp;
    FXComboBox*      type;
    FXTextField*     perm;
    FXCheckButton*   userbtn;
    FXCheckButton*   grpbtn;
    FXCheckButton*   typebtn;
    FXCheckButton*   permbtn;
    FXCheckButton*   emptybtn;
    FXCheckButton*   linkbtn;
    FXCheckButton*   norecbtn;
    FXCheckButton*   nofsbtn;
    FXButton*        resetoptions;

    SearchWindow() : application(NULL), searchresults(NULL), findfile(NULL), wheredir(NULL), greptext(NULL), dirbutton(NULL),
                     startbutton(NULL), stopbutton(NULL), searchpanel(NULL), warnwindow(NULL),
                     pid(0), count(0), running(false), moregroup(NULL), searchframe(NULL), grepigncase(NULL), findigncase(NULL),
                     findhidden(NULL), moreoptions(NULL), minsize(NULL), maxsize(NULL), mindays(NULL), maxdays(NULL), user(NULL),
                     grp(NULL), type(NULL), perm(NULL), userbtn(NULL), grpbtn(NULL), typebtn(NULL), permbtn(NULL), emptybtn(NULL),
                     linkbtn(NULL), norecbtn(NULL), nofsbtn(NULL), resetoptions(NULL)
    {}

    SearchWindow(const SearchWindow&)
    {}
public:
    FXuint execute(FXuint placement = PLACEMENT_CURSOR);

    int execCmd(FXString);
    int readData();

    long onKeyPress(FXObject*, FXSelector, void*);
    long onCmdClose(FXObject*, FXSelector, void*);
    long onCmdStart(FXObject*, FXSelector, void*);
    long onCmdBrowsePath(FXObject*, FXSelector, void*);
    long onReadData(FXObject*, FXSelector, void*);
    long onCmdStop(FXObject*, FXSelector, void*);
    long onPermVerify(FXObject*, FXSelector, void*);
    long onCmdMoreOptions(FXObject*, FXSelector, void*);
    long onCmdResetOptions(FXObject*, FXSelector, void*);
    long onUpdStart(FXObject*, FXSelector, void*);
    long onUpdStop(FXObject*, FXSelector, void*);
    long onUpdPerm(FXObject*, FXSelector, void*);
    long onUpdSize(FXObject*, FXSelector, void*);
public:
    // Change sort function
    void setSortFunc(IconListSortFunc func)
    {
        searchpanel->setSortFunc(func);
    }

    // Return sort function
    IconListSortFunc getSortFunc() const
    {
        return(searchpanel->getSortFunc());
    }

    // More option dialog shown ?
    FXbool shownMoreOptions(void) const
    {
        return(moreoptions->getCheck());
    }

    // Get ignore case in find
    FXbool getFindIgnoreCase(void) const
    {
        return(findigncase->getCheck());
    }

    // Set hidden files in find
    void setFindHidden(FXbool hidden)
    {
        findhidden->setCheck(hidden);
    }

    // Get hidden files in find
    FXbool getFindHidden(void) const
    {
        return(findhidden->getCheck());
    }

    // Set ignore case in find
    void setFindIgnoreCase(FXbool ignorecase)
    {
        findigncase->setCheck(ignorecase);
    }

    // Get ignore case in grep
    FXbool getGrepIgnoreCase(void) const
    {
        return(grepigncase->getCheck());
    }

    // Set ignore case in grep
    void setGrepIgnoreCase(FXbool ignorecase)
    {
        grepigncase->setCheck(ignorecase);
    }

    // Set ignore case
    void setIgnoreCase(FXbool ignorecase)
    {
        searchpanel->setIgnoreCase(ignorecase);
    }

    // Get ignore case
    FXbool getIgnoreCase(void)
    {
        return(searchpanel->getIgnoreCase());
    }

    // Set directory first
    void setDirsFirst(FXbool dirsfirst)
    {
        searchpanel->setDirsFirst(dirsfirst);
    }

    // Set directory first
    FXbool getDirsFirst(void)
    {
        return(searchpanel->getDirsFirst());
    }

    // Get the current icon list style
    FXuint getListStyle(void) const
    {
        return(searchpanel->getListStyle());
    }

    // Get the current icon list style
    void setListStyle(FXuint style)
    {
        searchpanel->setListStyle(style);
    }

    // Thumbnails shown?
    FXbool shownThumbnails(void) const
    {
        return(searchpanel->shownThumbnails());
    }

    // Show thumbnails
    void showThumbnails(FXbool shown)
    {
        searchpanel->showThumbnails(shown);
    }

    // Get header size given its index
    int getHeaderSize(int index) const
    {
        return(searchpanel->getHeaderSize(index));
    }

    // Set search directory
    void setSearchPath(const FXString dir)
    {
        wheredir->setText(dir);
    }
	
	// Deselect all items
	void deselectAll(void)
	{
		searchpanel->deselectAll();
	}
};

#endif
