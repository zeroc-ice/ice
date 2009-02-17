# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client
SERVER		= server

TARGETS		= $(CLIENT) $(SERVER) $(SERVERAMD) $(COLLOCATED)

COBJS		= A.o \
		  B.o \
		  Client.o \
		  AllTests.o

SOBJS		= A.o \
		  B.o \
		  AI.o \
		  BI.o \
		  Server.o


SRCS		= $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)


SLICE_SRCS	= A.ice B.ice


#
# the long allTests function crashes when built with Sun CC 5.9 on x86 with -O,
# inlining and -KPIC; work-around: we disable PIC
#
GENPIC          = no


include $(top_srcdir)/config/Make.rules

SLICE2CPPFLAGS	:= -I. $(SLICE2CPPFLAGS)

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)

$(CLIENT): $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(COBJS) $(LIBS)

$(SERVER): $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(SOBJS) $(LIBS)

include .depend
