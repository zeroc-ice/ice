# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= .

ifeq ($(PYTHON),)
    PYTHON = python
endif

include $(top_srcdir)/config/Make.rules

SUBDIRS		= modules python

INSTALL_SUBDIRS = $(install_pythondir) $(install_libdir)

install:: install-common
	@for subdir in $(INSTALL_SUBDIRS); \
	do \
	    if test ! -d $(DESTDIR)$$subdir ; \
	    then \
		echo "Creating $(DESTDIR)$$subdir..." ; \
		mkdir -p $(DESTDIR)$$subdir ; \
		chmod a+rx $(DESTDIR)$$subdir ; \
	    fi ; \
	done

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making $@ in $$subdir"; \
	    ( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	done

test::
	@$(PYTHON) $(top_srcdir)/allTests.py
