#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

top_srcdir      = ../..

include $(top_srcdir)/config/Make.rules

install::
	@if test ! -d $(DESTDIR)$(install_includedir)/IceBT ; \
	then \
	    echo "Creating $(DESTDIR)$(install_includedir)/IceBT..." ; \
	    $(call mkdir,$(DESTDIR)$(install_includedir)/IceBT) ; \
	fi

	@for i in *.h ; \
	do \
	    echo "Installing $$i" ; \
	    $(INSTALL_DATA) $$i $(DESTDIR)$(install_includedir)/IceBT/$$i ; \
	    chmod a+r $(DESTDIR)$(install_includedir)/IceBT/$$i ; \
	done
