#ifndef PATHLINKER_H
#define PATHLINKER_H

#include <vector>
#include "TextLabel.h"
#include "FileList.h"
#include "DirPanel.h"
#include "xfedefs.h"

class FXAPI PathLinker : public FXHorizontalFrame
{
    FXDECLARE(PathLinker)

protected:
    typedef std::vector<FXButton*>   vector_FXButton;
    vector_FXButton linkButtons;
    FXuint          nbActiveButtons;
    FXuint          currentButton;
    FXString        visitedPath;
    TextLabel*      focusButton;
    FXFont*         normalFont;
    FXFont*         highlightFont;
    FileList*       filelist;
    DirList*        dirlist;
    PathLinker() : nbActiveButtons(0), currentButton(0), focusButton(NULL), normalFont(NULL), highlightFont(NULL), filelist(NULL), dirlist(NULL)
    {}

private:
    void updatePath(FXString, FXuint);
    void setText(FXuint, FXString);

public:
    enum ButtonIds
    {
        ID_START_LINK = FXHorizontalFrame::ID_LAST,
        // Note: Place any additional id's AFTER ID_END_LINK
        ID_END_LINK = ID_START_LINK + MAX_LINKS - 1,
        ID_FOCUS_BUTTON,
        ID_LAST
    };

    PathLinker(FXComposite* a, FileList* flist, DirList* dlist = NULL, FXuint opts = 0);
    virtual void create();

    virtual ~PathLinker();
    long pathButtonPressed(FXObject*, FXSelector, void*);
    long onCmdFocusButton(FXObject*, FXSelector, void*);
    long onUpdPath(FXObject*, FXSelector, void*);
    void setPath(FXString);
    void focus();
    void unfocus();
};

#endif
