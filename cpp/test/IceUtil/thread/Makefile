# **********************************************************************
#
# Copyright (c) 2001
# Mutable Realms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client

TARGETS		= $(CLIENT)

OBJS		= TestBase.o \
		  CreateTest.o \
		  RWRecMutexTest.o \
		  RecMutexTest.o \
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
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(BASELIBS)

include .depend
