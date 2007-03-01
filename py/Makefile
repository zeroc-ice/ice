# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= .

include $(top_srcdir)/config/Make.rules

SUBDIRS		= python modules

install::
	@if test ! -d $(prefix) ; \
	then \
	    echo "Creating $(prefix)..." ; \
	    $(call mkdir,$(prefix)) ; \
	fi

	@if test ! -d $(install_libdir) ; \
	then \
	    echo "Creating $(install_libdir)..." ; \
	    $(call mkdir,$(install_libdir)) ; \
	fi

	@if test ! -d $(install_pythondir) ; \
	then \
	    echo "Creating $(install_pythondir)..." ; \
	    $(call mkdir,$(install_pythondir)) ; \
	fi

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making $@ in $$subdir"; \
	    ( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	done

install::
	$(call installdata,ICE_LICENSE,$(prefix))
	$(call installdata,LICENSE,$(prefix))

test::
	@python $(top_srcdir)/allTests.py
