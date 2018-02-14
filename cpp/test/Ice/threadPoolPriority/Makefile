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
SERVER		= $(call mktestname,server)
SERVERCUSTOM	= $(call mktestname,servercustom)

TARGETS		= $(CLIENT) $(SERVER) $(SERVERCUSTOM)

SLICE_OBJS	= Test.o

COBJS		= $(SLICE_OBJS) \
		  Client.o \
		  AllTests.o

SOBJS		= $(SLICE_OBJS) \
		  TestI.o \
		  Server.o

SCOBJS		= $(SLICE_OBJS) \
		  ServerCustomThreadPool.o \
		  TestI.o

OBJS		= $(COBJS) \
		  $(SOBJS) \
		  $(SCOBJS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)

$(CLIENT): $(COBJS)
	rm -f $@
	$(call mktest,$@,$(COBJS),$(LIBS))

$(SERVER): $(SOBJS)
	rm -f $@
	$(call mktest,$@,$(SOBJS),$(LIBS))

$(SERVERCUSTOM): $(SCOBJS)
	rm -f $@
	$(call mktest,$@,$(SCOBJS),$(LIBS))
