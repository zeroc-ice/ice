# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

ifeq ($(UNAME),Darwin)
    $(error Demo not supported on Mac OS X) 
endif

CPPFLAGS	:= -I. $(CPPFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(DB_RPATH_LINK) -lFreeze $(LIBS)


Database.h Database.cpp: ItemInfo.ice $(SLICE2FREEZE)
	rm -f Database.h Database.cpp
	$(SLICE2FREEZE) -I$(slicedir) --dict Database,string,Warehouse::ItemInfo Database ItemInfo.ice

cleandb:
	rm -f db/log.* db/__catalog db/items

clean:: cleandb
	rm -f Database.h Database.cpp

include .depend
