# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

TARGETS		= manual/index.html

SGMLFILES	= manual.sgml \
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

manual/index.html: $(SGMLFILES)
	rm -rf manual
	db2html manual.sgml
	(mkdir -p manual/stylesheet-images; \
	 cd manual/stylesheet-images; \
	 ln -f -s ../../images/*.gif .)

manual.pdf: $(SGMLFILES)
	rm -rf manual.pdf
	db2pdf manual.sgml

manual.dvi: $(SGMLFILES)
	rm -rf manual.dvi
	db2dvi manual.sgml

clean::
	rm -rf manual manual.out manual.texi

SliceDoc.sgml: $(SLICEFILES) $(SLICE2DOCBOOK)
	rm -f $@
	$(SLICE2DOCBOOK) --ice -I$(top_srcdir)/slice $@ $(SLICEFILES)

appendix:
	rm -f SliceDoc.sgml
	$(SLICE2DOCBOOK) --noindex --ice -I$(top_srcdir)/slice SliceDoc.sgml $(SLICEFILES)

clean::
	rm -f SliceDoc.sgml appendix.sgml

install::
	@if test -f manual/index.html ; \
	then \
	    if test ! -d $(install_docdir)/images ; \
	    then \
		echo "Creating $(install_docdir)/images..." ; \
		mkdir $(install_docdir)/images ; \
		chmod a+rx $(install_docdir)/images ; \
	    fi ; \
	    echo "Installing images..." ; \
	    ${INSTALL_DATA} images/*.gif $(install_docdir)/images ; \
	    if test ! -d $(install_docdir)/manual ; \
	    then \
		echo "Creating $(install_docdir)/manual..." ; \
		mkdir $(install_docdir)/manual ; \
		chmod a+rx $(install_docdir)/manual ; \
	    fi ; \
	    echo "Installing HTML..." ; \
	    ${INSTALL_DATA} manual/*.html $(install_docdir)/manual ; \
	    if test ! -d $(install_docdir)/manual/stylesheet-images ; \
	    then \
		echo "Creating $(install_docdir)/manual/stylesheet-images..." ; \
		mkdir $(install_docdir)/manual/stylesheet-images ; \
		chmod a+rx $(install_docdir)/manual/stylesheet-images ; \
	    fi ; \
	    echo "Installing image links..." ; \
            (cd $(install_docdir)/manual/stylesheet-images; ln -f -s ../../images/*.gif .) ; \
	fi
	@if test -f manual.pdf ; \
	then \
	    echo "Installing manual.pdf..." ; \
	    ${INSTALL_DATA} manual.pdf $(install_docdir) ; \
	fi
	@if test -f manual.dvi ; \
	then \
	    echo "Installing manual.dvi..." ; \
	    ${INSTALL_DATA} manual.dvi $(install_docdir) ; \
	fi
