# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT1		= client1
CLIENT2		= client2
SERVER		= server

TARGETS		= $(CLIENT1) $(CLIENT2) $(SERVER)

OBJS		= Greet.o

C1OBJS		= Client.o \
		  ClientWithConverter.o \
		  StringConverterI.o

C2OBJS		= Client.o \
		  ClientWithoutConverter.o

SOBJS		= GreetI.o \
		  Server.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

SLICE_SRCS	= Greet.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)

$(CLIENT1): $(OBJS) $(C1OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(C1OBJS) $(LIBS)

$(CLIENT2): $(OBJS) $(C2OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(C2OBJS) $(LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(LIBS)

include .depend
