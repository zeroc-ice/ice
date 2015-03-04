# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client

TARGETS		= $(CLIENT)

OBJS		= Client.o IntLongMap.o

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	=

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(DB_RPATH_LINK) -lFreeze $(LIBS)

# The slice2freeze rules are structured like this to avoid issues with
# parallel make.
IntLongMap.h: IntLongMap.cpp
IntLongMap.cpp: $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f IntLongMap.h IntLongMap.cpp
	$(SLICE2FREEZE) -I$(slicedir) --dict IntLongMap,int,long IntLongMap

clean::
	-rm -f IntLongMap.h IntLongMap.cpp
	-rm -rf db/__* db/data/* db/logs/* db/__Freeze
	-rm -rf hotbackup hotbackup.1

include .depend
