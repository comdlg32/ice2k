#include <fx.h>
// #include <ice2k/comctl32.h>

class FirstRunWindow : public FXMainWindow {
	FXDECLARE(FirstRunWindow);

protected:
	FirstRunWindow() {}

private:
public:
	enum {
		ID_MAINWIN = FXMainWindow::ID_LAST,
		ID_LAST
	};

public:
	FirstRunWindow(FXApp* a);

	virtual void create();
	virtual ~FirstRunWindow();
};

FXDEFMAP(FirstRunWindow) FirstRunWindowMap[] = {
};

FXIMPLEMENT(FirstRunWindow, FXMainWindow, FirstRunWindowMap, ARRAYNUMBER(FirstRunWindowMap));

FirstRunWindow::FirstRunWindow(FXApp *a) : FXMainWindow(a, "Personalized Settings", NULL, NULL, DECOR_MENU|DECOR_TITLE|DECOR_BORDER, 33,55,0,0, 10,7,8,9) {
	new FXLabel(this, "Setting up personalized settings for:                            ", NULL,  JUSTIFY_LEFT, 0,0,0,0, 2,2,2,10);
	new FXLabel(this, "Ice2K.sys\n\n", NULL,  JUSTIFY_LEFT);
}

FirstRunWindow::~FirstRunWindow() {
}

void FirstRunWindow::create() {
	FXMainWindow::create();

	show();
}

int main(int argc, char *argv[]) {
	FXApp application("FirstRun", "I2KTest");

	application.init(argc, argv);

	new FirstRunWindow(&application);

	application.create();
	return application.run();
}
