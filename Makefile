# Copyright (C) 2022 Vino Fernando Crescini <vfcrescini@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

# 2021-09-26 Makefile


.PHONY = all clean distclean build tarball debpkg

PKGNAME = dhtxx
SRCDIR = ./src
TMPDIR = ./tmp
ARCH = $(shell which dpkg > /dev/null 2>&1 && dpkg --print-architecture || uname -m)
TGZFILE = $(PKGNAME).tar.gz
DEBFILE = $(PKGNAME)-$(ARCH).deb
CNFFILE = filelist.conf


all: tarball


clean:
	$(RM) *.o
	$(RM) -r $(TMPDIR)
	$(MAKE) -C $(SRCDIR) clean


distclean: clean
	$(RM) $(TGZFILE)
	$(RM) $(DEBFILE)
	$(MAKE) -C $(SRCDIR) distclean


build:
	$(MAKE) -C $(SRCDIR) all


tarball: $(TGZFILE)


debpkg: $(DEBFILE)


$(TGZFILE): build $(CNFFILE)
	fakeroot ./gentar.sh $(CNFFILE) $(@)


$(DEBFILE): $(TGZFILE)
	mkdir -p $(TMPDIR)/DEBIAN
	cp -f debian/conffiles $(TMPDIR)/DEBIAN
	cp -f debian/shlibs $(TMPDIR)/DEBIAN
	cp -f debian/triggers $(TMPDIR)/DEBIAN
	sed -e "s/%ARCH%/$(ARCH)/g" debian/control > $(TMPDIR)/DEBIAN/control
	fakeroot -- /bin/sh -c "\
	  umask 0022; \
	  tar -C $(TMPDIR) -xzf $(TGZFILE); \
	  mkdir -p $(TMPDIR)/usr/share/doc/$(PKGNAME); \
	  cp debian/copyright $(TMPDIR)/usr/share/doc/$(PKGNAME)/copyright; \
	  gzip -9 < debian/changelog > $(TMPDIR)/usr/share/doc/$(PKGNAME)/changelog.Debian.gz; \
	  dpkg-deb --build $(TMPDIR) $(DEBFILE) \
	"
