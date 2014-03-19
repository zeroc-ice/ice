# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= .

!include $(top_srcdir)/config/Make.rules.mak.cs

SUBDIRS		= src

!if "$(SILVERLIGHT)" != "yes"
SUBDIRS		= $(SUBDIRS) test demo config
!endif

INSTALL_SUBDIRS	= "$(install_bindir)" "$(install_assembliesdir)" "$(install_configdir)"

install:: install-common
	@for %i in ( $(INSTALL_SUBDIRS) ) do \
	    @if not exist %i \
		@echo "Creating %i..." && \
		mkdir %i

$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @echo "making $@ in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1

test::
	@python $(top_srcdir)/allTests.py


SRC_FULL_PATH	= $(MAKEDIR:\.\=\)

!if "$(SILVERLIGHT)" == "yes"
register-assemblies::
	@echo Adding key "$(SILVERLIGHT_ASSEMBLIES_KEY)" in Windows registry
	@reg ADD "$(SILVERLIGHT_ASSEMBLIES_KEY)" /ve /d "$(SRC_FULL_PATH)\Assemblies\sl" /f || \
	echo "Could not add registry keyword $(SILVERLIGHT_ASSEMBLIES_KEY)" && exit 1
!elseif "$(COMPACT)" == "yes"
register-assemblies::
	@echo Adding key "$(POCKETPC_ASSEMBLIES_KEY)" in Windows registry
	@reg ADD "$(POCKETPC_ASSEMBLIES_KEY)" /ve /d "$(SRC_FULL_PATH)\Assemblies\cf" /f || \
	echo "Could not add registry keyword $(POCKETPC_ASSEMBLIES_KEY)" && exit 1

	@echo Adding key "$(SMARTPHONE_ASSEMBLIES_KEY)" in Windows registry
	@reg ADD "$(SMARTPHONE_ASSEMBLIES_KEY)" /ve /d "$(SRC_FULL_PATH)\Assemblies\cf" /f || \
	echo "Could not add registry keyword $(SMARTPHONE_ASSEMBLIES_KEY)" && exit 1

	@echo Adding key "$(WINDOWSCE_ASSEMBLIES_KEY)" in Windows registry
	@reg ADD "$(WINDOWSCE_ASSEMBLIES_KEY)" /ve /d "$(SRC_FULL_PATH)\Assemblies\cf" /f || \
	echo "Could not add registry keyword $(WINDOWSCE_ASSEMBLIES_KEY)" && exit 1
!else
register-assemblies::
	@echo Adding key "$(DOTNET_ASSEMBLIES_KEY)" in Windows registry
	@reg ADD "$(DOTNET_ASSEMBLIES_KEY)" /ve /d "$(SRC_FULL_PATH)\Assemblies" /f || \
	echo "Could not add registry keyword $(DOTNET_ASSEMBLIES_KEY)" && exit 1
!endif