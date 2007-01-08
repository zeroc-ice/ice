# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

reference/index.html: $(SLICEFILES)
	rm -rf reference
	slice2html --ice -I../slice --hdr=htmlHeader --output-dir=reference --index=3 --summary=120 $(SLICEFILES)

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
	    if test ! -d $(install_docdir)/reference/stylesheet-images ; \
	    then \
		echo "Creating $(install_docdir)/reference/stylesheet-images..." ; \
		$(call mkdir,$(install_docdir)/reference/stylesheet-images) ; \
	    fi ; \
	    echo "Installing images..." ; \
	    $(call installdata,images/*.gif,$(install_docdir)/reference/stylesheet-images) ; \
	fi
