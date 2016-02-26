# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= $(call mktestname,client)
SERVER		= $(call mktestname,server)

TARGETS		= $(CLIENT) $(SERVER)


SLICE_OBJS	= Test.o ServerTypes.o ClientTypes.o

COBJS		= Test.o \
		  ClientTypes.o \
		  Client.o \
		  AllTests.o
		  
SOBJS		= Test.o \
		  ServerTypes.o \
		  Server.o \
		  TestI.o

OBJS		= $(COBJS) $(SOBJS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)


SLICE2CPPFLAGS  := --checksum $(SLICE2CPPFLAGS)

$(CLIENT): $(COBJS)
	rm -f $@
	$(call mktest,$@,$(COBJS),$(TEST_LIBS))

$(SERVER): $(SOBJS)
	rm -f $@
	$(call mktest,$@,$(SOBJS),$(TEST_LIBS))
