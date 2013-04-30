# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= $(call mktestname,client)
SERVER		= $(call mktestname,server)

TARGETS		= $(CLIENT) $(SERVER)

OBJS		= Test.o \

COBJS		= Client.o \
		  AllTests.o

SOBJS		= TestI.o \
		  Server.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

SLICE_SRCS	= Test.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)
SLICE2CPPFLAGS	:= --stream $(SLICE2CPPFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(call mktest,$@,$(OBJS) $(COBJS),$(LIBS))

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(call mktest,$@,$(OBJS) $(SOBJS),$(LIBS))

include .depend
