# **********************************************************************
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

LIBFILENAME     = $(call mklibfilename,XMLTransform,$(VERSION))
SONAME          = $(call mksoname,XMLTransform,$(SOVERSION))  
LIBNAME		= $(call mklibname,XMLTransform)

TRANSFORM_DB	= $(top_srcdir)/bin/transformdb

TARGETS		= $(libdir)/$(LIBFILENAME) $(libdir)/$(SONAME) $(libdir)/$(LIBNAME) $(TRANSFORM_DB)

OBJS		= ErrorReporter.o \
		  XMLTransform.o

TRANSFORM_OBJS	= TransformDB.o

SRCS		= $(OBJS:.o=.cpp) \
                  $(TRANSFORM_OBJS:.o=.cpp)

HDIR		= $(includedir)/XMLTransform
SDIR		= $(slicedir)/XMLTransform

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DXML_TRANSFORM_API_EXPORTS $(XERCESC_FLAGS)

$(libdir)/$(LIBFILENAME): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

$(TRANSFORM_DB): $(TRANSFORM_OBJS) $(libdir)/$(LIBNAME)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(TRANSFORM_OBJS) -lFreeze -lIceXML -lXMLTransform $(LIBS) $(XERCESC_LIBS) $(DB_LIBS)

clean::
	rm -f $(TRANSFORM_DB)

install:: all
	$(INSTALL_LIBRARY) $(libdir)/$(LIBFILENAME) $(install_libdir)
	rm -f $(install_libdir)/$(SONAME)
	ln -s $(LIBFILENAME) $(install_libdir)/$(SONAME)
	rm -f $(install_libdir)/$(LIBNAME)
	$(INSTALL_PROGRAM) $(TRANSFORM_DB) $(install_bindir)

include .depend
