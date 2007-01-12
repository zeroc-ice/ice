# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

TARGETS		= reference/index.html

SLICEFILES	= $(wildcard ../slice/Ice/*.ice) \
		  $(wildcard ../slice/Freeze/*.ice) \
		  $(wildcard ../slice/IceBox/*.ice) \
		  $(wildcard ../slice/IceGrid/*.ice) \
		  $(wildcard ../slice/Glacier2/*.ice) \
		  $(wildcard ../slice/IceSSL/*.ice) \
		  $(wildcard ../slice/IceStorm/*.ice) \
		  $(wildcard ../slice/IcePatch2/*.ice)

include $(top_srcdir)/config/Make.rules

IMAGES		= images
JAVASCRIPT	= symboltree.js

reference/index.html: $(SLICEFILES)
	make clean
	slice2html --ice -I../slice --hdr=htmlHeader --indexhdr=indexHeader --indexftr=indexFooter --output-dir=reference --index=3 --summary=120 $(SLICEFILES)
	mkdir reference/$(IMAGES)
	cp $(IMAGES)/*.gif reference/$(IMAGES)
	cp $(JAVASCRIPT) reference

clean::
	rm -rf reference

install::
	@if test -f reference/index.html ; \
	then \
	    if test ! -d $(install_docdir)/reference ; \
	    then \
		echo "Creating $(install_docdir)/reference..." ; \
		$(call mkdir,$(install_docdir)/reference) ; \
	    fi ; \
	    echo "Installing HTML..." ; \
	    $(call installdata,README.html,$(install_docdir)) ; \
	    $(call installdata,reference/*.html,$(install_docdir)/reference) ; \
	    if test ! -d $(install_docdir)/reference/$(IMAGES) ; \
	    then \
		echo "Creating $(install_docdir)/reference/$(IMAGES)... ; \
		$(call mkdir,$(install_docdir)/reference/$(IMAGES)) ; \
	    fi ; \
	    echo "Installing images..." ; \
	    $(call installdata,$(IMAGES)/*.gif,$(install_docdir)/reference/$(IMAGES)) ; \
	fi
