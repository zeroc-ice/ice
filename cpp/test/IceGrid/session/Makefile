# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client

TARGETS		= $(CLIENT)

COBJS		= Client.o \
		  AllTests.o

SRCS		= $(COBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)
LINKWITH	:= -lIce -lIceUtil

$(CLIENT): $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(COBJS) -lIceGrid -lGlacier2 $(LIBS)

clean::
	rm -rf db/node/servers
	rm -f db/registry/*

include .depend
