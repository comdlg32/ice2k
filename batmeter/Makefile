.PHONY: clean uninstall install

CC = gcc
CXX = g++

CFLAGS = -Wall -c
CXXFLAGS = -Wall -c

PREFIX = /usr/local


# for non developers, do not touch this
CFLAGS += -D UHEXE_VERSION="0.0.1" -c --std=c99
CXXFLAGS += `fox-config --cflags` -D UHEXE_VERSION="\"0.0.1\"" -c --std=c++98 -static
LDFLAGS += `fox-config --libs | sed 's/-lGLU//g' | sed 's/-lGL//g' ` 

ICONS = res/ico/bat/*.gif res/ico/*.gif
EXECUTABLE = batmeter
VERSION = 0.0.1

#COBJECTS = $(CSOURCES:.c=.o)

CXXSOURCES = foxutils.cpp main.cpp BatMeter.cpp basecontrols.cpp Battery.cpp
CXXOBJECTS = $(CXXSOURCES:.cpp=.o)


OBJECTS = $(CXXOBJECTS) $(COBJECTS)

all: $(EXECUTABLE)

$(OBJECTS): res/foxres.h

# generate icon resources
res/foxres.h: $(ICONS)
	reswrap -s -p resico_ -z -d -o res/foxres.h $(ICONS)

# link executable
$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

clean:
	rm -f res/foxres.h $(OBJECTS) $(EXECUTABLE)

install: all
	cp $(EXECUTABLE) $(PREFIX)/bin/$(EXECUTABLE)

uninstall:
	rm -f $(PREFIX)/bin/$(EXECUTABLE)

BatMeter.o: foxutils.h BatMeter.h common.h
foxutils.o: foxutils.h common.h res/foxres.h
main.o: foxutils.h BatMeter.h common.h
Battery.o: Battery.h
