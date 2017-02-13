# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

all:: ByteIntMap.cpp IntIdentityMap.cpp IntIdentityMapWithIndex.cpp SortedMap.cpp

GENPIC          = no

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)

$(CLIENT): $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(OBJS) $(DB_RPATH_LINK) -lFreeze $(LIBS)

# The slice2freeze rules are structured like this to avoid issues with
# parallel make.
ByteIntMap.h: ByteIntMap.cpp
ByteIntMap.cpp: $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f ByteIntMap.h ByteIntMap.cpp
	$(SLICE2FREEZE) --dict Test::ByteIntMap,byte,int --dict-index "Test::ByteIntMap,sort" ByteIntMap

IntIdentityMap.h: IntIdentityMap.cpp
IntIdentityMap.cpp: $(slicedir)/Ice/Identity.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f IntIdentityMap.h IntIdentityMap.cpp
	$(SLICE2FREEZE) --ice $(SLICE2CPPFLAGS)  --dict Test::IntIdentityMap,int,Ice::Identity IntIdentityMap $(slicedir)/Ice/Identity.ice

IntIdentityMapWithIndex.h: IntIdentityMapWithIndex.cpp
IntIdentityMapWithIndex.cpp: $(slicedir)/Ice/Identity.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f IntIdentityMapWithIndex.h IntIdentityMapWithIndex.cpp
	$(SLICE2FREEZE) --ice $(SLICE2CPPFLAGS) --dict Test::IntIdentityMapWithIndex,int,Ice::Identity --dict-index Test::IntIdentityMapWithIndex,category IntIdentityMapWithIndex $(slicedir)/Ice/Identity.ice

SortedMap.h: SortedMap.cpp
SortedMap.cpp: $(slicedir)/Ice/Identity.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f SortedMap.h SortedMap.cpp
	$(SLICE2FREEZE)  --ice $(SLICE2CPPFLAGS) --dict Test::SortedMap,int,Ice::Identity,sort SortedMap --dict-index "Test::SortedMap,category,sort,std::greater<std::string>" $(slicedir)/Ice/Identity.ice

WstringWstringMap.h: WstringWstringMap.cpp
WstringWstringMap.cpp: $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f WstringWstringMap.h WstringWstringMap.cpp
	$(SLICE2FREEZE) --dict 'Test::WstringWstringMap,["cpp:type:wstring"]string,["cpp:type:wstring"]string' --dict-index Test::WstringWstringMap WstringWstringMap

clean::
	-rm -f ByteIntMap.h ByteIntMap.cpp
	-rm -f IntIdentityMap.h IntIdentityMap.cpp
	-rm -f IntIdentityMapWithIndex.h IntIdentityMapWithIndex.cpp
	-rm -f SortedMap.h SortedMap.cpp
	-rm -f WstringWstringMap.h WstringWstringMap.cpp
	-rm -rf db/*
