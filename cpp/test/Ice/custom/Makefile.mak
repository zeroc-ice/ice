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
SERVERAMD	= serveramd.exe
COLLOCATED	= collocated.exe

TARGETS		= $(CLIENT) $(SERVER) $(SERVERAMD) $(COLLOCATED)

COBJS           = Test.o \
		  Wstring.o \
		  Client.o \
		  AllTests.o \
		  MyByteSeq.o \
		  StringConverterI.o 

SOBJS           = Test.o \
		  Wstring.o \
		  TestI.o \
		  WstringI.o \
		  Server.o \
		  MyByteSeq.o \
		  StringConverterI.o

SAMDOBJS        = TestAMD.o \
		  TestAMDI.o \
		  WstringAMD.o \
		  WstringAMDI.o \
		  ServerAMD.o \
		  MyByteSeq.o \
		  StringConverterI.o

COLOBJS         = Test.o \
		  Wstring.o \
		  TestI.o \
		  WstringI.o \
		  Collocated.o \
		  AllTests.o \
		  MyByteSeq.o \
		  StringConverterI.o

SRCS		= $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp) \
		  $(SAMDOBJS:.o=.cpp) \
		  $(COLOBJS:.o=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../include $(CPPFLAGS)

$(CLIENT): $(COBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(COBJS), $@,, $(LIBS)

$(SERVER): $(SOBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(SOBJS), $@,, $(LIBS)

$(SERVERAMD): $(SAMDOBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(SAMDOBJS), $@,, $(LIBS)

$(COLLOCATED): $(COLOBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(COLOBJS), $@,, $(LIBS)

Test.cpp Test.h: Test.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) --stream $(SLICE2CPPFLAGS) Test.ice

TestAMD.cpp TestAMD.h: TestAMD.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) --stream $(SLICE2CPPFLAGS) TestAMD.ice

Wstring.cpp Wstring.h: Wstring.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) Wstring.ice

WstringAMD.cpp WstringAMD.h: WstringAMD.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) WstringAMD.ice

clean::
	del /q Test.cpp Test.h
	del /q TestAMD.cpp TestAMD.h
	del /q Wstring.cpp Wstring.h
	del /q WstringAMD.cpp WstringAMD.h

!include .depend
