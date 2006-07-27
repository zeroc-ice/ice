# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= .

include $(top_srcdir)/config/Make.rules

SUBDIRS		= src ruby

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

	@if test ! -d $(install_slicedir) ; \
	then \
	    echo "Creating $(install_slicedir)..." ; \
	    $(call mkdir,$(install_slicedir)) ; \
	fi

	@if test ! -d $(install_rubydir) ; \
	then \
	    echo "Creating $(install_rubydir)..." ; \
	    $(call mkdir,$(install_rubydir)) ; \
	fi

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making $@ in $$subdir"; \
	    ( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	done

install::
	@if test -d slice ; \
	then \
	    cd slice ; \
	    for i in * ; \
	    do \
		if test ! -d $(install_slicedir)/$$i ; \
		then \
		    echo "Creating $(install_slicedir)/$$i..." ; \
		    mkdir $(install_slicedir)/$$i ; \
		    chmod a+rx $(install_slicedir)/$$i ; \
		fi ; \
		cd $$i ; \
		for f in *.ice ; \
		do \
		    echo "Installing $$i/$$f" ; \
		    $(INSTALL_DATA) $$f $(install_slicedir)/$$i ; \
		    chmod a+r $(install_slicedir)/$$i/$$f ; \
		done ; \
		cd .. ; \
	    done \
	fi

install::
	$(call installdata,ICE_LICENSE,$(prefix))
	$(call installdata,LICENSE,$(prefix))

test::
	@python $(top_srcdir)/allTests.py
