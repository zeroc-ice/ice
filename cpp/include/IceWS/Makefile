# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

include $(top_srcdir)/config/Make.rules

install::
	@if test ! -d $(install_includedir)/IceWS ; \
	then \
	    echo "Creating $(install_includedir)/IceWS..." ; \
	    $(call mkdir,$(install_includedir)/IceWS) ; \
	fi

	@for i in *.h ; \
	do \
	    echo "Installing $$i" ; \
	    $(INSTALL_DATA) $$i $(install_includedir)/IceWS/$$i ; \
	    chmod a+r $(install_includedir)/IceWS/$$i ; \
	done
