#ifndef HISTINPUTDIALOG_H
#define HISTINPUTDIALOG_H

#include "DialogBox.h"

// Browse types
enum
{
    HIST_INPUT_FILE,
    HIST_INPUT_EXECUTABLE_FILE,
    HIST_INPUT_FOLDER,
    HIST_INPUT_MIXED
};

class XComApp;

class ComboBox : public FXComboBox
{
    FXDECLARE(ComboBox)
private:
    ComboBox()
    {
    }

public:
    FXTextField* getTextEntry()
    {
        return(field);
    }

    void CursorEnd()
    {
        field->onCmdCursorEnd(0, 0, 0);
        field->setFocus();
    }

    ComboBox(FXComposite* p, int cols, FXObject* tgt = NULL, FXSelector sel = 0, FXuint opts = COMBOBOX_NORMAL);
    virtual void create();
};

class HistInputDialog : public DialogBox
{
    FXDECLARE(HistInputDialog)
protected:
    FXHorizontalFrame* buttons;
    FXHorizontalFrame* checkbutton;
    ComboBox*          input;
    FXuint             browsetype;
    FXString           initialdir;
private:
    HistInputDialog() : buttons(NULL), checkbutton(NULL), input(NULL), browsetype(0)
    {}
public:
    enum
    {
        ID_BROWSE_PATH=DialogBox::ID_LAST,
        ID_LAST
    };
    HistInputDialog(FXWindow*, FXString, FXString, FXString, FXString label = "", FXIcon* ic = NULL, FXuint browse = HIST_INPUT_FILE, FXbool option = false, FXString = FXString::null);
    virtual void create();

    long onCmdKeyPress(FXObject*, FXSelector, void*);
    long onCmdBrowsePath(FXObject*, FXSelector, void*);
    FXString getText()
    {
        return(input->getText());
    }

    void setText(const FXString& text)
    {
        input->setText(text);
    }

    void CursorEnd();
    void selectAll();

    void appendItem(char* str)
    {
        input->appendItem(str);
    }

    void clearItems()
    {
        input->clearItems();
    }

    FXString getHistoryItem(int pos)
    {
        return(input->getItemText(pos));
    }

    int getHistorySize()
    {
        return(input->getNumItems());
    }

    void setDirectory(const FXString&);

    void sortItems()
    {
        input->setSortFunc(FXList::ascendingCase);
        input->sortItems();
    }

};
#endif
