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

TARGETS		= $(CLIENT)

OBJS		= ComplexDict.o \
		  Complex.o \
		  Grammar.o \
		  Scanner.o \
		  Parser.o \
		  Client.o

SRCS		= $(OBJS:.o=.cpp)

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I. -Idummyinclude -I../../include $(CPPFLAGS)

$(CLIENT): $(OBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(OBJS), $@,, $(LIBS) freeze$(LIBSUFFIX).lib

ComplexDict.h ComplexDict.cpp: Complex.ice $(SLICE2FREEZE)
	del /q ComplexDict.h ComplexDict.cpp
	$(SLICE2FREEZE) -I$(slicedir) --dict Complex::ComplexDict,Complex::Key,Complex::Node ComplexDict Complex.ice

Complex.cpp Complex.h: Complex.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) Complex.ice

Scanner.cpp : Scanner.l
	flex $(FLEXFLAGS) Scanner.l
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
	del /q ComplexDict.h ComplexDict.cpp

clean::
	del /q Complex.cpp Complex.h

clean::
	del /q Grammar.cpp Grammar.h
	del /q Scanner.cpp

clean::
	del /q db\test db\log.*

!include .depend
