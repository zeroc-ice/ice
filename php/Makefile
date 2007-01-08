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

SUBDIRS		= src

INSTALL_SUBDIRS	= $(install_libdir) $(install_slicedir)

install::
	@if test ! -d $(prefix) ; \
	then \
	    echo "Creating $(prefix)..." ; \
	    $(call mkdir,$(prefix)) ; \
	fi
	@for subdir in $(INSTALL_SUBDIRS); \
	do \
	    if test ! -d $$subdir ; \
	    then \
		echo "Creating $$subdir..." ; \
		mkdir -p $$subdir ; \
		chmod a+rx $$subdir ; \
	    fi ; \
	done

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
