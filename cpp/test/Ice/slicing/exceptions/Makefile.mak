# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..\..

CLIENT		= client.exe
SERVER		= server.exe
SERVERAMD	= serveramd.exe

TARGETS		= $(CLIENT) $(SERVER) $(SERVERAMD)

COBJS		= Test.obj \
		  Client.obj \
		  AllTests.obj

SOBJS		= Test.obj \
    		  ServerPrivate.obj \
		  TestI.obj \
		  Server.obj

SAMDOBJS	= TestAMD.obj \
    		  ServerPrivateAMD.obj \
		  TestAMDI.obj \
		  ServerAMD.obj

SRCS		= $(COBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp) \
		  $(SAMDOBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

ICECPPFLAGS	= -I. $(ICECPPFLAGS)

CPPFLAGS	= -I. -I../../../include $(CPPFLAGS)

$(CLIENT): $(COBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(COBJS), $@,, $(LIBS)

$(SERVER): $(SOBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(SOBJS), $@,, $(LIBS)

$(SERVERAMD): $(SAMDOBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(SAMDOBJS), $@,, $(LIBS)

Test.cpp Test.h: Test.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) Test.ice

TestAMD.cpp TestAMD.h: TestAMD.ice $(SLICE2CPP) $(SLICEPARSERLIB)
        $(SLICE2CPP) $(SLICE2CPPFLAGS) TestAMD.ice

ServerPrivate.cpp ServerPrivate.h: ServerPrivate.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) ServerPrivate.ice

ServerPrivateAMD.cpp ServerPrivateAMD.h: ServerPrivateAMD.ice $(SLICE2CPP) $(SLICEPARSERLIB)
        $(SLICE2CPP) $(SLICE2CPPFLAGS) ServerPrivateAMD.ice

clean::
	del /q Test.cpp Test.h
	del /q TestAMD.cpp TestAMD.h
	del /q ServerPrivate.cpp ServerPrivate.h
	del /q ServerPrivateAMD.cpp ServerPrivateAMD.h

!include .depend
