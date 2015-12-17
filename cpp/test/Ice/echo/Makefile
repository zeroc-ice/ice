# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

SERVER		= $(call mktestname,server)

TARGETS		= $(SERVER)

SLICE_OBJS	= Test.o

OBJS		= $(SLICE_OBJS) \
		  BlobjectI.o \
		  Server.o

SRCS		= $(SOBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)

LINKWITH	:= -lTestCommon $(LIBS)

$(SERVER): $(OBJS)
	rm -f $@
	$(call mktest,$@,$(OBJS),$(LINKWITH))
