# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

!include $(top_srcdir)/config/Make.rules.mak

!if "$(WINRT)" == "yes"
SUBDIRS		= IceUtil Slice slice2cpp
SUBDIRS_WINRT	= IceUtil\winrt \
		  Ice\winrt \
		  Glacier2Lib\winrt \
		  IceStormLib\winrt \
		  IceGridLib\winrt

!elseif "$(CPP_COMPILER)" == "VC100"
SUBDIRS		= IceUtil \
		  Slice \
		  slice2cpp \
		  Ice \
		  IceSSL \
		  IceDiscovery \
		  IceLocatorDiscovery
!else
SUBDIRS		= IceUtil \
		  Slice \
		  slice2cpp \
		  slice2freeze \
		  slice2cs \
		  slice2freezej \
		  slice2java \
		  slice2html \
		  slice2php \
		  slice2js \
		  Ice \
		  IceSSL \
		  IceDiscovery \
		  IceLocatorDiscovery \
		  IceXML \
		  Freeze \
		  FreezeScript \
		  IcePatch2Lib \
		  Glacier2Lib \
		  IceStormLib \
		  IceGridLib \
		  IceBox \
		  IcePatch2 \
		  Glacier2CryptPermissionsVerifier \
		  Glacier2 \
		  IceStorm \
		  IceGrid \
		  iceserviceinstall
!endif

!if "$(WINRT)" == "yes"
$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @if exist %i \
	        @echo "making $@ in %i" && \
	        cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak WINRT=no $@" || exit 1
	@for %i in ( $(SUBDIRS_WINRT) ) do \
	    @if exist %i \
	        @echo "making $@ in %i" && \
	        cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1
!else
$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @if exist %i \
	        @echo "making $@ in %i" && \
	        cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1
!endif