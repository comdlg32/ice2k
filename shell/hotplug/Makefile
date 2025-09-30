.PHONY: clean uninstall install

CC = g++

CFLAGS = -Wall -c -fno-rtti -fno-exceptions -g
CPPFLAGS = -Wall -c -fno-rtti -fno-exceptions -g
LDFLAGS = `fox-config --libs` -lctl32 -lpci -lXrandr -lXi -lserialport -li2kbrand

PREFIX=/usr/local

ICONS = res/*/*.gif res/*.gif res/*.bmp
SOURCES = devmgmt.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = ejtray

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
