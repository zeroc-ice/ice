# **********************************************************************
#
# Copyright (c) 2001
# MutableRealms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

top_srcdir	= ..

TARGETS		= manual/index.html

SGMLFILES	= manual.sgml \
		  SliceDoc.sgml \
		  Properties.sgml \
		  ReferencesEndpoints.sgml

SLICEFILES	= ../slice/Ice/LocalException.ice \
		  ../slice/Ice/Properties.ice \
		  ../slice/Ice/Logger.ice \
		  ../slice/Ice/Stream.ice \
		  ../slice/Ice/Communicator.ice \
		  ../slice/Ice/ObjectFactory.ice \
		  ../slice/Ice/UserExceptionFactory.ice \
		  ../slice/Ice/ObjectAdapter.ice \
		  ../slice/Ice/ServantLocator.ice \
		  ../slice/Ice/SecurityException.ice \
		  ../slice/Ice/Identity.ice \
		  ../slice/Ice/Current.ice \
		  ../slice/IcePack/Admin.ice \
		  ../slice/Freeze/DB.ice \
		  ../slice/Freeze/DBException.ice \
		  ../slice/Freeze/Evictor.ice \
		  ../slice/IceStorm/IceStorm.ice

include $(top_srcdir)/config/Make.rules

manual/index.html: $(SGMLFILES)
	rm -rf manual
	db2html manual.sgml
	(mkdir manual/stylesheet-images; \
	 cd manual/stylesheet-images; \
	 ln -s ../../images/*.gif .)

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
	$(SLICE2DOCBOOK) -I$(top_srcdir)/slice $@ $(SLICEFILES)

clean::
	rm -f SliceDoc.sgml
