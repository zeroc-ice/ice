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

TARGETS		= manual/index.html \
#		  manual.pdf

SGMLFILES	= manual.sgml \
		  ModuleIce.sgml \
		  ModuleIcePack.sgml \
		  Properties.sgml \
		  ReferencesEndpoints.sgml

ICEFILES	= ../slice/Ice/Communicator.ice \
		  ../slice/Ice/Logger.ice \
		  ../slice/Ice/ObjectAdapter.ice \
		  ../slice/Ice/Pickler.ice \
		  ../slice/Ice/Properties.ice \
		  ../slice/Ice/ValueFactory.ice

ICEPACKFILES	= ../slice/IcePack/Admin.ice

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

clean::
	rm -rf manual manual.out manual.texi

ModuleIce.sgml: $(ICEFILES) $(SLICE2DOCBOOK)
	rm -f $@
	$(SLICE2DOCBOOK) --no-globals -I$(top_srcdir)/slice $@ $(ICEFILES)

ModuleIcePack.sgml: $(ICEPACKFILES) $(SLICE2DOCBOOK)
	rm -f $@
	$(SLICE2DOCBOOK) --no-globals -I$(top_srcdir)/slice $@ $(ICEPACKFILES)

clean::
	rm -f ModuleIce.sgml
