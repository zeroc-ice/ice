# **********************************************************************
#
# Copyright (c) 2003 - 2004
# ZeroC, Inc.
# North Palm Beach, FL, USA
#
# All Rights Reserved.
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

CPPFLAGS	:= -I. $(DB_FLAGS) $(CPPFLAGS)

$(CLIENT): $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) -lFreeze $(DB_LIBS) $(LIBS)

IntSMap.h IntSMap.cpp: $(SLICE2FREEZE)
	rm -f IntSMap.h IntSMap.cpp
	$(SLICE2FREEZE) --dict IntSMap,int,::S IntSMap TestOld.ice

clean::
	rm -f IntSMap.h IntSMap.cpp

clean::
	rm -f db/*.db db/log.*

include .depend
