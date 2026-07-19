.PHONY: clean uninstall install

CC = gcc
C++ = g++

CFLAGS = -g -Wall -c --std=c99
CPPFLAGS = -g -Wall -c --std=c++98
LDFLAGS = `fox-config --libs` -lctl32 -li2kbrand

PREFIX=/usr/local

SOURCES = firstrun.cpp
OBJECTS = firstrun.o
EXECUTABLE = i2kfirstrundlg

CPPFLAGS += `fox-config --cflags`

all: $(EXECUTABLE)

# link executable
$(EXECUTABLE): $(OBJECTS)
	$(C++) $(OBJECTS) -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

install: $(EXECUTABLE)
	cp $(EXECUTABLE) $(PREFIX)/bin/$(EXECUTABLE)

uninstall:
	rm -f $(PREFIX)/bin/$(EXECUTABLE)
