# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

TRANSFORMDB	= $(bindir)/transformdb
DUMPDB		= $(bindir)/dumpdb

TARGETS		= $(TRANSFORMDB) $(DUMPDB)

COMMON_OBJS	= AssignVisitor.o \
		  Data.o \
		  Error.o \
		  Exception.o \
		  Functions.o \
		  Parser.o \
		  Print.o \
		  Scanner.o \
		  Util.o \
		  Grammar.o

TRANSFORM_OBJS 	= TransformAnalyzer.o \
		  TransformVisitor.o \
		  Transformer.o \
		  transformdb.o

DUMP_OBJS	= DumpDescriptors.o \
		  DumpDB.o

OBJS		= $(COMMON_OBJS) $(TRANSFORM_OBJS) $(DUMP_OBJS)

HDIR		= $(headerdir)/FreezeScript

RPATH_DIR	= $(LOADER_PATH)/../$(libsubdir)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) $(DB_FLAGS)
BISONFLAGS	:= --name-prefix "freeze_script_" $(BISONFLAGS)

$(TRANSFORMDB): $(TRANSFORM_OBJS) $(COMMON_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(TRANSFORM_OBJS) $(COMMON_OBJS) -lSlice $(EXPAT_RPATH_LINK) -lIceXML $(DB_RPATH_LINK) -lFreeze $(LIBS) $(DB_LIBS)

$(DUMPDB): $(DUMP_OBJS) $(COMMON_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(DUMP_OBJS) $(COMMON_OBJS) -lSlice $(EXPAT_RPATH_LINK) -lIceXML $(DB_RPATH_LINK) -lFreeze $(LIBS) $(DB_LIBS)

install:: all
	$(call installprogram,$(TRANSFORMDB),$(DESTDIR)$(install_bindir))
	$(call installdata,$(top_srcdir)/../man/man1/transformdb.1,$(DESTDIR)$(install_mandir))
	$(call installprogram,$(DUMPDB),$(DESTDIR)$(install_bindir))
	$(call installdata,$(top_srcdir)/../man/man1/dumpdb.1,$(DESTDIR)$(install_mandir))
