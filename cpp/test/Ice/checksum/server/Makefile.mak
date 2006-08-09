# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..\..

SERVER		= server.exe

TARGETS		= $(SERVER)

SOBJS		= Test.o \
		  Types.o \
		  TestI.o \
		  Server.o

SRCS		= $(SOBJS:.o=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../../include $(CPPFLAGS)

$(SERVER): $(SOBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(SOBJS), $@,, $(LIBS)

Test.cpp Test.h: Test.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) --checksum $(SLICE2CPPFLAGS) Test.ice

Types.cpp Types.h: Types.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) --checksum $(SLICE2CPPFLAGS) Types.ice

clean::
	del /q Test.cpp Test.h
	del /q Types.cpp Types.h

!include .depend
