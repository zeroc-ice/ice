# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client
SERVER		= server
COLLOCATED	= collocated

TARGETS		= $(CLIENT) $(SERVER) $(COLLOCATED)

COBJS		= Test.o \
		  Wstring.o \
		  Client.o \
		  AllTests.o \
		  MyByteSeq.o \
		  StringConverterI.o

SOBJS		= Test.o \
		  TestI.o \
		  Wstring.o \
		  WstringI.o \
		  Server.o \
		  MyByteSeq.o \
		  StringConverterI.o

COLOBJS		= Test.o \
		  TestI.o \
		  Wstring.o \
		  WstringI.o \
		  Collocated.o \
		  AllTests.o \
		  MyByteSeq.o \
		  StringConverterI.o

SRCS		= $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp) \
		  $(COLOBJS:.o=.cpp)

SLICE_SRCS	= Test.ice Wstring.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	 := -I. -I../../include $(CPPFLAGS)
SLICE2CPPEFLAGS  := $(SLICE2CPPFLAGS)

$(CLIENT): $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(COBJS) $(TESTLIBS)

$(SERVER): $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(SOBJS) $(TESTLIBS)

$(COLLOCATED): $(COLOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(COLOBJS) $(TESTLIBS)

include .depend
