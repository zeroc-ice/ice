# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

ICESTORMDB	= $(bindir)/icestormdb36

TARGETS		= $(ICESTORMDB)

SLICE_OBJS	= DBTypes.o

OBJS		= IceStormDB.o \
		  LLUMap.o \
		  SubscriberMap.o \
		  $(SLICE_OBJS)

SRCS		= $(OBJS:.o=.cpp)

SLICE2FREEZECMD = $(SLICE2FREEZE) --ice $(ICECPPFLAGS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include -I../../src $(CPPFLAGS)
SLICE2CPPFLAGS	:= --ice -I. $(SLICE2CPPFLAGS)
LIBS		:= -lIceStorm -lIcePatch2 -lFreeze $(LIBS)

$(ICESTORMDB): $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

IceStormDB.o: LLURecord.h SubscriberRecord.h

SubscriberRecord.ice: ../../src/IceStorm/SubscriberRecord.ice
	cp ../../src/IceStorm/SubscriberRecord.ice .

LLURecord.ice: ../../src/IceStorm/LLURecord.ice
	cp ../../src/IceStorm/LLURecord.ice .

LLUMap.h: LLUMap.cpp
LLUMap.cpp: LLURecord.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f LLUMap.h LLUMap.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::LLUMap,string,IceStormElection::LogUpdate \
	LLUMap LLURecord.ice

SubscriberMap.h: SubscriberMap.cpp
SubscriberMap.cpp: SubscriberRecord.ice $(slicedir)/Ice/Identity.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f SubscriberMap.h SubscriberMap.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::SubscriberMap,IceStorm::SubscriberRecordKey,IceStorm::SubscriberRecord,sort \
	SubscriberMap SubscriberRecord.ice

clean::
	-rm -f LLUMap.h LLUMap.cpp
	-rm -f SubscriberMap.h SubscriberMap.cpp
	-rm -f LLURecord.h LLURecord.cpp
	-rm -f SubscriberRecord.h SubscriberRecord.cpp
	-rm -f LLURecord.ice SubscriberRecord.ice

install:: all
	$(call installprogram,$(ICESTORMDB),$(DESTDIR)$(install_bindir))
	$(call installdata,$(top_srcdir)/../man/man1/icestormdb36.1,$(DESTDIR)$(install_mandir))
