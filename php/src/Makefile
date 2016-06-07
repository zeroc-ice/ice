# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

include $(top_srcdir)/config/Make.rules.php


ifeq ($(shell [ $$($(PHP_CONFIG) --vernum) -lt 70000 ] && echo true),true)
SUBDIRS		= php5
else
SUBDIRS		= php7
endif

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    if test -d $$subdir ; \
	    then \
		echo "making $@ in $$subdir"; \
		( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	    fi; \
	done
