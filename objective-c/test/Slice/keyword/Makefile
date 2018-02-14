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

SLICE_OBJS	= Key.o Inherit.o

OBJS		= $(SLICE_OBJS) \
			  Client.o

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I.. -I../../include $(CPPFLAGS)

$(CLIENT): $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(OBJS) $(TEST_LIBS)
