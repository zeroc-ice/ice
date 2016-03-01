# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

ifneq ($(prefix),)
INSTALL_FLAGS += -Dorg.gradle.project.prefix=$(prefix)
endif

ifneq ($(DESTDIR),)
INSTALL_FLAGS += -Dorg.gradle.project.DESTDIR=$(DESTDIR)
endif

ifeq ($(GRADLE),)
GRADLE = ./gradlew
endif

all:
	$(GRADLE) $(BUILD_FLAGS) build

dist:
	$(GRADLE) $(BUILD_FLAGS) :Ice:assemble :Freeze:assemble :Glacier2:assemble :IceGrid:assemble \
		:IceBox:assemble :IceDiscovery:assemble :IcePatch2:assemble :IceStorm:assemble :IceGridGUI:assemble

clean:
	$(GRADLE) clean

install::
	$(GRADLE) $(INSTALL_FLAGS) install

test::
	@python ./allTests.py
