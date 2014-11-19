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
keys="$(SILVERLIGHT_ASSEMBLIES_KEY)" "$(SILVERLIGHT_ASSEMBLIES_KEY)"
registerpath=$(SRC_FULL_PATH)\Assemblies\sl
installpath=$(prefix)\Assemblies\sl
!elseif "$(COMPACT)" == "yes"
keys="$(POCKETPC_ASSEMBLIES_KEY)" "$(SMARTPHONE_ASSEMBLIES_KEY)" "$(WINDOWSCE_ASSEMBLIES_KEY)"
registerpath=$(SRC_FULL_PATH)\Assemblies\cf
installpath=$(prefix)\Assemblies\cf
!else
keys="$(DOTNET_ASSEMBLIES_KEY)"
registerpath=$(SRC_FULL_PATH)\Assemblies
installpath=$(prefix)\Assemblies
!endif

install::
	@for %i in ( $(keys) ) do \
		@echo Adding key %i in Windows registry && \
		@reg ADD %i /ve /d "$(installpath)" /f || \
		echo Could not add registry keyword %i && exit 1

register-assemblies::
	@for %i in ( $(keys) ) do \
		@echo Adding key %i in Windows registry && \
		@reg ADD %i /ve /d "$(registerpath)" /f || \
		echo Could not add registry keyword %i && exit 1
