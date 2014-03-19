# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

PUBLISHER	= publisher
SUBSCRIBER	= subscriber
NOTIFIER	= notifier

TARGETS		= $(PUBLISHER) $(SUBSCRIBER) $(NOTIFIER)

OBJS		= Perf.o

POBJS		= Publisher.o \
		  Sync.o

SOBJS		= Subscriber.o \
		  PerfI.o

NOBJS		= Notifier.o \
		  Sync.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(POBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

SLICE_SRCS	= Perf.ice \
		  Sync.ice

include $(top_srcdir)/config/Make.rules
include $(top_srcdir)/config/Make.ice.rules

CPPFLAGS	:= -I. $(ICE_CPPFLAGS) $(CPPFLAGS)
LIBS		:= -lIceStorm $(ICE_LIBS) $(LIBS)

$(PUBLISHER): $(OBJS) $(POBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(POBJS) $(LIBS)

$(SUBSCRIBER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(LIBS)

$(NOTIFIER): $(NOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(NOBJS) $(LIBS)

include .depend
