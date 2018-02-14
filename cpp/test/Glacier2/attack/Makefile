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
SERVER		= server

TARGETS		= $(CLIENT) $(SERVER)

SLICE_OBJS	= Backend.o

COBJS		= $(SLICE_OBJS) \
		  BackendI.o \
		  Client.o

SOBJS		= $(SLICE_OBJS) \
		  BackendI.o \
		  Server.o

OBJS		= $(COBJS) \
		  $(SOBJS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)

$(CLIENT): $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(COBJS) -lGlacier2 $(LIBS)

$(SERVER): $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(SOBJS) $(LIBS)

clean::
	rm -rf passwords