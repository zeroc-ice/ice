# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

CA_FILES	=  iceca

CLASS_FILES	= ImportKey.class

TARGETS		= $(bindir)/iceca \
		  $(top_srcdir)/lib/ImportKey.class

include $(top_srcdir)/config/Make.rules

$(bindir)/iceca: iceca
	cp iceca $@

$(top_srcdir)/lib/ImportKey.class: ImportKey.class
	cp ImportKey.class $@

install::
	@for subdir in $(DESTDIR)$(prefix)/bin $(DESTDIR)$(prefix)/lib; \
	do \
	    if test ! -d $$subdir ; \
	    then \
		echo "Creating $$subdir..." ; \
		mkdir -p $$subdir ; \
		chmod a+rx $$subdir ; \
	    fi ; \
	done
	@for i in $(CA_FILES) ; \
	do \
	    echo "Installing $$i" ; \
	    $(INSTALL_PROGRAM) $$i $(DESTDIR)$(prefix)/bin ;\
	    chmod a+x $(DESTDIR)$(prefix)/bin/$$i ;\
	done
	$(call installdata,$(top_srcdir)/../man/man1/iceca.1,$(DESTDIR)$(install_mandir))
	@for i in $(CLASS_FILES) ; \
	do \
	    echo "Installing $$i" ; \
	    $(INSTALL_PROGRAM) $$i $(DESTDIR)$(prefix)/lib ;\
	    chmod a+x $(DESTDIR)$(prefix)/lib/$$i ;\
	done
