# **********************************************************************
#
# Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= .

include $(top_srcdir)/config/Make.rules.cs

SUBDIRS		= src test demo

install:: install-common
	@if test ! -d $(install_bindir) ; \
	then \
	    echo "Creating $(install_bindir)..." ; \
	    $(call mkdir,$(install_bindir)) ; \
	fi

ifeq ($(GACINSTALL),yes)
install:: install-common
	@if test ! -d $(install_libdir) ; \
	then \
	    echo "Creating $(install_libdir)..." ; \
	    $(call mkdir,$(install_libdir)) ; \
	fi
	@if test ! -d $(install_pkgconfigdir) ; \
	then \
	    echo "Creating $(install_pkgconfigdir)..." ; \
	    $(call mkdir,$(install_pkgconfigdir)) ; \
	fi

endif



$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making $@ in $$subdir"; \
	    ( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	done

test::
	@python $(top_srcdir)/allTests.py
