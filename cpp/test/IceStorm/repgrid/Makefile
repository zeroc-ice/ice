# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client

TARGETS		= $(CLIENT)

SLICE_OBJS	= Single.o

OBJS		= $(SLICE_OBJS) \
		  Client.o

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)
LIBS		:= -lIceStorm $(LIBS)

$(CLIENT): $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(OBJS) $(LIBS)

clean::
	-rm -f build.txt
	-rm -rf db/node/* db/registry/* db/replica-1/*
