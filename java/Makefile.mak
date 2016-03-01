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

all:
	$(GRADLE) build

dist:
	$(GRADLE) :Ice:assemble :Freeze:assemble :Glacier2:assemble :IceGrid:assemble :ant:assemble \
		:IceBox:assemble :IceDiscovery:assemble :IcePatch2:assemble :IceStorm:assemble :IceGridGUI:assemble

clean:
	$(GRADLE) clean

install::
!if "$(PREFIX)" != ""
	$(GRADLE) -Dorg.gradle.project.prefix="$(PREFIX)" install
!else
	$(GRADLE) install
!endif

test:
	@python .\allTests.py
