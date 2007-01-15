# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= .

include $(top_srcdir)/config/Make.rules

SUBDIRS		= config src include test demo slice doc

INSTALL_SUBDIRS	= $(install_bindir) $(install_libdir) $(install_includedir) $(install_slicedir) $(install_docdir)

install::
	@if test ! -d $(prefix) ; \
	then \
	    echo "Creating $(prefix)..." ; \
	    $(call mkdir,$(prefix)) ; \
	fi
ifneq ($(embedded_runpath_prefix),)
	@if test -h $(embedded_runpath_prefix) ; \
	then \
	     if `\rm -f $(embedded_runpath_prefix) 2>/dev/null`; \
              then echo "Removed symbolic link $(embedded_runpath_prefix)"; fi \
        fi
	@if `ln -s $(prefix) $(embedded_runpath_prefix) 2>/dev/null`;\
         then echo "Created symbolic link $(embedded_runpath_prefix) --> $(prefix)"; fi
endif
	@for subdir in $(INSTALL_SUBDIRS); \
	do \
	    if test ! -d $$subdir ; \
	    then \
		echo "Creating $$subdir..." ; \
		mkdir -p $$subdir ; \
		chmod a+rx $$subdir ; \
	    fi ; \
	done

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making $@ in $$subdir"; \
	    ( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	done

doc::
	@( cd doc && $(MAKE) ) || exit 1

install::
	@if test -d doc ; \
	then \
	    ( cd doc && $(MAKE) install ) || exit 1 ; \
	fi
	$(call installdata,ICE_LICENSE,$(prefix))
	$(call installdata,LICENSE,$(prefix))

clean::
	@if test -d doc ; \
	then \
	    ( cd doc && $(MAKE) clean ) || exit 1 ; \
	fi

test::
	@python $(top_srcdir)/allTests.py
