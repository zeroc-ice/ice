# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

include $(top_srcdir)/config/Make.rules

install::
	@if test ! -d $(DESTDIR)$(install_includedir)/IceUtil ; \
	then \
	    echo "Creating $(DESTDIR)$(install_includedir)/IceUtil..." ; \
	    $(call mkdir,$(DESTDIR)$(install_includedir)/IceUtil) ; \
	fi

	@for i in *.h ; \
	do \
	    echo "Installing $$i" ; \
	    $(INSTALL_DATA) $$i $(DESTDIR)$(install_includedir)/IceUtil/$$i ; \
	    chmod a+r $(DESTDIR)$(install_includedir)/IceUtil/$$i ; \
	done
