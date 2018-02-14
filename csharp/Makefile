# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= .

include $(top_srcdir)/config/Make.rules.cs

SUBDIRS		= src

ifneq ($(MAKECMDGOALS),install)
SUBDIRS		:= $(SUBDIRS) test
endif

INSTALL_SUBDIRS	= $(install_bindir) $(install_assembliesdir) $(install_mandir)

install:: install-common
	@for subdir in $(INSTALL_SUBDIRS); \
	do \
	    if test ! -d $(DESTDIR)$$subdir ; \
	    then \
		echo "Creating $$subdir..." ; \
		mkdir -p $(DESTDIR)$$subdir ; \
		chmod a+rx $(DESTDIR)$$subdir ; \
	    fi ; \
	done

ifeq ($(GACINSTALL),yes)
install:: install-common
	@if test ! -d $(DESTDIR)$(install_libdir) ; \
	then \
	    echo "Creating $(install_libdir)..." ; \
	    $(call mkdir,$(DESTDIR)$(install_libdir)) ; \
	fi
	@if test ! -d $(DESTDIR)$(install_pkgconfigdir) ; \
	then \
	    echo "Creating $(DESTDIR)$(install_pkgconfigdir)..." ; \
	    $(call mkdir,$(DESTDIR)$(install_pkgconfigdir)) ; \
	fi

endif

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making $@ in $$subdir"; \
	    ( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	done

test::
	@python $(top_srcdir)/allTests.py
