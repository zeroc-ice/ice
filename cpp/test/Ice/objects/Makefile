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
TESTLIBNAME	= libTestDerived.a

TARGETS		= $(CLIENT) $(SERVER) $(COLLOCATED) $(TESTLIBNAME)

SLICE_OBJS	= Test.o \
		  Derived.o \
		  DerivedEx.o

COBJS		= TestI.o \
		  Client.o \
		  AllTests.o

SOBJS		= $(SLICE_OBJS) \
		  TestI.o \
		  TestIntfI.o \
		  Server.o

COLOBJS		= $(SLICE_OBJS) \
		  TestI.o \
		  TestIntfI.o \
		  Collocated.o \
		  AllTests.o

OBJS		= $(COBJS) \
		  $(SOBJS) \
		  $(COLOBJS)

include $(top_srcdir)/config/Make.rules

SLICE2CPPFLAGS	:= -I. $(SLICE2CPPFLAGS)
CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)
LDFLAGS		:= -L. $(LDFLAGS)

$(TESTLIBNAME): $(SLICE_OBJS)
	rm -f $@	
	$(call mklib,$@,$(SLICE_OBJS))

$(CLIENT): $(COBJS) $(TESTLIBNAME)
	rm -f $@
	$(call mktest,$@,$(COBJS) -lTestDerived $(LIBS))

$(SERVER): $(SOBJS)
	rm -f $@
	$(call mktest,$@,$(SOBJS) $(LIBS))

$(COLLOCATED): $(COLOBJS)
	rm -f $@
	$(call mktest,$@,$(COLOBJS) $(LIBS))
