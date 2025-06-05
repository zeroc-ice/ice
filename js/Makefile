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
	mkdir -p dist
	node dts-bundle.js
	npx typedoc --router structure dist/bundle.d.ts --disableGit --disableSources
