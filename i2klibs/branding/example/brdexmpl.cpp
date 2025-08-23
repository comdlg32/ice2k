#include <fx.h>
#include <ice2k/comctl32.h>
#include <ice2k/branding.h>

int main(int argc, char *argv[]) {
  FXApp application("brdexmpl", "Ice2KProj");
  const unsigned char *banner = i2kBGetWinShutBrandingImage();

  FXIcon* bannericon = new FXGIFIcon(&application, banner,0,IMAGE_OPAQUE);

  application.init(argc, argv);
  FXMainWindow *main=new FXMainWindow(&application, "Branding Test Program", NULL, NULL, DECOR_TITLE|DECOR_BORDER|DECOR_MENU|DECOR_CLOSE, 0, 0, 0, 0);

  new FXLabel(main, "", bannericon, LAYOUT_CENTER_X, 0, 0, 0, 0, 0, 0, 0);
  application.create();

  main->show(PLACEMENT_SCREEN);
  return application.run();
}
