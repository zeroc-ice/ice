# **********************************************************************
#
# Copyright (c) 2001
# MutableRealms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

top_srcdir	= ../..

SERVER		= $(top_srcdir)/bin/icewall

TARGETS		= $(SERVER)

SOBJS		= Router.o \
		  Server.o

SRCS		= $(SOBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS)

$(SERVER): $(SOBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(SOBJS) $(LIBS)

include .depend
