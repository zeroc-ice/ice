# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

include $(top_srcdir)/config/Make.rules

ICE_SUBDIRS	= latency \
		  throughput

TAO_SUBDIRS	= 

SUBDIRS		=
ifdef ICE_HOME
    SUBDIRS := $(ICE_SUBDIRS) $(SUBDIRS)
endif
ifdef TAO_ROOT
    SUBDIRS := $(TAO_SUBDIRS) $(SUBDIRS)
endif

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making $@ in $$subdir"; \
	    ( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	done
