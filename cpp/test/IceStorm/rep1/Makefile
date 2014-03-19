# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

PUBLISHER	= publisher
SUBSCRIBER	= subscriber
SUB		= sub

TARGETS		= $(PUBLISHER) $(SUBSCRIBER) $(SUB)

OBJS		= Single.o

POBJS		= Publisher.o

SOBJS		= Subscriber.o

SUB_OBJS	= Sub.o

SRCS		= $(SUB_OBJS:.o=.cpp) \
		  $(OBJS:.o=.cpp) \
		  $(POBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

SLICE_SRCS	= Single.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)
LIBS		:= -lIceStorm $(LIBS)

$(PUBLISHER): $(OBJS) $(POBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(POBJS) $(LIBS)

$(SUBSCRIBER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(LIBS)

$(SUB): $(OBJS) $(SUB_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SUB_OBJS) $(LIBS)

clean::
	-rm -f build.txt
	-rm -rf 0.db/*
	-rm -rf 1.db/*
	-rm -rf 2.db/*

include .depend
