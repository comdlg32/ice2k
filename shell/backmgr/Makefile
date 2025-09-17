.PHONY: clean uninstall install

CC = gcc

CFLAGS = -Wall -c -fno-exceptions
LDFLAGS = -linih-i2k

PREFIX=/usr/local

SOURCES = backmgr.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = backmgr

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

install:
	cp $(EXECUTABLE) $(PREFIX)/bin/$(EXECUTABLE)

uninstall:
	rm -f $(PREFIX)/bin/$(EXECUTABLE)
