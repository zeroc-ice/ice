# **********************************************************************
#
# Copyright (c) 2001
# ZeroC, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

top_srcdir	= ../..

include $(top_srcdir)/config/Make.rules

SUBDIRS		= operations \
		  exceptions \
		  inheritance \
		  facets \
		  objects \
		  faultTolerance \
		  locationForward \
		  location \
		  adapterDeactivation

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making $@ in $$subdir"; \
	    ( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	done
