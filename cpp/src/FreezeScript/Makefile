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
DUMPDB		= $(top_srcdir)/bin/dumpdb

TARGETS		= $(TRANSFORMDB) $(DUMPDB)

COMMON_OBJS	= Grammar.o \
		  Scanner.o \
		  AssignVisitor.o \
		  Data.o \
		  Error.o \
		  Functions.o \
		  Exception.o \
		  Parser.o \
		  Print.o \
		  Util.o

TRANSFORM_OBJS	= TransformAnalyzer.o \
		  TransformDescriptors.o \
		  TransformVisitor.o \
		  Transformer.o \
		  TransformDB.o

DUMP_OBJS	= DumpDescriptors.o \
		  DumpDB.o

SRCS		= $(COMMON_OBJS:.o=.cpp) $(TRANSFORM_OBJS:.o=.cpp) $(DUMP_OBJS:.o=.cpp)

HDIR		= $(includedir)/FreezeScript

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) $(DB_FLAGS) 

$(TRANSFORMDB): $(TRANSFORM_OBJS) $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(TRANSFORM_OBJS) $(COMMON_OBJS) -lSlice -lIceXML $(LIBS) $(DB_LIBS)

$(DUMPDB): $(DUMP_OBJS) $(COMMON_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(DUMP_OBJS) $(COMMON_OBJS) -lSlice -lIceXML $(LIBS) $(DB_LIBS)

clean::
	rm -f Grammar.cpp Grammar.h
	rm -f Scanner.cpp

install:: all
	$(INSTALL_PROGRAM) $(TRANSFORMDB) $(install_bindir)
	$(INSTALL_PROGRAM) $(DUMPDB) $(install_bindir)

include .depend
