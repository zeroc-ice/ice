# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client
CLIENTF		= clientFail

TARGETS		= $(CLIENT) $(CLIENTF)

OBJS		= Client.o

OBJFS		= ClientFail.o

SRCS		= $(OBJS:.o=.cpp) $(OBJFS:.o=.cpp)

GENPIC          = no

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)

$(CLIENT): $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(DB_RPATH_LINK) -lFreeze $(LIBS)

$(CLIENTF): $(OBJFS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJFS) $(DB_RPATH_LINK) -lFreeze $(LIBS)


clean::
	-rm -rf db/*

include .depend
