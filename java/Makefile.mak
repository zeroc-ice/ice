# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

!if "$(GRADLE)" == ""
GRADLE = gradlew.bat
!endif

!if "$(GRADLE)" == ""
GRADLE = ./gradlew
!endif

!if "$(PREFIX)" != ""
GRADLEOPTS = $(GRADLEOPTS) -Pprefix=$(PREFIX)
!endif

all:
	$(GRADLE) $(GRADLEOPTS) build

dist:
	$(GRADLE) $(GRADLEOPTS) dist

clean:
	$(GRADLE) $(GRADLEOPTS) clean

tests:
        $(GRADLE) $(GRADLEOPTS) :test:assemble

install::
	$(GRADLE) $(GRADLEOPTS) install

test:
	@python .\allTests.py
