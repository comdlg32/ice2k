CC       = gcc
CFLAGS   = --std=c89

CXX      = g++
CXXFLAGS = `fox-config --cflags` --std=c++98 -fpic -g

AR       = ar

LD       = g++
LDFLAGS  = -g
LDLIBS   = `fox-config --libs`

LIBOUTPUT = libi2ktray.a
PICOUTPUT = libi2ktray.so

PREFIX    = /usr/local

all: $(LIBOUTPUT) $(PICOUTPUT)

$(LIBOUTPUT): I2KTrayApp.o I2KTrayIcon.o
	$(AR) rcs $(LIBOUTPUT) I2KTrayApp.o I2KTrayIcon.o

$(PICOUTPUT): I2KTrayApp.o I2KTrayIcon.o
	$(LD) $(LDFLAGS) -shared -o $(PICOUTPUT) I2KTrayApp.o I2KTrayIcon.o $(LDLIBS)

I2KTrayApp.o: I2KTrayApp.cpp ice2k/i2ktray/I2KTrayApp.h ice2k/i2ktray/I2KTrayIcon.h
	$(CXX) -c $(CXXFLAGS) I2KTrayApp.cpp

I2KTrayIcon.o: I2KTrayIcon.cpp ice2k/i2ktray/I2KTrayIcon.h
	$(CXX) -c $(CXXFLAGS) I2KTrayIcon.cpp

clean:
	rm -f $(LIBOUTPUT) $(PICOUTPUT) I2KTrayApp.o I2KTrayIcon.o

install: all
	cp -r ice2k $(PREFIX)/include
	rm -f $(PREFIX)/lib/$(PICOUTPUT)
	cp $(PICOUTPUT) $(PREFIX)/lib/$(PICOUTPUT)
	cp $(LIBOUTPUT) $(PREFIX)/lib/$(LIBOUTPUT)

uninstall:
	rm $(PREFIX)/include/ice2k/i2ktray/I2KTrayIcon.h || true
	rm $(PREFIX)/include/ice2k/i2ktray/I2KTrayApp.h || true
	rmdir $(PREFIX)/include/ice2k/i2ktray || true
	rmdir $(PREFIX)/include/ice2k 2>/dev/null || true
	rm -f $(PREFIX)/lib/$(PICOUTPUT) || true
	rm -f $(PREFIX)/lib/$(LIBOUTPUT) || true

