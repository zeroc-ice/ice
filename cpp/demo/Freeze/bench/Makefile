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

OBJS		= Client.o BenchTypes.o Test.o

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	= Test.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(DB_FLAGS) $(CPPFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) -lFreeze $(LIBS)

BenchTypes.h BenchTypes.cpp: Test.ice $(SLICE2FREEZE)
	rm -f BenchTypes.h BenchTypes.cpp
	$(SLICE2FREEZE) -I$(slicedir) --dict IntIntMap,int,int --dict Struct1Struct2Map,Struct1,Struct2 \
	--dict Struct1Class1Map,Struct1,Class1 \
	--dict Struct1ObjectMap,Struct1,Object BenchTypes Test.ice

clean::
	rm -f BenchTypes.h BenchTypes.cpp

include .depend
