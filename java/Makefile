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

ifneq ($(install_javadir),)
INSTALL_FLAGS += -Dorg.gradle.project.jarDir=$(install_javadir)
endif

ifneq ($(install_bindir),)
INSTALL_FLAGS += -Dorg.gradle.project.binDir=$(install_bindir)
endif

ifneq ($(DESTDIR),)
INSTALL_FLAGS += -PDESTDIR=$(DESTDIR)
endif

ifeq ($(GRADLE),)
GRADLE = ./gradlew
endif

all:
	$(GRADLE) $(BUILD_FLAGS) build

tests:
	$(GRADLE) $(BUILD_FLAGS) :test:assemble

srcs:
	$(GRADLE) $(BUILD_FLAGS) dist

distclean clean:
	$(GRADLE) clean

install::
	$(GRADLE) $(INSTALL_FLAGS) install

install-icegridgui::
	$(GRADLE) $(INSTALL_FLAGS) :IceGridGUI:install

test::
	@python ./allTests.py
