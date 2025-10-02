# $Id : Makefile.in 4.04 1997/07/10 $
#
# xlockmore Makefile.in for autoconf (UNIX/VMS X11 support)
############################################################

# 

datarootdir = ${prefix}/share
srcdir = ../../xmlock
top_srcdir = ../..
VPATH = ../../xmlock
BITMAPDIR = $(top_srcdir)/bitmaps/
XMLOCKDIR = $(top_srcdir)/xmlock/

N=
O=.o
#O=.obj
C=.c
#C=.cc
S=$(N) $(N)
#S=,
E=
#E=.exe

# please define
# C as the C source code extension
# O as the object extension
# S as the separator for object code

####################################################################
# List of object files
XMLOCKOBJS = option$(O)$(S)xmlock$(O)

# List of source files
XMLOCKSRCS = $(VPATH)/option$(C) $(VPATH)/xmlock$(C)

# default target
all : xmlock

# this tells GNU make not to export variables into the environment
# But other makes do not understand its significance, so it must
# not be the first target in the file. So it is here, before
# any variables are created, but after the default target
.NOEXPORT :

SHELL = /bin/sh

prefix = /usr/local
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
mandir = ${datarootdir}/man/man1
xapploaddir = /usr/lib/X11/app-defaults

CONFIGDIR = $(srcdir)/config/

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
INSTPGMFLAGS = -s -g shadow -m 2111

#CC = cc -g
#CC = cc -Ac -g
#CC = cc -Xc -g
#CC = acc -g
#CC = CC -g
#CC = gcc -g -Wall -ansi -pedantic
#CC = gcc -g -Wall -Wstrict-prototypes -Wnested-externs -Wno-format
#CC = gcc -g -Wall -W -Wshadow -Wpointer-arith -Wbad-function-cast -Wcast-align -Wwrite-strings -Waggregate-return -Wmissing-prototypes -Wstrict-prototypes
#CC = g++ -g -Wall
#CC = gcc -g -Wall -Wstrict-prototypes -Wnested-externs -Wmissing-prototypes -Wno-overlength-strings -Wdeclaration-after-statement
CC = gcc -Wnested-externs -Wno-format

#LINT = lint
LINT = lint -Ncheck=%all
#LINT = alint

#DEPEND = makedepend
DEPEND = makedepend
DEPEND_FLAGS = 
DEPEND_DEFINES = 

LN_S = ln -s
RM = rm -f
RM_S = $(RM)
ECHO = echo

#BLN_S = set file/enter=[]
#RM = delete/noconfirm/nolog
#RM_S = set file/remove/nolog
#ECHO = write sys$output

DEFS = -DHAVE_CONFIG_H $(DEFINES)
#DEFS = -DHAVE_CONFIG_H $(DEFINES) -DHAVE_MOTIF
#DEFS = -DHAVE_CONFIG_H $(DEFINES) -DHAVE_LIB_XAW3DXFT # change -lXm to -lXaw3dxft
#DEFS = -DHAVE_CONFIG_H $(DEFINES) -DHAVE_LIB_XAW3D # change -lXm to -lXaw3d
#DEFS = -DHAVE_CONFIG_H $(DEFINES) -DHAVE_LIB_NEXTAW # change -lXm to -lneXtaw
#DEFS = -DHAVE_CONFIG_H $(DEFINES) -DHAVE_LIB_XAWPLUS # change -lXm to -lXawPlus
#DEFS = -DHAVE_CONFIG_H $(DEFINES) -DHAVE_ATHENA # change -lXm to -lXaw
XMLOCKINC =  -I.. -I$(top_srcdir)
CFLAGS = -I/usr/include/freetype2 -I/usr/include/FTGL -O2 -pthread
#CFLAGS = -O
#CFLAGS = -g
XMLOCKLDFLAGS =  -L/usr/include/freetype2 -L/usr/include/FTGL
XLIBS = 
XMLOCKLIBS =  -lXpm -lXaw3dxft -lXt  -lX11
#If you have purify, and want to use it, uncomment this definition or
# run the make as `make PURIFY=purify'
# or run configure with the --with-purify argument.
PURIFY = 

