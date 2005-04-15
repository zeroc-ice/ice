# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client

TARGETS		= $(CLIENT)

OBJS		= Client.o \
                  ByteIntMap.o \
                  IntIdentityMap.o \
                  IntIdentityMapWithIndex.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(DB_FLAGS) $(CPPFLAGS)

$(CLIENT): $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) -lFreeze $(DB_LIBS) $(LIBS)

ByteIntMap.h ByteIntMap.cpp: $(SLICE2FREEZE)
	rm -f ByteIntMap.h ByteIntMap.cpp
	$(SLICE2FREEZE) --dict Test::ByteIntMap,byte,int --dict-index Test::ByteIntMap ByteIntMap

IntIdentityMap.h IntIdentityMap.cpp: $(SLICE2FREEZE)
	rm -f IntIdentityMap.h IntIdentityMap.cpp
	$(SLICE2FREEZE) --ice $(SLICE2CPPFLAGS)  --dict Test::IntIdentityMap,int,Ice::Identity IntIdentityMap $(slicedir)/Ice/Identity.ice

IntIdentityMapWithIndex.h IntIdentityMapWithIndex.cpp: $(SLICE2FREEZE)
	rm -f IntIdentityMapWithIndex.h IntIdentityMapWithIndex.cpp
	$(SLICE2FREEZE) --ice $(SLICE2CPPFLAGS) --dict Test::IntIdentityMapWithIndex,int,Ice::Identity --dict-index Test::IntIdentityMapWithIndex,category IntIdentityMapWithIndex $(slicedir)/Ice/Identity.ice


clean::
	rm -f ByteIntMap.h ByteIntMap.cpp IntIdentityMap.h IntIdentityMap.cpp IntIdentityMapWithIndex.h IntIdentityMapWithIndex.cpp

clean::
	rm -f db/binary db/binary.* db/intIdentity db/intIdentity.* db/__catalog db/log.*

include .depend
