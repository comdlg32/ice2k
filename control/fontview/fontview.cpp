#include <fx.h>
// #include <ice2k/comctl32.h>

#include "res/foxres.h"

FXIcon* ttficon;
FXIcon* bmpicon;

FXIcon* winicon = NULL;

char fontname[512] = {0};

class FontViewer : public FXMainWindow {
	FXDECLARE(FontViewer);

protected:
	FontViewer() {}

private:
	FXHorizontalFrame* buttoncont;
	FXScrollWindow* scrollwin;
	FXVerticalFrame* scrollcont;

	FXLabel* headerlbl;
	FXLabel* alphabetlbl;

	FXMatrix* fontmtx;

	FXLabel* num12;
	FXLabel* num18;
	FXLabel* num24;
	FXLabel* num36;
	FXLabel* num48;
	FXLabel* num60;
	FXLabel* num72;

	FXLabel* text12;
	FXLabel* text18;
	FXLabel* text24;
	FXLabel* text36;
	FXLabel* text48;
	FXLabel* text60;
	FXLabel* text72;

	FXFont* fonttitle;
	FXFont* fontnum;

	FXFont* font12;
	FXFont* font16;
	FXFont* font18;
	FXFont* font24;
	FXFont* font36;
	FXFont* font48;
	FXFont* font60;
	FXFont* font72;


private:
	FXFont* makeFont(FXFont* );

public:
	long onCmdHello(FXObject*, FXSelector, void*);
	void setFocus() {};

public:
	enum {
		ID_MAINWIN = FXMainWindow::ID_LAST,
		ID_HELLO,
		ID_LAST
	};

public:
	FontViewer(FXApp* a);

	virtual void create();
	virtual ~FontViewer();
};

FXDEFMAP(FontViewer) FontViewerMap[] = {
	FXMAPFUNC(SEL_COMMAND,           FontViewer::ID_HELLO,  FontViewer::onCmdHello),
};

FXIMPLEMENT(FontViewer, FXMainWindow, FontViewerMap, ARRAYNUMBER(FontViewerMap));

