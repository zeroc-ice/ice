# **********************************************************************
#
# Copyright (c) 2002
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
	@if test ! -d $(install_includedir)/XMLTransform ; \
	then \
	    echo "Creating $(install_includedir)/XMLTransform..." ; \
	    mkdir $(install_includedir)/XMLTransform ; \
	    chmod a+rx $(install_includedir)/XMLTransform ; \
	fi

	@for i in *.h ; \
	do \
	    echo "Installing $$i" ; \
	    $(INSTALL_DATA) $$i $(install_includedir)/XMLTransform/$$i ; \
	done
