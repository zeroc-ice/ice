# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME  = $(call mklibfilename,IceStormService,$(VERSION))
SONAME       = $(call mksoname,IceStormService,$(SOVERSION))
LIBNAME	= $(call mklibname,IceStormService)

ADMIN		= $(bindir)/icestormadmin
MIGRATE		= $(bindir)/icestormmigrate
LIBTARGETS   	= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))
TARGETS         = $(LIBTARGETS) $(ADMIN) $(MIGRATE)

SLICE_OBJS      = Election.o \
		  IceStormInternal.o \
		  Instrumentation.o \
		  LinkRecord.o \
		  LLURecord.o \
		  SubscriberRecord.o \
		  V31Format.o \
		  V32Format.o

OBJS		= Instance.o \
		  InstrumentationI.o \
		  LLUMap.o \
		  NodeI.o \
		  Observers.o \
		  Service.o \
		  Subscriber.o \
		  SubscriberMap.o \
		  TopicI.o \
		  TopicManagerI.o \
		  TraceLevels.o \
		  TransientTopicI.o \
		  TransientTopicManagerI.o \
		  Util.o \
		  V31FormatDB.o \
		  V32FormatDB.o \
                  $(SLICE_OBJS)

AOBJS		= Admin.o \
		  Grammar.o \
		  Parser.o \
		  Scanner.o \
                  $(SLICE_OBJS)

MOBJS		= LLUMap.o \
                  Migrate.o \
                  SubscriberMap.o \
                  V31FormatDB.o \
		  V32FormatDB.o \
                  $(SLICE_OBJS)

SRCS		= $(OBJS:.o=.cpp) \
		  $(AOBJS:.o=.cpp) \
		  $(MOBJS:.o=.cpp)

SLICE_SRCS	= $(addprefix ../IceStorm/, $(SLICE_OBJS:.o=.ice))

HDIR		= $(headerdir)/IceStorm
SDIR		= $(slicedir)/IceStorm

ifeq ($(CPP11),yes)
RPATH_DIR	= @loader_path/../../lib/c++11
else
RPATH_DIR	= @loader_path/../lib
endif

SLICE2FREEZECMD = $(SLICE2FREEZE) -I../.. --ice --include-dir IceStorm $(ICECPPFLAGS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= $(CPPFLAGS) -I.. -DICESTORM_SERVICE_API_EXPORTS $(READLINE_FLAGS)
ICECPPFLAGS	:= $(ICECPPFLAGS) -I..
SLICE2CPPFLAGS	:= --ice --include-dir IceStorm --dll-export ICE_STORM_SERVICE_API $(SLICE2CPPFLAGS)
LINKWITH	:= $(BZIP2_RPATH_LINK) -lIceStorm -lIceGrid -lGlacier2 -lFreeze -lIceBox -lIce -lIceUtil

$(libdir)/$(LIBFILENAME): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

$(ADMIN): $(AOBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(AOBJS) -lIceStorm $(READLINE_LIBS) $(LIBS)

$(MIGRATE): $(MOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(MOBJS) $(DB_RPATH_LINK) -lIceStorm -lFreeze $(LIBS)

# The slice2freeze rules are structured like this to avoid issues with
# parallel make.
../IceStorm/LLUMap.h: LLUMap.cpp
LLUMap.cpp: ../IceStorm/LLURecord.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f LLUMap.h LLUMap.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::LLUMap,string,IceStormElection::LogUpdate \
	LLUMap ../IceStorm/LLURecord.ice

../IceStorm/SubscriberMap.h: SubscriberMap.cpp
SubscriberMap.cpp: ../IceStorm/SubscriberRecord.ice $(slicedir)/Ice/Identity.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f SubscriberMap.h SubscriberMap.cpp
	$(SLICE2FREEZECMD) \
	--dict IceStorm::SubscriberMap,IceStorm::SubscriberRecordKey,IceStorm::SubscriberRecord,sort \
	SubscriberMap ../IceStorm/SubscriberRecord.ice

# Needed for migration.
../IceStorm/V32FormatDB.h: V32FormatDB.cpp
V32FormatDB.cpp: ../IceStorm/V32Format.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f V32FormatDB.h V32FormatDB.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::V32Format,Ice::Identity,IceStorm::LinkRecordSeq \
	V32FormatDB ../IceStorm/V32Format.ice

../IceStorm/V31FormatDB.h: V31FormatDB.cpp
V31FormatDB.cpp: ../IceStorm/V31Format.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f V31FormatDB.h V31FormatDB.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::V31Format,string,IceStorm::LinkRecordDict \
	V31FormatDB ../IceStorm/V31Format.ice

# Needed for make -jn to work.
../IceStorm/Grammar.y: Grammar.h

clean::
	-rm -f LinkRecord.h LinkRecord.cpp
	-rm -f LLUMap.h LLUMap.cpp
	-rm -f SubscriberMap.h SubscriberMap.cpp
	-rm -f V31FormatDB.h V31FormatDB.cpp
	-rm -f V32FormatDB.h V32FormatDB.cpp

install:: all
	$(call installlib,$(DESTDIR)$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))
	$(call installprogram,$(ADMIN),$(DESTDIR)$(install_bindir))
	$(call installdata,$(top_srcdir)/../man/man1/icestormadmin.1,$(DESTDIR)$(install_mandir))
	$(call installprogram,$(MIGRATE),$(DESTDIR)$(install_bindir))
	$(call installdata,$(top_srcdir)/../man/man1/icestormmigrate.1,$(DESTDIR)$(install_mandir))

include .depend
