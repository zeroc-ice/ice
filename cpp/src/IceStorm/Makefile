# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME  	= $(call mklibfilename,IceStormService,$(VERSION))
SONAME       	= $(call mksoname,IceStormService,$(SOVERSION))
LIBNAME	     	= $(call mklibname,IceStormService)

ADMIN		= $(bindir)/icestormadmin
DB		= $(bindir)/icestormdb
MIGRATE		= $(bindir)/icestormmigrate
LIBTARGETS   	= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)$(cpp11libdirsuffix)/$(LIBNAME))
TARGETS         = $(LIBTARGETS) $(ADMIN) $(MIGRATE) $(DB)

CSLICE_OBJS	= Election.o \
		  IceStormInternal.o \
		  Instrumentation.o \
		  LinkRecord.o \
		  LLURecord.o \
		  SubscriberRecord.o \
		  V31Format.o \
		  V32Format.o

LIB_OBJS	= Instance.o \
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
		  $(CSLICE_OBJS)

AOBJS		= Admin.o \
		  Grammar.o \
		  Parser.o \
		  Scanner.o \
		  $(CSLICE_OBJS)

MOBJS		= LLUMap.o \
		  Migrate.o \
		  SubscriberMap.o \
		  V31FormatDB.o \
		  V32FormatDB.o \
		  $(CSLICE_OBJS)

DSLICE_OBJS	= DBTypes.o

DOBJS		= IceStormDB.o \
		  LLUMap.o \
		  SubscriberMap.o \
		  $(DSLICE_OBJS)

OBJS		= $(LIB_OBJS) \
		  $(AOBJS) \
		  $(MOBJS) \
		  $(DOBJS)

SLICE_OBJS	= $(CSLICE_OBJS) \
		  $(DSLICE_OBJS)

RPATH_DIR	= $(LOADER_PATH)/../$(libsubdir)

SLICE2FREEZECMD = $(SLICE2FREEZE) -I../.. --ice --include-dir IceStorm $(ICECPPFLAGS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= $(CPPFLAGS) -I. -I.. $(READLINE_FLAGS)
ICECPPFLAGS	:= $(ICECPPFLAGS) -I..
SLICE2CPPFLAGS	:= --ice --include-dir IceStorm -I. $(SLICE2CPPFLAGS)
LINKWITH	:= $(BZIP2_RPATH_LINK) -lIceStorm -lIceGrid -lFreeze -lIceBox -lIce -lIceUtil

$(libdir)/$(LIBFILENAME): $(LIB_OBJS)
	@mkdir -p $(dir $@)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(LIB_OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)$(cpp11libdirsuffix)/$(LIBNAME): $(libdir)/$(SONAME)
	@mkdir -p $(libdir)$(cpp11libdirsuffix)
	rm -f $@
	ln -s $(cpp11sonamedir)$(SONAME) $@

$(ADMIN): $(AOBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(AOBJS) -lIceStorm $(READLINE_LIBS) $(LIBS)

$(DB): $(DOBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(DOBJS) -lIceStorm -lFreeze $(LIBS)

$(MIGRATE): $(MOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(DB_RPATH_LINK) $(LDEXEFLAGS) -o $@ $(MOBJS) -lIceStorm -lFreeze $(LIBS)

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
	$(call installprogram,$(DB),$(DESTDIR)$(install_bindir))
	$(call installdata,$(top_srcdir)/../man/man1/icestormadmin.1,$(DESTDIR)$(install_mandir))
	$(call installprogram,$(MIGRATE),$(DESTDIR)$(install_bindir))
	$(call installdata,$(top_srcdir)/../man/man1/icestormmigrate.1,$(DESTDIR)$(install_mandir))
	$(call installdata,$(top_srcdir)/../man/man1/icestormdb.1,$(DESTDIR)$(install_mandir))
