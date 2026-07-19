#define _USE_MATH_DEFINES
#include <fx.h>
#include <ice2k/comctl32.h>
#include <math.h>
#include <time.h>

#ifndef M_SQRT2
#define M_SQRT2 1.41421356237309504880
#endif

#include <ice2k/FXExtra/FXCalendarView.h>
#include "res/foxres.h"

#include <stdint.h>

FXIcon* mainIcon;
FXImage* globeImg;

class DSpinner : public FXSpinner {
	FXDECLARE(DSpinner);

protected:
	DSpinner() {}

};

time_t getLocalTime(struct tm** tmstruct) {
	time_t timeval = 0;
	time_t rtime = time(0);
	struct tm* ltime;

	ltime = localtime(&rtime);
	timeval = timegm(ltime);

	if (tmstruct != NULL) *tmstruct = ltime;

	return timeval;
}

time_t localToUtc(time_t local) {
	struct tm* ltime = gmtime(&local);
	if (ltime == NULL) return (time_t)0;

	ltime->tm_isdst = -1;

	return mktime(ltime);
}

time_t getLocalTimeOffset(struct tm** tmstruct) {
	time_t timeval = 0;
	time_t rtime = time(0);
		
	time_t ltime = 0;
	time_t utime = 0;

	struct tm* ltimetm;

	ltimetm = localtime(&rtime);
	ltime = mktime(ltimetm);
	utime = timegm(ltimetm);

	timeval = utime - ltime;

	if (tmstruct != NULL) *tmstruct = ltimetm;

	return timeval;
}

class I2KClock: public FXFrame {
	FXDECLARE(I2KClock);
protected:
	time_t        timeval;

	long long     days;
	
	int           hours;
	int           minutes;
	int           seconds;

	FXImage*      clockdot;
	FXImage*      bluedot;

	FXObject*     target;
	FXSelector    selector;

	int centerx;
	int centery;

	int facerad;

	int hourang;
	int minuteang;
	int secondang;

	FXDCWindow* globdc;

protected:
	I2KClock() {}
			

public:


	enum {
		ID_CANVAS = FXFrame::ID_LAST,
		//ID_TIMER,
		ID_LAST
	};

private:
	int drawBigHand(FXDCWindow*);
	
public:
	I2KClock(FXComposite* parent, FXuint opts=FRAME_NONE, FXint x=0, FXint y=0, FXint w=0, FXint h=0,
			FXint pl=DEFAULT_PAD, FXint pr=DEFAULT_PAD, FXint pt=DEFAULT_PAD, FXint pb=DEFAULT_PAD):
		FXFrame(parent, opts, x,y,w,h,pl,pr,pt,pb)
		/*time(1781040187)*/ /* as test */
	{
		FXColor* imgptr;

		/*FXColor backcolor = getBaseColor();
		FXColor hilitecolor = getHiliteColor();
		FXColor shadowcolor = getShadowColor();*/

		/*timeval = getLocalTime(NULL);*/

		clockdot = new FXImage(getApp(), NULL, IMAGE_OPAQUE|IMAGE_OWNED, 3,3);
		clockdot->resize(3, 3);

		imgptr = clockdot->getData();

		imgptr[0*3+0] = shadowColor;
		imgptr[0*3+1] = shadowColor;
		imgptr[0*3+2] = backColor;

		imgptr[1*3+0] = shadowColor;
		imgptr[1*3+1] = backColor;
		imgptr[1*3+2] = hiliteColor;

		imgptr[2*3+0] = backColor;
		imgptr[2*3+1] = hiliteColor;
		imgptr[2*3+2] = hiliteColor;

		clockdot->render();
		clockdot->create();



		bluedot = new FXImage(getApp(), NULL, IMAGE_OPAQUE|IMAGE_OWNED, 4,4);
		bluedot->resize(4, 4);

		imgptr = bluedot->getData();

		imgptr[0*4+0] = FXRGB(0,   255, 255);
		imgptr[0*4+1] = FXRGB(0,   255, 255);
		imgptr[0*4+2] = FXRGB(0,   255, 255);
		imgptr[0*4+3] = FXRGB(0,   0,   0);

		imgptr[1*4+0] = FXRGB(0,   255, 255);
		imgptr[1*4+1] = FXRGB(0,   128, 128);
		imgptr[1*4+2] = FXRGB(0,   128, 128);
		imgptr[1*4+3] = FXRGB(0,   0,   0);

		imgptr[2*4+0] = FXRGB(0,   255, 255);
		imgptr[2*4+1] = FXRGB(0,   128, 128);
		imgptr[2*4+2] = FXRGB(0,   128, 128);
		imgptr[2*4+3] = FXRGB(0,   0,   0);

		imgptr[3*4+0] = FXRGB(0,   255, 255);
		imgptr[3*4+1] = FXRGB(0,   0,   0);
		imgptr[3*4+2] = FXRGB(0,   0,   0);
		imgptr[3*4+3] = FXRGB(0,   0,   0);

		bluedot->render();
		bluedot->create();

		/*segmentimg = new FXBMPImage(getApp(), resico_segment, IMAGE_OPAQUE);
		segmentimg->create();*/
	}


