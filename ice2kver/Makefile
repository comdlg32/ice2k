.PHONY: clean uninstall install

CC = g++

CFLAGS = -Wall -c
CPPFLAGS = -Wall -c
LDFLAGS = `fox-config --libs`

PREFIX=/usr/local

SOURCES = controls.cpp ice2kver.cpp
OBJECTS = $(SOURCES:.cpp=.o)
ICONS = res/banner.gif res/banner2k.gif res/banner03.gif res/xterm.png
EXECUTABLE = ice2kver
# "-D_WINVER_WIN2K" for windows 2000
# "-D_WINVER_WINXP" for windows xp
# "-D_WINVER_WINSRV03" for windows server 2003
CPPFLAGS += `fox-config --cflags` -D_WINVER_WIN2K

all: $(EXECUTABLE)

$(OBJECTS): res/foxres.h

# generate icon resources
res/foxres.h: $(ICONS)
	reswrap -o res/foxres.h $(ICONS)

# link executable
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

clean:
	rm -f res/foxres.h $(OBJECTS) $(EXECUTABLE)

install:
	cp ice2kver $(PREFIX)/bin/ice2kver
	ln -sf $(PREFIX)/bin/ice2kver $(PREFIX)/bin/winver

uninstall:
	rm -f $(PREFIX)/bin/ice2kver
	rm -f $(PREFIX)/bin/winver
