CXX      = g++
CXXFLAGS = `fox-config --cflags` --std=c++98 -fpic

LD       = g++
LDFLAGS  = 
LDLIBS   = `fox-config --libs` -lfxextra

OUTPUT    = calendar

all: $(OUTPUT)

$(OUTPUT): calendar.o
	$(LD) $(LDFLAGS) -o calendar calendar.o $(LDLIBS)

calendar.o: calendar.cpp
	$(CXX) -c $(CXXFLAGS) calendar.cpp

clean:
	rm -f $(OUTPUT) calendar.o
