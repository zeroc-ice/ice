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
	@if test ! -d $(install_includedir)/Ice ; \
	then \
	    echo "Creating $(install_includedir)/Ice..." ; \
	    mkdir $(install_includedir)/Ice ; \
	    chmod a+rx $(install_includedir)/Ice ; \
	fi

	@for i in *.h ; \
	do \
	    echo "Installing $$i" ; \
	    $(INSTALL_DATA) $$i $(install_includedir)/Ice/$$i ; \
	done
