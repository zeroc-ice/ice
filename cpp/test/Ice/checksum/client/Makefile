# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../../..

CLIENT		= client

TARGETS		= $(CLIENT)

COBJS		= Test.o \
		  Types.o \
		  Client.o \
		  AllTests.o

SRCS		= $(COBJS:.o=.cpp)

SLICE_SRCS	= Test.ice Types.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../../include $(CPPFLAGS)

SLICE2CPPFLAGS  := --checksum $(SLICE2CPPFLAGS)

$(CLIENT): $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(COBJS) $(LIBS)

include .depend
