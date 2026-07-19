.PHONY: clean uninstall install

CC = g++
C++ = g++

OS = `uname -s | tr '[a-z]' '[A-Z]'`

CFLAGS = -Wall -c --std=c99 -D "OS_$(OS)" -O3
CPPFLAGS = -Wall -c --std=c++98 -D "OS_$(OS)" -O3
LDFLAGS = `fox-config --libs` -lctl32 -li2kbrand

PREFIX=/usr/local

SOURCES = i2klogoff.cpp
OBJECTS = i2klogoff.o
ICONS = res/*.png
EXECUTABLE = i2klogoff

CPPFLAGS += `fox-config --cflags`

all: $(EXECUTABLE)

$(OBJECTS): res/foxres.h

# generate icon resources
res/foxres.h: $(ICONS)
	reswrap -s -p resico_ -z -d -o res/foxres.h $(ICONS)

# link executable
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

clean:
	rm -f res/foxres.h $(OBJECTS) $(EXECUTABLE)

install: $(EXECUTABLE)
	cp i2klogoff $(PREFIX)/bin/i2klogoff

uninstall:
	rm -f $(PREFIX)/bin/i2klogoff
