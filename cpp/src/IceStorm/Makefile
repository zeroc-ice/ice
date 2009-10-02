# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

# Set appropriately if building IceStorm to use SQL database
#QTSQL_HOME	= /opt/qt

SVCLIBFILENAME  = $(call mklibfilename,IceStormService,$(VERSION))
SVCSONAME       = $(call mksoname,IceStormService,$(SOVERSION))  
SVCLIBNAME	= $(call mklibname,IceStormService)

ADMIN		= $(top_srcdir)/bin/icestormadmin
ifeq ($(QTSQL_HOME),)
MIGRATE		= $(top_srcdir)/bin/icestormmigrate
endif

SVCLIBTARGETS   = $(call mklibtargets,$(libdir)/$(SVCLIBFILENAME),$(libdir)/$(SVCSONAME),$(libdir)/$(SVCLIBNAME))
TARGETS         = $(SVCLIBTARGETS) $(ADMIN) $(MIGRATE)

SERVICE_OBJS	= NodeI.o \
		  Observers.o \
		  Election.o \
		  Instance.o \
		  TraceLevels.o \
		  Subscriber.o \
		  TopicI.o \
		  TopicManagerI.o \
		  TransientTopicI.o \
		  TransientTopicManagerI.o \
		  SubscriberRecord.o \
		  IceStormInternal.o \
		  Service.o \
		  DatabaseCache.o

ifneq ($(QTSQL_HOME),)
ICESQL_DIR      = $(top_srcdir)/src/IceSQL

%.o: $(ICESQL_DIR)/%.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

SERVICE_OBJS   += SqlLLU.o \
                  SqlSubscriberMap.o

ICESQL_OBJS	= SqlTypes.o
else
SERVICE_OBJS   += LLUMap.o \
                  SubscriberMap.o
endif

AOBJS		= Admin.o \
		  Grammar.o \
		  Scanner.o \
		  Parser.o \
		  Election.o \
		  SubscriberRecord.o \
		  IceStormInternal.o

ifeq ($(QTSQL_HOME),)
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
endif

SRCS		= $(SERVICE_OBJS:.o=.cpp) \
		  $(AOBJS:.o=.cpp) \
		  $(MOBJS:.o=.cpp)

ifneq ($(QTSQL_HOME),)
SRCS           += $(patsubst %.o,$(ICESQL_DIR)/%.cpp,$(ICESQL_DIR))
endif

SLICE_SRCS	= ../IceStorm/Election.ice \
		  ../IceStorm/IceStormInternal.ice \
		  ../IceStorm/SubscriberRecord.ice \
		  ../IceStorm/LinkRecord.ice

ifeq ($(QTSQL_HOME),)
SLICE_SRCS     += ../IceStorm/V32Format.ice \
		  ../IceStorm/V31Format.ice
endif

LOCAL_HDIR	= ../IceStorm

SLICE2FREEZECMD = $(SLICE2FREEZE) --ice --include-dir IceStorm $(ICECPPFLAGS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= $(CPPFLAGS) -I.. $(READLINE_FLAGS)  $(QTSQL_FLAGS)
ICECPPFLAGS	:= $(ICECPPFLAGS) -I..
SLICE2CPPFLAGS	:= --ice --include-dir IceStorm $(SLICE2CPPFLAGS)
LINKWITH	:= $(DB_RPATH_LINK) -lIceStorm -lIceGrid -lGlacier2 -lIceBox $(BZIP2_RPATH_LINK) -lIce -lIceUtil

ifneq ($(QTSQL_HOME),)
LINKWITH	+= $(QTSQL_RPATH_LINK) $(QTSQL_LIBS)
else
LINKWITH	+= $(DB_RPATH_LINK) -lFreeze
endif

$(libdir)/$(SVCLIBFILENAME): $(SERVICE_OBJS) $(ICESQL_OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SVCSONAME),$(SERVICE_OBJS)  $(ICESQL_OBJS),$(LINKWITH))

$(libdir)/$(SVCSONAME): $(libdir)/$(SVCLIBFILENAME)
	rm -f $@
	ln -s $(SVCLIBFILENAME) $@

$(libdir)/$(SVCLIBNAME): $(libdir)/$(SVCSONAME)
	rm -f $@
	ln -s $(SVCSONAME) $@

$(ADMIN): $(AOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(AOBJS) -lIceStorm $(READLINE_LIBS) $(LIBS)

ifneq ($(QTSQL_HOME),)
# Required because of inclusion in .depend
$(LOCAL_HDIR)/SubscriberMap.h:
	touch $@

$(LOCAL_HDIR)/LLUMap.h:
	touch $@
else
$(MIGRATE): $(MOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(MOBJS) $(DB_RPATH_LINK) -lIceStorm -lFreeze $(LIBS)

LLUMap.h LLUMap.cpp: ../IceStorm/Election.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f LLUMap.h LLUMap.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::LLUMap,string,IceStormElection::LogUpdate \
	LLUMap ../IceStorm/Election.ice

SubscriberMap.h SubscriberMap.cpp: ../IceStorm/SubscriberRecord.ice $(slicedir)/Ice/Identity.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f SubscriberMap.h SubscriberMap.cpp
	$(SLICE2FREEZECMD) \
	--dict IceStorm::SubscriberMap,IceStorm::SubscriberRecordKey,IceStorm::SubscriberRecord,sort \
	SubscriberMap ../IceStorm/SubscriberRecord.ice

# Needed for migration.
../IceStorm/V32FormatDB.h V32FormatDB.cpp: ../IceStorm/V32Format.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f V32FormatDB.h V32FormatDB.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::V32Format,Ice::Identity,IceStorm::LinkRecordSeq \
	V32FormatDB ../IceStorm/V32Format.ice

../IceStorm/V31FormatDB.h V31FormatDB.cpp: ../IceStorm/V31Format.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f V31FormatDB.h V31FormatDB.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::V31Format,string,IceStorm::LinkRecordDict \
	V31FormatDB ../IceStorm/V31Format.ice
endif

# Needed for make -jn to work.
../IceStorm/Grammar.y: Grammar.h

clean::
	-rm -f SubscriberMap.h SubscriberMap.cpp
	-rm -f V32FormatDB.h V32FormatDB.cpp
	-rm -f V31FormatDB.h V31FormatDB.cpp
	-rm -f LLUMap.h LLUMap.cpp

install:: all
	$(call installlib,$(install_libdir),$(libdir),$(SVCLIBFILENAME),$(SVCSONAME),$(SVCLIBNAME))
	$(call installprogram,$(ADMIN),$(install_bindir))
	$(call installprogram,$(MIGRATE),$(install_bindir))

include .depend
