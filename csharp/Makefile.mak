# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= .

!include $(top_srcdir)/config/Make.rules.mak.cs

SUBDIRS		= src test

INSTALL_SUBDIRS	= "$(install_bindir)" "$(install_assembliesdir)" "$(install_configdir)"

install:: install-common
	@for %i in ( $(INSTALL_SUBDIRS) ) do \
	    @if not exist %i \
		@echo "Creating %i..." && \
		$(MKDIR) %i

$(EVERYTHING_EXCEPT_INSTALL)::
	@for %i in ( $(SUBDIRS) ) do \
	    @echo "making $@ in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1

test::
	@python $(top_srcdir)/allTests.py



install::
	@for %i in ( src ) do \
	    @echo "making $@ in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1
