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

SLICEFILES	= ../slice/Ice/Communicator.ice \
		  ../slice/Ice/Logger.ice \
		  ../slice/Ice/ObjectAdapter.ice \
		  ../slice/Ice/Pickler.ice \
		  ../slice/Ice/Properties.ice \
		  ../slice/Ice/ValueFactory.ice

include $(top_srcdir)/config/Make.rules

manual/index.html: manual.sgml reference.sgml
	rm -rf manual
	db2html manual.sgml
	(mkdir manual/stylesheet-images; \
	 cd manual/stylesheet-images; \
	 ln -s ../../images/*.gif .)

manual.pdf: manual.sgml reference.sgml
	rm -rf manual.pdf
	db2pdf manual.sgml

clean::
	rm -rf manual manual.out manual.texi

reference.sgml: $(SLICEFILES) $(SLICE2DOCBOOK)
	rm -f $@
	$(SLICE2DOCBOOK) -I$(top_srcdir)/slice $@ $(SLICEFILES)

clean::
	rm -f reference.sgml
