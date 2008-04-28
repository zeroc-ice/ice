# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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
		  slice2cppe \
		  slice2cs \
		  slice2freeze \
		  slice2freezej \
		  slice2docbook \
		  slice2java \
		  slice2javae \
		  slice2py \
		  slice2rb \
		  slice2sl \
		  slice2html \
		  Ice \
		  IceXML \
		  IceSSL \
		  IceBox \
		  IcePatch2 \
		  Glacier2 \
		  Freeze \
		  FreezeScript \
		  IceGridLib \
		  IceStorm \
		  IceGrid \
                  iceserviceinstall \
		  ca

$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @if exist %i \
	        @echo "making $@ in %i" && \
	        cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1
