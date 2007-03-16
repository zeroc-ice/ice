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
	@if test ! -d $(install_includedir)/IceE ; \
	then \
	    echo "Creating $(install_includedir)/IceE..." ; \
	    $(call mkdir,$(install_includedir)/IceE) ; \
	fi

	@for i in *.h ; \
	do \
	    echo "Installing $$i" ; \
	    $(INSTALL_DATA) $$i $(install_includedir)/IceE/$$i ; \
	    chmod a+r $(install_includedir)/IceE/$$i ; \
	done
