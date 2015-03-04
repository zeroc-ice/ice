# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

TARGETS		= reference/index.html

SLICEFILES	= $(wildcard ../../slice/Ice/*.ice) \
		  $(wildcard ../../slice/Freeze/*.ice) \
		  $(wildcard ../../slice/IceBox/*.ice) \
		  $(wildcard ../../slice/IceGrid/*.ice) \
		  $(wildcard ../../slice/Glacier2/*.ice) \
		  $(wildcard ../../slice/IceSSL/*.ice) \
		  $(wildcard ../../slice/IceStorm/*.ice) \
		  $(wildcard ../../slice/IcePatch2/*.ice)

include $(top_srcdir)/config/Make.rules

IMAGES		= images
JAVASCRIPT	= symboltree.js

reference/index.html: $(SLICEFILES)
	$(MAKE) clean
	$(bindir)/slice2html --ice -I$(slicedir) --hdr=htmlHeader --ftr=htmlFooter \
	    --indexhdr=indexHeader --indexftr=indexFooter \
	    --image-dir=images --logo-url="http://www.zeroc.com" --search="/cgi-bin/swish.cgi" \
	    --output-dir=reference --index=3 --summary=120 $(SLICEFILES)
	mkdir reference/$(IMAGES)
	cp $(IMAGES)/*.gif reference/$(IMAGES)
	cp $(JAVASCRIPT) reference

clean::
	-rm -rf reference/*

install:: install-common reference/index.html
	@if test ! -d $(install_docdir) ; \
	then \
	    echo "Creating $(install_docdir)..." ; \
	    $(call mkdir,$(install_docdir)) ; \
	fi ; \
	if test ! -d $(install_docdir)/reference ; \
	then \
	    echo "Creating $(install_docdir)/reference..." ; \
	    $(call mkdir,$(install_docdir)/reference) ; \
	fi ; \
	echo "Installing HTML..." ; \
	$(call installdata,reference/*.*,$(install_docdir)/reference) ; \
        for i in reference/*; \
        do if test -d $$i ; \
           then \
              if test ! -d $(install_docdir)/$$i ; \
	      then \
	          echo "Creating $(install_docdir)/$$i..." ; \
		  $(call mkdir,$(install_docdir)/$$i) ; \
	      fi ; \
	      echo "Installing files in $$i..." ; \
	      $(call installdata,$$i/*.*,$(install_docdir)/$$i) ; \
	    fi; \
        done ; \

