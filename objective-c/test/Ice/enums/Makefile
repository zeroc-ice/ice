# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client
SERVER		= server

TARGETS		= $(CLIENT) $(SERVER)

SLICE_OBJS	= EnumTest.o

COBJS		= Client.o \
		  AllTests.o

SOBJS		= TestI.o \
		  Server.o

OBJS		= $(COBJS) $(SOBJS) $(SLICE_OBJS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I.. -I../../include $(CPPFLAGS)

$(CLIENT): $(COBJS) $(SLICE_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(COBJS) $(SLICE_OBJS) $(TEST_LIBS)

$(SERVER): $(SOBJS) $(SLICE_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(SOBJS) $(SLICE_OBJS) $(TEST_LIBS)
