# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

!include $(top_srcdir)/config/Make.rules.mak

!if "$(WINRT)" == "yes"
SUBDIRS		= IceUtil\winrt \
		  Ice\winrt \
		  Glacier2Lib\winrt \
		  IceStormLib\winrt \
		  IceGridLib\winrt
!else
SUBDIRS		= IceUtil \
		  Slice \
		  slice2cpp \
		  slice2freeze \
		  slice2cs \
		  slice2freezej \
		  slice2java \
		  slice2py \
		  slice2html \
		  slice2php \
		  slice2rb \
		  slice2js \
		  Ice \
		  IceSSL \
		  IceDiscovery \
		  ca \
		  IceXML \
		  Freeze \
		  FreezeScript \
		  IcePatch2Lib \
		  Glacier2Lib \
		  IceStormLib \
		  IceGridLib \
		  IceBox \
		  IcePatch2 \
		  Glacier2 \
		  IceStorm \
		  IceGrid \
		  iceserviceinstall
!endif

$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @if exist %i \
	        @echo "making $@ in %i" && \
	        cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1
