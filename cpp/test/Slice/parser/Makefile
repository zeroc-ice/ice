# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

TARGETS		= $(OBJS)

OBJS		= CircularA.o \
		  CircularB.o

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	= CircularA.ice \
		  CircularB.ice

include $(top_srcdir)/config/Make.rules

SLICE2CPPFLAGS	:= -I. $(SLICE2CPPFLAGS)
CPPFLAGS	:= -I. $(CPPFLAGS)

include .depend
