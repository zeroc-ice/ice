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

SLICE_OBJS	= Key.o

OBJS		= $(SLICE_OBJS) \
		  Client.o

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)
SLICE2CPPFLAGS	:= --underscore --stream $(SLICE2CPPFLAGS)

$(CLIENT): $(OBJS)
	rm -f $@
	$(call mktest,$@,$(OBJS),$(LIBS))
