# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice Touch is licensed to you under the terms described in the
# ICE_TOUCH_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= $(call mktestname,client)

TARGETS		= $(CLIENT)

SLICE_OBJS	= Test.o

COBJS		= Client.o \
		  AllTests.o \
		  $(SLICE_OBJS)

OBJS		= $(COBJS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)
LIBS 		:= -lGlacier2$(libsuffix) -lIceStorm$(libsuffix) -lIceGrid$(libsuffix) $(LIBS)

$(CLIENT): $(COBJS)
	rm -f $@
	$(call mktest,$@,$(COBJS),$(LIBS))
