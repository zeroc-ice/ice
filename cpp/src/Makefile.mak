# **********************************************************************
#
# Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

!include $(top_srcdir)/config/Make.rules.mak

SUBDIRS		= IceUtil \
		  Slice \
		  slice2cpp


!if "$(CPP_COMPILER)" != "VC60"
SUBDIRS		= $(SUBDIRS) \
		  slice2freeze

!if "$(BCPLUSPLUS)" != "yes"
SUBDIRS		= $(SUBDIRS) \
		  slice2cs \
		  slice2freezej \
		  slice2java \
		  slice2py \
		  slice2html
!endif

!endif

!if "$(BCPLUSPLUS)" != "yes"
SUBDIRS		= $(SUBDIRS) \
		  slice2php \
		  slice2rb
!endif

SUBDIRS		= $(SUBDIRS) \
		  Ice \
		  IceSSL \
		  ca

!if "$(CPP_COMPILER)" != "VC60"
SUBDIRS		= $(SUBDIRS) \
		  IceXML \
		  Freeze \
		  FreezeScript \
		  IcePatch2Lib \
		  Glacier2Lib \
		  IceStormLib \
		  IceGridLib \

!if "$(BCPLUSPLUS)" != "yes"
SUBDIRS		= $(SUBDIRS) \
		  IceBox \
		  IcePatch2 \
		  Glacier2 \
		  IceDB \
		  IceStorm \
		  IceGrid \
                  iceserviceinstall
!endif

!endif

$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @if exist %i \
	        @echo "making $@ in %i" && \
	        cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1
