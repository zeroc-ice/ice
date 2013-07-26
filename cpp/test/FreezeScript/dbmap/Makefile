# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= makedb

TARGETS		= $(CLIENT)

OBJS		= TestOld.o \
                  makedb.o \
                  IntSMap.o

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	= TestOld.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)

$(CLIENT): $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(DB_RPATH_LINK) -lFreeze $(LIBS)

# The slice2freeze rules are structured like this to avoid issues with
# parallel make.
IntSMap.h: IntSMap.cpp
IntSMap.cpp: TestOld.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f IntSMap.h IntSMap.cpp
	$(SLICE2FREEZE) -I. --dict IntSMap,int,::Test::S IntSMap TestOld.ice

clean::
	-rm -f IntSMap.h IntSMap.cpp
	-rm -rf db/* db_init db_check db_tmp

include .depend
