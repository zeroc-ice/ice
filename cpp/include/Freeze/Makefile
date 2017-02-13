# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

include $(top_srcdir)/config/Make.rules

install::
	@if test ! -d $(DESTDIR)$(install_includedir)/Freeze ; \
	then \
	    echo "Creating $(DESTDIR)$(install_includedir)/Freeze..." ; \
	    $(call mkdir,$(DESTDIR)$(install_includedir)/Freeze) ; \
	fi

	@for i in *.h ; \
	do \
	    echo "Installing $$i" ; \
	    $(INSTALL_DATA) $$i $(DESTDIR)$(install_includedir)/Freeze/$$i ; \
	    chmod a+r $(DESTDIR)$(install_includedir)/Freeze/$$i ; \
	done
