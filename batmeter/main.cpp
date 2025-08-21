// Based on scribble example

#include <fx.h>

#include "foxutils.h"
#include "BatMeter.h"
#include "common.h"
#include "Battery.h"

// Here we begin
int main(int argc,char *argv[]) {
	//test("BAT1");

	//printf(bytesToHex(str, sizeof(str)));
	//printf(bytesToReadable(str, sizeof(str)));
	//printf("\n");

	FXApp application("batmeter", "Ice2K");
	//FXApp* ptrapp = &application;

	application.init(argc, argv);

	loadIcons(&application);

	BatMeter* window = new BatMeter(&application);

	//window->show();

	// create windows
	application.create();
	window->show(PLACEMENT_SCREEN);

	// Run the application
	int errcode = application.run();

	unloadIcons(&application);

	return errcode;
}


