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
		  manual.pdf

include $(top_srcdir)/config/Make.rules

manual/index.html: manual.sgml
	rm -rf manual
	db2html manual.sgml

manual.pdf: manual.sgml
	rm -rf manual.pdf
	db2pdf manual.sgml

clean::
	rm -rf manual