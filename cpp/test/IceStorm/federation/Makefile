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

OBJS		= Event.o

POBJS		= Publisher.o

SOBJS		= Subscriber.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(POBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

SLICE_SRCS	= Event.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)
LIBS		:= -lIceStorm $(LIBS)

$(PUBLISHER): $(OBJS) $(POBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(POBJS) $(LIBS)

$(SUBSCRIBER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(LIBS)

Event.h Event.cpp: Event.ice $(SLICE2CPP)
	rm -f Event.h Event.cpp
	$(SLICE2CPP) Event.ice

clean::
	rm -f Event.h Event.cpp

include .depend
