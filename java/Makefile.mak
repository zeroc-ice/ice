# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

!if "$(GRADLE)" == ""
GRADLE = gradlew.bat
!endif

!if "$(JARSIGNER_KEYSTORE)" != ""
BUILD_FLAGS = -Dorg.gradle.project.keystore="$(JARSIGNER_KEYSTORE)"
!endif

!if "$(JARSIGNER_KEYSTORE_PASSWORD)" != ""
BUILD_FLAGS = -Dorg.gradle.project.keystore_password="$(JARSIGNER_KEYSTORE_PASSWORD)"
!endif

all:
	$(GRADLE) $(BUILD_FLAGS) build

dist:
	$(GRADLE) $(BUILD_FLAGS) :Ice:assemble :Freeze:assemble :Glacier2:assemble :IceGrid:assemble :ant:assemble \
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