	void setValue(FXint val) {
		if (val != timeval) {
			timeval = val;
			if (id()) {
				FXDCWindow dc(this);
				dc.setForeground(baseColor);
				dc.fillEllipse(centerx - (facerad-4), centery - (facerad-4), (facerad-4)*2.0, (facerad-4)*2.0);
				drawHands(&dc);
			}
		}
	}

	FXint getValue() { return timeval; }

	int drawBigHand(FXDCWindow* dc, int ang, float mainlen, float sidelen, float taillen) {
		FXPoint points[4];
	
		float rad = (90.0 - ang) * M_PI / 180.0;
		float siderad = rad + (M_PI / 2.0);
		float tailrad = rad + M_PI;

		float cos_siderad = cos(siderad);
		float sin_siderad = sin(siderad);

		float cos_rad = cos(rad);
		float sin_rad = sin(rad);

		float cos_tailrad = cos(tailrad);
		float sin_tailrad = sin(tailrad);

		/*float mainlen = facerad * 65 / 100;
		float sidelen = facerad *  7 / 100;
		float taillen = facerad * 15 / 100;*/

		int off;

		FXColor color;
		
		for (int i = 0; i < 3; ++i) {
			switch (i) {
				case 0:
					off = -2;
					color = hiliteColor;
					break;
				case 1:
					off = 2;
					color = shadowColor;
					break;
				case 2:
					off = 0;
					color = FXRGB(0, 128, 128);
					break;
				default: break;
			}
			
			points[0].x = off + centerx + (int)(sidelen * cos_siderad);
			points[0].y = off + centery - (int)(sidelen * sin_siderad);
		
			points[1].x = off + centerx + (int)(mainlen * cos_rad);
			points[1].y = off + centery - (int)(mainlen * sin_rad);

			points[2].x = off + centerx - (int)(sidelen * cos_siderad);
			points[2].y = off + centery + (int)(sidelen * sin_siderad);

			points[3].x = off + centerx + (int)(taillen * cos_tailrad);
			points[3].y = off + centery - (int)(taillen * sin_tailrad);

			dc->setForeground(color);
			dc->fillPolygon(points, 4);
		}
	}

	int drawHands(FXDCWindow* dc) {
		long long skiptime = 0;
		
		days = timeval / 86400;
		skiptime = timeval % 86400;

		hours = (skiptime / 3600) % 12;
		
		skiptime %= 3600;
		
		minutes = skiptime / 60;
		seconds = skiptime % 60;



		hourang = (hours * 30) + (minutes / 2);
		minuteang = minutes * 6;
		secondang = seconds * 6;

		dc->setForeground(FXRGB(0,128,128));
		drawBigHand(dc, minuteang, facerad*65/100, facerad*7/100, facerad*15/100);
		drawBigHand(dc, hourang, facerad*75/100, facerad*5/100, facerad*20/100);

		int secondlen = facerad * 80 / 100;
		float secondrad = (90.0 - secondang) * M_PI / 180.0;
		int sx = centerx + (int)(secondlen * cos(secondrad));
		int sy = centery - (int)(secondlen * sin(secondrad));

		dc->setFunction(BLT_SRC_XOR_DST);
		dc->setForeground(FXRGB(255,255,255));
		dc->drawLine(centerx, centery, sx, sy);
	}

	long onPaint(FXObject* sender, FXSelector sel, void* ptr) {
		FXFrame::onPaint(sender, sel, ptr);

		FXEvent* event = (FXEvent*)ptr;
		FXDCWindow dc(this, event);

		int startx = padleft+border;
		int starty = padtop+border;
		int realw = width-padleft-padright-(border*2);
		int realh = height-padtop-padbottom-(border*2);

		facerad = 0;

		if (width < height) {
			facerad = width / 2-3;
		} else {
			facerad = height / 2-3;
		}

		centerx = realw / 2 + startx;
		centery = realh / 2 + starty;

		//dc.clipChildren(FALSE);
		dc.setForeground(getBackColor());
		dc.fillRectangle(startx, starty, realw-1, realh-1);

		dc.setFunction(BLT_SRC);


		for (int i = 0; i < 60; i++) {
			if (i % 5) {
				float anglerad = (90.0 - (i * 6)) * M_PI / 180.0;
				int dotx = centerx + (int)(cos(anglerad) * facerad);
				int doty = centery + (int)(sin(anglerad) * facerad);
				dc.drawImage(clockdot, dotx - 1, doty - 1);
			} else {
				float anglerad = (90.0 - (i * 6)) * M_PI / 180.0;
				int dotx = centerx + (int)(cos(anglerad) * facerad);
				int doty = centery + (int)(sin(anglerad) * facerad);
				dc.drawImage(bluedot, dotx - 2, doty - 2);
			}
		}

		drawHands(&dc);

		return 1;
	}


