# **********************************************************************
#
# Copyright (c) 2002
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
SERVER		= server

TARGETS		= $(CLIENT) $(SERVER)

OBJS		= Test.o

COBJS		= Client.o

SOBJS		= TestI.o \
		  Server.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(DB_FLAGS) $(CPPFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) -lFreeze -lIceXML $(LIBS) $(DB_LIBS)

Test.h Test.cpp: Test.ice $(SLICE2CPP)
	rm -f Test.h Test.cpp
	$(SLICE2CPP) Test.ice

clean::
	rm -f Test.h Test.cpp

include .depend
