#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

ifeq ($(NPM),)
NPM = npm
endif

all: npminstall
	$(NPM) run build

dist: npminstall
	$(NPM) run dist

install:

distclean clean: npminstall
	$(NPM) run clean

lint: npminstall
	$(NPM) run lint

npminstall:
	$(NPM) install --no-optional
