# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client
SERVER		= server

TARGETS		= $(CLIENT) $(SERVER) $(COLLOCATED)

OBJS		= Casino.o

COBJS		= Client.o \

SOBJS		= CasinoStore.o \
                  BankI.o \
                  BetI.o \
                  PlayerI.o \
                  BetResolver.o \
		  Server.o


SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

SLICE_SRCS	= Casino.ice \
                  CasinoStore.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)

SLICE2CPPFLAGS	:= -I. $(SLICE2CPPFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(DB_RPATH_LINK) -lFreeze $(LIBS)

cleandb::
	-rm -f db/__catalog db/__catalogIndexLIst db/bet db/bank db/player
	-rm -f db/log.*	

clean:: cleandb
	-rm -f db/__catalog db/bet db/bank db/player
	-rm -f db/log.*

include .depend
