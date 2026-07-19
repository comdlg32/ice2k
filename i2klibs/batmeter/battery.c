#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#include "ice2k/batmeter/battery.h"


int getPsuInfoStr(const char* battery, const char* value, char* output, size_t len) {
	FILE* fptr;
	
	char path[
		(sizeof("/sys/class/power_supply/")-1)
		 +POWER_PSU_NAME_MAX+1
		 +(sizeof("/")-1)
		 +64
		 +1];

	sprintf(path, "/sys/class/power_supply/%.*s/%.*s",
			POWER_PSU_NAME_MAX, battery, 63, value);

	fptr = fopen(path, "r");
	if (fptr == NULL) return 0;

	fgets(output, len, fptr);

	fclose(fptr);

	return 1;
}


int getPsuInfoInt(const char* battery, const char* value, int* output) {
	char charoutput[32];
	int ret;
	
	ret = getPsuInfoStr(battery, value, charoutput, sizeof(charoutput));
	if (ret == 0) return 0;
	
	*output = atoi(charoutput);

	return 1;
}


int getBatteryCapacity(const char* battery, int* output) {
	return getPsuInfoInt(battery, "capacity", output);
}



int getPowerSupplyOnline(const char* psu) {
	int output = 0;
	getPsuInfoInt(psu, "online", &output);

	return output;
}

int getBatteryStatus(const char* battery) {
	char status[16];
	if (!getPsuInfoStr(battery, "status", status, sizeof(status))) return 0;

	switch (status[0]) {
		case 'U':
			return POWER_BAT_STATUS_UNKNOWN;
			break;
		case 'C':
			return POWER_BAT_STATUS_CHARGING;
			break;
		case 'D':
			return POWER_BAT_STATUS_DISCHARGING;
			break;
		case 'N':
			return POWER_BAT_STATUS_NOTCHARGING;
			break;
		case 'F':
			return POWER_BAT_STATUS_FULL;
			break;

		default:
			return 0;
	}
}


int getBatteryPresent(const char* battery) {
	int output = 0;
	getPsuInfoInt(battery, "present", &output);

	return output;
}


int getPowerSupplyType(const char* psu) {
	char type[4];
	
	if (!getPsuInfoStr(psu, "type", type, sizeof(type))) return 0;

	switch (type[0]) {
		case 'B':
			return POWER_PSU_TYPE_BATTERY;
			break;
		case 'M':
			return POWER_PSU_TYPE_MAINS;
			break;
		case 'W':
			return POWER_PSU_TYPE_WIRELESS;
			break;
		case 'U':
			if (type[1] == 'P') return POWER_PSU_TYPE_UPS;
			else return POWER_PSU_TYPE_USB;
			break;
		default:
			return 0;
	}
}


int getPowerSupplies(char* batteries, size_t size, unsigned int types) {
	int i = 0;
	DIR* dir;
	struct dirent* ent;

	if ((dir = opendir("/sys/class/power_supply")) == NULL) return 0;
	
	while ((ent = readdir(dir)) != NULL) {
		if (ent->d_name[0] == '.') continue;
		if ((getPowerSupplyType(ent->d_name) & types) == 0) continue;

		if (size >= (i+1)*POWER_PSU_NAME_MAX) {
			sprintf(batteries+i*POWER_PSU_NAME_MAX, "%.*s",
					POWER_PSU_NAME_MAX-1, ent->d_name);
			++i;
		} else {
			break;
		}
	}

	closedir(dir);

	return i;
}

