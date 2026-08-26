.PHONY: clean uninstall install

CC = gcc

CFLAGS = -Wall -c -fno-exceptions
LDFLAGS = -lX11

PREFIX=/usr/local

SOURCES = i2ktrail.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = i2ktrail

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

install:
	cp $(EXECUTABLE) $(PREFIX)/bin/$(EXECUTABLE)

uninstall:
	rm -f $(PREFIX)/bin/$(EXECUTABLE)
