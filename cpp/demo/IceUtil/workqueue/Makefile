# **********************************************************************
#
# Copyright (c) 2001
# MutableRealms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= workqueue

TARGETS		= $(CLIENT)

OBJS		= WorkQueue.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)

$(CLIENT): $(OBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(BASELIBS)

include .depend
