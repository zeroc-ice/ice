# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

include $(top_srcdir)/config/Make.rules.cs

SUBDIRS		= application \
		  adapterDeactivation \
		  echo \
		  exceptions \
		  facets \
		  faultTolerance \
		  inheritance \
		  invoke \
		  hash \
		  hold \
		  location \
		  objects \
		  binding \
		  proxy \
		  properties \
		  operations \
		  ami \
		  dispatcher \
		  slicing \
		  checksum \
		  stream \
		  retry \
		  timeout \
		  acm \
		  servantLocator \
		  interceptor \
		  dictMapping \
		  seqMapping \
		  background \
		  udp \
		  defaultServant \
		  defaultValue \
		  threadPoolPriority \
		  plugin \
		  admin \
		  optional \
		  metrics \
		  enums \
		  serialize \
		  networkProxy

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making $@ in $$subdir"; \
	    ( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	done

test::
	@python $(top_srcdir)/allTests.py