	FXint getDefaultWidth() {
		return 128+padleft+padright+(border*2);
	}

	FXint getDefaultHeight() {
		return 128+padtop+padbottom+(border*2);
	}

	long onCmdSetValue(FXObject*,FXSelector,void* ptr) {
		setValue((int)(long)ptr);
		return 1;
	}

	long onCmdSetIntValue(FXObject*,FXSelector,void* ptr) {
		setValue(*((int*)ptr));
		return 1;
	}

	long onCmdGetIntValue(FXObject*,FXSelector,void* ptr){
		*((int*)ptr) = timeval;
		return 1;
	}
	/*long onTimer(FXObject*,FXSelector,void* ptr){
		getApp()->addTimeout(this, ID_TIMER, 1000);
		timeval = getLocalTime(NULL);
		update();
		return 1;
	}*/
};

FXDEFMAP(I2KClock) I2KClockMap[] = {
	FXMAPFUNC(SEL_PAINT, 0, I2KClock::onPaint),
	FXMAPFUNC(SEL_UPDATE, 0, I2KClock::onUpdate),

	FXMAPFUNC(SEL_COMMAND, I2KClock::ID_SETVALUE,       I2KClock::onCmdSetValue),

	FXMAPFUNC(SEL_COMMAND, I2KClock::ID_SETINTVALUE,    I2KClock::onCmdSetIntValue),

	FXMAPFUNC(SEL_COMMAND, I2KClock::ID_GETINTVALUE,    I2KClock::onCmdGetIntValue),

	//FXMAPFUNC(SEL_TIMEOUT, I2KClock::ID_TIMER,    I2KClock::onTimer),
	//FXMAPFUNC(SEL_CHANGED, 0, MSSevenSegment::onChange),

};

FXIMPLEMENT(I2KClock, FXFrame, I2KClockMap, ARRAYNUMBER(I2KClockMap));

class I2KAMPMBox: public FXListBox {
	FXDECLARE(I2KAMPMBox);
protected:
	I2KAMPMBox() {}
public:
	FXint getDefaultWidth() {
		return FXListBox::getDefaultWidth()-12;
	}

	I2KAMPMBox(FXComposite *p, FXObject *tgt=NULL, FXSelector sel=0, FXuint opts=LISTBOX_NORMAL,
			FXint x=0, FXint y=0, FXint w=0, FXint h=0,
			FXint pl=DEFAULT_PAD, FXint pr=DEFAULT_PAD, FXint pt=DEFAULT_PAD, FXint pb=DEFAULT_PAD):
		FXListBox(p, tgt, sel, opts, x,y,w,h, pl,pr,pt,pb) {
		button->setButtonStyle(MENUBUTTON_NOARROWS);
		button->setFrameStyle(FRAME_NONE);
		button->setPadLeft(1);
		button->setBackColor(getBackColor());
		//button->setArrowSize(0);

		appendItem("AM");
		appendItem("PM");
		setNumVisible(2);
	}

};

FXIMPLEMENT(I2KAMPMBox, FXListBox, NULL, 0);


enum {
	TIMEBOX_12HOUR = 0x00020000,
	TIMEBOX_24HOUR = 0
};

class I2KTimeBox: public FXHorizontalFrame {
	FXDECLARE(I2KTimeBox);
protected:
	FXHorizontalFrame* timecont;

	FXTextField*       hourbox;
	FXTextField*       minutebox;
	FXTextField*       secondbox;

	FXVerticalFrame*   arrowbtns;
	FXArrowButton*     upbtn;
	FXArrowButton*     downbtn;

	time_t         timeval;

	long long      days;
	
	int            hours;
	int            minutes;
	int            seconds;

	int            pm;

	FXuint         options;

	I2KAMPMBox*    ampmbox;

	FXObject*      target;
	FXSelector     message;

protected:
	I2KTimeBox() {}
			

public:
	enum {
		ID_TEXTBOX = FXHorizontalFrame::ID_LAST,
		ID_INCREMENT,
		ID_DECREMENT,
		ID_SECBOX,
		ID_MINBOX,
		ID_HOURBOX,
		ID_LAST
	};

private:
	void splitUnix(time_t tval) {
		long long skiptime = 0;
		
		days = timeval / 86400;
		skiptime = timeval % 86400;

		hours = (skiptime / 3600) % 24;
		
		pm = 0;
		
		if (options & TIMEBOX_12HOUR) {
			if (hours >= 12) pm = 1;
			
			hours %= 12;

			if (hours == 0) {
				hours = 12;
			}
		}

		skiptime %= 3600;
		
		minutes = skiptime / 60;
		seconds = skiptime % 60;
	}
public:
	int getUnix() {
		if (options & TIMEBOX_12HOUR && pm) {
			return (days*86400) + (12*3600) + (hours * 3600) + (minutes*60) + seconds;
		} else {
			return (days*86400) + (hours * 3600) + (minutes*60) + seconds;
		}
	}
private:

