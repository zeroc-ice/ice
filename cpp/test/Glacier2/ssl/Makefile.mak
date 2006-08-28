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

TARGETS		= $(CLIENT) $(SERVER)

COBJS		= Client.obj

SOBJS		= Server.obj

SRCS		= $(COBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../include $(CPPFLAGS)

$(CLIENT): $(COBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(COBJS), $@,, $(LIBS) glacier2$(LIBSUFFIX).lib

$(SERVER): $(SOBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(SOBJS), $@,, $(LIBS) glacier2$(LIBSUFFIX).lib $(OPENSSL_LIBS) icessl$(LIBSUFFIX).lib

!include .depend
