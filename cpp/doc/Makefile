# **********************************************************************
#
# Copyright (c) 2002
# ZeroC, Inc.
# Billerica, MA, USA
#
# All Rights Reserved.
#
# Ice is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by
# the Free Software Foundation.
#
# **********************************************************************

top_srcdir	= ..

TARGETS		= manual/index.html

SGMLFILES	= manual.sgml \
		  SliceDoc.sgml \
		  Properties.sgml \
		  ProxiesEndpoints.sgml

SLICEFILES	= ../slice/Ice/BuiltinSequences.ice \
		  ../slice/Ice/Communicator.ice \
		  ../slice/Ice/Current.ice \
		  ../slice/Ice/Facet.ice \
		  ../slice/Ice/Identity.ice \
		  ../slice/Ice/LocalException.ice \
		  ../slice/Ice/Locator.ice \
		  ../slice/Ice/Logger.ice \
		  ../slice/Ice/ObjectAdapter.ice \
		  ../slice/Ice/ObjectFactory.ice \
		  ../slice/Ice/Plugin.ice \
		  ../slice/Ice/Properties.ice \
		  ../slice/Ice/Router.ice \
		  ../slice/Ice/ServantLocator.ice \
		  ../slice/Ice/Stream.ice \
		  ../slice/Ice/UserExceptionFactory.ice \
		  ../slice/Freeze/DBException.ice \
		  ../slice/Freeze/DB.ice \
		  ../slice/Freeze/Evictor.ice \
		  ../slice/Glacier/Router.ice \
		  ../slice/Glacier/Session.ice \
		  ../slice/Glacier/SessionManager.ice \
		  ../slice/Glacier/Starter.ice \
		  ../slice/IceBox/IceBox.ice \
		  ../slice/IcePack/Admin.ice \
		  ../slice/IcePatch/IcePatch.ice \
		  ../slice/IceSSL/CertificateVerifier.ice \
		  ../slice/IceSSL/Exception.ice \
		  ../slice/IceSSL/Plugin.ice \
		  ../slice/IceStorm/IceStorm.ice

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

clean::
	rm -f SliceDoc.sgml

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
