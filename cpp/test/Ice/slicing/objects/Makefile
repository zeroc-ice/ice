# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../../..

CLIENT		= $(call mktestname,client)
SERVER		= $(call mktestname,server)
SERVERAMD	= $(call mktestname,serveramd)

TARGETS		= $(CLIENT) $(SERVER) $(SERVERAMD)

COBJS		= Test.o \
		  ClientPrivate.o \
		  Client.o \
		  AllTests.o \
		  Forward.o

SOBJS		= Test.o \
		  ServerPrivate.o \
		  TestI.o \
		  Server.o \
		  Forward.o

SAMDOBJS	= TestAMD.o \
		  ServerPrivateAMD.o \
		  TestAMDI.o \
		  ServerAMD.o \
		  Forward.o

SRCS		= $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp) \
		  $(SAMDOBJS:.o=.cpp)

SLICE_SRCS	= Test.ice TestAMD.ice ClientPrivate.ice ServerPrivate.ice ServerPrivateAMD.ice Forward.ice

#
# the long allTests function crashes when built with Sun CC 5.9 on x86 with -O,
# inlining and -KPIC; work-around: we disable PIC
#
GENPIC          = no

include $(top_srcdir)/config/Make.rules

ICECPPFLAGS	:= -I. $(ICECPPFLAGS)

CPPFLAGS	:= -I. -I../../../include $(CPPFLAGS)

$(CLIENT): $(COBJS)
	rm -f $@
	$(call mktest,$@,$(COBJS),$(LIBS))

$(SERVER): $(SOBJS)
	rm -f $@
	$(call mktest,$@,$(SOBJS),$(LIBS))

$(SERVERAMD): $(SAMDOBJS)
	rm -f $@
	$(call mktest,$@,$(SAMDOBJS),$(LIBS))

include .depend
