#include <stddef.h>

char *i2kBGetWinVersion();
char *i2kBGetFullOSName();
char *i2kBGetOSName();
char *i2kBGetCopyrightSymbol();
char *i2kBGetMicrosoftName();


const unsigned char *i2kBGetWinBrandingImage(size_t *size = 0);
const unsigned char *i2kBGetWinShutBrandingImage(size_t *size = 0);
