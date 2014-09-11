# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

SERVER		= server

TARGETS		= $(SERVER)

OBJS		= Server.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

$(SERVER): $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean::
	-rm -rf db/node db/registry db/replica-*

include .depend
