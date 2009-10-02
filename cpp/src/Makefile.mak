# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

!include $(top_srcdir)/config/Make.rules.mak

SUBDIRS		= IceUtil \
		  Slice \
		  slice2cpp \
		  slice2freeze

!if "$(BCPLUSPLUS)" != "yes" && "$(CPP_COMPILER)" != "VC60"
SUBDIRS		= $(SUBDIRS) \
		  slice2cs \
		  slice2freezej \
		  slice2java \
		  slice2php \
		  slice2py \
		  slice2rb \
		  slice2html
!endif

SUBDIRS		= $(SUBDIRS) \
		  Ice \
		  IceSSL \
		  IceXML \
		  Freeze \
		  FreezeScript \
		  IcePatch2Lib \
		  Glacier2Lib \
		  IceStormLib \
		  IceGridLib \
		  ca

!if "$(BCPLUSPLUS)" != "yes" && "$(CPP_COMPILER)" != "VC60"
SUBDIRS		= $(SUBDIRS) \
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
