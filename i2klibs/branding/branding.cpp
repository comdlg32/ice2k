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

static char buf[64];

static void loadSettings() {
  if (!loaded) {
    FXString path = FXSystem::getHomeDirectory() + "/.foxrc/Desktop";
    settings.parseFile(path, true);

    loaded = 1;
  }
}

int i2kBGetWinVersionInt() {
	loadSettings();
	char* windows = (char*)settings.readStringEntry("ICE2K", "windows", "2k");

	if (!strcmp(windows, "xp")) {
		return ICE2K_BRAND_WINXP;
	} else if (!strcmp(windows, "srv03")) {
		return ICE2K_BRAND_WINSRV03;
	} else {
		return ICE2K_BRAND_WIN2K;
	}
}

char *i2kBGetWinVersion() {
	loadSettings();
	char* windows = (char*)settings.readStringEntry("ICE2K", "windows", "2k");

	if (!strcmp(windows, "xp") || !strcmp(windows, "srv03")) {
		return windows;
	} else {
		return (char*)"2k";
	}
}

const unsigned char *i2kBGetWinBrandingImage(size_t *size) {
	loadSettings();
	int	windows = i2kBGetWinVersionInt();

	if (!wbanner) {	  
		if (windows == ICE2K_BRAND_WINXP) {
			wbanner = banner;
		} else if (windows == ICE2K_BRAND_WINSRV03) {
			wbanner = banner03;
		} else {
			wbanner = banner2k;
		}

		if (size != NULL) *size = sizeof(wbanner);
	}

	return wbanner;
}

const unsigned char *i2kBGetWinShutBrandingImage(size_t *size) {
	loadSettings();
	int windows = i2kBGetWinVersionInt();

	if (!wbanner) {
		if (windows == ICE2K_BRAND_WINXP) {
			wbanner = banner;
		} else if (windows == ICE2K_BRAND_WINSRV03) {
			wbanner = banner03;
		} else {
			wbanner = banner2kshut;
		}

		if (size != NULL) *size = sizeof(wbanner);
	}
	
	return wbanner;
}

int i2kBGetMicrosoftNameSafe(char* str, int len) {
	loadSettings();
	int windows = i2kBGetWinVersionInt();
	
	if (len > 2) {
		if (windows == ICE2K_BRAND_WINXP || windows == ICE2K_BRAND_WINSRV03) {
			strncpy(str, "Microsoft ®", len-1);
		} else {
			strncpy(str, "Microsoft (R)", len-1);
		}

		str[len-1] = '\0';
		return 1;
	}

	return 0;
}



char *i2kBGetMicrosoftName() {
	i2kBGetMicrosoftNameSafe(buf, sizeof(buf));
	return (char*)buf;
}

int i2kBGetCopyrightSymbolSafe(char* str, int len) {
	loadSettings();
	int windows = i2kBGetWinVersionInt();
	
	if (len > 2) {
		if (windows == ICE2K_BRAND_WINXP || windows == ICE2K_BRAND_WINSRV03) {
			strncpy(str, "©", len-1);
		} else {
			strncpy(str, "(C)", len-1);
		}

		str[len-1] = '\0';
		return 1;
	}

	return 0;
}


char *i2kBGetCopyrightSymbol() {
	i2kBGetCopyrightSymbolSafe(buf, sizeof(buf));
	return (char*)buf;
}

int i2kBGetFullOSNameSafe(char* str, int len) {
	loadSettings();
	int windows = i2kBGetWinVersionInt();
	
	if (len > 2) {
		if (windows == ICE2K_BRAND_WINXP) {
			strncpy(str, "Microsoft Windows XP", len-1);
		} else if (windows == ICE2K_BRAND_WINSRV03) {
			strncpy(str, "Microsoft Windows Server 2003", len-1);
		} else {
			strncpy(str, "Microsoft Windows 2000", len-1);
		}

		str[len-1] = '\0';
		return 1;
	}

	return 0;
}

char *i2kBGetFullOSName() {
	i2kBGetFullOSNameSafe(buf, sizeof(buf));
	return (char*)buf;
}


int i2kBGetOSNameSafe(char* str, int len) {
	loadSettings();
	int windows = i2kBGetWinVersionInt();
	
	if (len > 2) {
		if (windows == ICE2K_BRAND_WINXP) {
			strncpy(str, "Windows XP", len-1);
		} else if (windows == ICE2K_BRAND_WINSRV03) {
			strncpy(str, "Windows Server 2003", len-1);
		} else {
			strncpy(str, "Windows 2000", len-1);
		}

		str[len-1] = '\0';
		return 1;
	}

	return 0;
}

char *i2kBGetOSName() {
	i2kBGetOSNameSafe(buf, sizeof(buf));
	return (char*)buf;
}
