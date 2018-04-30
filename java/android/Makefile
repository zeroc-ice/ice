# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

ifeq ($(ANDROID_SDK_ROOT),)
$(error Please set ANDROID_SDK_ROOT to the Android SDK installation directory)
endif

ifeq ($(GRADLE),)
GRADLE = ./gradlew
endif

all: local.properties
	$(GRADLE) $(GRADLEARGS) build

tests: local.properties
	$(GRADLE) $(GRADLEARGS) :test:assemble

srcs: local.properties
	$(GRADLE) $(GRADLEARGS) dist

distclean clean: local.properties
	$(GRADLE) $(GRADLEARGS) clean

local.properties:
	@echo sdk.dir=$(ANDROID_SDK_ROOT) > local.properties

test::
	@python ./allTests.py --androidemulator --controller-app
