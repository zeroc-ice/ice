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

SUBDIRS		= python modules

install::
	@if test ! -d $(prefix) ; \
	then \
	    echo "Creating $(prefix)..." ; \
	    mkdir $(prefix) ; \
	    chmod a+rx $(prefix) ; \
	fi

	@if test ! -d $(install_libdir) ; \
	then \
	    echo "Creating $(install_libdir)..." ; \
	    mkdir -p $(install_libdir) ; \
	    chmod a+rx $(install_libdir) ; \
	fi

	@if test ! -d $(install_slicedir) ; \
	then \
	    echo "Creating $(install_slicedir)..." ; \
	    mkdir $(install_slicedir) ; \
	    chmod a+rx $(install_slicedir) ; \
	fi

	@if test ! -d $(install_pythondir) ; \
	then \
	    echo "Creating $(install_pythondir)..." ; \
	    mkdir $(install_pythondir) ; \
	    chmod a+rx $(install_pythondir) ; \
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
		    $(INSTALL_DATA) $$f $(install_slicedir)/$$i/$$f ; \
		done ; \
		cd .. ; \
	    done \
	fi

install::
	$(INSTALL) ICE_LICENSE $(prefix)
	$(INSTALL) LICENSE $(prefix)

test::
	@python $(top_srcdir)/allTests.py
