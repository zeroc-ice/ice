# **********************************************************************
#
# Copyright (c) 2004
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

TRANSFORMDB	= $(top_srcdir)/bin/transformdb

TARGETS		= $(TRANSFORMDB)

OBJS		= Grammar.o \
		  Scanner.o \
		  AssignVisitor.o \
		  Data.o \
		  Error.o \
		  Functions.o \
		  Exception.o \
		  Parser.o \
		  Print.o \
		  TransformAnalyzer.o \
		  TransformDescriptors.o \
		  TransformVisitor.o \
		  Transformer.o \
		  Util.o

TRANSFORM_OBJS	= TransformDB.o

SRCS		= $(OBJS:.o=.cpp) $(TRANSFORM_OBJS:.o=.cpp)

HDIR		= $(includedir)/FreezeScript

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) $(DB_FLAGS) 

$(TRANSFORMDB): $(TRANSFORM_OBJS) $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(TRANSFORM_OBJS) $(OBJS) -lSlice -lIceXML $(LIBS) $(DB_LIBS)

clean::
	rm -f Grammar.cpp Grammar.h
	rm -f Scanner.cpp

install:: all
	$(INSTALL_PROGRAM) $(TRANSFORMDB) $(install_bindir)

include .depend
