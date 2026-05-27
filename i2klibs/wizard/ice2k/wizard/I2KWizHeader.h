#pragma once
#include "fx.h"

class I2KWizHeader : public FXFrame {
	FXDECLARE(I2KWizHeader)

protected:
	FXImage* image;
	FXString txt;
	FXString subtxt;

	FXFont* normalfont;
	FXFont* boldfont;

	FXuint ttextoff;
	FXuint ftextoff;
	FXuint tboldoff;
	FXuint fboldoff;

	FXColor textColor;
	FXColor subtextColor;

	FXuint imgwidth;
	FXuint imgheight;

	FXbool noboldchange;

protected:
	I2KWizHeader();

private:
	I2KWizHeader(const I2KWizHeader&);
	I2KWizHeader& operator=(const I2KWizHeader&);

public:
	long onPaint(FXObject*,FXSelector,void*);

public:
	I2KWizHeader(FXComposite* p,
	            FXImage* img,
	            const FXString& text,
	            const FXString& subtext,
	            FXuint opts=FRAME_NONE,
	            FXint x=0,FXint y=0,FXint w=0,FXint h=0,
	            FXint pl=0,FXint pr=0,FXint pt=0,FXint pb=0);

	virtual void create();

	virtual FXint getDefaultWidth();
	virtual FXint getDefaultHeight();

	void setImage(FXImage* img);
	FXImage* getImage() const;

	void setText(const FXString& text);
	const FXString& getText() const;

	void setSubtext(const FXString& text);
	const FXString& getSubtext() const;

	FXColor getTextColor() const;
	FXColor getSubtextColor() const;

	FXFont* getFont() const;
	FXFont* getSubtextFont() const;

	void setTextColor(FXColor clr);
	void setSubtextColor(FXColor clr);

	void setFont(FXFont* newFont);
	void setSubtextFont(FXFont* newFont);

	virtual void save(FXStream& store) const;
	virtual void load(FXStream& store);

	virtual ~I2KWizHeader();
};

