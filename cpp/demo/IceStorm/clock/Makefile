# **********************************************************************
#
# Copyright (c) 2001
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

top_srcdir	= ../../..

PUBLISHER	= publisher
SUBSCRIBER	= subscriber

TARGETS		= $(PUBLISHER) $(SUBSCRIBER)

OBJS		= Clock.o

POBJS		= Publisher.o

SOBJS		= Subscriber.o \
		  ClockI.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(POBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

SLICE_SRCS	= Clock.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)
LIBS		:= -lIceStorm $(LIBS)

$(PUBLISHER): $(OBJS) $(POBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(POBJS) $(LIBS)

$(SUBSCRIBER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(LIBS)

Clock.h Clock.cpp: Clock.ice $(SLICE2CPP)
	rm -f Clock.h Clock.cpp
	$(SLICE2CPP) Clock.ice

clean::
	rm -f Clock.h Clock.cpp

include .depend
