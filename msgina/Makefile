.PHONY: clean uninstall install

CC = g++
C++ = g++

OS = `uname -s | tr '[a-z]' '[A-Z]'`
# OS = OPENBSD

CFLAGS = -Wall -c --std=c99 -D "OS_$(OS)"
CPPFLAGS = -Wall -c --std=c++98 -D "OS_$(OS)"
LDFLAGS = `fox-config --libs`

PREFIX=/usr/local

SOURCES = controls.cpp ctrlaltdel.cpp
OBJECTS = controls.o   ctrlaltdel.o
ICONS = res/banner.gif res/banner2k.gif res/banner03.gif
EXECUTABLE = ctrlaltdel

CPPFLAGS += `fox-config --cflags`

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

install: $(EXECUTABLE)
	cp ctrlaltdel $(PREFIX)/bin/ctrlaltdel

uninstall:
	rm -f $(PREFIX)/bin/ctrlaltdel
