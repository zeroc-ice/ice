# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= .

!include $(top_srcdir)/config/Make.rules.mak

INSTALL_SUBDIRS	= "$(install_bindir)" "$(install_libdir)" "$(install_includedir)" "$(install_configdir)"

!if "$(WINRT)" != "yes"

SUBDIRS		= config src include test 

install:: install-common
	@for %i in ( $(INSTALL_SUBDIRS) ) do \
	    @if not exist %i \
	        @echo "Creating %i..." && \
	        $(MKDIR) %i

	@for %i in ( config src include ) do \
	    @echo "making $@ in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1

!if "$(CPP_COMPILER)" == "VC100"
install::
	@if not exist "$(install_bindir)" $(MKDIR) "$(install_bindir)"
	copy "$(BZIP2_HOME)\build\native\bin\$(PLATFORM)\$(CONFIGURATION)\*" "$(install_bindir)"
!else
install::
	@if not exist "$(install_bindir)" $(MKDIR) "$(install_bindir)"
	copy "$(BZIP2_HOME)\build\native\bin\$(PLATFORM)\$(CONFIGURATION)\*" "$(install_bindir)"
	copy "$(DB_HOME)\build\native\bin\$(PLATFORM)\$(CONFIGURATION)\*" "$(install_bindir)"
	copy "$(EXPAT_HOME)\build\native\bin\$(PLATFORM)\$(CONFIGURATION)\*" "$(install_bindir)"

!if "$(CONFIGURATION)" == "Debug"
#
# If that is a Debug build install also the DB tools from the release bin directory
install::
	@if not exist "$(install_bindir)" $(MKDIR) "$(install_bindir)"
	copy "$(DB_HOME)\build\native\bin\$(PLATFORM)\Release\*" "$(install_bindir)"
!endif
!endif

test::
	@python $(top_srcdir)/allTests.py

!else

SUBDIRS		= src include test

INSTALL_SUBDIRS	= $(INSTALL_SUBDIRS) $(prefix)\SDKs

install:: install-common
	@for %i in ( $(INSTALL_SUBDIRS) ) do \
		@if not exist %i \
		@echo "Creating %i..." && \
		$(MKDIR) %i
!endif

$(EVERYTHING_EXCEPT_INSTALL)::
	@for %i in ( $(SUBDIRS) ) do \
		@echo "making $@ in %i" && \
		cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1

!if "$(WINRT)" == "yes"

install::
	@for %i in ( src include ) do \
		@echo "making $@ in %i" && \
		cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1

install::
	xcopy /s /y "$(top_srcdir)\SDKs" "$(prefix)\SDKs"

!endif
