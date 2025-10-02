APP=xlockApp
APPLET=xlockApplet

J=.java
C=.class

VPATH=classes
DEST=$(VPATH)/

JAVAC=javac -deprecation -Xlint

LINT = jlint

SHELL=/bin/sh
RM=rm -f
# RM=del
NICE=nice

CLASSES=$(APPLET)$(C)

SOURCES=$(CLASSES:%$(C)=%$(J))

TARGETS=$(CLASSES:%=$(DEST)%)

all : jar

$(DEST)META-INF/MANIFEST.MF :
	mkdir -p $(DEST)META-INF
	echo "Manifest-Version: 1.0" > $(DEST)META-INF/MANIFEST.MF
	echo "Created-By: $(USER)@$(DOMAIN)" >> $(DEST)META-INF/MANIFEST.MF
	echo "Main-Class: $(APPLET)" >> $(DEST)META-INF/MANIFEST.MF

manifest : $(DEST)META-INF/MANIFEST.MF

$(APP).jar : $(DEST)META-INF/MANIFEST.MF $(CLASSES)
	cd $(DEST);\
	jar cvmf META-INF/MANIFEST.MF ../$(APP).jar *$(C)

jar : $(APP).jar

%$(C) : %$(J)
	$(JAVAC) $< -d $(DEST)

clean :
	$(RM) jlock.exe jlock
	$(RM)r $(DEST)

run : $(APP).jar
	$(NICE) java -jar $(APP).jar

gcj :
	gcj --main=$(APPLET) -o $(APPLET) $(APPLET)$(J)

antic :
	antic *.c *$(J)

lint :
	$(LINT) $(DEST)*.class
