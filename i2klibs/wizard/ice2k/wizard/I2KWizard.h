#pragma once
#include <fx.h>
#include <ice2k/comctl32.h>
enum {
	IWIZARD_NODISABLEBACK = 0x00020000, // Do not disable back button
	IWIZARD_NOFOCUSNEXT   = 0x00040000, // Do not focus next button
	
	IWIZARD_NORMAL        = 0
};


enum {
	IWIZARD_ABACK = 1,
	IWIZARD_ANEXT = 2,
	IWIZARD_ACANCEL = 0
};

class I2KWizard : public FXVerticalFrame {
	FXDECLARE(I2KWizard);
protected:
	FXSwitcher            *switcher;
	FXHorizontalSeparator *sep;
	
	FXHorizontalFrame     *bottom;
	FXHorizontalFrame     *rbuttons;

	FXButton              *backbtn;
	FXButton              *nextbtn;
	FXButton              *cancelbtn;

	FXObject              *target;
	FXSelector             selector;

protected:
	I2KWizard() {}
public:
	long onCmdBack(FXObject*,FXSelector,void*);	
	long onCmdNext(FXObject*,FXSelector,void*);	
	long onCmdCancel(FXObject*,FXSelector,void*);	
public:
	enum {
		ID_WIZARD = FXMainWindow::ID_LAST,
		ID_BACK,
		ID_NEXT,
		ID_CANCEL,
		ID_LAST,
	};
public:
	I2KWizard(FXComposite* p,
		FXObject* tgt=NULL, FXSelector sel=0,
		FXuint opts=LAYOUT_FILL_Y|LAYOUT_FILL_X,
		FXint x=0,  FXint y=0,  FXint w=0,  FXint=0, 
		FXint pl=0, FXint pr=0, FXint pt=0, FXint pb=0);


	virtual ~I2KWizard();

	virtual void enable();
	virtual void disable();
	
	/* FXString getText() const;
	void setText(const FXString& text);
	
	void setVolRange(FXint lo, FXint hi, FXbool notify=FALSE);
	void setBalRange(FXint lo, FXint hi, FXbool notify=FALSE);

	void setVolValue(FXint val, FXbool notify=FALSE);
	void setBalValue(FXint val, FXbool notify=FALSE);

	int getVolValue();
	int getBalValue();

	void setMuteCheck(FXbool val);
	FXbool getMuteCheck();

	void setMixerStyle(FXuint style);
	FXuint getMixerStyle() const; */
	
	FXSwitcher* getSwitcher();

	FXint       getCurrent();
	void setCurrent(FXint index, FXbool notify = FALSE);

	FXButton* getBackButton();
	FXButton* getNextButton();
	FXButton* getCancelButton();

	void setFinish(FXbool finish);



};

