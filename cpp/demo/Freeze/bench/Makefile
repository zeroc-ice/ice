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

CLIENT		= client

TARGETS		= $(CLIENT) $(SERVER) $(COLLOCATED)

OBJS		= Client.o BenchTypes.o Test.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I$(DB_HOME)/include $(CPPFLAGS)
LDFLAGS		:= $(LDFLAGS) -L$(DB_HOME)/lib

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) -lFreeze $(LIBS) -ldb

Test.h Test.cpp: Test.ice $(SLICE2CPP)
	rm -f Test.h Test.cpp
	$(SLICE2CPP) -I$(slicedir) Test.ice

clean::
	rm -f Test.h Test.cpp

BenchTypes.h BenchTypes.cpp: Test.ice $(SLICE2FREEZE)
	rm -f BenchTypes.h BenchTypes.cpp
	$(SLICE2FREEZE) -I$(slicedir) --dict IntIntMap,int,int --dict Struct1Struct2Map,Struct1,Struct2 \
	--dict Struct1Class1Map,Struct1,Class1 \
	--dict Struct1ObjectMap,Struct1,Object BenchTypes Test.ice

clean::
	rm -f BenchTypes.h BenchTypes.cpp

include .depend
