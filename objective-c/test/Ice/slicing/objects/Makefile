# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../../..

CLIENT		= client
SERVER		= server

TARGETS		= $(CLIENT) $(SERVER)

SLICE_OBJS	= SlicingObjectsTestClient.o \
		  SlicingObjectsForwardClient.o \
		  SlicingObjectsTestServer.o \
		  SlicingObjectsForwardServer.o

COBJS		= SlicingObjectsTestClient.o \
		  SlicingObjectsForwardClient.o \
		  Client.o \
		  AllTests.o

SOBJS		= SlicingObjectsTestServer.o \
		  SlicingObjectsForwardServer.o \
		  TestI.o \
		  Server.o

OBJS		= $(COBJS) $(SOBJS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../.. -I../../../include $(CPPFLAGS)
SLICE2OBJCFLAGS := -I. $(SLICE2OBJCFLAGS)

$(CLIENT): $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(COBJS) $(TEST_LIBS)

$(SERVER): $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(SOBJS) $(TEST_LIBS)
