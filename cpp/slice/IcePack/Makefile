# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

include $(top_srcdir)/config/Make.rules

install::
	@if test ! -d $(install_slicedir)/IcePack ; \
	then \
	    echo "Creating $(install_slicedir)/IcePack..." ; \
	    mkdir $(install_slicedir)/IcePack ; \
	    chmod a+rx $(install_slicedir)/IcePack ; \
	fi

	@for i in *.ice ; \
	do \
	    echo "Installing $$i" ; \
	    $(INSTALL_DATA) $$i $(install_slicedir)/IcePack/$$i ; \
	done
