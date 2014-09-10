# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../../..

SERVER		= $(call mktestname,server)

TARGETS		= $(SERVER)

SLICE_OBJS	= Test.o Types.o

OBJS		= $(SLICE_OBJS) \
		  TestI.o \
		  Server.o

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../../include $(CPPFLAGS)

SLICE2CPPFLAGS  := --checksum $(SLICE2CPPFLAGS)

$(SERVER): $(OBJS)
	rm -f $@
	$(call mktest,$@,$(OBJS),$(LIBS))
