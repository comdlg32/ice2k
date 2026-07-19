#include <stddef.h>

#define ICE2K_BRAND_WIN2K 0
#define ICE2K_BRAND_WINXP 1
#define ICE2K_BRAND_WINSRV03 2

char *i2kBGetWinVersion();
char *i2kBGetFullOSName();
char *i2kBGetOSName();
char *i2kBGetCopyrightSymbol();
char *i2kBGetMicrosoftName();

int i2kBGetWinVersionInt();
int i2kBGetMicrosoftNameSafe(char* str, int len);
int i2kBGetCopyrightSymbolSafe(char* str, int len);
int i2kBGetFullOSNameSafe(char* str, int len);
int i2kBGetOSNameSafe(char* str, int len);

const unsigned char *i2kBGetWinBrandingImage(size_t *size = 0);
const unsigned char *i2kBGetWinShutBrandingImage(size_t *size = 0);
