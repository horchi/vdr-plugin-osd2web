#
# Makefile for a Video Disk Recorder plugin
#

PLUGIN    = osd2web
HLIB      = -L./lib -lhorchi
HISTFILE  = "HISTORY.h"

include Make.config


### The version number of this plugin (taken from the main source file):

VERSION = $(shell grep 'define _VERSION ' $(HISTFILE) | awk '{ print $$3 }' | sed -e 's/[";]//g')

LASTHIST    = $(shell grep '^20[0-3][0-9]' $(HISTFILE) | head -1)
LASTCOMMENT = $(subst |,\n,$(shell sed -n '/$(LASTHIST)/,/^ *$$/p' $(HISTFILE) | tr '\n' '|'))
LASTTAG     = $(shell git describe --tags --abbrev=0)
BRANCH      = $(shell git rev-parse --abbrev-ref HEAD)
GIT_REV = $(shell git describe --always 2>/dev/null)

### The directory environment:

# Use package data if installed...otherwise assume we're under the VDR source directory:
PKGCFG = $(if $(VDRDIR),$(shell pkg-config --variable=$(1) $(VDRDIR)/vdr.pc),$(shell pkg-config --variable=$(1) vdr || pkg-config --variable=$(1) ../../../vdr.pc))

LIBDIR   = $(call PKGCFG,libdir)
LOCDIR   = $(call PKGCFG,locdir)
PLGCFG   = $(call PKGCFG,plgcfg)
CONFDEST = $(call PKGCFG,configdir)/plugins/$(PLUGIN)
HTTPDEST = $(CONFDEST)/http

#

TMPDIR ?= /tmp

### The compiler options:

export CFLAGS   += $(call PKGCFG,cflags)
export CXXFLAGS += $(call PKGCFG,cxxflags)

### The version number of VDR's plugin API:

APIVERSION = $(call PKGCFG,apiversion)

### Allow user defined options to overwrite defaults:

-include $(PLGCFG)

OBJS = $(PLUGIN).o \
       config.o update.o scraper2vdr.o dia.o websock.o fileservice.o status.o tools.o skin.o jobject.o

LIBS  = $(HLIB)
LIBS += -lrt -lcrypto
LIBS += $(shell pkg-config --libs uuid)
LIBS += $(shell pkg-config --libs tinyxml2)
LIBS += $(shell pkg-config --libs libwebsockets)
LIBS += $(shell python-config --libs) $(shell pkg-config --cflags --libs jansson)
LIBS += $(shell pkg-config libexif --libs)

### The name of the distribution archive:

ARCHIVE = $(PLUGIN)-$(VERSION)
PACKAGE = vdr-$(ARCHIVE)

### The name of the shared object file:

SOFILE = libvdr-$(PLUGIN).so

### Includes and Defines (add further entries here):

DEFINES += -DOSD2WEB -DLOG_PREFIX='"$(PLUGIN): "' $(USES)

ifdef PATCHED
	DEFINES += -DPATCHED
endif

ifdef GIT_REV
   DEFINES += -DGIT_REV='"$(GIT_REV)"'
endif

### The main target:

all: $(SOFILE) i18n

hlib:
	(cd lib && $(MAKE) -s lib)

### Implicit rules:

%.o: %.c
	$(doCompile) $(INCLUDES) -o $@ $<
#	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) -o $@ $<

### Dependencies:

MAKEDEP = $(CC) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	$(MAKEDEP) $(CXXFLAGS) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.c) > $@

-include $(DEPFILE)

### Internationalization (I18N):

