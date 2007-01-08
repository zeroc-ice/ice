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

TARGETS		= $(CLIENT)

OBJS		= Client.o \
                  ByteIntMap.o \
                  IntIdentityMap.o \
                  IntIdentityMapWithIndex.o \
                  SortedMap.o \
		  WstringWstringMap.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)

$(CLIENT): $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(DB_RPATH_LINK) -lFreeze $(LIBS)

ByteIntMap.h ByteIntMap.cpp: $(SLICE2FREEZE)
	rm -f ByteIntMap.h ByteIntMap.cpp
	$(SLICE2FREEZE) --dict Test::ByteIntMap,byte,int --dict-index Test::ByteIntMap ByteIntMap

IntIdentityMap.h IntIdentityMap.cpp: $(SLICE2FREEZE)
	rm -f IntIdentityMap.h IntIdentityMap.cpp
	$(SLICE2FREEZE) --ice $(SLICE2CPPFLAGS)  --dict Test::IntIdentityMap,int,Ice::Identity IntIdentityMap $(slicedir)/Ice/Identity.ice

IntIdentityMapWithIndex.h IntIdentityMapWithIndex.cpp: $(SLICE2FREEZE)
	rm -f IntIdentityMapWithIndex.h IntIdentityMapWithIndex.cpp
	$(SLICE2FREEZE) --ice $(SLICE2CPPFLAGS) --dict Test::IntIdentityMapWithIndex,int,Ice::Identity --dict-index Test::IntIdentityMapWithIndex,category IntIdentityMapWithIndex $(slicedir)/Ice/Identity.ice

SortedMap.h SortedMap.cpp: $(SLICE2FREEZE)
	rm -f SortedMap.h SortedMap.cpp
	$(SLICE2FREEZE)  --ice $(SLICE2CPPFLAGS) --dict Test::SortedMap,int,Ice::Identity,sort SortedMap --dict-index "Test::SortedMap,category,sort,std::greater<std::string>" $(slicedir)/Ice/Identity.ice

WstringWstringMap.h WstringWstringMap.cpp: $(SLICE2FREEZE)
	rm -f WstringWstringMap.h WstringWstringMap.cpp
	$(SLICE2FREEZE) --dict 'Test::WstringWstringMap,["cpp:type:wstring"]string,["cpp:type:wstring"]string' --dict-index Test::WstringWstringMap WstringWstringMap

clean::
	rm -f ByteIntMap.h ByteIntMap.cpp IntIdentityMap.h IntIdentityMap.cpp IntIdentityMapWithIndex.h IntIdentityMapWithIndex.cpp SortedMap.h SortedMap.cpp WstringWstringMap.h WstringWstringMap.cpp

clean::
	rm -f db/binary db/binary.* db/intIdentity db/intIdentity.* db/__catalog db/log.*

include .depend
