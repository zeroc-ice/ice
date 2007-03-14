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

TARGETS		= $(CLIENT)

OBJS		= Client.o IntLongMap.o

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	=

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(DB_RPATH_LINK) -lFreeze $(LIBS)

IntLongMap.h IntLongMap.cpp: $(SLICE2FREEZE)
	rm -f IntLongMap.h IntLongMap.cpp
	$(SLICE2FREEZE) -I$(slicedir) --dict IntLongMap,int,long IntLongMap

clean::
	rm -f IntLongMap.h IntLongMap.cpp
	rm -f db/data/IntLongMap db/data/__catalog db/logs/log.* db/__db.* 
	rm -rf hotbackup hotbackup.1

include .depend
