# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= .

include $(top_srcdir)/config/Make.rules

SUBDIRS		= src include test demo slice

install::
	@if test ! -d $(prefix) ; \
	then \
	    echo "Creating $(prefix)..." ; \
	    $(call mkdir,$(prefix)) ; \
	fi

	@if test ! -d $(install_bindir) ; \
	then \
	    echo "Creating $(install_bindir)..." ; \
	    $(call mkdir,$(install_bindir)) ; \
	fi

	@if test ! -d $(install_libdir) ; \
	then \
	    echo "Creating $(install_libdir)..." ; \
	    $(call mkdir,$(install_libdir)) ; \
	fi

	@if test ! -d $(install_includedir) ; \
	then \
	    echo "Creating $(install_includedir)..." ; \
	    $(call mkdir,$(install_includedir)) ; \
	fi

	@if test ! -d $(install_slicedir) ; \
	then \
	    echo "Creating $(install_slicedir)..." ; \
	    $(call mkdir,$(install_slicedir)) ; \
	fi

	@if test ! -d $(install_docdir) ; \
	then \
	    echo "Creating $(install_docdir)..." ; \
	    $(call mkdir,$(install_docdir)) ; \
	fi

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making $@ in $$subdir"; \
	    ( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	done

doc::
	@( cd doc && $(MAKE) ) || exit 1

install::
	@( cd doc && $(MAKE) install ) || exit 1
	$(call installdata,ICE_LICENSE,$(prefix))
	$(call installdata,LICENSE,$(prefix))

clean::
	@( cd doc && $(MAKE) clean ) || exit 1

test::
	@python $(top_srcdir)/allTests.py
