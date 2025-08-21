#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "Battery.h"

#define SYSFS_PSU "/sys/class/power_supply"

int getBatInfo(const char* battery, const char* info, char* buf, int bufsize ) {
	char batpath[64];
	int fd;
	int len = 0;

	snprintf(batpath, sizeof(batpath), "%s/%s/%s", SYSFS_PSU, battery, info);

	fd = open(batpath, O_RDONLY);

	if (fd < 0) {
		return 1;
	}

	len = read(fd, buf, bufsize - 1);
	if (len < 0) {
		close(fd);
		return 1;
	}

	buf[len] = '\0';

	for (int i = 0; buf[i] != '\0'; ++i) {
		if (buf[i] == '\n') {
			buf[i] = '\0';
			break;
		}
	}

	close(fd);
	return 0;
}

int getBatInfoInt(const char* battery, const char* info) {
	char buf[20];
	int bufsize = sizeof(buf);

	getBatInfo(battery, info, buf, bufsize);

	return atoi(buf);
}

/* int getBatInfoInt(const char* battery, const char* info) {
	char batpath[64];
	int fd;
	int len = 0;

	snprintf(batpath, sizeof(batpath), "%s/%s/%s", SYSFS_PSU, battery, info);

	char buf[20];
	int bufsize = sizeof(buf);


	fd = open(batpath, O_RDONLY);

	if (fd < 0) {
		return 1;
	}

	len = read(fd, buf, bufsize - 1);
	if (len < 0) {
		close(fd);
		return 1;
	}

	buf[len] = '\0';

	for (int i = 0; buf[i] != '\0'; ++i) {
		if (buf[i] == '\n') {
			buf[i] = '\0';
			break;
		}
	}

	int result = atoi(buf);

	close(fd);
	return result;
} */



int getBatteries(char *buf, int bufsize) {
	DIR *dir = opendir(SYSFS_PSU);
	struct dirent *entry;
	//size_t len = 0;
	int len = 0;

	if (!dir) {
		perror("opendir");
		if (bufsize > 0) buf[0] = '\0';
		return 1;
	}

	buf[0] = '\0';

	while ((entry = readdir(dir)) != NULL) {
		if (strncmp(entry->d_name, "BAT", 3) != 0)
			continue;

		char full_path[32];
		snprintf(full_path, sizeof(full_path), "%s/%s", SYSFS_PSU, entry->d_name);

		//size_t entry_len = strlen(entry->d_name);
		int entry_len = strlen(entry->d_name);

		if (len > 0) {
			if (len + 1 < bufsize) {
				buf[len] = ',';
				len++;
				buf[len] = '\0';
			} else {
				break;
			}
		}

		if (len + entry_len < bufsize) {
			strcat(buf, entry->d_name);
			len += entry_len;
		} else {
			break;
		}
	}

	closedir(dir);

	return 0;
}

int getNumBatteries() {
	char batteries[40];
	getBatteries(batteries, sizeof(batteries));
	char *battery = strtok(batteries, ",");

	int numbatteries = 0;

	while (battery) {
		numbatteries += 1;
		//puts(battery);
		battery = strtok(NULL, ",");
	}

	return numbatteries;
}

int getCombBatInfoInt(const char* info) {
	char batteries[40];
	getBatteries(batteries, sizeof(batteries));

	int bat1;
	int bat2;

	char *battery = strtok(batteries, ",");

	bat1 = getBatInfoInt(battery, info);
	//printf("%s: %d\n", battery, bat1);
	battery = strtok(NULL, ",");

	while (battery) {
		bat2 = getBatInfoInt(battery, info);
		bat1 = bat1 + bat2;

		battery = strtok(NULL, ",");
	}

	//printf("All: %d\n", bat1);

	return bat1;
}


int getAllBatteryStatus(char* buf, int bufsize) {
	char batteries[40];
	char status[13];

	if (getBatteries(batteries, sizeof(batteries)) != 0) {
		if (bufsize > 0) buf[0] = '\0';
		return 1;
	}

	char combstatus[13] = "Full";

	char *battery = strtok(batteries, ",");
	while (battery) {
		if (getBatInfo(battery, "status", status, sizeof(status)) != 0) {
			strcpy(combstatus, "Unknown");
			break;
		}

		if (!(strcmp(status, "Charging"))) {
			strcpy(combstatus, "Charging");
			break;
		} else if (!(strcmp(status, "Unknown"))) {
			if (strcmp(combstatus, "Full") == 0 || strcmp(combstatus, "Not charging") == 0) {
				strcpy(combstatus, "Unknown");
				break;
			}
		} else if (strcmp(status, "Not charging") == 0) {
			if (strcmp(combstatus, "Full") == 0) {
				strcpy(combstatus, "Not charging");
			}
		} else if (strcmp(status, "Full") == 0) {
		} else {
			strcpy(combstatus, "Discharging");
		}

		battery = strtok(NULL, ",");
	}

	strcpy(buf, combstatus);

	return 0;
}
