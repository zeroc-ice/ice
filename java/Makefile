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

ifneq ($(prefix),)
  GRADLEOPTS += -Pprefix=$(prefix)
endif

ifneq ($(DESTDIR),)
  GRADLEOPTS += -PDESTDIR=$(DESTDIR)
endif

ifneq ($(install_javadir),)
  GRADLEOPTS += -Dorg.gradle.project.jarDir=$(install_javadir)
endif

ifneq ($(install_bindir),)
  GRADLEOPTS += -Dorg.gradle.project.binDir=$(install_bindir)
endif

all:
	$(GRADLE) $(GRADLEOPTS) build

tests:
	$(GRADLE) $(GRADLEOPTS) :test:assemble

dist:
	$(GRADLE) $(GRADLEOPTS) dist

clean:
	$(GRADLE) $(GRADLEOPTS) clean

install::
	$(GRADLE) $(GRADLEOPTS) install

test::
	@python ./allTests.py
