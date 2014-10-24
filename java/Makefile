# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

ifeq ($(APPEND_VERSION_SUFFIX),yes)
BUILD_FLAGS += -Dorg.gradle.project.appendVersionSuffix=yes
endif

INSTALL_FLAGS += $(BUILD_FLAGS)

ifneq ($(prefix),)
INSTALL_FLAGS += -Dorg.gradle.project.prefix=$(prefix)
endif

ifneq ($(DESTDIR),)
INSTALL_FLAGS += -Dorg.gradle.project.DESTDIR=$(DESTDIR)
endif

all:
	./gradlew $(BUILD_FLAGS) assemble

clean:
	./gradlew $(BUILD_FLAGS) clean

install::
	./gradlew $(INSTALL_FLAGS) install

test::
	@python ./allTests.py
