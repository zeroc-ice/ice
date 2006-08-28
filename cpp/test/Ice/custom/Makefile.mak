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

COBJS           = Test.obj \
		  Wstring.obj \
		  Client.obj \
		  AllTests.obj \
		  MyByteSeq.obj \
		  StringConverterI.obj 

SOBJS           = Test.obj \
		  Wstring.obj \
		  TestI.obj \
		  WstringI.obj \
		  Server.obj \
		  MyByteSeq.obj \
		  StringConverterI.obj

SAMDOBJS        = TestAMD.obj \
		  TestAMDI.obj \
		  WstringAMD.obj \
		  WstringAMDI.obj \
		  ServerAMD.obj \
		  MyByteSeq.obj \
		  StringConverterI.obj

COLOBJS         = Test.obj \
		  Wstring.obj \
		  TestI.obj \
		  WstringI.obj \
		  Collocated.obj \
		  AllTests.obj \
		  MyByteSeq.obj \
		  StringConverterI.obj

SRCS		= $(COBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp) \
		  $(SAMDOBJS:.obj=.cpp) \
		  $(COLOBJS:.obj=.cpp)

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
