# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

TARGETS		= $(SLICE_OBJS)

SLICE_OBJS	= CircularA.o \
		  CircularB.o

OBJS		= $(SLICE_OBJS)

include $(top_srcdir)/config/Make.rules

SLICE2CPPFLAGS	:= -I. $(SLICE2CPPFLAGS)
CPPFLAGS	:= -I. $(CPPFLAGS)
