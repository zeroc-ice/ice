# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= .

!include $(top_srcdir)/config/Make.rules.mak.cs

SUBDIRS		= config generate src test demo

INSTALL_SUBDIRS	= $(install_bindir) $(install_slicedir)

install:: createdir

createdir::
	@if not exist $(prefix) \
	    @echo "Creating $(prefix)..." && \
	    mkdir $(prefix)

	@for %i in ( $(INSTALL_SUBDIRS) ) do \
	    @if not exist %i \
		@echo "Creating %i..." && \
		mkdir %i

$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @echo "making $@ in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1

install::
	@if exist slice \
	    @echo "making $@ in slice" && \
	    cmd /c "cd slice && $(MAKE) -nologo -f Makefile.mak $@" || exit 1

install::
	copy ICE_LICENSE $(prefix)
	copy LICENSE $(prefix)

test::
	@python $(top_srcdir)/allTests.py
