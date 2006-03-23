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
		  Worker_Thread.o

SOBJS		= server.o \
		  Roundtrip.o \
		  Worker_Thread.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I$(TAO_HOME)/include -I$(ICE_HOME)/include $(CPPFLAGS)
CXXFLAGS	:= -O3 -pthread -Wall -D_REENTRANT
LDFLAGS		:=  -L$(TAO_HOME)/lib -L$(ICE_HOME)/lib
LIBS		:= -lTAO_Messaging -lTAO_Strategies -lTAO_PortableServer -lIceUtil -lpthread

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(LIBS)

TestS.cpp: Test.idl
	$(TAO_HOME)/bin/tao_idl -Ge 1 -GC Test.idl

include .depend
