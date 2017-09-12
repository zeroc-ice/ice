# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

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

test:
	@python ./allTests.py

npminstall:
	$(NPM) install --no-optional
