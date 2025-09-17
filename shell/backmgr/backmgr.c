#include <string.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>

#include <ice2k/ini.h>

#define _IMGMODE_TILED   0
#define _IMGMODE_CENTER  1
#define _IMGMODE_STRETCH 2
#define _IMGMODE_FILL    3

typedef struct {
	int mode;
	const char* color;
	const char* image;
} cfg;

int inihandle(void* udata, const char* section, const char* name, const char* value) {
	cfg* wallcfg = (cfg*)udata;

	if (!strcmp(section, "Wallpaper")) {
		if (!strcmp(name, "Mode")) {
			if (!strcasecmp(value, "Tiled")) {
				wallcfg->mode = _IMGMODE_TILED;
			} else if (!strcasecmp(value, "Center")) {
				wallcfg->mode = _IMGMODE_CENTER;
			} else if (!strcasecmp(value, "Stretch")) {
				wallcfg->mode = _IMGMODE_STRETCH;
			}
		} else if (!strcmp(name, "Color")) {
			if ( wallcfg->color[0] == '#' && strlen(value) == 7) {
				wallcfg->color = strdup(value);
			} else {
				return 0;
			}
		} else if (!strcmp(name, "Image")) {
			wallcfg->image = strdup(value);
		} else {
			return 0;
		}
	} else {
		return 0;
	}

	return 1;
}

int makeDirectory(const char *dir) {
	char tmp[PATH_MAX];
	char *p = NULL;
	size_t len;

	snprintf(tmp, sizeof(tmp), "%s", dir);
	len = strlen(tmp);
	
	if (tmp[len - 1] == '/')
		tmp[len - 1] = 0;

	for (p = tmp + 1; *p; p++) {
		if (*p == '/') {
			*p = 0;
			
			if (mkdir(tmp, S_IRWXU) == -1) {
				if (errno =! EEXIST)
					return -1;
			}

			*p = '/';
		}
	}

	if (mkdir(tmp, S_IRWXU) == -1) {
		if (errno =! EEXIST)
			return -1;
	}

	//puts("Success");

	return 1;
}

const char* getHomeDir() {
	const char *homedir;

	if ((homedir = getenv("HOME")) == NULL) {
		homedir = getpwuid(getuid())->pw_dir;
	}

	return homedir;
}

int main() {
	cfg wallcfg;

	wallcfg.mode = _IMGMODE_FILL; // usually what the user wants
	wallcfg.color = "#3a6ea5";
	wallcfg.image = "";

	char cfgpath[512] = "";

	snprintf(cfgpath, sizeof(cfgpath), "%s/%s", getHomeDir(), ".icewm/cfg/backmgr.ini");

	if (ini_parse(cfgpath, inihandle, &wallcfg) < 0) {
		return 1;
	}

	char fehcmd[1024] = "";

	char mode[16];

	if (wallcfg.mode == _IMGMODE_TILED) {
		strcpy(mode, "tile");
	} else if (wallcfg.mode == _IMGMODE_CENTER) {
		strcpy(mode, "center");
	} else if (wallcfg.mode == _IMGMODE_STRETCH) {
		strcpy(mode, "scale");
	} else {
		strcpy(mode, "fill");
	}

	char* image = strdup(wallcfg.image);

	if (!strcmp(image, "")) {
		snprintf(fehcmd, sizeof(fehcmd), "feh --no-fehbg --bg-center --image-bg=%s %s/%s", wallcfg.color, getHomeDir(), ".icewm/clear.png"); // temporary hack until we make our own proper wallpaper manager
	} else {
		snprintf(fehcmd, sizeof(fehcmd), "feh --no-fehbg --bg-%s --image-bg=%s %s", mode, wallcfg.color, image);
	}

	system(fehcmd);
}
