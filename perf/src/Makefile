# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

include $(top_srcdir)/config/Make.rules

SUBDIRS		= 

ifdef ICE_HOME
    SUBDIRS	:= Ice IceStorm  $(SUBDIRS)
endif

ifdef TAO_ROOT
    export ACE_ROOT := $(TAO_ROOT))
    SUBDIRS 	:= TAO CosEvent $(SUBDIRS)
endif

ifdef ICEE_HOME
    SUBDIRS 	:= IceE $(SUBDIRS)
endif

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making $@ in $$subdir"; \
	    ( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	done

ifdef TAO_ROOT
clean::
	( cd CosEvent && make realclean )
endif
