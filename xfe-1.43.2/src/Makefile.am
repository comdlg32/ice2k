AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = xfe xfp xfw xfi

xfe_SOURCES = ../st/st.c \
              icons.cpp \
			  xfeutils.cpp \
			  startupnotification.cpp \
			  StringList.cpp \
              File.cpp \
			  FileDict.cpp \
			  IconList.cpp \
			  FileList.cpp \
			  FileDialog.cpp \
			  DirList.cpp \
			  DialogBox.cpp \
			  MessageBox.cpp \
			  Bookmarks.cpp \
			  HistInputDialog.cpp \
			  InputDialog.cpp \
			  OverwriteBox.cpp \
			  ExecuteBox.cpp \
			  TextWindow.cpp \
			  CommandWindow.cpp \
              Properties.cpp \
			  Preferences.cpp \
			  FilePanel.cpp \
			  DirPanel.cpp \
			  DirHistBox.cpp \
			  PathLinker.cpp \
			  BrowseInputDialog.cpp \
			  ArchInputDialog.cpp \
			  FontDialog.cpp \
			  TextLabel.cpp \
			  Keybindings.cpp \
			  KeybindingsDialog.cpp \
			  SearchWindow.cpp \
			  SearchPanel.cpp \
			  XFileExplorer.cpp \
              main.cpp

if STARTUPNOTIFY
xfe_SOURCES += ../libsn/sn-common.c  \
              ../libsn/sn-launchee.c \
              ../libsn/sn-launcher.c \
              ../libsn/sn-list.c \
              ../libsn/sn-monitor.c \
              ../libsn/sn-util.c \
              ../libsn/sn-xmessages.c \
              ../libsn/sn-xutils.c
endif

xfe_LDADD = @LIBINTL@ -lutil


xfp_SOURCES = ../st/st.c \
              icons.cpp \
			  xfeutils.cpp \
			  startupnotification.cpp \
			  StringList.cpp \
              CommandWindow.cpp \
              InputDialog.cpp \
			  DialogBox.cpp \
              OverwriteBox.cpp \
              FileDict.cpp \
			  IconList.cpp \
			  File.cpp \
              FileList.cpp \
			  DirList.cpp \
              FileDialog.cpp \
			  PathLinker.cpp \
			  TextLabel.cpp \
              MessageBox.cpp \
 			  DirHistBox.cpp \
              XFilePackage.cpp

if STARTUPNOTIFY
xfp_SOURCES += ../libsn/sn-common.c  \
              ../libsn/sn-launchee.c \
              ../libsn/sn-launcher.c \
              ../libsn/sn-list.c \
              ../libsn/sn-monitor.c \
              ../libsn/sn-util.c \
              ../libsn/sn-xmessages.c \
              ../libsn/sn-xutils.c
endif

xfp_LDADD = @LIBINTL@ -lutil
	  

xfw_SOURCES = ../st/st.c \
              icons.cpp \
			  xfeutils.cpp \
			  startupnotification.cpp \
			  StringList.cpp \
			  CommandWindow.cpp \
              OverwriteBox.cpp \
			  MessageBox.cpp \
			  IconList.cpp \
			  File.cpp \
              FileList.cpp \
			  DirList.cpp \
              InputDialog.cpp \
			  DialogBox.cpp \
			  FileDict.cpp \
              FileDialog.cpp \
			  PathLinker.cpp \
			  TextLabel.cpp \
              WriteWindow.cpp \
 			  DirHistBox.cpp \
			  FontDialog.cpp \
              XFileWrite.cpp

if STARTUPNOTIFY
xfw_SOURCES += ../libsn/sn-common.c  \
              ../libsn/sn-launchee.c \
              ../libsn/sn-launcher.c \
              ../libsn/sn-list.c \
              ../libsn/sn-monitor.c \
              ../libsn/sn-util.c \
              ../libsn/sn-xmessages.c \
              ../libsn/sn-xutils.c
endif

xfw_LDADD = @LIBINTL@ -lutil

				
xfi_SOURCES = ../st/st.c \
              icons.cpp \
			  xfeutils.cpp \
			  startupnotification.cpp \
 			  StringList.cpp \
              CommandWindow.cpp \
              InputDialog.cpp \
			  DialogBox.cpp \
              OverwriteBox.cpp \
              FileDict.cpp \
			  IconList.cpp \
			  File.cpp \
              FileList.cpp \
			  DirList.cpp \
              FileDialog.cpp \
              MessageBox.cpp \
 			  DirHistBox.cpp \
 			  TextLabel.cpp \
 			  PathLinker.cpp \
              XFileImage.cpp

if STARTUPNOTIFY
xfi_SOURCES += ../libsn/sn-common.c  \
              ../libsn/sn-launchee.c \
              ../libsn/sn-launcher.c \
              ../libsn/sn-list.c \
              ../libsn/sn-monitor.c \
              ../libsn/sn-util.c \
              ../libsn/sn-xmessages.c \
              ../libsn/sn-xutils.c
endif

xfi_LDADD = @LIBINTL@ -lutil


localedir = $(datadir)/locale


AM_CPPFLAGS = -I. -I$(top_srcdir) -I$(top_srcdir)/intl
AM_CFLAGS = -I. -I$(top_srcdir) -I$(top_srcdir)/intl

DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

EXTRA_DIST =  ../libsn/sn-common.h	\
	          ../libsn/sn-internals.h \
	          ../libsn/sn-launchee.h \
			  ../libsn/sn-launcher.h \
	          ../libsn/sn-list.h \
	          ../libsn/sn-monitor.h \
	          ../libsn/sn-util.h \
	          ../libsn/sn-xmessages.h \
			  ../libsn/sn-xutils.h \
			  ../libsn/sn.h \
			  ../st/config.h \
			  ../st/arg.h \
			  ../st/st.h \
			  xfedefs.h \
              icons.h \
			  xfeutils.h \
              help.h \
			  startupnotification.h \
			  StringList.h \
			  FileDialog.h \
			  FileDict.h \
			  IconList.h \
			  FileList.h \
			  DirList.h \
			  DirPanel.h \
			  Properties.h \
              File.h \
			  DialogBox.h \
			  MessageBox.h \
			  HistInputDialog.h \
			  InputDialog.h \
			  Preferences.h \
			  TextWindow.h \
			  CommandWindow.h \
			  OverwriteBox.h \
			  ExecuteBox.h \
			  FilePanel.h \
			  Bookmarks.h \
			  XFileExplorer.h \
			  XFileImage.h \
			  XFilePackage.h \
			  WriteWindow.h \
			  XFileWrite.h \
			  DirHistBox.h \
			  PathLinker.h \
			  BrowseInputDialog.h \
			  ArchInputDialog.h \
			  FontDialog.h \
			  TextLabel.h \
			  Keybindings.h \
			  KeybindingsDialog.h \
			  SearchPanel.h \
			  SearchWindow.h \
			  foxhacks.cpp \
			  clearlooks.cpp

