# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

include $(top_srcdir)/config/Make.rules.js

SUBDIRS		= binding \
		defaultValue \
		enums \
		exceptions \
		exceptionsBidir \
		facets \
		facetsBidir \
		inheritance \
		inheritanceBidir \
		location \
		objects \
		operations \
		operationsBidir \
		optional \
		optionalBidir \
		promise \
		properties \
		proxy \
		retry \
		slicing \
		timeout \
		acm

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making $@ in $$subdir"; \
	    ( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	done

