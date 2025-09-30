# options

### verbosity: 0 for off, 1 for on (default: off)
V = 0

### whether to link against gtk3 or gtk2 (default: gtk3)
WITH_GTK3 = 0

### libnotify support: 0 for off, 1 for on (default: on)
WITH_NOTIFY = 0

# programs

CC ?= gcc
MSGFMT = msgfmt
PKG_CONFIG ?= pkg-config
RM = rm -f
GETTEXT=xgettext
INSTALL = install
INSTALL_BIN = $(INSTALL) -m755
INSTALL_DATA = $(INSTALL) -m644

# variables

PACKAGE_NAME = cbatticon
VERSION = $(shell grep CBATTICON_VERSION_NUMBER cbatticon.c | awk '{print $$3}')
PREFIX ?= /usr
BINDIR = $(PREFIX)/bin
DOCDIR = $(PREFIX)/share/doc/$(PACKAGE_NAME)-$(VERSION)
MANDIR = $(PREFIX)/share/man/man1
NLSDIR = $(PREFIX)/share/locale
LANGUAGES = bs de el es fr he hr id ja pt_BR ru sk sr tr zh_TW

BIN = $(PACKAGE_NAME)
SOURCEFILES := $(wildcard *.c)
OBJECTS := $(patsubst %.c,%.o,$(SOURCEFILES))
SOURCECATALOGS := $(wildcard *.po)
TRANSLATIONS := $(patsubst %.po,%.mo,$(SOURCECATALOGS))

# flags and libs

ifeq ($(V),0)
VERBOSE=@
else
VERBOSE=
endif

ifeq ($(WITH_NOTIFY),1)
CPPFLAGS += -DWITH_NOTIFY
endif
CPPFLAGS += -DNLSDIR=\"$(NLSDIR)\"

CFLAGS ?= -O2
CFLAGS += -Wall -Wno-deprecated-declarations -std=c99
CFLAGS += $(shell $(PKG_CONFIG) --cflags $(PKG_DEPS))

ifeq ($(WITH_GTK3), 0)
PKG_DEPS = gtk+-2.0
else
PKG_DEPS = gtk+-3.0
endif

ifeq ($(WITH_NOTIFY),1)
PKG_DEPS += libnotify
endif

LIBS += $(shell $(PKG_CONFIG) --libs $(PKG_DEPS)) -lm

# targets

all: $(BIN) $(TRANSLATIONS)

$(BIN): $(OBJECTS)
	@echo -e '\033[0;35mLinking executable $@\033[0m'
	$(VERBOSE) $(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OBJECTS): $(SOURCEFILES)
	@echo -e '\033[0;32mBuilding object $@\033[0m'
	$(VERBOSE) $(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

$(TRANSLATIONS): %.mo: %.po
	@echo -e '\033[0;36mCompiling messages catalog $@\033[0m'
	$(VERBOSE) $(MSGFMT) -o $@ $<

install: $(BIN) $(TRANSLATIONS)
	@echo -e '\033[0;33mInstalling $(PACKAGE_NAME)\033[0m'
	$(VERBOSE) $(INSTALL) -d "$(DESTDIR)$(BINDIR)"
	$(VERBOSE) $(INSTALL_BIN) $(BIN) "$(DESTDIR)$(BINDIR)"/
	$(VERBOSE) $(INSTALL) -d "$(DESTDIR)$(DOCDIR)"
	$(VERBOSE) $(INSTALL_DATA) README "$(DESTDIR)$(DOCDIR)"/
	$(VERBOSE) $(INSTALL) -d "$(DESTDIR)$(MANDIR)"
	$(VERBOSE) $(INSTALL_DATA) cbatticon.1 "$(DESTDIR)$(MANDIR)"/
	$(VERBOSE) for language in $(LANGUAGES); \
	do \
		$(INSTALL) -d "$(DESTDIR)$(NLSDIR)"/$$language/LC_MESSAGES; \
		$(INSTALL_DATA) $$language.mo "$(DESTDIR)$(NLSDIR)"/$$language/LC_MESSAGES/$(PACKAGE_NAME).mo; \
	done

uninstall:
	@echo -e '\033[0;33mUninstalling $(PACKAGE_NAME)\033[0m'
	$(VERBOSE) $(RM) "$(DESTDIR)$(BINDIR)"/$(BIN)
	$(VERBOSE) $(RM) "$(DESTDIR)$(DOCDIR)"/README
	$(VERBOSE) $(RM) "$(DESTDIR)$(MANDIR)"/cbatticon.1
	$(VERBOSE) for language in $(LANGUAGES); \
	do \
		$(VERBOSE) $(RM) "$(DESTDIR)$(NLSDIR)"/$$language/LC_MESSAGES/$(PACKAGE_NAME).mo; \
	done

clean:
	@echo -e '\033[0;33mCleaning up source directory\033[0m'
	$(VERBOSE) $(RM) $(BIN) $(OBJECTS) $(TRANSLATIONS)

translation-refresh-pot:
	$(VERBOSE) $(GETTEXT) --default-domain=$(PACKAGE_NAME) --add-comments \
		--keyword=_ --keyword=N_ --keyword=g_dngettext:2,3 $(SOURCEFILES) \
		--output=$(PACKAGE_NAME).pot

translation-status:
	$(VERBOSE) for catalog in $(SOURCECATALOGS); \
	do \
		$(MSGFMT) -v --statistics -o /dev/null $$catalog; \
	done

.PHONY: install uninstall clean translation-status
