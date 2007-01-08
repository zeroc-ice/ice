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

CA_FILES =  iceca \
	    ImportKey.class

install::
	@for i in $(CA_FILES) ; \
	do \
	    echo "Installing $$i" ; \
	    $(INSTALL_PROGRAM) $$i $(install_bindir) ;\
	    chmod a+x $(install_bindir)/$$i ;\
	done
