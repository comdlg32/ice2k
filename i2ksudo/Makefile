.PHONY: clean uninstall install

CC = gcc
C++ = g++

CFLAGS = -g -Wall -c --std=c99
CPPFLAGS = -g -Wall -c --std=c++98
LDFLAGS = `fox-config --libs` -lctl32 -li2kbrand

PREFIX=/usr/local

SOURCES = i2ksudo-askpass.cpp
OBJECTS = i2ksudo-askpass.o res/foxres.o
ICONS = res/*.gif res/*.png
EXECUTABLE = i2ksudo-askpass

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
	cp i2ksudo $(PREFIX)/bin/i2ksudo
	sed -i 's#PUTPREFIXHERE#'"$(PREFIX)"'#' $(PREFIX)/bin/i2ksudo
	cp i2ksudox $(PREFIX)/bin/i2ksudox
	sed -i 's#PUTPREFIXHERE#'"$(PREFIX)"'#' $(PREFIX)/bin/i2ksudox
	cp $(EXECUTABLE) $(PREFIX)/libexec/ice2k/$(EXECUTABLE)

install-pkexec: $(PREFIX)/bin/i2ksudox
	ln -s $(PREFIX)/bin/i2ksudox $(PREFIX)/bin/pkexec

uninstall:
	rm -f $(PREFIX)/bin/i2ksudo
	rm -f $(PREFIX)/bin/i2ksudox
	rm -f $(PREFIX)/libexec/ice2k/$(EXECUTABLE)
