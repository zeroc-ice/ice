# Copyright (c) ZeroC, Inc.

ifeq ($(GRADLE),)
GRADLE = ./gradlew
endif

GRADLEARGS += -x checkStyleMain -x checkStyleTest

all:
	$(GRADLE) $(GRADLEARGS) build

tests:
	$(GRADLE) $(GRADLEARGS) :test:assemble

srcs:
	$(GRADLE) $(GRADLEARGS) dist

distclean clean:
	$(GRADLE) $(GRADLEARGS) clean
