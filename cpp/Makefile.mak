# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= .

!include $(top_srcdir)/config/Make.rules.mak

!if "$(WINRT)" != "yes"
SUBDIRS		= config src include test demo

INSTALL_SUBDIRS	= "$(install_bindir)" "$(install_libdir)" "$(install_includedir)" "$(install_configdir)"

install:: install-common
	@for %i in ( $(INSTALL_SUBDIRS) ) do \
	    @if not exist %i \
		@echo "Creating %i..." && \
		mkdir %i

test::
	@python $(top_srcdir)/allTests.py

!else
SUBDIRS		= src include test

SDK_FULL_PATH	= $(MAKEDIR)\$(SDK_BASE_PATH)
SDK_FULL_PATH	= $(SDK_FULL_PATH:\.\=\)
!if "$(PROCESSOR_ARCHITECTURE)" == "AMD64"
SDK_KEY 	= HKLM\SOFTWARE\Wow6432Node\Microsoft\Microsoft SDKs\Windows\v$(TARGET_PLATFORM_VERSION)\ExtensionSDKs\$(SDK_NAME)\$(SDK_VERSION)
!else
SDK_KEY 	= HKLM\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v$(TARGET_PLATFORM_VERSION)\ExtensionSDKs\$(SDK_NAME)\$(SDK_VERSION)
!endif

INSTALL_SUBDIRS	= $(prefix)\SDKs

register-sdk:
	@echo Register SDK "$(SDK_NAME)" in Windows registry "$(SDK_KEY)"
	@reg ADD "$(SDK_KEY)" /ve /d "$(SDK_FULL_PATH)" /f || \
	@echo "Could not add registry keyword $(SDK_KEY)"

unregister-sdk:
	@echo Unregister SDK "$(SDK_NAME)" delete Windows registry key "$(SDK_KEY)"
	@reg DELETE "$(SDK_KEY)" /f || \
	@echo "Registry Keyword $(SDK_KEY) not exists"

install:: install-common
	@for %i in ( $(INSTALL_SUBDIRS) ) do \
	    @if not exist %i \
		@echo "Creating %i..." && \
		mkdir %i
!endif

$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @echo "making $@ in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1
	    
	    
!if "$(WINRT)" == "yes"
install::
	xcopy /s /y "$(top_srcdir)\SDKs" "$(prefix)\SDKs"
	@echo Register SDK "$(SDK_NAME)" in Windows registry "$(SDK_KEY)"
	@reg ADD "$(SDK_KEY)" /ve /d "$(prefix)\SDKs\$(SDK_NAME)\$(SDK_VERSION)" /f || \
	echo "Could not add registry keyword $(SDK_KEY)" && exit 1
!endif
