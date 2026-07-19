CC       = gcc
CFLAGS   = --std=c89 -fpic -O2

CXX      = g++
CXXFLAGS = `fox-config --cflags` --std=c++98 -fpic -O2

AR       = ar

LD       = g++
LDFLAGS  = -g
LDLIBS   = `fox-config --libs` -lctl32

LIBOUTPUT = libi2kbatmeter.a
PICOUTPUT = libi2kbatmeter.so

PREFIX    = /usr/local
ICONS     = res/*.gif

all: $(LIBOUTPUT) $(PICOUTPUT)

res/foxres.h: $(ICONS)
	reswrap -i -d -p resico_ -o res/foxres.h $(ICONS)

res/foxres.cpp: res/foxres.h $(ICONS)
	reswrap -e -d -p resico_ -o res/foxres.cpp $(ICONS)

$(LIBOUTPUT): battery.o I2KBatButton.o I2KBatDialog.o I2KBatMeter.o foxres.o
	$(AR) rcs $(LIBOUTPUT) battery.o I2KBatButton.o I2KBatDialog.o I2KBatMeter.o foxres.o

$(PICOUTPUT): battery.o I2KBatButton.o I2KBatDialog.o I2KBatMeter.o foxres.o
	$(LD) $(LDFLAGS) -shared -o $(PICOUTPUT) battery.o I2KBatButton.o I2KBatDialog.o I2KBatMeter.o foxres.o $(LDLIBS)

battery.o: battery.c ice2k/batmeter/battery.h
	$(CC) -c $(CFLAGS) battery.c

foxres.o: res/foxres.cpp res/foxres.h
	$(CXX) -c $(CXXFLAGS) res/foxres.cpp

I2KBatButton.o: res/foxres.h I2KBatButton.cpp ice2k/batmeter/I2KBatButton.h ice2k/batmeter/battery.h
	$(CXX) -c $(CXXFLAGS) I2KBatButton.cpp

I2KBatDialog.o: res/foxres.h I2KBatDialog.cpp ice2k/batmeter/I2KBatDialog.h ice2k/batmeter/battery.h
	$(CXX) -c $(CXXFLAGS) I2KBatDialog.cpp

I2KBatMeter.o: res/foxres.h I2KBatMeter.cpp ice2k/batmeter/I2KBatMeter.h ice2k/batmeter/I2KBatButton.h ice2k/batmeter/I2KBatDialog.h ice2k/batmeter/battery.h
	$(CXX) -c $(CXXFLAGS) I2KBatMeter.cpp

clean:
	rm -f $(LIBOUTPUT) $(PICOUTPUT) battery.o I2KBatButton.o I2KBatDialog.o I2KBatMeter.o foxres.o

install: all
	cp -r ice2k $(PREFIX)/include
	cp $(PICOUTPUT) $(PREFIX)/lib/$(PICOUTPUT)
	cp $(LIBOUTPUT) $(PREFIX)/lib/$(LIBOUTPUT)

uninstall:
	rm -f $(PREFIX)/include/ice2k/batmeter/I2KBatMeter.h
	rm -f $(PREFIX)/include/ice2k/batmeter/I2KBatButton.h 
	rm -f $(PREFIX)/include/ice2k/batmeter/I2KBatDialog.h 
	rm -f $(PREFIX)/include/ice2k/batmeter/battery.h
	rmdir $(PREFIX)/include/ice2k/batmeter || true
	rmdir $(PREFIX)/include/ice2k 2>/dev/null || true
	rm -f $(PREFIX)/lib/$(PICOUTPUT) || true
	rm -f $(PREFIX)/lib/$(LIBOUTPUT) || true

