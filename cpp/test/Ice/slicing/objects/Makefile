# **********************************************************************
#
# Copyright (c) 2003
# ZeroC, Inc.
# Billerica, MA, USA
#
# All Rights Reserved.
#
# Ice is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by
# the Free Software Foundation.
#
# **********************************************************************

top_srcdir	= ../../../..

CLIENT		= client
SERVER		= server

TARGETS		= $(CLIENT) $(SERVER)

COBJS		= ClientTest.o \
		  ClientPrivate.o \
		  Client.o \
		  AllTests.o \
		  Forward.o

SOBJS		= ClientTest.o \
    		  ServerTest.o \
		  TestI.o \
		  Server.o \
		  Forward.o

SRCS		= $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

SLICE_SRCS	= ClientTest.ice ClientPrivate.ice ServerTest.ice Forward.ice

include $(top_srcdir)/config/Make.rules

ICECPPFLAGS	:= -I. $(ICECPPFLAGS)

CPPFLAGS	:= -I. -I../../../include $(CPPFLAGS)

$(CLIENT): $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(COBJS) $(LIBS)

$(SERVER): $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(SOBJS) $(LIBS)

include .depend
