.PHONY: clean uninstall install

CC = g++
C++ = g++

OS = `uname -s | tr '[a-z]' '[A-Z]'`
# OS = OPENBSD

CFLAGS = -Wall -c --std=c99 -D "OS_$(OS)" -O3 -g
CPPFLAGS = -Wall -c --std=c++98 -D "OS_$(OS)" -O3 -g
LDFLAGS = `fox-config --libs` -lctl32 -li2kbrand

PREFIX=/usr/local

SOURCES = shutdlg.cpp
OBJECTS = shutdlg.o res/foxres.o
RES = res/shutdlg.gif
EXECUTABLE = shutdlg

CPPFLAGS += `fox-config --cflags`

all: $(EXECUTABLE)

$(OBJECTS): res/foxres.h


# generate icon resources
res/foxres.h: $(RES)
	reswrap -i -d -p res_ -o res/foxres.h $(RES)

res/foxres.cpp: res/foxres.h $(RES)
	reswrap -e -d -p res_ -o res/foxres.cpp $(RES)

# link executable
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

clean:
	rm -f res/foxres.h $(OBJECTS) $(EXECUTABLE)

install: $(EXECUTABLE)
	cp shutdlg $(PREFIX)/bin/shutdlg

uninstall:
	rm -f $(PREFIX)/bin/shutdlg
