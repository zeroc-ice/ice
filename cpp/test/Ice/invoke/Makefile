# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= $(call mktestname,client)
SERVER		= $(call mktestname,server)

TARGETS		= $(CLIENT) $(SERVER)

COBJS		= Test.o \
		  Client.o \
		  AllTests.o

SOBJS		= Test.o \
		  BlobjectI.o \
		  Server.o

SRCS		= $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

SLICE_SRCS	= Test.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)

$(CLIENT): $(COBJS)
	rm -f $@
	$(call mktest,$@,$(COBJS),$(LIBS))

$(SERVER): $(SOBJS)
	rm -f $@
	$(call mktest,$@,$(SOBJS),$(LIBS))

include .depend
