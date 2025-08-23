#pragma once
#include <fx.h>



/* class FXAPI I2KGroupBox : public FXGroupBox {
	FXDECLARE(I2KGroupBox)

	protected:
		I2KGroupBox(){}

	private:
		I2KGroupBox(const I2KGroupBox&);
		I2KGroupBox &operator=(const I2KGroupBox&);

	public:
		long onPaint(FXObject*,FXSelector,void*);
    I2KGroupBox(FXComposite* p, const FXString& text, FXuint opts=GROUPBOX_NORMAL, FXint x=0, FXint y=0, 
                FXint w=0, FXint h=0, FXint pl=DEFAULT_SPACING, FXint pr=DEFAULT_SPACING,
                FXint pt=DEFAULT_SPACING, FXint pb=DEFAULT_SPACING, FXint hs=DEFAULT_SPACING, FXint vs=DEFAULT_SPACING);

}; */
class FXAPI I2KGroupBox : public FXGroupBox {
    FXDECLARE(I2KGroupBox)

protected:
    // No default constructor needed
    I2KGroupBox(){}

private:
    I2KGroupBox(const I2KGroupBox&);
    I2KGroupBox &operator=(const I2KGroupBox&);

public:
    long onPaint(FXObject*, FXSelector, void*);

    I2KGroupBox(FXComposite* p, const FXString& text, FXuint opts=GROUPBOX_NORMAL, FXint x=0, FXint y=0,
                FXint w=0, FXint h=0, FXint pl=DEFAULT_SPACING, FXint pr=DEFAULT_SPACING,
                FXint pt=DEFAULT_SPACING, FXint pb=DEFAULT_SPACING, FXint hs=DEFAULT_SPACING, FXint vs=DEFAULT_SPACING);
};


class FXAPI I2KDragCorner : public FXDragCorner {
	FXDECLARE(I2KDragCorner)

	protected:
		I2KDragCorner(){}

	private:
		I2KDragCorner(const I2KDragCorner&);
		I2KDragCorner &operator=(const I2KDragCorner&);

	public:
		long onPaint(FXObject*,FXSelector,void*);
		virtual FXint getDefaultWidth();
		virtual FXint getDefaultHeight();

		I2KDragCorner(FXComposite* p);
};

class FXAPI I2KButton : public FXButton {
	FXDECLARE(I2KButton)

	protected:
		I2KButton(){}

	private:
		I2KButton(const I2KButton&);
		I2KButton &operator=(const I2KButton&);

	public:
		long onPaint(FXObject*,FXSelector,void*);
            void drawBorderRectangle(FXDCWindow&,FXint,FXint,FXint,FXint);

		I2KButton(FXComposite* p,const FXString& text,FXIcon* ic=NULL,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=BUTTON_NORMAL,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_PAD,FXint pr=DEFAULT_PAD,FXint pt=DEFAULT_PAD,FXint pb=DEFAULT_PAD);

};


#ifndef _DO_NOT_OVERRIDE_FOX_CONTROLS
	#define FXButton I2KButton
	#define FXDragCorner I2KDragCorner
	#define FXGroupBox I2KGroupBox
	#define _DO_NOT_OVERRIDE_FOX_CONTROLS
#endif
