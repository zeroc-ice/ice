# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
		  slice2cs \
		  slice2vb \
		  slice2freeze \
		  slice2freezej \
		  slice2docbook \
		  slice2java \
		  slice2py \
		  slice2cppe \
		  slice2javae \
		  Ice \
		  IceXML \
		  IceSSL \
		  IceBox \
		  IcePatch2 \
		  Glacier2 \
		  Freeze \
		  FreezeScript \
		  IceStorm \
		  IceGrid \
		  ca

$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @echo "making $@ in %i" & \
	    cmd /c "cd %i & $(MAKE) -f Makefile.mak $@"