	int setSecVal(int sval) {
		char numval[4];
		seconds = sval;
		sprintf(numval, "%02hhu", seconds);
		secondbox->setText(numval);
	}

	int setMinVal(int sval) {
		char numval[4];
		minutes = sval;
		sprintf(numval, "%02hhu", minutes);
		minutebox->setText(numval);
	}

	int setHourVal(int sval) {
		char numval[4];
		hours = sval;
		sprintf(numval, "%02hhu", hours);
		hourbox->setText(numval);
	}
public:

	void setValue(FXint val) {
		if (val != timeval) {
			char numval[4];
			
			timeval = val;
			splitUnix(val);

			setHourVal(hours);
			setMinVal(minutes);
			setSecVal(seconds);

			if (options & TIMEBOX_12HOUR) {
				if (pm) {
					ampmbox->setCurrentItem(1);
				} else {
					ampmbox->setCurrentItem(0);
				}
			}
			update();
		}
	}

	I2KTimeBox(FXComposite* parent, FXObject* tgt = NULL, FXSelector sel = 0,
			FXuint opts=FRAME_NONE, FXint x=0, FXint y=0, FXint w=0, FXint h=0):
		FXHorizontalFrame(parent, opts, x,y,w,h, 0,0,0,0, 2,2)
		/*time(1781040187)*/
	{
		FXLabel* lbl;

		target = tgt;
		message = sel;
		
		options = opts;

		timecont = new FXHorizontalFrame(this, LAYOUT_CENTER_Y|FRAME_NORMAL|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0, 0,0);		
		timecont->setBackColor(getApp()->getBackColor());

		hourbox   = new FXTextField(timecont, 2, this, ID_HOURBOX, TEXTFIELD_INTEGER|LAYOUT_CENTER_Y, 0,0,0,0, 1,1,0,0);

		lbl       = new FXLabel(timecont, ":", NULL, LAYOUT_CENTER_Y, 0,0,0,0, 0,0,0,0);
		lbl->setBackColor(getApp()->getBackColor());

		minutebox = new FXTextField(timecont, 2, this, ID_MINBOX, TEXTFIELD_INTEGER|LAYOUT_CENTER_Y, 0,0,0,0, 1,1,0,0);

		lbl       = new FXLabel(timecont, ":", NULL, LAYOUT_CENTER_Y, 0,0,0,0, 0,0,0,0);
		lbl->setBackColor(getApp()->getBackColor());

		secondbox = new FXTextField(timecont, 2, this, ID_SECBOX, TEXTFIELD_INTEGER|LAYOUT_CENTER_Y, 0,0,0,0, 1,1,0,0);
		ampmbox = new I2KAMPMBox(timecont, NULL, 0, FRAME_NONE, 0,0,0,0, 2,2,0,0);

		if (!(options & TIMEBOX_12HOUR)) {
			ampmbox->hide();
		}

		arrowbtns = new FXVerticalFrame(this, LAYOUT_CENTER_Y, 0,0,0,0, 0,0,0,0, 0,0);
		upbtn = new FXArrowButton(arrowbtns, this, ID_INCREMENT, ARROW_UP|ARROW_REPEAT|FRAME_THICK|FRAME_RAISED|LAYOUT_FIX_WIDTH|LAYOUT_FILL_Y, 0,0,16,0, 0,0,1,0);
		upbtn->setArrowSize(3);
		downbtn = new FXArrowButton(arrowbtns, this, ID_DECREMENT, ARROW_DOWN|ARROW_REPEAT|FRAME_THICK|FRAME_RAISED|LAYOUT_FIX_WIDTH|LAYOUT_FILL_Y, 0,0,16,0, 0,0,0,1);
		downbtn->setArrowSize(3);


		setValue(0);
	}


	long onCmdIncrement(FXObject*,FXSelector,void*){
		setValue(timeval+1);
		if (target) target->tryHandle(this, FXSEL(SEL_CHANGED,message), (void*)&timeval);
		return 1;
	}
	long onCmdDecrement(FXObject*,FXSelector,void*){
		setValue(timeval-1);
		if (target) target->tryHandle(this, FXSEL(SEL_CHANGED,message), (void*)&timeval);
		return 1;
	}

