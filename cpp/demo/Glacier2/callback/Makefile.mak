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
SESSION_SERVER	= sessionserver.exe

TARGETS		= $(CLIENT) $(SERVER) $(SESSION_SERVER)

OBJS		= Callback.obj \
		  CallbackI.obj

COBJS		= Client.obj

SOBJS		= Server.obj

SSOBJS		= SessionServer.obj \
		  SessionI.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(COBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp) \
		  $(SSOBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(OBJS) $(COBJS), $@,, $(LIBS) glacier2$(LIBSUFFIX).lib

$(SERVER): $(OBJS) $(SOBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(OBJS) $(SOBJS), $@,, $(LIBS)

$(SESSION_SERVER): $(SSOBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(SSOBJS), $@,, $(LIBS) glacier2$(LIBSUFFIX).lib

Callback.cpp Callback.h: Callback.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) Callback.ice

clean::
	del /q Callback.cpp Callback.h

!include .depend