int getCurrentPowerSource() {
	char psus[POWER_PSU_NAME_MAX*32];

	int batteryamount = getPowerSupplies(psus, sizeof(psus),
			POWER_PSU_TYPE_BATTERY);
	int amount = getPowerSupplies(psus, sizeof(psus),
			POWER_PSU_TYPE_MAINS|POWER_PSU_TYPE_WIRELESS|POWER_PSU_TYPE_USB);

	int ret = POWER_PSU_TYPE_MAINS;

	if (batteryamount != 0) {
		int i = 0;
		ret = POWER_PSU_TYPE_BATTERY;
		
		while (amount > i) {
			if (getPowerSupplyOnline(psus+(i*POWER_PSU_NAME_MAX))) {
				ret = POWER_PSU_TYPE_MAINS;
				break;
			}
			++i;
		}
	}

	return ret;
}

int getOverallPercentage(int* output) {
	char batteries[POWER_PSU_NAME_MAX*32];

	int amount = getPowerSupplies(batteries, sizeof(batteries),
			POWER_PSU_TYPE_BATTERY);

	int allcombined = 0;
	int percentage = 0;
	int ret = 0;

	if (amount != 0) {
		int i = 0;

		while (amount > i) {
			getBatteryCapacity(batteries+(i*POWER_PSU_NAME_MAX), &percentage);
			/*printf("%d\n", percentage);*/
			allcombined += percentage;
			++i;
		}

		ret = 1;
		*output = allcombined / amount;
	}

	return ret;
}

int getBatteryDuration(const char* battery, int charging, int* output) {
	int chargenow = 0;
	int chargefull = 0;
	int currentnow = 0;

	long long duration;

	getPsuInfoInt(battery, "charge_now",  &chargenow);
	getPsuInfoInt(battery, "current_now", &currentnow);

	if (charging) getPsuInfoInt(battery, "charge_full", &chargefull);

	if (currentnow != 0) {
		if (charging) {
			duration = ( (long long)chargefull - (long long)chargenow ) * (long long)3600 / (long long)currentnow;
		} else {
			duration = (long long)chargenow * (long long)3600 / (long long)currentnow;
		}
	}

	*output = (int)duration;
	return 1;
}

int getOverallDuration(int charging, int* output) {
	char batteries[POWER_PSU_NAME_MAX*32];

	int amount = getPowerSupplies(batteries, sizeof(batteries),
			POWER_PSU_TYPE_BATTERY);

	int allcombined = 0;
	int duration = 0;
	int ret = 0;

	if (amount != 0) {
		int i = 0;

		while (amount > i) {
			getBatteryDuration(batteries+(i*POWER_PSU_NAME_MAX), charging, &duration);
			/*printf("%d\n", duration);*/
			allcombined += duration;
			++i;
		}

		ret = 1;
		*output = allcombined;
	}

	return ret;
}

int getOverallStatus() {
	char batteries[POWER_PSU_NAME_MAX*32];

	int amount = getPowerSupplies(batteries, sizeof(batteries),
			POWER_PSU_TYPE_BATTERY);

	int ret = 0;
	int status = 0;

	int charging = 0;
	int discharging = 0;
	int notcharging = 0;
	int full = 0;

	if (amount != 0) {
		int i = 0;

		while (amount > i) {
			status = getBatteryStatus(batteries+(i*POWER_PSU_NAME_MAX));
			
			switch (status) {
				case POWER_BAT_STATUS_UNKNOWN:
					return POWER_BAT_STATUS_UNKNOWN;
					break;
				case POWER_BAT_STATUS_CHARGING:
					charging = 1;
					break;
				case POWER_BAT_STATUS_DISCHARGING:
					discharging = 1;
					break;
				case POWER_BAT_STATUS_NOTCHARGING:
					notcharging = 1;
					break;
				case POWER_BAT_STATUS_FULL:
					full = 1;
					break;
			}
			
			++i;
		}
	}

	if (full == 1 && discharging == 0 && notcharging == 0 && charging == 0) {
		return POWER_BAT_STATUS_FULL;
	} else if (charging == 1) {
		return POWER_BAT_STATUS_CHARGING;
	} else if (notcharging == 1) {
		return POWER_BAT_STATUS_NOTCHARGING;
	}
	
	return POWER_BAT_STATUS_DISCHARGING;
}

