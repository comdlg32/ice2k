#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include "DialogBox.h"

class XComApp;

class InputDialog : public DialogBox
{
    FXDECLARE(InputDialog)
protected:
    FXTextField*       input;
    FXHorizontalFrame* checkbutton;
    FXLabel*           msg;
private:
    InputDialog() : input(NULL), checkbutton(NULL), msg(NULL)
    {}
public:
    InputDialog(FXWindow*, FXString, FXString, FXString, FXString label = "", FXIcon* icon = NULL, FXbool option = false, FXString = FXString::null);
    virtual void create();

    long onCmdKeyPress(FXObject*, FXSelector, void*);
    FXString getText()
    {
        return(input->getText());
    }

    void setText(const FXString& text)
    {
        input->setText(text);
    }

	void setMessage(const FXString& text)
	{
		msg->setText(text);
	}

    void selectAll()
    {
        input->setSelection(0, (input->getText()).length());
    }

    void CursorEnd()
    {
        input->onCmdCursorEnd(0, 0, 0);
    }

    void setSelection(int pos, int len)
    {
        input->setSelection(pos, len);
    }
};
#endif
