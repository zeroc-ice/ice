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

OBJS		= Client.o \
                  ByteIntMap.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(DB_FLAGS) $(CPPFLAGS)

$(CLIENT): $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) -lFreeze $(LIBS) $(DB_LIBS)

ByteIntMap.h ByteIntMap.cpp: $(SLICE2FREEZE)
	rm -f ByteIntMap.h ByteIntMap.cpp
	$(SLICE2FREEZE) --dict ByteIntMap,byte,int ByteIntMap

clean::
	rm -f ByteIntMap.h ByteIntMap.cpp

clean::
	rm -f db/binary db/log.*

include .depend
