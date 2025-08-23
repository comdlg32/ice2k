.PHONY: clean uninstall install

CC = gcc
CXX = g++

CFLAGS = -Wall -c
CXXFLAGS = -Wall -c

PREFIX = /usr/local

CFLAGS += -c --std=c99
CXXFLAGS += `fox-config --cflags` -c --std=c++98 -fpic
LDFLAGS += `fox-config --libs`

ICONS = res/banner.gif res/banner2k.gif res/banner03.gif res/banner2kshut.gif

OUTPUT = libi2kbrand.so
VERSION = 0.0.1

CXXSOURCES = branding.cpp
CXXOBJECTS = $(CXXSOURCES:.cpp=.o)

OBJECTS = $(CXXOBJECTS) $(COBJECTS)

all: $(OUTPUT)

$(OBJECTS): res/foxres.h

# generate icon resources
res/foxres.h: $(ICONS)
	reswrap -o res/foxres.h $(ICONS)

# link executable
$(OUTPUT): $(OBJECTS)
	$(CXX) $(OBJECTS) -shared -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJECTS) $(OUTPUT) res/foxres.h

install: all
	mkdir -p $(PREFIX)/include/ice2k
	cp branding.h $(PREFIX)/include/ice2k/branding.h
	cp $(OUTPUT) $(PREFIX)/lib/$(OUTPUT)

uninstall:
	rm $(PREFIX)/include/ice2k/branding.h
	rmdir $(PREFIX)/include/ice2k >/dev/null 2>&1 || true
	rm -f $(PREFIX)/lib/$(OUTPUT)

