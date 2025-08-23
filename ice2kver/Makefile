.PHONY: clean uninstall install

CC = g++

CFLAGS = -Wall -c
CPPFLAGS = -Wall -c
LDFLAGS = `fox-config --libs` -lctl32 -li2kbrand

PREFIX=/usr/local

SOURCES = ice2kver.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = ice2kver

CPPFLAGS += `fox-config --cflags`

all: $(EXECUTABLE)

# link executable
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

install:
	cp ice2kver $(PREFIX)/bin/ice2kver
	ln -sf $(PREFIX)/bin/ice2kver $(PREFIX)/bin/winver

uninstall:
	rm -f $(PREFIX)/bin/ice2kver
	rm -f $(PREFIX)/bin/winver
