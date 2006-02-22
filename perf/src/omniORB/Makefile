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

OBJS		= pingSK.o

COBJS		= Client.o

SOBJS		= Server.o

SRCS		= $(OBJS:.o=.cc) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I$(OMNIORB_HOME)/include -D__OSVERSION__=2 -D__linux__ \
		   -D__x86__ -D__OMNIORB4__ -D_REENTRANT $(CPPFLAGS)
CXXFLAGS	:= -O3 -Wall -Wno-unused -fexceptions
LDFLAGS		:=  -L$(OMNIORB_HOME)/lib
LIBS		:= -lomniORB4 -lomnithread -lpthread

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(LIBS)

pingSK.cc: ping.idl
	$(OMNIORB_HOME)/bin/omniidl  -bcxx -Wba ping.idl

include .depend
