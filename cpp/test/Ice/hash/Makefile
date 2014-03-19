# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= $(call mktestname,client)

TARGETS		= $(CLIENT)

COBJS		= Client.o

OBJS		= Test.o

SRCS		= $(COBJS:.o=.cpp) \
		  $(OBJS:.o=.cpp)

SLICE_SRCS	= Test.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS) $(NO_DEPRECATED_FLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(call mktest,$@,$(COBJS),$(OBJS) $(LIBS))

include .depend
