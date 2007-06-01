# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

WQ		= workqueue
MATCH		= match

TARGETS		= $(WQ) $(MATCH)

OBJS		= WorkQueue.o \
		  Match.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)

$(WQ): WorkQueue.o
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ WorkQueue.o $(BASELIBS)

$(MATCH): Match.o
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ Match.o $(BASELIBS)

include .depend
