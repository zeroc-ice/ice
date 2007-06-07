# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= .

!include $(top_srcdir)/config/Make.rules.mak

SUBDIRS		= config src include test demo slice doc

INSTALL_SUBDIRS	= $(install_bindir) $(install_libdir) $(install_includedir) $(install_slicedir) $(install_docdir)

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
	copy ICE_LICENSE $(prefix)
	copy LICENSE $(prefix)

test::
	@python $(top_srcdir)/allTests.py
