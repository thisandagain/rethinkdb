# Copyright 2010-2012 RethinkDB, all rights reserved.

DEBUG?=0

all:
	cd src ; $(MAKE) DEBUG=$(DEBUG) PACKAGING=1 WEBRESDIR=/usr/share/rethinkdb/web ;

install: all
	cd src ; $(MAKE) DEBUG=$(DEBUG) PACKAGING=1 install ;

clean:
	rm -rf build

distclean: clean

build-deb-src-control:
	cd src ; $(MAKE) DEBUG=$(DEBUG) build-deb-src-control ;

build-deb-src: build-deb-src-control
#	$(shell scripts/gen-version.sh > VERSION)
	yes | debuild -S -sa ;

deb:
	cd src ; $(MAKE) DEBUG=$(DEBUG) deb ;

.PHONY: all install clean build-deb-src-control build-deb-src deb