PODIR     = po
I18Npo    = $(wildcard $(PODIR)/*.po)
I18Nmo    = $(addsuffix .mo, $(foreach file, $(I18Npo), $(basename $(file))))
I18Nmsgs  = $(addprefix $(DESTDIR)$(LOCDIR)/, $(addsuffix /LC_MESSAGES/vdr-$(PLUGIN).mo, $(notdir $(foreach file, $(I18Npo), $(basename $(file))))))
I18Npot   = $(PODIR)/$(PLUGIN).pot

%.mo: %.po
	msgfmt -c -o $@ $<

$(I18Npot): $(wildcard *.c *.h)
	xgettext -C -cTRANSLATORS --no-wrap --no-location -k -ktr -ktrNOOP --package-name=vdr-$(PLUGIN) --package-version=$(VERSION) --msgid-bugs-address='<vdr@jwendel.de>' -o $@ `ls $^`

%.po: $(I18Npot)
	msgmerge -U --no-wrap --no-location --backup=none -q -N $@ $<
	@touch $@

$(I18Nmsgs): $(DESTDIR)$(LOCDIR)/%/LC_MESSAGES/vdr-$(PLUGIN).mo: $(PODIR)/%.mo
	install -D -m644 $< $@

.PHONY: i18n
i18n: $(I18Nmo) $(I18Npot)

install-i18n: $(I18Nmsgs)

### Targets:

$(SOFILE): hlib $(OBJS)
	$(CC) $(LDFLAGS) -shared $(OBJS) $(LIBS) -o $@

cbuild:
	(cd client && $(MAKE) cbuild)

buildenv:
	(cd client && $(MAKE) buildenv)

install-lib: $(SOFILE)
	install -D -m644 $^ $(DESTDIR)$(LIBDIR)/$^.$(APIVERSION)

install: install-lib install-i18n install-config install-http

install-config:
	if ! test -d $(DESTDIR)$(CONFDEST); then \
	   mkdir -p $(DESTDIR)$(CONFDEST); \
	   chmod a+rx $(DESTDIR)$(CONFDEST); \
	fi
ifdef VDR_USER
	if test -n $(VDR_USER); then \
		chown -R $(VDR_USER) $(DESTDIR)$(CONFDEST); \
	fi
endif

install-http:
	if ! test -d $(DESTDIR)$(HTTPDEST); then \
	   mkdir -p $(DESTDIR)$(HTTPDEST); \
	   chmod a+rx $(DESTDIR)$(HTTPDEST); \
	fi
	cp -r ./client/dist/* $(DESTDIR)$(HTTPDEST)
	chmod -R a+rX $(DESTDIR)$(HTTPDEST)
	cp ./scripts/startBrowser.sh $(DESTDIR)$(CONFDEST)
	cp ./scripts/browser.conf.tmpl $(DESTDIR)$(CONFDEST)/
	if ! test -f $(DESTDIR)$(CONFDEST)/browser.conf; then \
		cp ./scripts/browser.conf.tmpl $(DESTDIR)$(CONFDEST)/browser.conf; \
   fi
ifdef VDR_USER
	if test -n $(VDR_USER); then \
		chown -R $(VDR_USER) $(DESTDIR)$(HTTPDEST); \
	fi
endif

dist: clean
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@mkdir $(TMPDIR)/$(ARCHIVE)
	@cp -a * $(TMPDIR)/$(ARCHIVE)
	@tar czf $(PACKAGE).tgz --exclude=$(ARCHIVE)'/client/node_modules/*' -C $(TMPDIR) $(ARCHIVE)
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@echo Distribution package created as $(PACKAGE).tgz

clean:
	@-rm -f $(OBJS) $(DEPFILE) *.so *.core* *~ ./configs/*~
	@-rm -f $(PODIR)/*.mo $(PODIR)/*.pot $(PODIR)/*~
	@-rm -f $(PACKAGE).tgz
	$(MAKE) -C lib/ clean

cppchk:
	cppcheck --language=c++ --template="{file}:{line}:{severity}:{message}" --quiet --force *.c *.h

# ------------------------------------------------------
# Git / Versioning / Tagging
# ------------------------------------------------------

vcheck:
	git fetch
	if test "$(LASTTAG)" = "$(VERSION)"; then \
		echo "Warning: tag/version '$(VERSION)' already exists, update HISTORY first. Aborting!"; \
		exit 1; \
	fi

push: vcheck
	echo "tagging git with $(VERSION)"
	git push
	git tag $(VERSION)
	git push --tags

commit: vcheck
	git commit -m "$(LASTCOMMENT)" -a

git: commit push

showv:
	@echo "Git ($(BRANCH)):\\n  Version: $(LASTTAG) (tag)"
	@echo "Local:"
	@echo "  Version: $(VERSION)"
	@echo "  Change:"
	@echo -n "   $(LASTCOMMENT)"

update:
	git pull
	@make clean install
	restart vdr
