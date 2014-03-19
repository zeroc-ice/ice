# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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

OBJS		= Test.o \

COBJS		= Client.o \
		  AllTests.o

SOBJS		= TestI.o \
		  ServantLocatorI.o \
		  Server.o

COLOBJS		= TestI.o \
		  ServantLocatorI.o \
		  Collocated.o \
		  AllTests.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp) \
		  $(COLOBJS:.o=.cpp)

SLICE_SRCS	= Test.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(call mktest,$@,$(OBJS) $(COBJS),$(LIBS))

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(call mktest,$@,$(OBJS) $(SOBJS),$(LIBS))

$(COLLOCATED): $(OBJS) $(COLOBJS)
	rm -f $@
	$(call mktest,$@,$(OBJS) $(COLOBJS),$(LIBS))

include .depend
