# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

include $(top_srcdir)/config/Make.rules

install::
	@if test ! -d $(install_slicedir)/IceStorm ; \
	then \
	    echo "Creating $(install_slicedir)/IceStorm..." ; \
	    $(call mkdir,$(install_slicedir)/IceStorm) ; \
	fi

	@for i in *.ice ; \
	do \
	    echo "Installing $$i" ; \
	    $(INSTALL_DATA) $$i $(install_slicedir)/IceStorm/$$i ; \
	    chmod a+r $(install_slicedir)/IceStorm/$$i ; \
	done
