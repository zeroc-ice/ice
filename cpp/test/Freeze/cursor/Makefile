# **********************************************************************
#
# Copyright (c) 2001
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

OBJS		= Client.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include -I$(DB_HOME)/include $(CPPFLAGS) 
LDFLAGS		:= $(LDFLAGS) -L$(DB_HOME)/lib

$(CLIENT): $(OBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) -lFreeze -lIceXML $(LIBS) -ldb

include .depend
