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

top_srcdir	= ../../..

CLIENT		= client

TARGETS		= $(CLIENT)

OBJS		= Client.o IntLongMap.o

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	= Test.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(DB_FLAGS) $(CPPFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) -lFreeze $(LIBS) $(DB_LIBS)

IntLongMap.h IntLongMap.cpp: $(SLICE2FREEZE)
	rm -f IntLongMap.h IntLongMap.cpp
	$(SLICE2FREEZE) -I$(slicedir) --dict IntLongMap,int,long IntLongMap

clean::
	rm -f IntLongMap.h IntLongMap.cpp
	rm -f db/data/IntLongMap db/logs/log* db/logs-* full/*.tar.gz incremental/*.tar.gz

include .depend
