# **********************************************************************
#
# Copyright (c) 2003 - 2004
# ZeroC, Inc.
# North Palm Beach, FL, USA
#
# All Rights Reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************


top_srcdir	= ../..

include $(top_srcdir)/config/Make.rules

install::
	@if test ! -d $(install_slicedir)/Glacier ; \
	then \
	    echo "Creating $(install_slicedir)/Glacier..." ; \
	    mkdir $(install_slicedir)/Glacier ; \
	    chmod a+rx $(install_slicedir)/Glacier ; \
	fi

	@for i in *.ice ; \
	do \
	    echo "Installing $$i" ; \
	    $(INSTALL_DATA) $$i $(install_slicedir)/Glacier/$$i ; \
	done
