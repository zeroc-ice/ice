# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

ifeq ($(GRADLE),)
GRADLE = ./gradlew
endif

ifneq ($(prefix),)
  GRADLEARGS += -Pprefix=$(prefix)
endif

ifneq ($(DESTDIR),)
  GRADLEARGS += -PDESTDIR=$(DESTDIR)
endif

ifneq ($(install_javadir),)
  GRADLEARGS += -Dorg.gradle.project.jarDir=$(install_javadir)
endif

ifneq ($(install_bindir),)
  GRADLEARGS += -Dorg.gradle.project.binDir=$(install_bindir)
endif

all:
	$(GRADLE) $(GRADLEARGS) build

tests:
	$(GRADLE) $(GRADLEARGS) :test:assemble

srcs:
	$(GRADLE) $(GRADLEARGS) dist

distclean clean:
	$(GRADLE) $(GRADLEARGS) clean

install::
	$(GRADLE) $(GRADLEARGS) install

test::
	@python ./allTests.py
