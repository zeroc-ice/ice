# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client

TARGETS		= $(CLIENT)

OBJS		= Client.o BenchTypes.o Test.o

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	= Test.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(DB_FLAGS) $(CPPFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) -lFreeze $(DB_LIBS) $(LIBS)

BenchTypes.h BenchTypes.cpp: Test.ice $(SLICE2FREEZE)
	rm -f BenchTypes.h BenchTypes.cpp
	$(SLICE2FREEZE) -I$(slicedir) --dict IntIntMap,int,int --dict Struct1Struct2Map,Struct1,Struct2 \
	--dict Struct1Class1Map,Struct1,Class1 \
	--dict Struct1ObjectMap,Struct1,Object BenchTypes Test.ice

clean::
	rm -f BenchTypes.h BenchTypes.cpp

include .depend
