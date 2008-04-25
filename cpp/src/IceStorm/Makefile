# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

SVCLIBFILENAME  = $(call mklibfilename,IceStormService,$(VERSION))
SVCSONAME       = $(call mksoname,IceStormService,$(SOVERSION))  
SVCLIBNAME	= $(call mklibname,IceStormService)

ADMIN		= $(top_srcdir)/bin/icestormadmin
MIGRATE		= $(top_srcdir)/bin/icestormmigrate

SVCLIBTARGETS   = $(call mklibtargets,$(libdir)/$(SVCLIBFILENAME),$(libdir)/$(SVCSONAME),$(libdir)/$(SVCLIBNAME))
TARGETS         = $(SVCLIBTARGETS) $(ADMIN) $(MIGRATE)

SERVICE_OBJS	= NodeI.o \
		  Observers.o \
		  LLUMap.o \
		  Election.o \
		  Instance.o \
		  TraceLevels.o \
		  Subscriber.o \
		  TopicI.o \
		  TopicManagerI.o \
		  TransientTopicI.o \
		  TransientTopicManagerI.o \
                  SubscriberMap.o \
		  SubscriberRecord.o \
		  IceStormInternal.o \
		  Service.o

AOBJS		= Admin.o \
		  Grammar.o \
		  Scanner.o \
		  Parser.o \
		  Election.o \
		  SubscriberRecord.o \
		  IceStormInternal.o

MOBJS		= Migrate.o \
		  SubscriberRecord.o \
                  SubscriberMap.o \
                  LLUMap.o \
		  LinkRecord.o \
		  IceStormInternal.o \
		  Election.o \
		  V32FormatDB.o \
		  V31FormatDB.o \
		  V32Format.o \
		  V31Format.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(SERVICE_OBJS:.o=.cpp) \
		  $(AOBJS:.o=.cpp) \
		  $(MOBJS:.o=.cpp)

SLICE_SRCS	= ../IceStorm/Election.ice \
		  ../IceStorm/IceStormInternal.ice \
		  ../IceStorm/SubscriberRecord.ice \
		  ../IceStorm/LinkRecord.ice \
		  ../IceStorm/V32Format.ice \
		  ../IceStorm/V31Format.ice

HDIR		= $(headerdir)/IceStorm
SDIR		= $(slicedir)/IceStorm

SLICE2FREEZECMD = $(SLICE2FREEZE) --ice --include-dir IceStorm $(ICECPPFLAGS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= $(CPPFLAGS) -I.. -DICE_STORM_API_EXPORTS $(READLINE_FLAGS) 
ICECPPFLAGS	:= $(ICECPPFLAGS) -I..
SLICE2CPPFLAGS	:= --checksum --ice --include-dir IceStorm $(SLICE2CPPFLAGS)
LINKWITH	:= $(BZIP2_RPATH_LINK) -lIce -lIceUtil
SVCLINKWITH	:= $(DB_RPATH_LINK) -lIceGrid -lGlacier2 -lFreeze -lIceBox -lIceStorm $(BZIP2_RPATH_LINK) -lIce -lIceUtil

$(libdir)/$(SVCLIBFILENAME): $(SERVICE_OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SVCSONAME),$(SERVICE_OBJS),$(SVCLINKWITH))

$(libdir)/$(SVCSONAME): $(libdir)/$(SVCLIBFILENAME)
	rm -f $@
	ln -s $(SVCLIBFILENAME) $@

$(libdir)/$(SVCLIBNAME): $(libdir)/$(SVCSONAME)
	rm -f $@
	ln -s $(SVCSONAME) $@

$(ADMIN): $(AOBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(AOBJS) -lIceStorm $(READLINE_LIBS) $(LIBS)

$(MIGRATE): $(MOBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(MOBJS) $(DB_RPATH_LINK) -lIceStorm -lFreeze $(LIBS)

LLUMap.h LLUMap.cpp: $(SLICE2FREEZE) ../IceStorm/Election.ice
	rm -f LLUMap.h LLUMap.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::LLUMap,string,IceStormElection::LogUpdate \
	LLUMap ../IceStorm/Election.ice

SubscriberMap.h SubscriberMap.cpp: ../IceStorm/SubscriberRecord.ice $(slicedir)/Ice/Identity.ice $(SLICE2FREEZE)
	rm -f SubscriberMap.h SubscriberMap.cpp
	$(SLICE2FREEZECMD) \
	--dict IceStorm::SubscriberMap,IceStorm::SubscriberRecordKey,IceStorm::SubscriberRecord,sort \
	SubscriberMap ../IceStorm/SubscriberRecord.ice

# Needed for migration.
V32FormatDB.h V32FormatDB.cpp: ../IceStorm/V32Format.ice $(SLICE2FREEZE)
	rm -f V32FormatDB.h V32FormatDB.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::V32Format,Ice::Identity,IceStorm::LinkRecordSeq \
	V32FormatDB ../IceStorm/V32Format.ice

V31FormatDB.h V31FormatDB.cpp: ../IceStorm/V31Format.ice $(SLICE2FREEZE)
	rm -f V31FormatDB.h V31FormatDB.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::V31Format,string,IceStorm::LinkRecordDict \
	V31FormatDB ../IceStorm/V31Format.ice

# Needed for make -jn to work.
../IceStorm/Grammar.y: Grammar.h

clean::
	-rm -f SubscriberMap.h SubscriberMap.cpp
	-rm -f PersistentTopicMap.h PersistentTopicMap.cpp
	-rm -f V32FormatDB.h V32FormatDB.cpp
	-rm -f V31FormatDB.h V31FormatDB.cpp
	-rm -f LLUMap.h LLUMap.cpp
	-rm -f Grammar.cpp Grammar.h
	-rm -f Scanner.cpp

install:: all
	$(call installlib,$(install_libdir),$(libdir),$(SVCLIBFILENAME),$(SVCSONAME),$(SVCLIBNAME))
	$(call installprogram,$(ADMIN),$(install_bindir))
	$(call installprogram,$(MIGRATE),$(install_bindir))

include .depend
