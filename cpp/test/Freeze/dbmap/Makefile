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
                  ByteIntMapXML.o \
                  ByteIntMapBinary.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(DB_FLAGS) $(CPPFLAGS)

$(CLIENT): $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) -lFreeze -lIceXML $(LIBS) $(DB_LIBS)

ByteIntMapXML.h ByteIntMapXML.cpp: $(SLICE2FREEZE)
	rm -f ByteIntMapXML.h ByteIntMapXML.cpp
	$(SLICE2FREEZE) --dict ByteIntMapXML,byte,int ByteIntMapXML

ByteIntMapBinary.h ByteIntMapBinary.cpp: $(SLICE2FREEZE)
	rm -f ByteIntMapBinary.h ByteIntMapBinary.cpp
	$(SLICE2FREEZE) --binary --dict ByteIntMapBinary,byte,int ByteIntMapBinary

clean::
	rm -f ByteIntMapXML.h ByteIntMapXML.cpp
	rm -f ByteIntMapBinary.h ByteIntMapBinary.cpp

clean::
	rm -f db/xml db/log.*

include .depend