xmlock : $(XMLOCKOBJS)
	$(PURIFY) $(CC) -o $@ $(XMLOCKOBJS) $(XMLOCKLDFLAGS) $(XMLOCKLIBS)
	@ $(ECHO) "$@ BUILD COMPLETE"
	@ $(ECHO) ""

.SUFFIXES : $(C) $(O)

$(C)$(O) :
	$(CC) -c $(CPPFLAGS) $(DEFS) $(XMLOCKINC) $(CFLAGS) $<

install : install_xmlock

install_xmlock : install-program install-man install-ad
	@ $(ECHO) "$@ COMPLETE"
	@ $(ECHO) ""

install-program : xmlock
	$(top_srcdir)/mkinstalldirs $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) xmlock$(E) $(DESTDIR)$(bindir)

install-man :

install-ad :
	$(top_srcdir)/mkinstalldirs $(DESTDIR)$(xapploaddir)
	$(INSTALL_DATA) $(top_srcdir)/xmlock/XmLock.ad $(DESTDIR)$(xapploaddir)/XmLock

uninstall : uninstall_xmlock

uninstall_xmlock : uninstall-program uninstall-man uninstall-ad

uninstall-program :
	$(RM) $(DESTDIR)$(bindir)/xmlock$(E)

uninstall-man :

uninstall-ad :
	$(RM) $(DESTDIR)$(xapploaddir)/XmLock

install.program : install-program

install.man : install-man

install.ad : install-ad

uninstall.program : uninstall-program

uninstall.man : uninstall-man

uninstall.ad : uninstall-ad

lint :
	$(LINT) -ax -DLINT $(DEFS) $(XMLOCKINC) $(XMLOCKSRCS)

antic :
	antic $(XMLOCKDIR)*.h $(XMLOCKDIR)*.c

cppcheck :
	cppcheck -f -q $(XMLOCKDIR)*.c

xrdb :
	xrdb $(top_srcdir)/xmlock/XmLock.ad

man :

html :

hlp :

clean :
	$(RM) *.o core *~ *% *.bak *.orig *.rej *.patch .pure
	$(RM) make.log MakeOut Makefile.dep
	$(RM) xmlock xmlock.exe

distclean : clean
	$(RM) Makefile

clean.all : distclean

# Adds all current dependencies to Makefile
depend :
	$(DEPEND) -s '# DO NOT DELETE: updated by make depend'              \
	$(DEPEND_FLAGS) --                                                  \
	$(XMLOCKINC) $(DEFS) $(DEPEND_DEFINES) $(CFLAGS) --                 \
	$(XMLOCKSRCS)

# Adds some dependencies to Makefile.in -- not totally accurate, but pretty
# close.  This excludes dependencies on files in /usr/include, etc.  It tries
# to include only dependencies on files which are themselves a part of this
# package. -w 0
distdepend :
	@echo updating dependencies in `pwd`/Makefile.in... ;               \
	$(DEPEND) -w 0 -f -                                                 \
	-s '# DO NOT DELETE: updated by make distdepend' $(DEPEND_FLAGS) -- \
	$(XMLOCKINC) $(DEFS) $(DEPEND_DEFINES) $(CFLAGS) --                 \
	$(XMLOCKSRCS) |                                                     \
	(                                                                   \
	  awk '/^#.*Id : Makefile.in/,/^# DO .*distdepend/' < Makefile.in ; \
	  sed -e 's@ \./@ @g;s@ /[^ ]*@@g;/^.*:$$/d'                        \
	      -e 's@\.\./bitmaps@$$(BITMAPDIR)@g' ;                         \
	  echo ''                                                           \
	) > /tmp/distdepend.$$$$ &&                                         \
	mv Makefile.in Makefile.in.bak &&                                   \
	mv /tmp/distdepend.$$$$ Makefile.in

##############################################################################
#
# DO NOT DELETE: updated by make distdepend

option.o: option.h
xmlock.o: ../config.h
xmlock.o: option.h
xmlock.o: $(BITMAPDIR)/m-xlock.xbm
xmlock.o: modes.h

