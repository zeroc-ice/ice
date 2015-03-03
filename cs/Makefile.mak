# **********************************************************************
#
# Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= .

!include $(top_srcdir)/config/Make.rules.mak.cs

SUBDIRS		= src test demo

install:: install-common
	@if not exist "$(install_bindir)" \
	    @echo "Creating $(install_bindir)..." && \
	    mkdir "$(install_bindir)"

$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @echo "making $@ in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1

test::
	@python $(top_srcdir)/allTests.py
