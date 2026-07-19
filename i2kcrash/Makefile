.PHONY: clean uninstall install

CC = gcc
C++ = g++

CFLAGS = -g -Wall -c --std=c99
CPPFLAGS = -g -Wall -c --std=c++98
LDFLAGS = `fox-config --libs` -lctl32 -li2kbrand -liwiz97

PREFIX=/usr/local

SOURCES = i2khello.cpp
OBJECTS = i2khello.o res/foxres.o
ICONS = res/*.gif
EXECUTABLE = i2khello

CPPFLAGS += `fox-config --cflags`

all: $(EXECUTABLE)

$(OBJECTS): res/foxres.h res/foxres.cpp

# generate icon resources
res/foxres.h: $(ICONS)
	reswrap -i -d -p resico_ -o res/foxres.h $(ICONS)

res/foxres.cpp: res/foxres.h $(ICONS)
	reswrap -e -d -p resico_ -o res/foxres.cpp $(ICONS)

# link executable
$(EXECUTABLE): $(OBJECTS)
	$(C++) $(OBJECTS) -o $@ $(LDFLAGS)

clean:
	rm -f res/foxres.h res/foxres.cpp $(OBJECTS) $(EXECUTABLE)

install: $(EXECUTABLE)
	cp $(EXECUTABLE) $(PREFIX)/bin/$(EXECUTABLE)

uninstall:
	rm -f $(PREFIX)/bin/$(EXECUTABLE)
