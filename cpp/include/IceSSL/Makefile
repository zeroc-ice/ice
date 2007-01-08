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
	@if test ! -d $(install_includedir)/IceSSL ; \
	then \
	    echo "Creating $(install_includedir)/IceSSL..." ; \
	    $(call mkdir,$(install_includedir)/IceSSL) ; \
	fi

	@for i in *.h ; \
	do \
	    echo "Installing $$i" ; \
	    $(INSTALL_DATA) $$i $(install_includedir)/IceSSL/$$i ; \
	    chmod a+r $(install_includedir)/IceSSL/$$i ; \
	done
