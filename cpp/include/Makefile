# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

include $(top_srcdir)/config/Make.rules

SUBDIRS		= Ice \
		  IceSSL \
		  IceUtil \
		  Slice

ifeq ($(findstring MINGW,$(UNAME)),)
SUBDIRS		:= $(SUBDIRS) \
		Glacier2 \
		IceBox \
		IceGrid \
		IcePatch2 \
		IceStorm
endif

ifeq ($(BUILD_ICE_BT),yes)
SUBDIRS		:= $(SUBDIRS) \
		IceBT
endif

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making $@ in $$subdir"; \
	    ( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	done
