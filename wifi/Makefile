.PHONY: clean uninstall install

CC = gcc
C++ = g++

PREFIX=/usr/local

CFLAGS = -g -Wall -c --std=c99 -DPREFIX='"$(PREFIX)"'
CPPFLAGS = -g -Wall -c --std=c++98 -DPREFIX='"$(PREFIX)"'
LDFLAGS = `fox-config --libs` -lctl32 -li2kbrand -liwiz97

SOURCES = i2kwlan.cpp
OBJECTS = i2kwlan.o   res/foxres.o
ICONS = res/*.gif
EXECUTABLE = i2kwlan

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
	mkdir -p $(PREFIX)/bin
	mkdir -p $(PREFIX)/libexec/ice2k
	cp $(EXECUTABLE) $(PREFIX)/bin/$(EXECUTABLE)
	cp wifiiwd $(PREFIX)/libexec/ice2k/wifiiwd

uninstall:
	rm -f $(PREFIX)/bin/$(EXECUTABLE)
	rm -f $(PREFIX)/libexec/ice2k/wifiiwd
