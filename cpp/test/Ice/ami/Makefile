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
COLLOCATED	= $(call mktestname,collocated)

TARGETS		= $(CLIENT) $(SERVER) $(COLLOCATED)

SLICE_OBJS	= Test.o \

COBJS		= $(SLICE_OBJS) \
		  Client.o \
		  AllTests.o

SOBJS		= $(SLICE_OBJS) \
		  TestI.o \
		  Server.o

COLOBJS		= $(SLICE_OBJS) \
		  TestI.o \
		  Collocated.o \
		  AllTests.o \

OBJS		= $(COBJS) \
		  $(SOBJS) \
		  $(COLOBJS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)

$(CLIENT): $(COBJS)
	rm -f $@
	$(call mktest,$@,$(COBJS),$(LIBS))

$(SERVER): $(SOBJS)
	rm -f $@
	$(call mktest,$@,$(SOBJS),$(LIBS))

$(COLLOCATED): $(COLOBJS)
	rm -f $@
	$(call mktest,$@,$(COLOBJS),$(LIBS))
