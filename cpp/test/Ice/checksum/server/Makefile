# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../../..

SERVER		= $(call mktestname,server)

TARGETS		= $(SERVER)

SLICE_OBJS	= Test.o \
		  Types.o \

SOBJS		= $(SLICE_OBJS) \
		  TestI.o \
		  Server.o

OBJS 		= $(SOBJS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../../include $(CPPFLAGS)

SLICE2CPPFLAGS  := --checksum $(SLICE2CPPFLAGS)

$(SERVER): $(SOBJS)
	rm -f $@
	$(call mktest,$@,$(SOBJS),$(LIBS))
