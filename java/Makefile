# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

ifeq ($(GRADLE),)
GRADLE = ./gradlew
endif

all:
	$(GRADLE) $(GRADLEOPTS) build

tests:
	$(GRADLE) $(GRADLEOPTS) :test:assemble

srcs:
	$(GRADLE) $(GRADLEOPTS) dist

distclean clean:
	$(GRADLE) $(GRADLEOPTS) clean

install::
	$(GRADLE) $(GRADLEOPTS) install

install-icegridgui::
	$(GRADLE) $(GRADLEOPTS) :IceGridGUI:install

test::
	@python ./allTests.py
