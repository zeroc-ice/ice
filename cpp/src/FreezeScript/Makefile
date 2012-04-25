# **********************************************************************
#
# Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

TRANSFORMDB	= $(top_srcdir)/bin/transformdb
DUMPDB		= $(top_srcdir)/bin/dumpdb

TARGETS		= $(TRANSFORMDB) $(DUMPDB)

COMMON_OBJS	= ../FreezeScript/Grammar.o \
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
		  TransformVisitor.o \
		  Transformer.o \
		  transformdb.o

DUMP_OBJS	= DumpDescriptors.o \
		  DumpDB.o

SRCS		= $(COMMON_OBJS:.o=.cpp) $(TRANSFORM_OBJS:.o=.cpp) $(DUMP_OBJS:.o=.cpp)

HDIR		= $(headerdir)/FreezeScript

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) $(DB_FLAGS) 
BISONFLAGS	:= --name-prefix "freeze_script_" $(BISONFLAGS)

$(TRANSFORMDB): $(TRANSFORM_OBJS) $(COMMON_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(TRANSFORM_OBJS) $(COMMON_OBJS) -lSlice $(EXPAT_RPATH_LINK) -lIceXML $(DB_RPATH_LINK) -lFreeze $(LIBS) $(DB_LIBS)

$(DUMPDB): $(DUMP_OBJS) $(COMMON_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(DUMP_OBJS) $(COMMON_OBJS) -lSlice $(EXPAT_RPATH_LINK) -lIceXML $(DB_RPATH_LINK) -lFreeze $(LIBS) $(DB_LIBS)

install:: all
	$(call installprogram,$(TRANSFORMDB),$(install_bindir))
	$(call installprogram,$(DUMPDB),$(install_bindir))

include .depend
