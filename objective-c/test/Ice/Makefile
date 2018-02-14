# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

include $(top_srcdir)/config/Make.rules

SUBDIRS		= proxy \
		  acm \
		  admin \
		  ami \
		  operations \
		  exceptions \
		  inheritance \
		  facets \
		  objects \
		  optional \
		  interceptor \
		  invoke \
		  dispatcher \
		  defaultServant \
		  defaultValue \
		  enums \
		  faultTolerance \
		  location \
		  adapterDeactivation \
		  slicing \
		  binding \
		  hold \
		  retry \
		  stream \
		  timeout \
		  hash \
		  info \
		  metrics \
		  services \
		  servantLocator

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making $@ in $$subdir"; \
	    ( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	done
