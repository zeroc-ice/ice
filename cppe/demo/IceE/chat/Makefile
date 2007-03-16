# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client

TARGETS		= $(CLIENT) 

OBJS		= Client.o \
		  Chat.o \
		  PingThread.o \
		  Router.o \
		  Session.o

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	= Chat.ice \
		  Router.ice \
		  Session.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)

SLICE2CPPEFLAGS	:= -I. --ice $(SLICE2CPPEFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS)

include .depend
