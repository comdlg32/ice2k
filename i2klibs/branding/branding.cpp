//#define onPaint onPaint2
//#define onPaint onPaint2
//#define drawBorderRectangle drawBorderRectangle2
#include <fx.h>
#include <stddef.h>
#include "branding.h"
#include "res/foxres.h"

#define _BANNER banner2k
#define _BANNERSHUT banner2k

static FXSettings settings;
static int loaded = 0;
const unsigned char* wbanner;

// gpt helped me for certain stuff here

static void loadSettings() {
  if (!loaded) {
    FXString path = FXSystem::getHomeDirectory() + "/.foxrc/Desktop";
    settings.parseFile(path, true);

    loaded = 1;
  }
}

char *i2kBGetWinVersion() {
  loadSettings();
  char* windows = (char*)settings.readStringEntry("ICE2K", "windows", "2k");
  if (!strcmp(windows, "xp") || !strcmp(windows, "srv03"))
    return windows;
  else
    return (char*)"2k";
}

const unsigned char *i2kBGetWinBrandingImage(size_t *size) {
  loadSettings();
  char* windows = i2kBGetWinVersion();
  if (!wbanner) {	  
    if (!(strcmp(windows, "xp")))
      wbanner = banner;
    else if (!(strcmp(windows, "srv03")))
      wbanner = banner03;
    else
      wbanner = banner2k;

    if (size)
      *size = sizeof(wbanner);
  }

  return wbanner;
}

const unsigned char *i2kBGetWinShutBrandingImage(size_t *size) {
  loadSettings();
  char* windows = i2kBGetWinVersion();
  if (!wbanner) {	  
    if (!(strcmp(windows, "xp")))
      wbanner = banner;
    else if (!(strcmp(windows, "srv03")))
      wbanner = banner03;
    else
      wbanner = banner2kshut;

    if (size)
      *size = sizeof(wbanner);
  }

  return wbanner;
}

char *i2kBGetMicrosoftName() {
  loadSettings();
  char* windows = i2kBGetWinVersion();
  if (!strcmp(windows, "xp") || !strcmp(windows, "srv03"))
    return (char*)"Microsoft ®";
  else
    return (char*)"Microsoft (R)";
}


char *i2kBGetCopyrightSymbol() {
  loadSettings();
  char* windows = i2kBGetWinVersion();
  if (!strcmp(windows, "xp") || !strcmp(windows, "srv03"))
    return (char*)"©";
  else
    return (char*)"(C)";
}

char *i2kBGetFullOSName() {
  loadSettings();
  char* windows = i2kBGetWinVersion();
  if (!strcmp(windows, "xp"))
    return (char*)"Microsoft Windows XP";
  else if (!strcmp(windows, "srv03"))
    return (char*)"Microsoft Windows Server 2003";
  else
    return (char*)"Microsoft Windows 2000";
}

char *i2kBGetOSName() {
  loadSettings();
  char* windows = i2kBGetWinVersion();
  if (!strcmp(windows, "xp"))
    return (char*)"Windows XP";
  else if (!strcmp(windows, "srv03"))
    return (char*)"Windows Server 2003";
  else
    return (char*)"Windows 2000";
}
