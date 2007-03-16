# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

include $(top_srcdir)/config/Make.rules

install::
	@if test ! -d $(install_slicedir)/IceE ; \
	then \
	    echo "Creating $(install_slicedir)/IceE..." ; \
	    $(call mkdir,$(install_slicedir)/IceE) ; \
	fi

	@for i in *.ice ; \
	do \
	    echo "Installing $$i" ; \
	    $(INSTALL_DATA) $$i $(install_slicedir)/IceE/$$i ; \
	    chmod a+r $(install_slicedir)/IceE/$$i ; \
	done
