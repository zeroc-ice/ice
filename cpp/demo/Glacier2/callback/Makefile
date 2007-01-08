# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client
SERVER		= server
SESSION_SERVER	= sessionserver

TARGETS		= $(CLIENT) $(SERVER) $(SESSION_SERVER)

OBJS		= Callback.o \
		  CallbackI.o

COBJS		= Client.o

SOBJS		= Server.o

SSOBJS		= SessionServer.o \
		  SessionI.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp) \
		  $(SSOBJS:.o=.cpp)

SLICE_SRCS	= Callback.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) -lGlacier2 $(LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(LIBS)

$(SESSION_SERVER): $(SSOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(SSOBJS) -lGlacier2 $(LIBS)

include .depend
