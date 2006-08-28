# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

CPPFLAGS	= -I. -Idummyinclude $(CPPFLAGS)
SLICE2CPPFLAGS	= --ice $(SLICE2CPPFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(OBJS) $(COBJS), $@,, $(LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(OBJS) $(SOBJS), $@,, $(LIBS) freeze$(LIBSUFFIX).lib

$(COLLOCATED): $(OBJS) $(COLOBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(OBJS) $(COLOBJS), $@,, $(LIBS) freeze$(LIBSUFFIX).lib

LibraryTypes.h LibraryTypes.cpp: Library.ice $(SLICE2FREEZE)
	del /q LibraryTypes.h LibraryTypes.cpp
	$(SLICE2FREEZE) --ice -I$(slicedir) --dict StringIsbnSeqDict,string,Ice::StringSeq LibraryTypes $(slicedir)/Ice/BuiltinSequences.ice Library.ice

Library.cpp Library.h: Library.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) Library.ice

Scanner.cpp : Scanner.l
	flex Scanner.l
	del /q $@
	echo #include "IceUtil/Config.h" > Scanner.cpp
	type lex.yy.c >> Scanner.cpp
	del /q lex.yy.c

Grammar.cpp Grammar.h: Grammar.y
	del /q Grammar.h Grammar.cpp
	bison -dvt Grammar.y
	move Grammar.tab.c Grammar.cpp
	move Grammar.tab.h Grammar.h
	del /q Grammar.output

clean::
	del /q LibraryTypes.h LibraryTypes.cpp

clean::
	del /q Library.cpp Library.h

clean::
	del /q Grammar.cpp Grammar.h
	del /q Scanner.cpp

!include .depend
