# **********************************************************************
#
# Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client
SERVER		= server
SERVERAMD	= serveramd
COLLOCATED	= collocated

TARGETS		= $(CLIENT) $(SERVER) $(SERVERAMD) $(COLLOCATED)

OBJS		= ExceptionsI.o

COBJS		= Test.o \
		  Client.o \
		  AllTests.o

SOBJS		= Test.o \
		  TestI.o \
		  Server.o

SAMDOBJS	= TestAMD.o \
		  TestAMDI.o \
		  ServerAMD.o

COLOBJS		= Test.o \
		  TestI.o \
		  Collocated.o \
		  AllTests.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp) \
		  $(SAMDOBJS:.o=.cpp) \
		  $(COLOBJS:.o=.cpp)

SLICE_SRCS	= Test.ice TestAMD.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)

$(CLIENT): $(COBJS) $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(COBJS) $(OBJS) $(LIBS)

$(SERVER): $(SOBJS) $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(SOBJS) $(OBJS) $(LIBS)

$(SERVERAMD): $(SAMDOBJS) $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(SAMDOBJS) $(OBJS) $(LIBS)

$(COLLOCATED): $(COLOBJS) $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(COLOBJS) $(OBJS) $(LIBS)

include .depend
