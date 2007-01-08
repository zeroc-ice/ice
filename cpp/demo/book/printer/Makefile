# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client
SERVER		= server

TARGETS		= $(CLIENT) $(SERVER)

SLICE_SRCS	= Printer.ice

OBJS		= $(SLICE_SRCS:.ice=.o)

COBJS		= Client.o

SOBJS		= Server.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

SLICE_GEN	= $(SLICE_SRCS:.ice=.cpp) $(SLICE_SRCS:.ice=.h)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(LIBS)

$(SLICE_GEN): $(SLICE_SRCS) $(SLICE2CPP)
	rm -f $(SLICE_GEN)
	$(SLICE2CPP) $(SLICE_SRCS)

test::

clean::
	rm -f $(SLICE_GEN)
	rm -fr Debug Release *.opt *.plg *.ncb *.vcproj

include .depend
