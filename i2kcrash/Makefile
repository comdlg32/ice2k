.PHONY: clean uninstall install

CC = gcc
C++ = g++

CFLAGS = -g -Wall -c --std=c99
CPPFLAGS = -g -Wall -c --std=c++98
LDFLAGS = `fox-config --libs` -lctl32 -li2kbrand -liwiz97

PREFIX=/usr/local

SOURCES = i2kcrash.cpp
OBJECTS = i2kcrash.o res/foxres.o
ICONS = res/*.gif res/*.png
EXECUTABLE = i2kcrash

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
	mkdir -p $(PREFIX)/libexec/ice2k
	cp -f $(EXECUTABLE) $(PREFIX)/libexec/ice2k/$(EXECUTABLE)
	mkdir -p $(PREFIX)/bin
	sed 's#REPLACETHISWITHPREFIX#/usr/local#; s#REPLACETHISWITHPROGRAM#i2kcrash#' i2khandlecrash > "$(PREFIX)/bin/i2khandlecrash"
	chmod 755 "$(PREFIX)/bin/i2khandlecrash"

install-core: $(PREFIX)/bin/i2khandlecrash
	echo kernel.core_pattern=\""|$(PREFIX)/bin/i2khandlecrash %u %e"\" >> /etc/sysctl.conf
	sysctl -w kernel.core_pattern="|$(PREFIX)/bin/i2khandlecrash %u %e"

uninstall:
	rm -f $(PREFIX)/bin/$(EXECUTABLE)
