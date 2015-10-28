# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

ICEGRIDDB	= $(bindir)/icegriddb35

TARGETS		= $(ICEGRIDDB)

OBJS		= IceGridDB.o \
		  DBTypes.o \
		  StringApplicationInfoDict.o \
		  IdentityObjectInfoDict.o \
		  StringAdapterInfoDict.o \
		  SerialsDict.o

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	= DBTypes.ice

SLICE2FREEZECMD = $(SLICE2FREEZE) --ice $(ICECPPFLAGS)

SDIR		= $(slicedir)/IceGrid

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)
SLICE2CPPFLAGS	:= --ice -I. $(SLICE2CPPFLAGS)
LIBS		:= -lIceGrid -lIcePatch2 -lGlacier2 -lFreeze $(LIBS)

$(ICEGRIDDB): $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

StringApplicationInfoDict.h StringApplicationInfoDict.cpp: $(SDIR)/Admin.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f StringApplicationInfoDict.h StringApplicationInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringApplicationInfoDict,string,IceGrid::ApplicationInfo \
	StringApplicationInfoDict $(SDIR)/Admin.ice

IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp: $(slicedir)/Ice/Identity.ice $(SDIR)/Admin.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::IdentityObjectInfoDict,Ice::Identity,IceGrid::ObjectInfo \
	--dict-index IceGrid::IdentityObjectInfoDict,type \
	IdentityObjectInfoDict $(slicedir)/Ice/Identity.ice $(SDIR)/Admin.ice

StringAdapterInfoDict.h StringAdapterInfoDict.cpp: $(SDIR)/Admin.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f StringAdapterInfoDict.h StringAdapterInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringAdapterInfoDict,string,IceGrid::AdapterInfo \
	--dict-index IceGrid::StringAdapterInfoDict,replicaGroupId StringAdapterInfoDict $(SDIR)/Admin.ice

SerialsDict.h SerialsDict.cpp: $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f SerialsDict.h SerialsDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::SerialsDict,string,long SerialsDict

clean::
	-rm -f StringApplicationInfoDict.h StringApplicationInfoDict.cpp
	-rm -f IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp
	-rm -f StringAdapterInfoDict.h StringAdapterInfoDict.cpp
	-rm -f SerialsDict.h SerialsDict.cpp

install:: all
	$(call installprogram,$(ICEGRIDDB),$(DESTDIR)$(install_bindir))

include .depend
