# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
		  $(wildcard ../slice/IcePack/*.ice) \
		  $(wildcard ../slice/IceSSL/*.ice) \
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
	    if test ! -d $(install_docdir)/images ; \
	    then \
		echo "Creating $(install_docdir)/images..." ; \
		mkdir $(install_docdir)/images ; \
		chmod a+rx $(install_docdir)/images ; \
	    fi ; \
	    echo "Installing images..." ; \
	    ${INSTALL_DATA} images/*.gif $(install_docdir)/images ; \
	    if test ! -d $(install_docdir)/reference ; \
	    then \
		echo "Creating $(install_docdir)/reference..." ; \
		mkdir $(install_docdir)/reference ; \
		chmod a+rx $(install_docdir)/reference ; \
	    fi ; \
	    echo "Installing HTML..." ; \
	    ${INSTALL_DATA} README.html $(install_docdir)/README.html ; \
	    ${INSTALL_DATA} reference/*.html $(install_docdir)/reference ; \
	    if test ! -d $(install_docdir)/reference/stylesheet-images ; \
	    then \
		echo "Creating $(install_docdir)/reference/stylesheet-images..." ; \
		mkdir $(install_docdir)/reference/stylesheet-images ; \
		chmod a+rx $(install_docdir)/reference/stylesheet-images ; \
	    fi ; \
	    echo "Installing image links..." ; \
            (cd $(install_docdir)/reference/stylesheet-images; ln -f -s ../../images/*.gif .) ; \
	fi
	@if test -f reference.pdf ; \
	then \
	    echo "Installing reference.pdf..." ; \
	    ${INSTALL_DATA} reference.pdf $(install_docdir) ; \
	fi
	@if test -f reference.dvi ; \
	then \
	    echo "Installing reference.dvi..." ; \
	    ${INSTALL_DATA} reference.dvi $(install_docdir) ; \
	fi
