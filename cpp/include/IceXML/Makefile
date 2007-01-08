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
	@if test ! -d $(install_includedir)/IceXML ; \
	then \
	    echo "Creating $(install_includedir)/IceXML..." ; \
	    $(call mkdir,$(install_includedir)/IceXML) ; \
	fi

	@for i in *.h ; \
	do \
	    echo "Installing $$i" ; \
	    $(INSTALL_DATA) $$i $(install_includedir)/IceXML/$$i ; \
	    chmod a+r $(install_includedir)/IceXML/$$i ; \
	done
