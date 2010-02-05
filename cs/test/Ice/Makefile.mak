# **********************************************************************
#
# Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

!include $(top_srcdir)/config/Make.rules.mak.cs

SUBDIRS         = application \
                  adapterDeactivation \
                  exceptions \
                  facets \
                  faultTolerance \
		  info \
                  inheritance \
		  invoke \
                  hold \
                  location \
                  objects \
                  binding \
                  proxy \
                  operations \
		  ami \
		  dispatcher \
                  slicing \
                  checksum \
                  stream \
                  retry \
                  timeout \
                  servantLocator \
                  interceptor \
                  dictMapping \
                  seqMapping \
                  background \
                  udp \
                  defaultServant \
                  defaultValue \
                  threadPoolPriority

$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @echo "making $@ in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1

test::
	@python $(top_srcdir)/allTests.py
