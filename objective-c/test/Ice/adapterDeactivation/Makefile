# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client
SERVER		= server
COLLOCATED	= collocated

TARGETS		= $(CLIENT) $(SERVER) $(COLLOCATED)

SLICE_OBJS	= AdapterDeactivationTest.o

COBJS		= Client.o \
		  AllTests.o

SOBJS		= TestI.o \
		  Server.o

COLOBJS		= Collocated.o \
		  TestI.o \
		  AllTests.o

OBJS		= $(COBJS) $(SOBJS) $(COLOBJS) $(SLICE_OBJS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I.. -I../../include $(CPPFLAGS)

$(CLIENT): $(COBJS) $(SLICE_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(COBJS) $(SLICE_OBJS) $(TEST_LIBS)

$(SERVER): $(SOBJS) $(SLICE_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(SOBJS) $(SLICE_OBJS) $(TEST_LIBS)

$(COLLOCATED): $(COLOBJS) $(SLICE_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(COLOBJS) $(SLICE_OBJS) $(TEST_LIBS)
