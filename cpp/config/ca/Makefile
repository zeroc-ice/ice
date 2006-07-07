# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

include $(top_srcdir)/config/Make.rules

CA_FILES =  initca.py \
	    req.py \
	    cautil.py  \
	    import.py \
	    sign.py \
	    ImportKey.class

install::
	@if test ! -d $(prefix)/config/ca ; \
	then \
	    echo "Creating $(prefix)/config/ca..." ; \
	    $(call mkdir,$(prefix)/config/ca) ; \
	fi
	@for i in $(CA_FILES) ; \
	do \
	    echo "Installing $$i" ; \
	    $(INSTALL_PROGRAM) $$i $(prefix)/config/ca ;\
	    chmod a+x $(prefix)/config/ca/$$i ;\
	done
	$(INSTALL_DATA) README $(prefix)/config/ca
	chmod a+r $(prefix)/config/ca/README
