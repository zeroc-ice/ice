# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

include $(top_srcdir)/config/Make.rules

SUBDIRS		= IceUtil \
		  icecpp \
		  Slice \
		  slice2cpp \
		  slice2cs \
		  slice2freeze \
		  slice2freezej \
		  slice2docbook \
		  slice2java \
		  Ice \
		  IceXML \
		  IceSSL \
		  Freeze \
		  FreezeScript \
		  IceBox \
		  IceStorm \
		  IcePack \
		  Glacier \
		  IcePatch

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making $@ in $$subdir"; \
	    ( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	done
