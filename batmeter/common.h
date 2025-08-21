#ifndef GLOBALS_H
#define GLOBALS_H

#include <fx.h>

// Resources
char* bytesToHex(const unsigned char* data, size_t length);
char* bytesToReadable(const unsigned char* data, size_t length);

extern FXIcon            *ico_bat_ac;                 // Plugged into AC
extern FXIcon            *ico_bat_charging;           // Plugged into AC and charging
extern FXIcon            *ico_bat_chargeov;           // Charging overlay

extern FXIcon            *ico_bat_chargin2;           // Second battery charging icon? No idea where used

extern FXIcon            *ico_bat_full;               // Battery full
extern FXIcon            *ico_bat_half;               // Battery half full
extern FXIcon            *ico_bat_low;                // Battery low
extern FXIcon            *ico_bat_zero;               // Battery has zero charge?

extern FXIcon            *ico_bat_notpresent;         // Battery not present
extern FXIcon            *ico_bat_unknown;            // Battery information is unknown?

extern FXIcon            *ico_bat_what;               // wtf?

extern FXIcon            *ico_mainapp_16;             // Small Window icon
extern FXIcon            *ico_mainapp_32;             // Large Window icon

#endif
