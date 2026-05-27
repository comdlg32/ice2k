.PHONY: clean uninstall install

CC = gcc
CXX = g++

CFLAGS = -Wall -c
CXXFLAGS = -Wall -c

PREFIX = /usr/local

CFLAGS += -c --std=c99
CXXFLAGS += `fox-config --cflags` -c --std=c++98 -fpic
LDFLAGS += `fox-config --libs` -lctl32

OUTPUT = libiwiz97.so
VERSION = 0.0.1

CXXSOURCES = I2KWizHeader.cpp I2KWizard.cpp
CXXOBJECTS = $(CXXSOURCES:.cpp=.o)


OBJECTS = $(CXXOBJECTS) $(COBJECTS)

all: $(OUTPUT)

# link library
$(OUTPUT): $(OBJECTS)
	$(CXX) $(OBJECTS) -shared -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJECTS) $(OUTPUT)

install: all
	mkdir -p $(PREFIX)/include/ice2k
	cp -r ice2k/wizard $(PREFIX)/include/ice2k/
	cp $(OUTPUT) $(PREFIX)/lib/$(OUTPUT)

uninstall:
	rm -f $(PREFIX)/include/ice2k/wizard/* || true
	rmdir $(PREFIX)/include/ice2k/wizard >/dev/null 2>&1 || true
	rm -f $(PREFIX)/lib/$(OUTPUT) || true

