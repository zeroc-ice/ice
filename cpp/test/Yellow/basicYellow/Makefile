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

CLIENT		= client

TARGETS		= $(CLIENT)

OBJS		= Client.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)
LDFLAGS		:= $(LDFLAGS)

$(CLIENT): $(OBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) -lYellow $(LIBS)

include .depend
