CC       = gcc
CFLAGS   = --std=c89

CXX      = g++
CXXFLAGS = `fox-config --cflags` --std=c++98 -fpic -g

AR       = ar

LD       = g++
LDFLAGS  = -g
LDLIBS   = `fox-config --libs`

LIBOUTPUT = libfxextra.a
PICOUTPUT = libfxextra.so

PREFIX    = /usr/local

all: $(LIBOUTPUT) $(PICOUTPUT)

$(LIBOUTPUT): FXCalendar.o FXCalendarView.o
	$(AR) rcs $(LIBOUTPUT) FXCalendar.o FXCalendarView.o

$(PICOUTPUT): FXCalendar.o FXCalendarView.o
	$(LD) $(LDFLAGS) -shared -o libfxextra.so FXCalendar.o FXCalendarView.o $(LDLIBS)

FXCalendar.o: FXCalendar.cpp ice2k/FXExtra/FXCalendar.h ice2k/FXExtra/FXCalendarView.h
	$(CXX) -c $(CXXFLAGS) FXCalendar.cpp

FXCalendarView.o: FXCalendarView.cpp ice2k/FXExtra/FXCalendarView.h
	$(CXX) -c $(CXXFLAGS) FXCalendarView.cpp

clean:
	rm -f $(LIBOUTPUT) $(PICOUTPUT) FXCalendar.o FXCalendarView.o

install: all
	cp -r ice2k $(PREFIX)/include
	cp $(PICOUTPUT) $(PREFIX)/lib/$(PICOUTPUT)
	cp $(LIBOUTPUT) $(PREFIX)/lib/$(LIBOUTPUT)

uninstall:
	rm $(PREFIX)/include/ice2k/FXExtra/FXCalendarView.h || true
	rm $(PREFIX)/include/ice2k/FXExtra/FXCalendar.h || true
	rmdir $(PREFIX)/include/ice2k/FXExtra || true
	rmdir $(PREFIX)/include/ice2k 2>/dev/null || true
	rm -f $(PREFIX)/lib/$(PICOUTPUT) || true
	rm -f $(PREFIX)/lib/$(LIBOUTPUT) || true

