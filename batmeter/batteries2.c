#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define POWER_SUPPLY_PATH "/sys/class/power_supply"

void getBatteries(char *buffer, size_t size) {
    DIR *dir = opendir(POWER_SUPPLY_PATH);
    struct dirent *entry;
    size_t len = 0;

    if (!dir) {
        perror("opendir");
        if (size > 0) buffer[0] = '\0';
        return;
    }

    buffer[0] = '\0';

    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, "BAT", 3) != 0)
            continue;

        char full_path[32];
        snprintf(full_path, sizeof(full_path), "%s/%s", POWER_SUPPLY_PATH, entry->d_name);

        size_t entry_len = strlen(entry->d_name);

        if (len > 0) {
            if (len + 1 < size) {
                buffer[len] = ',';
                len++;
                buffer[len] = '\0';
            } else {
                break;
            }
        }

        if (len + entry_len < size) {
            strcat(buffer, entry->d_name);
            len += entry_len;
        } else {
            break;
        }
    }

    closedir(dir);
}

int main() {
    char batteries[256];
    getBatteries(batteries, sizeof(batteries));
    printf("Batteries found: %s\n", batteries);
    return 0;
}
