# **********************************************************************
#
# Copyright (c) 2001
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
		  ../slice/Ice/Identity.ice \
		  ../slice/Ice/Facet.ice \
		  ../slice/Ice/Current.ice \
		  ../slice/Ice/LocalException.ice \
		  ../slice/Ice/Logger.ice \
		  ../slice/Ice/ObjectAdapter.ice \
		  ../slice/Ice/ObjectFactory.ice \
		  ../slice/Ice/Plugin.ice \
		  ../slice/Ice/Properties.ice \
		  ../slice/Ice/Router.ice \
		  ../slice/Ice/Locator.ice \
		  ../slice/Ice/ServantLocator.ice \
		  ../slice/Ice/Stream.ice \
		  ../slice/Ice/UserExceptionFactory.ice \
		  ../slice/IceSSL/CertificateVerifier.ice \
		  ../slice/IceSSL/Exception.ice \
		  ../slice/IceSSL/Plugin.ice \
		  ../slice/Freeze/DB.ice \
		  ../slice/Freeze/DBException.ice \
		  ../slice/Freeze/Evictor.ice \
		  ../slice/IceStorm/IceStorm.ice \
		  ../slice/IcePack/Admin.ice \
		  ../slice/Glacier/Router.ice \
		  ../slice/Glacier/Starter.ice \
		  ../slice/Glacier/Session.ice \
		  ../slice/Glacier/SessionManager.ice \
		  ../slice/IceBox/IceBox.ice \
		  ../slice/IcePatch/IcePatch.ice

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
