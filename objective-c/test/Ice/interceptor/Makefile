# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client

TARGETS		= $(CLIENT)

SLICE_OBJS	= InterceptorTest.o

COBJS		= Client.o \
		  InterceptorI.o \
		  MyObjectI.o \

OBJS		= $(COBJS) $(SLICE_OBJS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I.. -I../../include $(CPPFLAGS)

$(CLIENT): $(COBJS) $(SLICE_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(COBJS) $(SLICE_OBJS) $(TEST_LIBS)
