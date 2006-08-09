# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

TRANSFORMDB	= $(top_srcdir)\bin\transformdb.exe
DUMPDB		= $(top_srcdir)\bin\dumpdb.exe

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

HDIR		= $(includedir)\FreezeScript

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I.. $(CPPFLAGS)
LINKWITH	= slice$(LIBSUFFIX).lib icexml$(LIBSUFFIX).lib freeze$(LIBSUFFIX).lib $(LIBS) $(DB_LIBS)

$(TRANSFORMDB): $(TRANSFORM_OBJS) $(COMMON_OBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(TRANSFORM_OBJS) $(COMMON_OBJS), $@,, $(LINKWITH)

$(DUMPDB): $(DUMP_OBJS) $(COMMON_OBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(DUMP_OBJS) $(COMMON_OBJS), $@,, $(LINKWITH)

Scanner.cpp : Scanner.l
	flex $(FLEXFLAGS) Scanner.l
	del /q $@
	echo #include "IceUtil/Config.h" > Scanner.cpp
	type lex.yy.c >> Scanner.cpp
	del /q lex.yy.c

Grammar.cpp Grammar.h: Grammar.y
	del /q Grammar.h Grammar.cpp
	bison -dvt --name-prefix "freeze_script_" Grammar.y
	move Grammar.tab.c Grammar.cpp
	move Grammar.tab.h Grammar.h
	del /q Grammar.output

clean::
	del /q Grammar.cpp Grammar.h
	del /q Scanner.cpp

install:: all
	copy $(TRANSFORMDB) $(install_bindir)
	copy $(DUMPDB) $(install_bindir)

!include .depend
