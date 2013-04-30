# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../../..

SERVER		= $(call mktestname,server)

TARGETS		= $(SERVER)

SOBJS		= Test.o \
		  Types.o \
		  TestI.o \
		  Server.o

SRCS		= $(SOBJS:.o=.cpp)

SLICE_SRCS	= Test.ice Types.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../../include $(CPPFLAGS)

SLICE2CPPFLAGS  := --checksum $(SLICE2CPPFLAGS)

$(SERVER): $(SOBJS)
	rm -f $@
	$(call mktest,$@,$(SOBJS),$(LIBS))

include .depend