	FXint getValue() { return timeval; }
	long onVerifyHourBox(FXObject* obj,FXSelector,void* ptr) {
		FXTextField* txtbox = (FXTextField*)obj;

		int input = atoi((char*)ptr);
		
		if (options & TIMEBOX_12HOUR) {
			if (input >= 1 && input <= 12) {
				return 0;
			} else {
				hours = 12;
				txtbox->setText("12");
				return 1;
			}
		} else {
			if (input >= 0 && input <= 23) {
				return 0;
			} else {
				hours = 23;
				txtbox->setText("23");
				return 1;
			}
		}
	}

	long onVerifySecMinBox(FXObject* obj,FXSelector sel,void* ptr) {
		FXTextField* txtbox = (FXTextField*)obj;
		
		if (60 > atoi((char*)ptr)) {
			return 0;
		} else {
			if (sel == ID_MINBOX) {
				minutes = 59;
				txtbox->setText("59");
			} else {
				seconds = 59;
				txtbox->setText("59");
			}
			return 1;
		}
	}

	long onCommandMinBox(FXObject* obj,FXSelector sel,void* ptr) {
		FXTextField* txtbox = (FXTextField*)obj;
		char* str = (char*)ptr;
		int val = atoi((char*)ptr);
		char textstr[4];
		
		if (str[0] != 0 && str[1] == '\0') {
			minutes = val;
			sprintf(textstr, "%02hhu", val);
			txtbox->setText(textstr);
		}

		timeval = getUnix();

		if (target) target->tryHandle(this, FXSEL(SEL_CHANGED,message), (void*)&timeval);
	}

	long onCommandSecBox(FXObject* obj,FXSelector sel,void* ptr) {
		FXTextField* txtbox = (FXTextField*)obj;
		char* str = (char*)ptr;
		int val = atoi((char*)ptr);
		char textstr[4];
		
		if (str[0] != 0 && str[1] == '\0') {
			seconds = val;
			sprintf(textstr, "%02hhu", val);
			txtbox->setText(textstr);
		}

		timeval = getUnix();

		if (target) target->tryHandle(this, FXSEL(SEL_CHANGED,message), (void*)&timeval);
	}

	long onCommandHourBox(FXObject* obj,FXSelector sel,void* ptr) {
		FXTextField* txtbox = (FXTextField*)obj;
		char* str = (char*)ptr;
		int val = atoi((char*)ptr);
		char textstr[4];
		
		if (str[0] != 0 && str[1] == '\0') {
			hours = val;
			sprintf(textstr, "%02hhu", val);
			txtbox->setText(textstr);
		}

		timeval = getUnix();

		if (target) target->tryHandle(this, FXSEL(SEL_CHANGED,message), (void*)&timeval);
	}


	long onCmdSetValue(FXObject*,FXSelector,void* ptr) {
		setValue((int)(long)ptr);
		return 1;
	}

	long onCmdSetIntValue(FXObject*,FXSelector,void* ptr) {
		setValue(*((int*)ptr));
		return 1;
	}

	long onCmdGetIntValue(FXObject*,FXSelector,void* ptr){
		*((int*)ptr) = timeval;
		return 1;
	}
};

FXDEFMAP(I2KTimeBox) I2KTimeBoxMap[] = {
	/*FXMAPFUNC(SEL_PAINT, 0, I2KTimeBox::onPaint),
	FXMAPFUNC(SEL_UPDATE, 0, I2KTimeBox::onUpdate),*/
	FXMAPFUNC(SEL_VERIFY,  I2KTimeBox::ID_SECBOX,         I2KTimeBox::onVerifySecMinBox),
	FXMAPFUNC(SEL_VERIFY,  I2KTimeBox::ID_MINBOX,         I2KTimeBox::onVerifySecMinBox),
	FXMAPFUNC(SEL_VERIFY,  I2KTimeBox::ID_HOURBOX,        I2KTimeBox::onVerifyHourBox),


	FXMAPFUNC(SEL_COMMAND, I2KTimeBox::ID_SECBOX,         I2KTimeBox::onCommandSecBox),
	FXMAPFUNC(SEL_COMMAND, I2KTimeBox::ID_MINBOX,         I2KTimeBox::onCommandMinBox),
	FXMAPFUNC(SEL_COMMAND, I2KTimeBox::ID_HOURBOX,        I2KTimeBox::onCommandHourBox),


	FXMAPFUNC(SEL_COMMAND, I2KTimeBox::ID_INCREMENT,      I2KTimeBox::onCmdIncrement),
	FXMAPFUNC(SEL_COMMAND, I2KTimeBox::ID_DECREMENT,      I2KTimeBox::onCmdDecrement),

	FXMAPFUNC(SEL_COMMAND, I2KTimeBox::ID_SETVALUE,       I2KTimeBox::onCmdSetValue),

	FXMAPFUNC(SEL_COMMAND, I2KTimeBox::ID_SETINTVALUE,    I2KTimeBox::onCmdSetIntValue),

	FXMAPFUNC(SEL_COMMAND, I2KTimeBox::ID_GETINTVALUE,    I2KTimeBox::onCmdGetIntValue),
};

