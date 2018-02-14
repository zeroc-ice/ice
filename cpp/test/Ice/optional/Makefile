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
SERVERAMD	= $(call mktestname,serveramd)

TARGETS		= $(CLIENT) $(SERVER) $(SERVERAMD)

SLICE_OBJS	= Test.o \
		  TestAMD.o

COBJS		= Test.o \
		  TestI.o \
		  Client.o \
		  AllTests.o

SOBJS		= Test.o \
		  TestI.o \
		  Server.o

SAMDOBJS	= TestAMD.o \
		  TestAMDI.o \
		  ServerAMD.o

OBJS		= $(COBJS) \
		  $(SOBJS) \
		  $(SAMDOBJS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)

$(CLIENT): $(COBJS)
	rm -f $@
	$(call mktest,$@,$(COBJS) $(LIBS))

$(SERVER): $(SOBJS)
	rm -f $@
	$(call mktest,$@,$(SOBJS) $(LIBS))

$(SERVERAMD): $(SAMDOBJS)
	rm -f $@
	$(call mktest,$@,$(SAMDOBJS) $(LIBS))
