.PHONY: clean uninstall install

CC = g++

CFLAGS = -g -O0 -Wall -c -fno-rtti -fno-exceptions
CPPFLAGS = -g -O0 -Wall -c -fno-rtti -fno-exceptions
LDFLAGS = `fox-config --libs` -lctl32 -linih-i2k -lXcomposite -li2kbrand

PREFIX=/usr/local

ICONS = res/*.gif res/*.bmp
SOURCES = desk.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = deskcpl

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
	cp $(EXECUTABLE) $(PREFIX)/bin/$(EXECUTABLE)

uninstall:
	rm -f $(PREFIX)/bin/$(EXECUTABLE)
