#include "config.h"
#include "i18n.h"

#include <fx.h>
#include <fxdefs.h>
#include <FXPNGIcon.h>

#include "icons.h"
#include "xfedefs.h"
#include "xfeutils.h"
#include "OverwriteBox.h"

// Padding for message box buttons
#define HORZ_PAD    30
#define VERT_PAD    2


// Map
FXDEFMAP(OverwriteBox) OverwriteBoxMap[] =
{
    FXMAPFUNCS(SEL_COMMAND, OverwriteBox::ID_CLICKED_CANCEL, OverwriteBox::ID_CLICKED_SKIP_ALL, OverwriteBox::onCmdClicked),
};



// Object implementation
FXIMPLEMENT(OverwriteBox, DialogBox, OverwriteBoxMap, ARRAYNUMBER(OverwriteBoxMap))


// Create message box with text
OverwriteBox::OverwriteBox(FXWindow* win, const FXString& name, const FXString& text, FXuint type, FXuint opts, int x, int y) :
    DialogBox(win, name, opts|DECOR_TITLE|DECOR_BORDER|DECOR_RESIZE|DECOR_CLOSE, x, y, 0, 0)
{
    FXVerticalFrame*   content = new FXVerticalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXHorizontalFrame* info = new FXHorizontalFrame(content, LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 0, 0, 0, 10, 10, 10, 10);

    new FXLabel(info, FXString::null, questionbigicon, ICON_BEFORE_TEXT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    // Set message text with a maximum of MAX_MESSAGE_LENGTH characters per line
    FXString str = ::multiLines(text, MAX_MESSAGE_LENGTH);
    new FXLabel(info, str, NULL, JUSTIFY_LEFT|ICON_BEFORE_TEXT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    FXHorizontalFrame* buttons = new FXHorizontalFrame(content, LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH, 0, 0, 0, 0, 10, 10, 10, 10);
	
	// Dialog with five options for multiple files
	if (type == OVWBOX_MULTIPLE_FILES)
	{
		new FXButton(buttons, _("&Cancel"), NULL, this, ID_CLICKED_CANCEL, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
		new FXButton(buttons, _("&Skip"), NULL, this, ID_CLICKED_SKIP, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
		new FXButton(buttons, _("Skip A&ll"), NULL, this, ID_CLICKED_SKIP_ALL, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
		new FXButton(buttons, _("&Yes"), NULL, this, ID_CLICKED_OVERWRITE, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
		new FXButton(buttons, _("Yes for &All"), NULL, this, ID_CLICKED_OVERWRITE_ALL, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
	}
	
	// Dialog with two options for single file
	else
	{
		new FXButton(buttons, _("&Cancel"), NULL, this, ID_CLICKED_CANCEL, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
		new FXButton(buttons, _("&Yes"), NULL, this, ID_CLICKED_OVERWRITE, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
	}
}


// Create message box with text, source and target size, source and target modified time
OverwriteBox::OverwriteBox(FXWindow* win, const FXString& name, const FXString& text, FXString& srcsize, FXString& srcmtime, FXString& tgtsize, FXString& tgtmtime, FXuint type, FXuint opts, int x, int y) :
    DialogBox(win, name, opts|DECOR_TITLE|DECOR_BORDER|DECOR_RESIZE|DECOR_CLOSE, x, y, 0, 0)
{
    FXVerticalFrame* content = new FXVerticalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXVerticalFrame* vframe = new FXVerticalFrame(content, LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 0, 0, 0, 0, 0, 0, 0);

    FXHorizontalFrame* info = new FXHorizontalFrame(vframe, LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 0, 0, 0, 0, 0, 0, 0);

    new FXLabel(info, FXString::null, questionbigicon, ICON_BEFORE_TEXT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    // Set message text with a maximum of MAX_MESSAGE_LENGTH characters per line
    FXString str = ::multiLines(text, MAX_MESSAGE_LENGTH);
    new FXLabel(info, str, NULL, JUSTIFY_LEFT|ICON_BEFORE_TEXT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    FXMatrix* matrix = new FXMatrix(vframe, 5, MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    new FXVerticalSeparator(matrix, SEPARATOR_NONE|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_CENTER_Y, 0, 0, 0, 0, 0, 40);
    new FXLabel(matrix, _("Source size:"), NULL, JUSTIFY_LEFT|ICON_BEFORE_TEXT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(matrix, srcsize, NULL, JUSTIFY_LEFT|ICON_BEFORE_TEXT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(matrix, _("- Modified date:"), NULL, JUSTIFY_LEFT|ICON_BEFORE_TEXT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(matrix, srcmtime, NULL, JUSTIFY_LEFT|ICON_BEFORE_TEXT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    new FXVerticalSeparator(matrix, SEPARATOR_NONE|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_CENTER_Y, 0, 0, 0, 0, 0, 40);
    new FXLabel(matrix, _("Target size:"), NULL, JUSTIFY_LEFT|ICON_BEFORE_TEXT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(matrix, tgtsize, NULL, JUSTIFY_LEFT|ICON_BEFORE_TEXT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(matrix, _("- Modified date:"), NULL, JUSTIFY_LEFT|ICON_BEFORE_TEXT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(matrix, tgtmtime, NULL, JUSTIFY_LEFT|ICON_BEFORE_TEXT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    FXHorizontalFrame* buttons = new FXHorizontalFrame(content, LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH, 0, 0, 0, 0, 10, 10, 10, 10);

	// Dialog with five options for multiple files
	if (type == OVWBOX_MULTIPLE_FILES)
	{
		new FXButton(buttons, _("&Cancel"), NULL, this, ID_CLICKED_CANCEL, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
		new FXButton(buttons, _("&Skip"), NULL, this, ID_CLICKED_SKIP, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
		new FXButton(buttons, _("Skip A&ll"), NULL, this, ID_CLICKED_SKIP_ALL, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
		new FXButton(buttons, _("&Yes"), NULL, this, ID_CLICKED_OVERWRITE, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
		new FXButton(buttons, _("Yes for &All"), NULL, this, ID_CLICKED_OVERWRITE_ALL, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
	}

	// Dialog with two options for single file
	else
	{
		new FXButton(buttons, _("&Cancel"), NULL, this, ID_CLICKED_CANCEL, BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
		new FXButton(buttons, _("&Yes"), NULL, this, ID_CLICKED_OVERWRITE, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X, 0, 0, 0, 0, HORZ_PAD, HORZ_PAD, VERT_PAD, VERT_PAD);
	}
}



// Close dialog
long OverwriteBox::onCmdClicked(FXObject*, FXSelector sel, void*)
{
    getApp()->stopModal(this, OVWBOX_CLICKED_CANCEL+(FXSELID(sel)-ID_CLICKED_CANCEL));
    hide();
    return(1);
}
