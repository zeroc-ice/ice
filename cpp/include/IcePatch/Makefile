# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

include $(top_srcdir)/config/Make.rules

install::
	@if test ! -d $(install_includedir)/IcePatch ; \
	then \
	    echo "Creating $(install_includedir)/IcePatch..." ; \
	    mkdir $(install_includedir)/IcePatch ; \
	    chmod a+rx $(install_includedir)/IcePatch ; \
	fi

	@for i in *.h ; \
	do \
	    echo "Installing $$i" ; \
	    $(INSTALL_DATA) $$i $(install_includedir)/IcePatch/$$i ; \
	done
