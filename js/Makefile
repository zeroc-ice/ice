# Copyright (c) ZeroC, Inc.

ifeq ($(NPM),)
NPM = npm
endif

all: npminstall
	$(NPM) run build

srcs: npminstall
	$(NPM) run dist

install:

distclean clean: npminstall
	$(NPM) run clean

lint: npminstall
	$(NPM) run lint

npminstall:
	$(NPM) install

doc: srcs
	$(NPM) run build-doc
