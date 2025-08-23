.PHONY: clean uninstall install

CC = gcc
CXX = g++

CFLAGS = -Wall -c
CXXFLAGS = -Wall -c

PREFIX = /usr/local


# for non developers, do not touch this
CFLAGS += -c --std=c99
CXXFLAGS += `fox-config --cflags` -c --std=c++98 -fpic
LDFLAGS += `fox-config --libs`

OUTPUT = libctl32.so
VERSION = 0.0.1

#COBJECTS = $(CSOURCES:.c=.o)

CXXSOURCES = branding.cpp
CXXOBJECTS = $(CXXSOURCES:.cpp=.o)


OBJECTS = $(CXXOBJECTS) $(COBJECTS)

all: $(OUTPUT)

# link executable
$(OUTPUT): $(OBJECTS)
	$(CXX) $(OBJECTS) -shared -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJECTS) $(OUTPUT)

install: all
	mkdir -p $(PREFIX)/include/ice2k
	cp branding.h $(PREFIX)/include/ice2k/branding.h
	cp $(OUTPUT) $(PREFIX)/lib/$(OUTPUT)

uninstall:
	rm $(PREFIX)/include/ice2k/branding.h
	rmdir $(PREFIX)/include/ice2k >/dev/null 2>&1 || true
	rm -f $(PREFIX)/lib/$(OUTPUT)