FXIMPLEMENT(I2KTimeBox, FXHorizontalFrame, I2KTimeBoxMap, ARRAYNUMBER(I2KTimeBoxMap));


class TimeDateCPL : public FXMainWindow {
	FXDECLARE(TimeDateCPL);

protected:
	TimeDateCPL() {}

private:
	FXVerticalFrame* cont;
	FXTabBook*         tabbook;

	FXVerticalFrame*   generalcont;
	FXVerticalFrame*   timezonecont;

	FXHorizontalFrame* generalhorcont;
	FXPacker*          calcont;

	FXGroupBox*        dategrp;
	FXHorizontalFrame* datetop;

	FXGroupBox*        timegrp;
	FXHorizontalFrame* timebot;
	FXHorizontalFrame* timecont;
	FXSpinner*         timespin;

	FXListBox*         monthbox;
	FXSpinner*         datespin;

	FXListBox*         periodbox;

	FXHorizontalFrame* buttoncont;

	I2KClock*          clock;

	FXTextField*       hourtxt;
	FXTextField*       minutetxt;
	FXTextField*       secondtxt;

	I2KTimeBox* timebox;
	FXCalendarView* calview;


public:
	long onTimer(FXObject*, FXSelector, void*);
	long onAccept(FXObject*, FXSelector, void*);
	long onApply(FXObject*, FXSelector, void*);

	time_t calcDate();


	long onChangeTimebox(FXObject*, FXSelector, void*);
	long onCommandYear(FXObject*, FXSelector, void*);
	long onCommandMonth(FXObject*, FXSelector, void*);

	long onChangeCalendar(FXObject*, FXSelector, void*);
	long onCommandCalendar(FXObject*, FXSelector, void*);


public:
	enum {
		ID_MAINWIN = FXMainWindow::ID_LAST,
		ID_TIMER,
		ID_TIMEBOX,

		ID_CALVIEW,

		ID_MONTHBOX,
		ID_YEARBOX,
		
		ID_ACCEPT,
		ID_APPLY,

		ID_LAST
	};

public:
	TimeDateCPL(FXApp* a);

	virtual void create();
	virtual ~TimeDateCPL();
};

FXDEFMAP(TimeDateCPL) TimeDateCPLMap[] = {
	FXMAPFUNC(SEL_TIMEOUT, TimeDateCPL::ID_TIMER,    TimeDateCPL::onTimer),
	FXMAPFUNC(SEL_CHANGED, TimeDateCPL::ID_TIMEBOX,    TimeDateCPL::onChangeTimebox),

	FXMAPFUNC(SEL_COMMAND, TimeDateCPL::ID_YEARBOX,    TimeDateCPL::onCommandYear),
	FXMAPFUNC(SEL_COMMAND, TimeDateCPL::ID_MONTHBOX,   TimeDateCPL::onCommandMonth),

	FXMAPFUNC(SEL_CHANGED, TimeDateCPL::ID_CALVIEW,   TimeDateCPL::onCommandCalendar),

	FXMAPFUNC(SEL_COMMAND, TimeDateCPL::ID_ACCEPT,   TimeDateCPL::onAccept),
	FXMAPFUNC(SEL_COMMAND, TimeDateCPL::ID_APPLY,   TimeDateCPL::onApply),


	/*FXMAPFUNC(SEL_COMMAND,           HelloWindow::ID_HELLO,  HelloWindow::onCmdHello),*/
};

FXIMPLEMENT(TimeDateCPL, FXMainWindow, TimeDateCPLMap, ARRAYNUMBER(TimeDateCPLMap));

long TimeDateCPL::onTimer(FXObject*,FXSelector,void* ptr){
	int localtime = getLocalTime(NULL);
	getApp()->addTimeout(this, ID_TIMER, 1000);
	clock->setValue(localtime);
	timebox->setValue(localtime);
	return 1;
}

time_t TimeDateCPL::calcDate() {
	char cmd[4096];
	FXDate date = calview->getCurrentDate();
	int year = date.year();
	int month = date.month()-1;
	int day = date.day();

	struct tm time = {0};

	time.tm_year = year - 1900;
	time.tm_mon = month;
	time.tm_mday = day;
	time.tm_hour = timebox->getUnix() % 86400 / 3600;
	time.tm_min = (timebox->getUnix() % 86400 % 3600) / 60;
	time.tm_sec = timebox->getUnix() % 60;
	time.tm_isdst = -1;

	return mktime(&time);
}