FontViewer::FontViewer(FXApp *a) : FXMainWindow(a, "Hello World!", winicon, NULL, DECOR_ALL, 0,0,640,480) {
	const char brownfox[] = "The quick brown fox jumps over the lazy dog. 1234567890";

	buttoncont = new FXHorizontalFrame(this, LAYOUT_FILL_X|PACK_UNIFORM_WIDTH, 0,0,0,0, 8,8,8,6, 0,0);
	new FXButton(buttoncont, "&Done", NULL, getApp(), FXApp::ID_QUIT, BUTTON_NORMAL|BUTTON_DEFAULT, 0,0,0,0, 14,14,1,2);
	FXButton* printbtn = new FXButton(buttoncont, "&Print", NULL, NULL, 0, BUTTON_NORMAL|LAYOUT_RIGHT, 0,0,0,0, 14,14,1,2);
	printbtn->disable();
	FXFrame* sep = new FXFrame(this, FRAME_NONE|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_X, 0,0,0,2, 0,0,0,0);
	sep->setBackColor(getApp()->getShadowColor());

	scrollwin = new FXScrollWindow(this, LAYOUT_FILL_X|LAYOUT_FILL_Y|HSCROLLER_NEVER|SCROLLERS_DONT_TRACK);
	scrollcont = new FXVerticalFrame(scrollwin, LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0, 0,0);
	scrollcont->setBackColor(getApp()->getBackColor());

	headerlbl = new FXLabel(scrollcont, fontname, NULL, LAYOUT_FILL_X|JUSTIFY_LEFT, 0,0,0,0, 0,0,0,0);
	headerlbl->setBackColor(getApp()->getBackColor());

	new FXHorizontalSeparator(scrollcont, SEPARATOR_LINE|LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0);

	//headerlbl = new FXLabel(scrollcont, fontname, NULL, 0,0,0,0, 0,0,0,0);

	setTitle(fontname);
	alphabetlbl = new FXLabel(scrollcont,
			"abcdefghijklmnopqrstuvwxyz\n"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"
			"1234567890.:,;(:*!?')", NULL,
			LAYOUT_FILL_X|JUSTIFY_LEFT, 0,0,0,0, 0,0,0,0);
	alphabetlbl->setBackColor(getApp()->getBackColor());
	new FXHorizontalSeparator(scrollcont, SEPARATOR_LINE|LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0);


	fontmtx = new FXMatrix(scrollcont, 2, MATRIX_BY_COLUMNS, 0,0,0,0, 0,0,0,0, 16, 0);
	fontmtx->setBackColor(getApp()->getBackColor());

	num12  = new FXLabel(fontmtx, "12", NULL, LAYOUT_BOTTOM|LAYOUT_FILL_X|JUSTIFY_LEFT, 0,0,0,0, 0,0,2,2);
	text12 = new FXLabel(fontmtx, brownfox, NULL, LAYOUT_BOTTOM|LAYOUT_FILL_X|JUSTIFY_LEFT, 0,0,0,0, 0,0,2,2);

	num12->setBackColor(getApp()->getBackColor());
	text12->setBackColor(getApp()->getBackColor());

	num18  = new FXLabel(fontmtx, "18", NULL, LAYOUT_BOTTOM|LAYOUT_FILL_X|JUSTIFY_LEFT, 0,0,0,0, 0,0,2,2);
	text18 = new FXLabel(fontmtx, brownfox, NULL, LAYOUT_BOTTOM|LAYOUT_FILL_X|JUSTIFY_LEFT, 0,0,0,0, 0,0,2,2);

	num18->setBackColor(getApp()->getBackColor());
	text18->setBackColor(getApp()->getBackColor());

	num24  = new FXLabel(fontmtx, "24", NULL, LAYOUT_BOTTOM|LAYOUT_FILL_X|JUSTIFY_LEFT, 0,0,0,0, 0,0,2,2);
	text24 = new FXLabel(fontmtx, brownfox, NULL, LAYOUT_BOTTOM|LAYOUT_FILL_X|JUSTIFY_LEFT, 0,0,0,0, 0,0,2,2);

	num24->setBackColor(getApp()->getBackColor());
	text24->setBackColor(getApp()->getBackColor());

	num36  = new FXLabel(fontmtx, "36", NULL, LAYOUT_BOTTOM|LAYOUT_FILL_X|JUSTIFY_LEFT, 0,0,0,0, 0,0,2,2);
	text36 = new FXLabel(fontmtx, brownfox, NULL, LAYOUT_BOTTOM|LAYOUT_FILL_X|JUSTIFY_LEFT, 0,0,0,0, 0,0,2,2);

	num36->setBackColor(getApp()->getBackColor());
	text36->setBackColor(getApp()->getBackColor());

	num48  = new FXLabel(fontmtx, "48", NULL, LAYOUT_BOTTOM|LAYOUT_FILL_X|JUSTIFY_LEFT, 0,0,0,0, 0,0,2,2);
	text48 = new FXLabel(fontmtx, brownfox, NULL, LAYOUT_BOTTOM|LAYOUT_FILL_X|JUSTIFY_LEFT, 0,0,0,0, 0,0,2,2);

	num48->setBackColor(getApp()->getBackColor());
	text48->setBackColor(getApp()->getBackColor());

	num60  = new FXLabel(fontmtx, "60", NULL, LAYOUT_BOTTOM|LAYOUT_FILL_X|JUSTIFY_LEFT, 0,0,0,0, 0,0,2,2);
	text60 = new FXLabel(fontmtx, brownfox, NULL, LAYOUT_BOTTOM|LAYOUT_FILL_X|JUSTIFY_LEFT, 0,0,0,0, 0,0,2,2);

	num60->setBackColor(getApp()->getBackColor());
	text60->setBackColor(getApp()->getBackColor());

	num72  = new FXLabel(fontmtx, "72", NULL, LAYOUT_BOTTOM|LAYOUT_FILL_X|JUSTIFY_LEFT, 0,0,0,0, 0,0,2,2);
	text72 = new FXLabel(fontmtx, brownfox, NULL, LAYOUT_BOTTOM|LAYOUT_FILL_X|JUSTIFY_LEFT, 0,0,0,0, 0,0,2,2);

	num72->setBackColor(getApp()->getBackColor());
	text72->setBackColor(getApp()->getBackColor());


	//new FXButton(this, "Hello World!", NULL, this, ID_HELLO, BUTTON_NORMAL);
}

