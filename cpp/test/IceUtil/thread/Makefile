# **********************************************************************
#
# Copyright (c) 2002
# ZeroC, Inc.
# Billerica, MA, USA
#
# All Rights Reserved.
#
# Ice is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by
# the Free Software Foundation.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client

TARGETS		= $(CLIENT)

OBJS		= TestBase.o \
		  CreateTest.o \
		  DetachTest.o \
		  StartTest.o \
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
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(BASELIBS)

include .depend
