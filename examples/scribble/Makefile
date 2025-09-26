.PHONY: clean uninstall install

CC = gcc
C++ = g++

# OS = OPENBSD

CFLAGS = -Wall -c --std=c99
CPPFLAGS = -Wall -c --std=c++98
LDFLAGS = `fox-config --libs` -lctl32 -li2kbrand

PREFIX=/usr/local

SOURCES = scribble.cpp
OBJECTS = scribble.o
ICONS = res/*.gif
EXECUTABLE = scribble

CPPFLAGS += `fox-config --cflags`

all: $(EXECUTABLE)

$(OBJECTS): res/foxres.h

# generate icon resources
res/foxres.h: $(ICONS)
	reswrap -d -p resico_ -o res/foxres.h $(ICONS)

# link executable
$(EXECUTABLE): $(OBJECTS)
	$(C++) $(OBJECTS) -o $@ $(LDFLAGS)

clean:
	rm -f res/foxres.h $(OBJECTS) $(EXECUTABLE)

install: $(EXECUTABLE)
	cp scribble $(PREFIX)/bin/scribble

uninstall:
	rm -f $(PREFIX)/bin/scribble
