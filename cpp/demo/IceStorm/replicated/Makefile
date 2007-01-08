# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

PUBLISHER	= publisher
SUBSCRIBER	= subscriber

TARGETS		= $(PUBLISHER) $(SUBSCRIBER)

OBJS		= Clock.o

POBJS		= Publisher.o

SOBJS		= Subscriber.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(POBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

SLICE_SRCS	= Clock.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)
LIBS		:= -lIceStorm -lIceGrid -lGlacier2 $(LIBS)

$(PUBLISHER): $(OBJS) $(POBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(POBJS) $(LIBS)

$(SUBSCRIBER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(LIBS)

include .depend
