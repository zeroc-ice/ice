# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

CLIENT		= client
SERVER		= server

TARGETS		= $(CLIENT) $(SERVER)

OBJS		= TestC.o \
		  TestS.o

COBJS		= client.o \
		  Roundtrip_Handler.o \
		  WorkerThread.o

SOBJS		= server.o \
		  Roundtrip.o \
		  WorkerThread.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules
include $(top_srcdir)/config/Make.tao.rules

LIBS		:= -lTAO_Messaging -lTAO_Strategies -lTAO_PortableServer -lpthread

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(ICE_LIBS) $(LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(ICE_LIBS) $(LIBS)

TestS.cpp TestC.cpp: Test.idl
	$(TAO_ROOT)/bin/tao_idl -Ge 1 -GC Test.idl

include .depend
