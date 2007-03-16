# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= .

include $(top_srcdir)/config/Make.rules

SUBDIRS		= src slice include test demo

install::
	@if test ! -d $(prefix) ; \
	then \
	    echo "Creating $(prefix)..." ; \
	    mkdir $(prefix) ; \
	    chmod a+rx $(prefix) ; \
	fi

	@if test ! -d $(install_bindir) ; \
	then \
	    echo "Creating $(install_bindir)..." ; \
	    mkdir -p $(install_bindir) ; \
	    chmod a+rx $(install_bindir) ; \
	fi

	@if test ! -d $(install_libdir) ; \
	then \
	    echo "Creating $(install_libdir)..." ; \
	    mkdir -p $(install_libdir) ; \
	    chmod a+rx $(install_libdir) ; \
	fi

	@if test ! -d $(install_includedir) ; \
	then \
	    echo "Creating $(install_includedir)..." ; \
	    mkdir $(install_includedir) ; \
	    chmod a+rx $(install_includedir) ; \
	fi

	@if test ! -d $(install_slicedir) ; \
	then \
	    echo "Creating $(install_slicedir)..." ; \
	    mkdir $(install_slicedir) ; \
	    chmod a+rx $(install_slicedir) ; \
	fi

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making $@ in $$subdir"; \
	    ( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	done

install::
	$(call installdata,ICEE_LICENSE,$(prefix))
	$(call installdata,LICENSE,$(prefix))

test::
	@python $(top_srcdir)/allTests.py
