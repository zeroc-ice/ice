# **********************************************************************
#
# Copyright (c) 2003
# ZeroC, Inc.
# Billerica, MA, USA
#
# All Rights Reserved.
#
# Ice is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by
# the Free Software Foundation.
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
