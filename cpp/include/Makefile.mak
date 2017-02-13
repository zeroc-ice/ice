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
SUBDIRS		= IceUtil \
		  Ice \
		  IceSSL \
		  Glacier2 \
		  IceStorm \
		  IceGrid \
!elseif "$(CPP_COMPILER)" == "VC100"
SUBDIRS		= IceUtil \
		  Slice \
		  Ice \
		  IceSSL
!else
SUBDIRS		= Glacier2 \
		  Ice \
		  IceSSL \
		  IceGrid \
		  IceStorm \
		  IceUtil \
		  Freeze \
		  IceBox \
		  IcePatch2 \
		  IceSSL \
		  Slice
!endif

$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @echo "making $@ in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1
