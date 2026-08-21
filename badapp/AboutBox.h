#include <fx.h>

class AboutBox: public FXDialogBox {
	FXDECLARE(AboutBox);
protected:
	FXXPMIcon* ico_main32;
	AboutBox() {}
public:
	AboutBox(FXWindow* owner);
	virtual void create();
	void setFocus() {};
	virtual ~AboutBox();
};

