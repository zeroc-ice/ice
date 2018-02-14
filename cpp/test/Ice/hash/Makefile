# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= $(call mktestname,client)

TARGETS		= $(CLIENT)

SLICE_OBJS	= Test.o

COBJS		= $(SLICE_OBJS) \
		  Client.o

OBJS		= $(COBJS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS) $(NO_DEPRECATED_FLAGS)

$(CLIENT): $(COBJS)
	rm -f $@
	$(call mktest,$@,$(COBJS) $(LIBS))
