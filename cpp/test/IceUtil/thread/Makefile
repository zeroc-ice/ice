# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= $(call mktestname,client)

TARGETS		= $(CLIENT)

OBJS		= TestBase.o \
		  CreateTest.o \
		  AliveTest.o \
		  StartTest.o \
		  SleepTest.o \
		  RecMutexTest.o \
		  MutexTest.o \
		  MonitorMutexTest.o \
		  MonitorRecMutexTest.o \
		  CountDownLatchTest.o \
		  TestSuite.o \
		  Client.o

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)

$(CLIENT): $(OBJS)
	rm -f $@
	$(call mktest,$@,$(OBJS),$(BASELIBS))
