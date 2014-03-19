# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client
SERVER		= server

TARGETS		= $(CLIENT) $(SERVER)

OBJS            = Item.o

COBJS		= Client.o

SOBJS		= ItemInfo.o \
	          Database.o \
                  CurrentDatabase.o \
                  ItemI.o \
	          Evictor.o \
                  EvictorBase.o \
                  SimpleEvictor.o \
                  Server.o

SRCS		= $(OBJS:.o=.cpp) \
                  $(COBJS:.o=.cpp) \
                  $(SOBJS:.o=.cpp)

SLICE_SRCS	= Item.ice ItemInfo.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(DB_RPATH_LINK) -lFreeze $(LIBS)

# The slice2freeze rules are structured like this to avoid issues with
# parallel make.
Database.h: Database.cpp
Database.cpp: ItemInfo.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f Database.h Database.cpp
	$(SLICE2FREEZE) -I. -I$(slicedir) --dict Database,string,Warehouse::ItemInfo Database ItemInfo.ice

clean::
	-rm -rf db/__* db/items db/log* db/__Freeze
	-rm -f Database.h Database.cpp

include .depend
