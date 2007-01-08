# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= .

include $(top_srcdir)/config/Make.rules.cs

SUBDIRS		= config src test demo

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

	@if test ! -d $(install_slicedir) ; \
	then \
	    echo "Creating $(install_slicedir)..." ; \
	    $(call mkdir,$(install_slicedir)) ; \
	fi

	@if test -d slice ; \
	then \
	    cd slice ; \
	    for subdir in * ; \
	    do \
		echo "Copying slice/$$subdir to $(install_slicedir)..." ; \
		cp -fpr $$subdir $(install_slicedir) ; \
	    done ; \
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

test::
	@python $(top_srcdir)/allTests.py
