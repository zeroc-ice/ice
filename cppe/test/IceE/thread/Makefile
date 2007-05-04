# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client

TARGETS		= $(CLIENT)

OBJS		= TestBase.o \
		  CreateTest.o \
		  AliveTest.o \
		  StartTest.o \
		  RecMutexTest.o \
		  StaticMutexTest.o \
		  MutexTest.o \
		  MonitorMutexTest.o \
		  MonitorRecMutexTest.o \
		  TestSuite.o \
		  Client.o


SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)

$(CLIENT): $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(TESTLIBS)

include .depend
