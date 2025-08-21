#ifndef BROWSEINPUTDIALOG_H
#define BROWSEINPUTDIALOG_H

#include "TextLabel.h"
#include "DialogBox.h"

// Browse types
enum
{
    BROWSE_INPUT_FILE,
    BROWSE_INPUT_FOLDER,
    BROWSE_INPUT_MIXED
};

class XComApp;

class BrowseInputDialog : public DialogBox
{
    FXDECLARE(BrowseInputDialog)
protected:
    FXTextField*       input;
    TextLabel*         msg;
    FXLabel*           iconlabel;
    FXHorizontalFrame* checkbutton;
    FXuint             browsetype;
    FXString           initialdir;
private:
    BrowseInputDialog() : input(NULL), msg(NULL), iconlabel(NULL), checkbutton(NULL), browsetype(0)
    {}
public:
    enum
    {
        ID_BROWSE_PATH=DialogBox::ID_LAST,
        ID_LAST
    };
    BrowseInputDialog(FXWindow*, FXString, FXString, FXString, FXString label = "", FXIcon* ic = NULL, FXuint browse = BROWSE_INPUT_FILE, FXbool option = false, FXString = FXString::null);
    virtual void create();

    virtual ~BrowseInputDialog();
    long onCmdKeyPress(FXObject*, FXSelector, void*);
    long onCmdBrowsePath(FXObject*, FXSelector, void*);
    void setMessage(FXString);
    void setIcon(FXIcon*);
    void setDirectory(const FXString&);
    FXString getText();
    void setText(const FXString&);
    void selectAll();
    void CursorEnd();
    void setSelection(int, int);
};
#endif
