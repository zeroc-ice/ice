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

LIBFILENAME     = $(call mklibfilename,Transform,$(VERSION))
SONAME          = $(call mksoname,Transform,$(SOVERSION))  
LIBNAME		= $(call mklibname,Transform)

TRANSFORMDB	= $(top_srcdir)/bin/transformdb

TARGETS		= $(libdir)/$(LIBFILENAME) $(libdir)/$(SONAME) $(libdir)/$(LIBNAME) $(TRANSFORMDB)

OBJS		= Grammar.o \
                  Scanner.o \
		  Analyzer.o \
		  Transformer.o \
		  Data.o \
		  Error.o \
		  Exception.o \
                  Node.o \
                  TransformUtil.o \
                  Parser.o

TRANSFORM_OBJS	= TransformDB.o

SRCS		= $(OBJS:.o=.cpp) $(TRANSFORM_OBJS:.o=.cpp)

HDIR		= $(includedir)/Transform

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DTRANSFORM_API_EXPORTS $(DB_FLAGS) 

$(libdir)/$(LIBFILENAME): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

$(TRANSFORMDB): $(TRANSFORM_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(TRANSFORM_OBJS) -lTransform -lSlice -lIceXML $(LIBS) $(DB_LIBS)

clean::
	rm -f Grammar.cpp Grammar.h
	rm -f Scanner.cpp

install:: all
	$(INSTALL_LIBRARY) $(libdir)/$(LIBFILENAME) $(install_libdir)
	rm -f $(install_libdir)/$(SONAME)
	ln -s $(LIBFILENAME) $(install_libdir)/$(SONAME)
	rm -f $(install_libdir)/$(LIBNAME)
	ln -s $(SONAME) $(install_libdir)/$(LIBNAME)
	$(INSTALL_PROGRAM) $(TRANSFORMDB) $(install_bindir)

include .depend
