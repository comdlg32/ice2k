.PHONY: clean uninstall install

CC = gcc
CFLAGS = -Wall -c
PREFIX = /usr/local

OUTPUT = libinih-i2k.so

#COBJECTS = $(CSOURCES:.c=.o)

SOURCES = ini.c
OBJECTS = $(SOURCES:.c=.o)

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(CC) $(OBJECTS) -shared -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJECTS) $(OUTPUT)

install: all
	mkdir -p $(PREFIX)/include/ice2k
	mkdir -p $(PREFIX)/lib
	cp ice2k/ini.h $(PREFIX)/include/ice2k/ini.h
	rm -f $(PREFIX)/lib/$(OUTPUT)
	cp $(OUTPUT) $(PREFIX)/lib/$(OUTPUT)
	

uninstall:
	rm -f $(PREFIX)/lib/$(OUTPUT)
	rm $(PREFIX)/include/ice2k/ini.h

ini.o: ice2k/ini.h
