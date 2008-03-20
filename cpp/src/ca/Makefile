# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

include $(top_srcdir)/config/Make.rules

CA_FILES =  iceca

CLASS_FILES = ImportKey.class

install::
	@for i in $(CA_FILES) ; \
	do \
	    echo "Installing $$i" ; \
	    $(INSTALL_PROGRAM) $$i $(prefix)/bin ;\
	    chmod a+x $(prefix)/bin/$$i ;\
	done
	@for i in $(CLASS_FILES) ; \
	do \
	    echo "Installing $$i" ; \
	    $(INSTALL_PROGRAM) $$i $(prefix)/lib ;\
	    chmod a+x $(prefix)/lib/$$i ;\
	done
