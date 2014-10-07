# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

CLIENT		= client.exe
SERVER		= server.exe
COLLOCATED	= collocated.exe

TARGETS		= $(CLIENT) $(SERVER) $(COLLOCATED)

SLICE_OBJS	= .\Library.obj

COBJS		= $(SLICE_OBJS) \
		  .\Client.obj \
		  .\Grammar.obj \
		  .\Parser.obj \
		  .\RunParser.obj \
		  .\Scanner.obj

SOBJS		= $(SLICE_OBJS) \
		  .\LibraryTypes.obj \
		  .\BookFactory.obj \
		  .\LibraryI.obj \
		  .\Server.obj

COLOBJS		= $(SLICE_OBJS) \
		  .\LibraryTypes.obj \
		  .\BookFactory.obj \
		  .\Collocated.obj \
		  .\Grammar.obj \
		  .\LibraryI.obj \
		  .\Parser.obj \
		  .\RunParser.obj \
		  .\Scanner.obj

OBJS		= $(COBJS) \
		  $(SOBJS) \
		  $(COLOBJS)

all:: LibraryTypes.cpp LibraryTypes.h

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
SLICE2CPPFLAGS	= --ice $(SLICE2CPPFLAGS)

!if "$(GENERATE_PDB)" == "yes"
CPDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
COPDBFLAGS       = /pdb:$(COLLOCATED:.exe=.pdb)
!endif

$(CLIENT): $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(SETARGV) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SERVER): $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SETARGV) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) 
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(COLLOCATED): $(COLOBJS)
	$(LINK) $(LD_EXEFLAGS) $(COPDBFLAGS) $(SETARGV) $(COLOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) 
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

LibraryTypes.h LibraryTypes.cpp: Library.ice "$(SLICE2FREEZE)" "$(SLICEPARSERLIB)"
	del /q LibraryTypes.h LibraryTypes.cpp
	"$(SLICE2FREEZE)" --ice -I. -I"$(slicedir)" --dict StringIsbnSeqDict,string,Ice::StringSeq LibraryTypes "$(slicedir)/Ice/BuiltinSequences.ice" Library.ice

Scanner.cpp : Scanner.l
	flex Scanner.l
	del /q $@
	echo #include "IceUtil/ScannerConfig.h" >> Scanner.cpp
	type lex.yy.c >> Scanner.cpp
	del /q lex.yy.c

Grammar.cpp Grammar.h: Grammar.y
	del /q Grammar.h Grammar.cpp
	bison -dvt Grammar.y
	move Grammar.tab.c Grammar.cpp
	move Grammar.tab.h Grammar.h
	del /q Grammar.output

clean::
	del /q Library.cpp Library.h

clean::
	-del /q LibraryTypes.h LibraryTypes.cpp
	-if exist db\__Freeze rmdir /q /s db\__Freeze
	-for %f in (db\*) do if not %f == db\.gitignore del /q %f
