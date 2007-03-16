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

TARGETS		= $(CLIENT) $(SERVER)

OBJS		= Hello.o

COBJS		= Client.o

SOBJS		= Server.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

SLICE_SRCS	= Hello.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ HelloC.o $(COBJS) $(MINLIBS)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(LIBS)

Client.o: Client.cpp
	$(CXX) -c -DICEE_PURE_CLIENT $(CPPFLAGS) $(CXXFLAGS) $<

HelloC.o Hello.o: Hello.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<
	$(CXX) -c -DICEE_PURE_CLIENT -o HelloC.o $(CPPFLAGS) $(CXXFLAGS) $<

include .depend
