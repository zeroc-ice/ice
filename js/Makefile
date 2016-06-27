# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

ifeq ($(NPM),)
NPM = npm
endif

all: npminstall
	$(NPM) run gulp:build

dist: npminstall
	$(NPM) run gulp:dist

install:


distclean clean: npminstall
	$(NPM) run gulp:clean

lint: npminstall
	$(NPM) run gulp:lint

test:
	@python ./allTests.py

npminstall:
	$(NPM) install
