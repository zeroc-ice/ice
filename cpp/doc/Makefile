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

TARGETS		= index.html \
		  intro.html \
		  slice.html \
		  cppmap.html

include $(top_srcdir)/config/Make.rules

$(TARGETS): main.sgml
	rm -f $(TARGETS)
	$(SGML2HTML) main.sgml