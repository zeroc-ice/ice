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

COBJS		= Test.obj \
		  Client.obj \
		  AllTests.obj \
		  Twoways.obj \
		  TwowaysAMI.obj \
		  BatchOneways.obj

SOBJS		= Test.obj \
		  TestI.obj \
		  Server.obj

SAMDOBJS	= TestAMD.obj \
		  TestAMDI.obj \
		  ServerAMD.obj

COLOBJS		= Test.obj \
		  TestI.obj \
		  Collocated.obj \
		  AllTests.obj \
		  Twoways.obj \
		  TwowaysAMI.obj \
		  BatchOneways.obj

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
	$(SLICE2CPP) $(SLICE2CPPFLAGS) Test.ice

TestAMD.cpp TestAMD.h: TestAMD.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) TestAMD.ice

clean::
	del /q Test.cpp Test.h
	del /q TestAMD.cpp TestAMD.h

!include .depend
