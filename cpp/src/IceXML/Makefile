#**********************************************************************
#
# Copyright (c) 2003
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

top_srcdir	= ../..
 
LIBFILENAME     = $(call mklibfilename,IceXML,$(VERSION))
SONAME          = $(call mksoname,IceXML,$(SOVERSION))  
LIBNAME		= $(call mklibname,IceXML)

TARGETS		= $(libdir)/$(LIBFILENAME) $(libdir)/$(SONAME) $(libdir)/$(LIBNAME)

OBJS		= Parser.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= $(CPPFLAGS) -DICE_XML_API_EXPORTS $(EXPAT_FLAGS)

LINKWITH        := $(EXPAT_LIBS)

$(libdir)/$(LIBFILENAME): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

install:: all
	$(INSTALL_LIBRARY) $(libdir)/$(LIBFILENAME) $(install_libdir)
	rm -f $(install_libdir)/$(SONAME)
	ln -s $(LIBFILENAME) $(install_libdir)/$(SONAME)
	rm -f $(install_libdir)/$(LIBNAME)
	ln -s $(SONAME) $(install_libdir)/$(LIBNAME)



include .depend
