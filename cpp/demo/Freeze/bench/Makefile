# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client

TARGETS		= $(CLIENT)

SLICE_OBJS	= Test.o

OBJS		= $(SLICE_OBJS) \
		  BenchTypes.o \
		  Client.o

all:: BenchTypes.cpp BenchTypes.h

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)

$(CLIENT): $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(OBJS) $(DB_RPATH_LINK) -lFreeze $(LIBS)

# The slice2freeze rules are structured like this to avoid issues with
# parallel make.
BenchTypes.h: BenchTypes.cpp
BenchTypes.cpp: Test.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f BenchTypes.h BenchTypes.cpp
	$(SLICE2FREEZE) -I. -I$(slicedir) --dict Demo::IntIntMap,int,int --dict Demo::Struct1Struct2Map,Demo::Struct1,Demo::Struct2 \
	--dict Demo::Struct1Class1Map,Demo::Struct1,Demo::Class1 \
	--dict Demo::Struct1ObjectMap,Demo::Struct1,Object \
        --dict Demo::IndexedIntIntMap,int,int --dict-index Demo::IndexedIntIntMap \
        --dict Demo::IndexedStruct1Struct2Map,Demo::Struct1,Demo::Struct2 \
        --dict-index Demo::IndexedStruct1Struct2Map,s,case-insensitive \
        --dict-index Demo::IndexedStruct1Struct2Map,s1 \
        --dict Demo::IndexedStruct1Class1Map,Demo::Struct1,Demo::Class1 \
        --dict-index Demo::IndexedStruct1Class1Map,s,case-sensitive \
        BenchTypes Test.ice

clean::
	-rm -rf db/*
	-rm -f BenchTypes.h BenchTypes.cpp