long TimeDateCPL::onAccept(FXObject*,FXSelector,void* ptr){
	char cmd[1024];
	getApp()->removeTimeout(this, ID_TIMER);

	sprintf(cmd, "pkexec date -s @%llu" , (long long)calcDate());
	if (!system(cmd)) getApp()->exit();
	
	getApp()->addTimeout(this, ID_TIMER, 10);
	
	return 1;
}

long TimeDateCPL::onApply(FXObject*,FXSelector,void* ptr){
	char cmd[1024];
	getApp()->removeTimeout(this, ID_TIMER);
	
	sprintf(cmd, "pkexec date -s @%llu" , (long long)calcDate());
	system(cmd);
	
	getApp()->addTimeout(this, ID_TIMER, 10);
	
	return 1;
}

long TimeDateCPL::onCommandCalendar(FXObject*,FXSelector,void* ptr){
	monthbox->setCurrentItem(calview->getCurrentMonth()-1);
	datespin->setValue(calview->getCurrentDate().year());
	return 1;
}

long TimeDateCPL::onCommandYear(FXObject*,FXSelector,void* ptr){
	/*printf("%d\n", (int)(FXival)ptr);*/
	FXDate date = calview->getCurrentDate();
	date.setDate((int)(FXival)ptr, date.month(), date.day());
	calview->setCurrentDate(date, 1);
	return 1;
}

long TimeDateCPL::onCommandMonth(FXObject*,FXSelector,void* ptr){
	FXDate date = calview->getCurrentDate();
	date.setDate(date.year(), (int)(FXival)ptr+1, date.day());
	calview->setCurrentDate(date, 1);

	return 1;
}

long TimeDateCPL::onChangeTimebox(FXObject*,FXSelector,void* ptr){
	char cmd[512];
	time_t time = *(time_t*)ptr;
	/*sprintf(cmd, "date -d @%llu", time - getLocalTimeOffset(NULL));
	system(cmd);*/

	getApp()->removeTimeout(this, ID_TIMER);

	clock->setValue(time);
	return 1;
}

