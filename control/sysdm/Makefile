.PHONY: clean uninstall install

CC = g++

PREFIX=/usr/local

CFLAGS = -Wall -c -DINSTPREFIX=\"$(PREFIX)\"
CPPFLAGS = -Wall -c -DINSTPREFIX=\"$(PREFIX)\"
LDFLAGS = `fox-config --libs` -lctl32 -li2kbrand

ICONS = res/*.gif res/*.bmp
SOURCES = sysdm.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = sysdm.cpi

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
	mkdir -p $(PREFIX)/bin
	mkdir -p $(PREFIX)/libexec/ice2k
	cp $(EXECUTABLE) $(PREFIX)/bin/$(EXECUTABLE)
	cp sethostname $(PREFIX)/libexec/ice2k/sethostname

uninstall:
	rm -f $(PREFIX)/bin/$(EXECUTABLE)
	rm -f $(PREFIX)/libexec/ice2k/sethostname
