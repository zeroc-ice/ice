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
	@if test ! -d $(install_slicedir)/IceSSL ; \
	then \
	    echo "Creating $(install_slicedir)/IceSSL..." ; \
	    mkdir $(install_slicedir)/IceSSL ; \
	    chmod a+rx $(install_slicedir)/IceSSL ; \
	fi

	@for i in *.ice ; \
	do \
	    echo "Installing $$i" ; \
	    $(INSTALL_DATA) $$i $(install_slicedir)/IceSSL/$$i ; \
	done
