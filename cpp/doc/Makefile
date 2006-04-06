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

SGMLFILES	= reference.sgml \
		  SliceDoc.sgml \
		  Properties.sgml \
		  ProxiesEndpoints.sgml

SLICEFILES	= $(wildcard ../slice/Ice/*.ice) \
		  $(wildcard ../slice/Freeze/*.ice) \
		  $(wildcard ../slice/IceBox/*.ice) \
		  $(wildcard ../slice/IceGrid/*.ice) \
		  $(wildcard ../slice/Glacier2/*.ice) \
		  $(wildcard ../slice/IceStorm/*.ice) \
		  $(wildcard ../slice/IcePatch2/*.ice)

include $(top_srcdir)/config/Make.rules

reference/index.html: $(SGMLFILES)
	rm -rf reference
	db2html reference.sgml
	(mkdir -p reference/stylesheet-images; \
	 cd reference/stylesheet-images; \
	 ln -f -s ../../images/*.gif .)

reference.pdf: $(SGMLFILES)
	rm -rf reference.pdf
	db2pdf reference.sgml

reference.dvi: $(SGMLFILES)
	rm -rf reference.dvi
	db2dvi reference.sgml

clean::
	rm -rf reference reference.out reference.texi

SliceDoc.sgml: $(SLICEFILES) $(SLICE2DOCBOOK)
	rm -f $@
	$(SLICE2DOCBOOK) --ice -I$(top_srcdir)/slice $@ $(SLICEFILES)

appendix:
	rm -f SliceDoc.sgml
	$(SLICE2DOCBOOK) --noindex --ice -I$(top_srcdir)/slice SliceDoc.sgml $(SLICEFILES)

clean::
	rm -f SliceDoc.sgml appendix.sgml

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
	@if test -f reference.pdf ; \
	then \
	    echo "Installing reference.pdf..." ; \
	    $(call installdata,reference.pdf,$(install_docdir)) ; \
	fi
	@if test -f reference.dvi ; \
	then \
	    echo "Installing reference.dvi..." ; \
	    $(call installdata,reference.dvi,$(install_docdir)) ; \
	fi
