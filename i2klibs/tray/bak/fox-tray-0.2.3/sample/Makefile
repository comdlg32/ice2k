CXX=g++
LD=g++
CFLAGS=-Wall -ansi $(shell fox-config --cflags) -I../src
LIBS=$(shell fox-config --libs)
PROG=tray_test
SRC=TrayApp.cpp \
	../src/FXTrayIcon.cpp \
	../src/FXTrayApp.cpp

# Build rules
all: $(PROG)

# Common Variables
OBJ=$(SRC:.cpp=.o)
DEPS=$(SRC:.cpp=.dep)

$(PROG): $(OBJ)
	@echo "(LD) $@"
	@$(LD) -o $@ $(OBJ) $(LIBS)

%.o: %.cpp
	@# $*
	@echo "(CXX) $< ==> $@"
	@$(CXX) -MD -MF $*.dep $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJ) $(PROG) $(DEPS)

run: $(PROG)
	@./$(PROG)

-include $(DEPS)

