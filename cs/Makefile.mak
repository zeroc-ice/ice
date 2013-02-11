# **********************************************************************
#
# Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
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
register-assembleis::
	@echo Adding key "$(DOTNET_ASSEMBLEIS_KEY)" in Windows registry
	@reg ADD "$(SILVERLIGH_ASSEMBLEIS_KEY)" /ve /d "$(SRC_FULL_PATH)\Assemblies\sl" /f || \
	echo "Could not add registry keyword $(SILVERLIGH_ASSEMBLEIS_KEY)" && exit 1
!elseif "$(COMPACT)" == "yes"
register-assembleis::
	@echo Adding key "$(POCKETPC_ASSEMBLEIS_KEY)" in Windows registry
	@reg ADD "$(POCKETPC_ASSEMBLEIS_KEY)" /ve /d "$(SRC_FULL_PATH)\Assemblies\cf" /f || \
	echo "Could not add registry keyword $(POCKETPC_ASSEMBLEIS_KEY)" && exit 1

	@echo Adding key "$(SMARTPHONE_ASSEMBLEIS_KEY)" in Windows registry
	@reg ADD "$(SMARTPHONE_ASSEMBLEIS_KEY)" /ve /d "$(SRC_FULL_PATH)\Assemblies\cf" /f || \
	echo "Could not add registry keyword $(SMARTPHONE_ASSEMBLEIS_KEY)" && exit 1

	@echo Adding key "$(WINDOWSCE_ASSEMBLEIS_KEY)" in Windows registry
	@reg ADD "$(WINDOWSCE_ASSEMBLEIS_KEY)" /ve /d "$(SRC_FULL_PATH)\Assemblies\cf" /f || \
	echo "Could not add registry keyword $(WINDOWSCE_ASSEMBLEIS_KEY)" && exit 1
!else
register-assembleis::
	@echo Adding key "$(DOTNET_ASSEMBLEIS_KEY)" in Windows registry
	@reg ADD "$(DOTNET_ASSEMBLEIS_KEY)" /ve /d "$(SRC_FULL_PATH)\Assemblies" /f || \
	echo "Could not add registry keyword $(DOTNET_ASSEMBLEIS_KEY)" && exit 1
!endif