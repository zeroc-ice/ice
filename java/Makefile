# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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

all:
	./gradlew assemble

dist:
	./gradlew :Ice:assemble :Freeze:assemble :Glacier2:assemble :IceGrid:assemble :ant:assemble \
		:IceBox:assemble :IceDiscovery:assemble :IcePatch2:assemble :IceStorm:assemble :IceGridGUI:assemble

clean:
	./gradlew clean

install::
	./gradlew $(INSTALL_FLAGS) install

test::
	@python ./allTests.py
