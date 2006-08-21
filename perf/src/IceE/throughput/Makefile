# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

ICEE_USE_ARRAY_MAPPING = yes

CLIENT		= client
SERVER		= server

TARGETS		= $(CLIENT) $(SERVER)

OBJS		= Throughput.o

COBJS		= Client.o

SOBJS		= Server.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

SLICE_SRCS	= Throughput.ice

include $(top_srcdir)/config/Make.rules
include $(top_srcdir)/config/Make.icee.rules

ifeq ($(ICEE_USE_ARRAY_MAPPING),yes)
	CPPFLAGS := -DICEE_USE_ARRAY_MAPPING $(CPPFLAGS)
	SLICE2CPPFLAGS := -DICEE_USE_ARRAY_MAPPING $(SLICE2CPPFLAGS)
endif

CPPFLAGS	:= -D_ICEE -I. -I$(top_srcdir)/include $(ICE_CPPFLAGS) $(CPPFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(ICE_LIBS) 

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(ICE_LIBS)

include .depend
