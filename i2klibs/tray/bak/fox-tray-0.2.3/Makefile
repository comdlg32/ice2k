SUBDIRS=sample doc

export VERSION=0.2.3
export TARGET=

all: subdirs
	@echo "============================================="
	@echo "Sample application is built in ./sample/"
	@echo "To use FXTrayIcon consult the documentation"
	@echo "============================================="

dist: clean
	tar -zcvf fox-tray-$(VERSION).tar.gz fox-tray-$(VERSION)
	rm -r fox-tray-$(VERSION)

clean: export TARGET=clean
clean: subdirs
	rm -f tags

docs: export TARGET=docs
docs: doc

tags:
	ctags src/*.{cpp,h}

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(TARGET)

.PHONY: subdirs $(SUBDIRS) tags

