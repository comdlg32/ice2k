.PHONY: clean uninstall install

CC = g++

CFLAGS = -Wall -c
CPPFLAGS = -Wall -c
LDFLAGS = `fox-config --libs` -lctl32 -li2kbrand

PREFIX=/usr/local

ICONS = res/*.gif res/*.bmp
SOURCES = brdexmpl.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = sysdm

CPPFLAGS += `fox-config --cflags`

all: $(EXECUTABLE)

$(OBJECTS): res/foxres.h

# generate icon resources
res/foxres.h: $(ICONS)
	reswrap -s -p resico_ -z -d -o res/foxres.h $(ICONS)


$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

clean:
	rm -f res/foxres.h $(OBJECTS) $(EXECUTABLE)

install:
	cp brdexmpl $(PREFIX)/bin/brdexmpl

uninstall:
	rm -f $(PREFIX)/bin/brdexmpl
