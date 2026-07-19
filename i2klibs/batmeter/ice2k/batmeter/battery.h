#ifdef __cplusplus
extern "C" {
#endif
#ifndef BATTERY_H
#define BATTERY_H

#define POWER_PSU_NAME_MAX 32

#define POWER_BAT_STATUS_DISCHARGING (1 << 0)
#define POWER_BAT_STATUS_CHARGING (1 << 1)
#define POWER_BAT_STATUS_NOTCHARGING (1 << 2)
#define POWER_BAT_STATUS_FULL (1 << 3)
#define POWER_BAT_STATUS_UNKNOWN (1 << 4)

#define POWER_PSU_TYPE_BATTERY (1 << 0)
#define POWER_PSU_TYPE_MAINS (1 << 1)
#define POWER_PSU_TYPE_UPS (1 << 2)
#define POWER_PSU_TYPE_USB (1 << 3)
#define POWER_PSU_TYPE_WIRELESS (1 << 4)

int getPsuInfoStr(const char* battery, const char* value, char* output, size_t len);
int getPsuInfoInt(const char* battery, const char* value, int* output);
int getBatteryCapacity(const char* battery, int* output);
int getPowerSupplyOnline(const char* psu);
int getBatteryStatus(const char* battery);
int getBatteryPresent(const char* battery);
int getPowerSupplyType(const char* psu);
int getPowerSupplies(char* batteries, size_t size, unsigned int types);
int getCurrentPowerSource();
int getOverallPercentage(int* output);
int getBatteryDuration(const char* battery, int charging, int* output); 
int getOverallDuration(int charging, int* output);
int getOverallStatus();
#endif
#ifdef __cplusplus
}
#endif

