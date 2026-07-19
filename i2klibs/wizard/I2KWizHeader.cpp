#include <ice2k/comctl32.h>
#include "ice2k/wizard/I2KWizHeader.h"

FXDEFMAP(I2KWizHeader) I2KWizHeaderMap[] = {
	FXMAPFUNC(SEL_PAINT, 0, I2KWizHeader::onPaint),
};

FXIMPLEMENT(I2KWizHeader, FXFrame, I2KWizHeaderMap, ARRAYNUMBER(I2KWizHeaderMap));

I2KWizHeader::I2KWizHeader()
	: FXFrame(), image(NULL), txt(""), subtxt("")
{
}

I2KWizHeader::I2KWizHeader(FXComposite* p, FXImage* img,
	const FXString& text, const FXString& subtext,
	FXuint opts,
	FXint x, FXint y, FXint w, FXint h,
	FXint pl, FXint pr, FXint pt, FXint pb)
	: FXFrame(p, opts, x, y, w, h, pl, pr, pt, pb),
	image(img), txt(text), subtxt(subtext)
{
	imgwidth = 0;
	imgheight = 0;

	noboldchange = TRUE;
	
	setBackColor(getApp()->getBackColor());

	textColor = getApp()->getForeColor();
	subtextColor = textColor;

	if (img) {
		imgwidth = img->getWidth();
		imgheight = img->getHeight();
	}

	normalfont = NULL;
	boldfont = NULL;
	
	//puts("Begin create font");
	normalfont = getApp()->getNormalFont();

	ttextoff = normalfont->getFontAscent();
	ftextoff = normalfont->getFontHeight();

	FXFontDesc desc;
	normalfont->getFontDesc(desc);
	desc.weight = FXFont::Bold;

	boldfont = new FXFont(getApp(), desc);
	//puts("Creating font NOW");
	boldfont->create();

	tboldoff = boldfont->getFontAscent();
	fboldoff = boldfont->getFontHeight();

	//puts("End create font");

	if (img) img->create();

}

I2KWizHeader::~I2KWizHeader() {
	if (boldfont && noboldchange) {
		delete boldfont;
		boldfont = NULL;
	}
}

//#define IMGWIDTH 49
//#define IMGHEIGHT 49

long I2KWizHeader::onPaint(FXObject* sender, FXSelector sel, void* ptr) {
	FXEvent* event = (FXEvent*)ptr;



	FXDCWindow dc(this, event);

	drawFrame(dc,0,0,width,height);

	FXuint loff = padleft+border;
	FXuint toff = padtop+border;
	FXuint roff = loff*2;
	FXuint boff = toff*2;

	FXuint textxoff = loff+23;
	FXuint textyoff = loff+10;

	FXuint boldxoff = loff+45;

	dc.setForeground(backColor);

	dc.fillRectangle(loff, toff, width-roff, height-boff);

	dc.setForeground(textColor);
	dc.setFont(boldfont);
	
	int pos = 0;
	int start = 0;
	int titleh = 0;

	while(start < txt.length()) {
		pos = txt.find('\n', start);
		titleh += tboldoff;

		if (pos == -1) {
			int len = txt.length() - start;
			dc.drawText(textxoff,textyoff+titleh,txt.text()+start,len);
			break;
		} else {
			int len = pos - start;
			dc.drawText(textxoff,textyoff+titleh,txt.text()+start,len);

			start = pos + 1;
			titleh += 3;
			//titleh += tboldoff;
		}
	}

	dc.setForeground(subtextColor);
	dc.setFont(normalfont);
	dc.drawText(boldxoff,textyoff+tboldoff+fboldoff+1,subtxt);

	FXuint imgtoff = toff+(height-imgheight)/2-boff;

	//dc.fillRectangle(width-loff-5-imgwidth, toff+imgtoff, imgwidth, imgheight);
	if (image) dc.drawImage(image, width-loff-5-imgwidth, toff+imgtoff);

	dc.setForeground(hiliteColor);
	dc.fillRectangle(loff, height-boff-1, width-roff, 1);
	//dc.setForeground(FXRGB(255,255,0));
	dc.drawPoint(width-roff-1, height-boff-2);
	dc.setForeground(shadowColor);
	dc.fillRectangle(loff, height-boff-2, width-roff-1, 1);
	return 1;
}

void I2KWizHeader::setFont(FXFont* newFont) {
	if (newFont == NULL) return;

	if (boldfont && noboldchange) {
		delete boldfont;

		noboldchange = FALSE;
	}

	boldfont = newFont;

	boldfont->create();

	tboldoff = boldfont->getFontAscent();
	fboldoff = boldfont->getFontHeight();
}

void I2KWizHeader::setSubtextFont(FXFont* newFont) {
	if (newFont == NULL) return;

	normalfont = newFont;

	normalfont->create();

	ttextoff = normalfont->getFontAscent();
	ftextoff = normalfont->getFontHeight();

	update();
}

FXFont* I2KWizHeader::getFont() const {
	return boldfont;
}

FXFont* I2KWizHeader::getSubtextFont() const {
	return normalfont;
}

void I2KWizHeader::create() {
	FXFrame::create();
}

FXint I2KWizHeader::getDefaultWidth() {
	return 497;
}

FXint I2KWizHeader::getDefaultHeight() {
	return 60;
}

void I2KWizHeader::setImage(FXImage* img) {
	image = img;

	if (img) {
		imgwidth = img->getWidth();
		imgheight = img->getHeight();
	}

	update();
}

FXImage* I2KWizHeader::getImage() const {
	return image;
}

void I2KWizHeader::setText(const FXString& text) {
	txt = text;
	update();
}

const FXString& I2KWizHeader::getText() const {
	return txt;
}

void I2KWizHeader::setSubtext(const FXString& text) {
	subtxt = text;
	update();
}

const FXString& I2KWizHeader::getSubtext() const {
	return subtxt;
}

FXColor I2KWizHeader::getTextColor() const {
	return textColor;
}

FXColor I2KWizHeader::getSubtextColor() const {
	return subtextColor;
}

void I2KWizHeader::setTextColor(FXColor clr) {
	textColor = clr;
	update();
}

void I2KWizHeader::setSubtextColor(FXColor clr) {
	subtextColor = clr;
	update();
}

void I2KWizHeader::save(FXStream& store) const {
	FXFrame::save(store);

	store << txt;
	store << subtxt;

	store << image;

	store << textColor;
	store << subtextColor;
}

void I2KWizHeader::load(FXStream& store) {
	FXFrame::load(store);

	store >> txt;
	store >> subtxt;

	store >> image;

	if (image) {
		imgwidth = image->getWidth();
		imgheight = image->getHeight();
	} else {
		imgwidth = imgheight = 0;
	}

	store >> textColor;
	store >> subtextColor;

	update();
}

