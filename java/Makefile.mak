# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

all:
	gradlew.bat assemble

dist:
	gradlew.bat :Ice:assemble :Freeze:assemble :Glacier2:assemble :IceGrid:assemble :ant:assemble \
		:IceBox:assemble :IceDiscovery:assemble :IcePatch2:assemble :IceStorm:assemble :IceGridGUI:assemble

clean:
	gradlew.bat clean

install::
!if "$(prefix)" != ""
	gradlew.bat -Dorg.gradle.project.prefix="$(prefix)" install
!else
	gradlew.bat install
!endif

test:
	@python .\allTests.py
