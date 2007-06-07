# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

TRANSFORMDB	= $(top_srcdir)\bin\transformdb.exe
DUMPDB		= $(top_srcdir)\bin\dumpdb.exe

TARGETS		= $(TRANSFORMDB) $(DUMPDB)

COMMON_OBJS	= Grammar.obj \
		  Scanner.obj \
		  AssignVisitor.obj \
		  Data.obj \
		  Error.obj \
		  Functions.obj \
		  Exception.obj \
		  Parser.obj \
		  Print.obj \
		  Util.obj

TRANSFORM_OBJS	= TransformAnalyzer.obj \
		  TransformVisitor.obj \
		  Transformer.obj \
		  transformdb.obj

DUMP_OBJS	= DumpDescriptors.obj \
		  DumpDB.obj

SRCS		= $(COMMON_OBJS:.obj=.cpp) $(TRANSFORM_OBJS:.obj=.cpp) $(DUMP_OBJS:.obj=.cpp)

HDIR		= $(includedir)\FreezeScript

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I.. -Idummyinclude $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
LINKWITH	= slice$(LIBSUFFIX).lib icexml$(LIBSUFFIX).lib freeze$(LIBSUFFIX).lib $(LIBS) $(DB_LIBS)

!if "$(CPP_COMPILER)" != "BCC2006" && "$(OPTIMIZE)" != "yes"
TPDBFLAGS        = /pdb:$(TRANSFORMDB:.exe=.pdb)
DPDBFLAGS        = /pdb:$(DUMPDB:.exe=.pdb)
!endif

$(TRANSFORMDB): $(TRANSFORM_OBJS) $(COMMON_OBJS)
	$(LINK) $(LD_EXEFLAGS) $(TPDBFLAGS) $(TRANSFORM_OBJS) $(COMMON_OBJS) $(SETARGV) $(PREOUT)$@ \
		$(PRELIBS)$(LINKWITH)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(DUMPDB): $(DUMP_OBJS) $(COMMON_OBJS)
	$(LINK) $(LD_EXEFLAGS) $(DPDBFLAGS) $(DUMP_OBJS) $(COMMON_OBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(LINKWITH)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

Scanner.cpp : Scanner.l
	flex Scanner.l
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
	del /q $(TRANSFORMDB:.exe=.*)
	del /q $(DUMPDB:.exe=.*)
	del /q Grammar.cpp Grammar.h
	del /q Scanner.cpp

install:: all
	copy $(TRANSFORMDB) $(install_bindir)
	copy $(DUMPDB) $(install_bindir)

!if "$(OPTIMIZE)" != "yes"

!if "$(CPP_COMPILER)" == "BCC2006"

install:: all
	copy $(TRANSFORMDB:.exe=.tds) $(install_bindir)
	copy $(DUMPDB:.exe=.tds) $(install_bindir)

!else

install:: all
	copy $(TRANSFORMDB:.exe=.pdb) $(install_bindir)
	copy $(DUMPDB:.exe=.pdb) $(install_bindir)

!endif

!endif

!include .depend
