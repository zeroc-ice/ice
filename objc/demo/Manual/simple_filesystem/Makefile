# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice Touch is licensed to you under the terms described in the
# ICE_TOUCH_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client
SERVER		= server

TARGETS		= $(CLIENT) $(SERVER)

SLICE_OBJS	= Filesystem.o

COBJS		= Client.o

SOBJS		= Server.o \
			  DirectoryI.o \
			  FileI.o

OBJS		= $(SLICE_OBJS) $(COBJS) $(SOBJS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)

$(CLIENT): $(SLICE_OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(SLICE_OBJS) $(COBJS) $(LIBS)

$(SERVER): $(SLICE_OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(SLICE_OBJS) $(SOBJS) $(LIBS)
