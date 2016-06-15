# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

ifneq ($(prefix),)
INSTALL_FLAGS += -Pprefix=$(prefix)
endif

ifneq ($(DESTDIR),)
INSTALL_FLAGS += -PDESTDIR=$(DESTDIR)
endif

ifeq ($(GRADLE),)
GRADLE = ./gradlew
endif

all:
	$(GRADLE) $(BUILD_FLAGS) build

dist:
	$(GRADLE) $(BUILD_FLAGS) dist

clean:
	$(GRADLE) clean

install::
	$(GRADLE) $(INSTALL_FLAGS) install

test::
	@python ./allTests.py
