# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

!include $(top_srcdir)/config/Make.rules.mak

!if "$(WINRT)" == "yes"

SUBDIRS		= Common \
		  Ice

!elseif "$(CPP11_MAPPING)" == "yes"
SUBDIRS		= IceUtil \
		  Slice \
		  Ice
!elseif "$(CPP_COMPILER)" == "VC100"
SUBDIRS		= IceUtil \
		  Slice \
		  Common \
		  Ice \
		  IceSSL \
		  IceDiscovery
!else
SUBDIRS		= IceUtil \
		  Slice \
		  Common \
		  Ice \
		  IceSSL \
		  Glacier2 \
		  IceStorm \
		  IceGrid \
		  IceBox \
		  IceDiscovery

!endif

$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @echo "making $@ in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1
