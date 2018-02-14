# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

SLICE_OBJS	= Single.o

POBJS		= $(SLICE_OBJS) \
		  Publisher.o

SOBJS		= $(SLICE_OBJS) \
		  Subscriber.o

SUB_OBJS	= $(SLICE_OBJS) \
		  Sub.o

OBJS		= $(SUB_OBJS) \
		  $(POBJS) \
		  $(SOBJS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)
LIBS		:= -lIceStorm $(LIBS)

$(PUBLISHER): $(POBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(POBJS) $(LIBS)

$(SUBSCRIBER): $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(SOBJS) $(LIBS)

$(SUB): $(SUB_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(SUB_OBJS) $(LIBS)

clean::
	-rm -f build.txt
	-rm -rf 0.db/*
	-rm -rf 1.db/*
	-rm -rf 2.db/*
