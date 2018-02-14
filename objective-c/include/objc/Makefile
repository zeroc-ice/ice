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

SUBDIRS		= Ice IceSSL Glacier2 IceStorm IceGrid

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making $@ in $$subdir"; \
	    ( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	done

install::
	@for i in *.h ; \
	do \
	    echo "Installing $$i" ; \
	    $(INSTALL_DATA) $$i $(DESTDIR)$(install_includedir)/$$i ; \
	    chmod a+r $(DESTDIR)$(install_includedir)/$$i ; \
	done
