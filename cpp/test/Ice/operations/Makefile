# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client
SERVER		= server
SERVERAMD	= serveramd
COLLOCATED	= collocated

TARGETS		= $(CLIENT) $(SERVER) $(SERVERAMD) $(COLLOCATED)

COBJS		= Test.o \
		  Client.o \
		  AllTests.o \
		  Twoways.o \
		  TwowaysAMI.o \
		  BatchOneways.o

SOBJS		= Test.o \
		  TestI.o \
		  Server.o

SAMDOBJS	= TestAMD.o \
		  TestAMDI.o \
		  ServerAMD.o

COLOBJS		= Test.o \
		  TestI.o \
		  Collocated.o \
		  AllTests.o \
		  Twoways.o \
		  TwowaysAMI.o \
		  BatchOneways.o

SRCS		= $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp) \
		  $(SAMDOBJS:.o=.cpp) \
		  $(COLOBJS:.o=.cpp)

SLICE_SRCS	= Test.ice TestAMD.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)

$(CLIENT): $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(COBJS) $(LIBS)

$(SERVER): $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(SOBJS) $(LIBS)

$(SERVERAMD): $(SAMDOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(SAMDOBJS) $(LIBS)

$(COLLOCATED): $(COLOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(COLOBJS) $(LIBS)

include .depend
