# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
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
		  TransformVisitor.o \
		  Transformer.o \
		  transformdb.o

DUMP_OBJS	= DumpDescriptors.o \
		  DumpDB.o

SRCS		= $(COMMON_OBJS:.o=.cpp) $(TRANSFORM_OBJS:.o=.cpp) $(DUMP_OBJS:.o=.cpp)

HDIR		= $(includedir)/FreezeScript

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) $(DB_FLAGS) 
BISONFLAGS	:= --name-prefix "freeze_script_" $(BISONFLAGS)

$(TRANSFORMDB): $(TRANSFORM_OBJS) $(COMMON_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(TRANSFORM_OBJS) $(COMMON_OBJS) -lSlice -lIceXML -lFreeze $(LIBS) $(DB_LIBS) $(EXPAT_LIBS)

$(DUMPDB): $(DUMP_OBJS) $(COMMON_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(DUMP_OBJS) $(COMMON_OBJS) -lSlice -lIceXML $(LIBS) $(DB_LIBS) $(EXPAT_LIBS)

clean::
	rm -f Grammar.cpp Grammar.h
	rm -f Scanner.cpp

install:: all
	$(INSTALL_PROGRAM) $(TRANSFORMDB) $(install_bindir)
	$(INSTALL_PROGRAM) $(DUMPDB) $(install_bindir)

include .depend
