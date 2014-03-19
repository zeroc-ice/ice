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

OBJS		= Library.obj

COBJS		= Grammar.obj \
		  Scanner.obj \
		  Parser.obj \
		  RunParser.obj \
		  Client.obj \

SOBJS		= LibraryI.obj \
		  LibraryTypes.obj \
		  BookFactory.obj \
		  Server.obj

COLOBJS		= Grammar.obj \
		  Scanner.obj \
		  Parser.obj \
		  RunParser.obj \
		  LibraryI.obj \
		  LibraryTypes.obj \
		  BookFactory.obj \
		  Collocated.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(COBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp) \
		  $(COLOBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -Idummyinclude $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
SLICE2CPPFLAGS	= --ice $(SLICE2CPPFLAGS)

!if "$(GENERATE_PDB)" == "yes"
CPDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
COPDBFLAGS       = /pdb:$(COLLOCATED:.exe=.pdb)
!endif

$(CLIENT): $(OBJS) $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(SETARGV) $(OBJS) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SERVER): $(OBJS) $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SETARGV) $(OBJS) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) freeze$(LIBSUFFIX).lib
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(COLLOCATED): $(OBJS) $(COLOBJS)
	$(LINK) $(LD_EXEFLAGS) $(COPDBFLAGS) $(SETARGV) $(OBJS) $(COLOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) freeze$(LIBSUFFIX).lib
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

!include .depend.mak
