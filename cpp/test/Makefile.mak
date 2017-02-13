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

SUBDIRS		= Ice

!elseif "$(CPP_COMPILER)" == "VC100"
SUBDIRS		= IceUtil \
		  Slice \
		  Ice \
		  IceSSL \
		  IceDiscovery
!else
SUBDIRS		= IceUtil \
		  Slice \
		  Ice \
		  IceSSL \
		  Glacier2 \
		  Freeze \
		  IceStorm \
		  FreezeScript \
		  IceGrid \
		  IceBox \
		  IceDiscovery

!endif

$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @echo "making $@ in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1
