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
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) -lFreeze $(LIBS) $(DB_LIBS)

IntSMap.h IntSMap.cpp: $(SLICE2FREEZE)
	rm -f IntSMap.h IntSMap.cpp
	$(SLICE2FREEZE) --dict IntSMap,int,::S IntSMap TestOld.ice

clean::
	rm -f IntSMap.h IntSMap.cpp

clean::
	rm -f db/*.db db/log.*

include .depend