TimeDateCPL::TimeDateCPL(FXApp *a) : FXMainWindow(a, "Date and Time Properties", mainIcon, NULL, DECOR_TITLE|DECOR_CLOSE|DECOR_BORDER|DECOR_MENU, 0,0,0,0) {
	getApp()->addTimeout(this, ID_TIMER, 1000);
	int timeval = getLocalTime(NULL);

	cont = new FXVerticalFrame(this, LAYOUT_FILL_Y|LAYOUT_FILL_X, 0,0,0,0, 6,6,6,6, 8,8);
	tabbook = new FXTabBook(cont, NULL, 0,
			TABBOOK_NORMAL|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0);

	new FXTabItem(tabbook, "Date && Time", NULL, TAB_TOP_NORMAL, 0,0,0,0, 6, 6, 1, 2);

	generalcont = new FXVerticalFrame(tabbook, FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 7,7,6,10, 8, 8);
	generalhorcont = new FXHorizontalFrame(generalcont, LAYOUT_FILL_X|LAYOUT_FILL_Y	, 0,0,0,0, 0,0,0,0, 8,8);

	dategrp = new FXGroupBox(generalhorcont, "Date",
			GROUPBOX_NORMAL|FRAME_GROOVE|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 12,12,0,4, 4,4);
	FXMatrix* datemtx = new FXMatrix(dategrp, 1, LAYOUT_FILL_X|LAYOUT_FILL_Y|MATRIX_BY_COLUMNS, 0,0,0,0, 0,0,0,0, 0,0);

	datetop = new FXHorizontalFrame(datemtx, LAYOUT_FILL_X|PACK_UNIFORM_WIDTH|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW, 0,0,0,0, 0,0,3,6, 10,10);

	monthbox = new FXListBox(datetop, this, ID_MONTHBOX, FRAME_NORMAL|LISTBOX_NORMAL|LAYOUT_CENTER_Y|LAYOUT_FILL_X);

	for (int i = 1; i <= 12; ++i) {
		monthbox->appendItem(tr(FXDate::monthName(i)));
    }
	monthbox->setCurrentItem(FXDate::localDate().month()-1);

	monthbox->setNumVisible(12);

	datespin = new FXSpinner(datetop, 6, this, ID_YEARBOX, FRAME_NORMAL|LAYOUT_FILL_X|LAYOUT_CENTER_Y);
	datespin->setRange(1901, 9999);
	datespin->setValue(FXDate::localDate().year());

	/*datespin->textField;*/

	calcont = new FXPacker(datemtx, FRAME_NORMAL|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW, 0,0,0,0, 0,0,0,0, 0,0);
	calview = new FXCalendarView(calcont, this, ID_CALVIEW, 0);

	calview->setTitleColor(calview->getDayColor());
	calview->setWeekendColor(calview->getDayColor());
	calview->setOtherWeekendColor(calview->getOtherDayColor());

	timegrp = new FXGroupBox(generalhorcont, "Time",
			GROUPBOX_NORMAL|FRAME_GROOVE|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 12,12,0,4, 4,4);

	FXMatrix* timemtx = new FXMatrix(timegrp, 1, LAYOUT_FILL_Y|MATRIX_BY_COLUMNS, 0,0,0,0, 4,4,0,0, 0,0);

	clock = new I2KClock(timemtx, LAYOUT_CENTER_Y|LAYOUT_FILL_Y|LAYOUT_FILL_ROW, 0,0,0,0, 0,0,0,6);
	clock->setValue(timeval);

	timebot = new FXHorizontalFrame(timemtx, LAYOUT_CENTER_X|LAYOUT_FILL_Y|LAYOUT_FILL_ROW, 0,0,0,0, 0,0,0,6, 2, 2);

	/*timecont = new FXHorizontalFrame(timebot, FRAME_NORMAL, 0,0,0,0, 1,40,1,1, 0, 0);
	timecont->setBackColor(getApp()->getBackColor());
	FXLabel* lbl;
	hourtxt = new FXTextField(timecont, 2, NULL, 0, TEXTFIELD_INTEGER, 0,0,0,0, 1,1,0,0);
	lbl = new FXLabel(timecont, ":", NULL, LABEL_NORMAL, 0,0,0,0, 0,0,0,0);
	lbl->setBackColor(getApp()->getBackColor());
	minutetxt = new FXTextField(timecont, 2, NULL, 0, TEXTFIELD_INTEGER, 0,0,0,0, 1,1,0,0);
	lbl = new FXLabel(timecont, ":", NULL, LABEL_NORMAL, 0,0,0,0, 0,0,0,0);
	lbl->setBackColor(getApp()->getBackColor());
	secondtxt = new FXTextField(timecont, 2, NULL, 0, TEXTFIELD_INTEGER, 0,0,0,0, 1,1,0,0);

	timespin = new FXSpinner(timebot, 0, NULL, 0, SPIN_NOTEXT, 0,0,0,0, 2,2,2,2);*/

	timebox = new I2KTimeBox(timebot, this, ID_TIMEBOX, TIMEBOX_24HOUR);
	timebox->setValue(timeval);

	/*new FXSpinner(timecont, 2);
	new FXSpinner(timecont, 2);
	new FXSpinner(timecont, 2);*/

	/*periodbox = new FXListBox(timecont, NULL, 0, LISTBOX_NORMAL);
	periodbox->appendItem("AM");
	periodbox->appendItem("PM");*/

	//new FXLabel(generalcont, "Current time zone: .....");

	buttoncont = new FXHorizontalFrame(cont, PACK_UNIFORM_WIDTH|LAYOUT_RIGHT, 0,0,0,0, 0,0,0,0);

	new I2KButton(buttoncont, "&OK", NULL, this,
			ID_ACCEPT,
			BUTTON_NORMAL|BUTTON_DEFAULT,
			0,0,0,0, 19, 19);

	new I2KButton(buttoncont, "&Cancel", NULL, getApp(),
			FXApp::ID_QUIT,
			BUTTON_NORMAL|BUTTON_DEFAULT,
			0,0,0,0, 19, 19);

	new I2KButton(buttoncont, "&Apply", NULL, this,
			ID_APPLY,
			BUTTON_NORMAL|BUTTON_DEFAULT,
			0,0,0,0, 19, 19);

	/*new FXTabItem(tabbook, "Time Zone", NULL, TAB_TOP_NORMAL, 0,0,0,0, 5, 4, 1, 2);
	timezonecont = new FXVerticalFrame(tabbook, FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 10,14,6,6, 4, 4);

	new FXListBox(timezonecont, NULL, 0, LISTBOX_NORMAL|FRAME_NORMAL|LAYOUT_FILL_X);
	new FXImageFrame(timezonecont, globeImg, LAYOUT_CENTER_X|FRAME_NORMAL);

	new FXCheckButton(timezonecont, "A&utomatically adjust clock for daylight saving changes");*/
	/*periodbox->appendItem("AM");
	periodbox->appendItem("PM");*/
	//getApp()->addTimeout(this, ID_TIMER, 1000);

}

TimeDateCPL::~TimeDateCPL() {
}

void TimeDateCPL::create() {
	FXMainWindow::create();

	show(PLACEMENT_SCREEN);
}


int main(int argc, char *argv[]) {
	FXApp application("TimeDate", "I2KProject");
	mainIcon = new FXGIFIcon(&application, resico_mainicon);
	globeImg = new FXGIFImage(&application, resico_globe, IMAGE_OPAQUE);

	application.init(argc, argv);

	new TimeDateCPL(&application);

	application.create();
	return application.run();
}




