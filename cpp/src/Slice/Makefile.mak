# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\slice$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\slice$(SOVERSION)$(LIBSUFFIX)$(COMPSUFFIX).dll

TARGETS		= $(LIBNAME) $(DLLNAME)

OBJS		= Scanner.obj \
		  Grammar.obj \
		  Parser.obj \
		  CPlusPlusUtil.obj \
		  CsUtil.obj \
		  JavaUtil.obj \
		  Preprocessor.obj \
		  Checksum.obj \
		  PythonUtil.obj \
		  DotNetNames.obj \
		  RubyUtil.obj \
		  PHPUtil.obj \
		  Util.obj \
		  FileTracker.obj \
		  MD5.obj \
		  MD5I.obj

SRCS		= $(OBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I.. -Idummyinclude $(CPPFLAGS) -DSLICE_API_EXPORTS  -DWIN32_LEAN_AND_MEAN

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
!endif

RES_FILE        = Slice.res

!if "$(STATICLIBS)" == "yes"

$(DLLNAME):

$(LIBNAME): $(OBJS)
	$(AR) $(ARFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@

!else

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS) Slice.res
	$(LINK) $(BASE):0x21000000 $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(BASELIBS) $(MCPP_LIBS) \
	    $(RES_FILE)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

!endif

Scanner.cpp : Scanner.l
	flex Scanner.l
	del /q $@
	echo #include "IceUtil/ScannerConfig.h" >> Scanner.cpp
	type lex.yy.c >> Scanner.cpp
	del /q lex.yy.c

Grammar.cpp Grammar.h: Grammar.y
	del /q Grammar.h Grammar.cpp
	bison -dvt --name-prefix "slice_" Grammar.y
	move Grammar.tab.c Grammar.cpp
	move Grammar.tab.h Grammar.h
	del /q Grammar.output

clean::
	-del /q Slice.res

install:: all
	copy $(LIBNAME) "$(install_libdir)"
	copy $(DLLNAME) "$(install_bindir)"


!if "$(GENERATE_PDB)" == "yes"

install:: all
	copy $(DLLNAME:.dll=.pdb) "$(install_bindir)"

!endif

!include .depend.mak