FontViewer::~FontViewer() {
}

void FontViewer::create() {
	FXMainWindow::create();

	FXString normalfontname;
	FXuint size;
	FXuint weight;
	FXuint slant;
	FXuint encoding;
	FXuint setwidth;
	FXuint hints;
	
	FXFont* normalfont = getApp()->getNormalFont();

	normalfontname = normalfont->getFont();
	size = normalfont->getSize()/10;
	weight = normalfont->getWeight();
	slant = normalfont->getSlant();
	encoding = normalfont->getEncoding();
	setwidth = normalfont->getSetWidth();
	hints = normalfont->getHints();

	fonttitle = new FXFont(getApp(), normalfontname, 30, weight, slant, encoding, setwidth, hints);
	if (fonttitle == NULL) {
		fonttitle = normalfont;
	} else {
		fonttitle->create();
	}

	fontnum = new FXFont(getApp(), normalfontname, size+1, weight, slant, encoding, setwidth, hints);
	if  (fontnum == NULL) fontnum = normalfont;
	else fontnum->create();


	font12 = new FXFont(getApp(), fontname, 12);
	if  (font12 == NULL) font12 = normalfont;
	else font12->create();

	num12->setFont(fontnum);
	text12->setFont(font12);

	font16 = new FXFont(getApp(), fontname, 16);
	if  (font16 == NULL) font16 = normalfont;
	else font16->create();

	alphabetlbl->setFont(font16);

	font18 = new FXFont(getApp(), fontname, 18);
	if  (font18 == NULL) font18 = normalfont;
	else font18->create();

	num18->setFont(fontnum);
	text18->setFont(font18);

	font24 = new FXFont(getApp(), fontname, 24);
	if	(font24 == NULL) font24 = normalfont;
	else font24->create();

	num24->setFont(fontnum);
	text24->setFont(font24);

	font36 = new FXFont(getApp(), fontname, 36);
	if	(font36 == NULL) font36 = normalfont;
	else font36->create();

	num36->setFont(fontnum);
	text36->setFont(font36);

	font48 = new FXFont(getApp(), fontname, 48);
	if	(font48 == NULL) font48 = normalfont;
	else font48->create();

	num48->setFont(fontnum);
	text48->setFont(font48);

	font60 = new FXFont(getApp(), fontname, 60);
	if	(font60 == NULL) font60 = normalfont;
	else font60->create();

	num60->setFont(fontnum);
	text60->setFont(font60);

	font72 = new FXFont(getApp(), fontname, 72);
	if	(font72 == NULL) font72 = normalfont;
	else font72->create();

	num72->setFont(fontnum);
	text72->setFont(font72);

	headerlbl->setFont(fonttitle);

	show(PLACEMENT_SCREEN);
}

long FontViewer::onCmdHello(FXObject*, FXSelector, void*) {
	puts("Hello world!");
	return 1;
}

int main(int argc, char *argv[]) {
	FXApp application("FontViewer", "I2KProject");
	bmpicon = new FXGIFIcon(&application, resico_bmpfont);
	ttficon = new FXGIFIcon(&application, resico_ttffont);

	application.init(argc, argv);

	if (argv[1] == NULL || argv[1][0] == '\0') {
		application.create();
		FXMessageBox::error(&application, MBOX_OK, "Error", "Please specify font name in argument!");
		return 1;
	}

	sprintf(fontname, "%.*s", (int)sizeof(fontname)-1, argv[1]);

	FXFontDesc* fonts;
	FXuint numfonts;

	if (FXFont::listFonts(fonts, numfonts, fontname)) {
		winicon = bmpicon;

		for (FXuint i = 0; i < numfonts; i++) {
			if (fonts[i].flags & FXFont::Scalable) {
				winicon = ttficon;
				break;
			}
		}

		FXFREE(&fonts);
	} else {
		application.create();
		FXMessageBox::error(&application, MBOX_OK, "Error", "Font does not exist!");
		return 1;
	}

	new FontViewer(&application);

	application.create();
	return application.run();
}
