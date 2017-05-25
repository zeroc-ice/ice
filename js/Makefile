# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

ifeq ($(YARN),)
YARN = yarn
endif

all: npminstall
	$(YARN) run build

dist: npminstall
	$(YARN) run dist

install:

distclean clean: npminstall
	$(YARN) run clean

lint: npminstall
	$(YARN) run lint

test:
	@python ./allTests.py

npminstall:
	$(YARN) install
