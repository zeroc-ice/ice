# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

LIBFILENAME     = $(call mklibfilename,IceStormFreezeDB,$(VERSION))
SONAME          = $(call mksoname,IceStormFreezeDB,$(SOVERSION))  
LIBNAME		= $(call mklibname,IceStormFreezeDB)

MIGRATE		= $(bindir)/icestormmigrate

TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME)) $(MIGRATE)

OBJS   		= LLUMap.o \
                  SubscriberMap.o \
		  FreezeDB.o

DB_OBJS		= FreezeTypes.o

MOBJS		= Migrate.o \
                  SubscriberMap.o \
                  LLUMap.o \
		  LinkRecord.o \
		  V32FormatDB.o \
		  V31FormatDB.o \
		  V32Format.o \
		  V31Format.o

%.o: $(top_srcdir)/src/IceDB/%.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

SRCS		= $(OBJS:.o=.cpp) \
		  $(MOBJS:.o=.cpp)

SLICE_SRCS     	= ../../IceStorm/FreezeDB/V32Format.ice \
		  ../../IceStorm/FreezeDB/V31Format.ice

HDIR		= $(headerdir)/IceStorm
SDIR		= $(slicedir)/IceStorm

SLICE2FREEZECMD = $(SLICE2FREEZE) -I../.. --ice --include-dir IceStorm/FreezeDB $(ICECPPFLAGS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I../.. $(CPPFLAGS)
SLICE2CPPFLAGS	:= -I../.. --ice --include-dir IceStorm/FreezeDB $(SLICE2CPPFLAGS)
LINKWITH        := $(DB_RPATH_LINK) -lIceStormService -lIceStorm -lFreeze -lIceDB -lIce -lIceUtil 

$(libdir)/$(LIBFILENAME): $(OBJS) $(DB_OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS) $(DB_OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

$(MIGRATE): $(MOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(MOBJS) $(DB_RPATH_LINK) -lIceStormService -lIceStorm -lFreeze $(LIBS)

# The slice2freeze rules are structured like this to avoid issues with
# parallel make.
../../IceStorm/FreezeDB/LLUMap.h: LLUMap.cpp
LLUMap.cpp: ../../IceStorm/LLURecord.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f LLUMap.h LLUMap.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::LLUMap,string,IceStormElection::LogUpdate \
	LLUMap ../../IceStorm/LLURecord.ice

../../IceStorm/FreezeDB/SubscriberMap.h: SubscriberMap.cpp
SubscriberMap.cpp: ../../IceStorm/SubscriberRecord.ice $(slicedir)/Ice/Identity.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f SubscriberMap.h SubscriberMap.cpp
	$(SLICE2FREEZECMD) \
	--dict IceStorm::SubscriberMap,IceStorm::SubscriberRecordKey,IceStorm::SubscriberRecord,sort \
	SubscriberMap ../../IceStorm/SubscriberRecord.ice

# Needed for migration.
../../IceStorm/FreezeDB/V32FormatDB.h: V32FormatDB.cpp
V32FormatDB.cpp: ../../IceStorm/FreezeDB/V32Format.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f V32FormatDB.h V32FormatDB.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::V32Format,Ice::Identity,IceStorm::LinkRecordSeq \
	V32FormatDB ../../IceStorm/FreezeDB/V32Format.ice

../../IceStorm/FreezeDB/V31FormatDB.h: V31FormatDB.cpp
V31FormatDB.cpp: ../../IceStorm/FreezeDB/V31Format.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f V31FormatDB.h V31FormatDB.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::V31Format,string,IceStorm::LinkRecordDict \
	V31FormatDB ../../IceStorm/FreezeDB/V31Format.ice

clean::
	-rm -f SubscriberMap.h SubscriberMap.cpp
	-rm -f V32FormatDB.h V32FormatDB.cpp
	-rm -f V31FormatDB.h V31FormatDB.cpp
	-rm -f LLUMap.h LLUMap.cpp
	-rm -f LinkRecord.h LinkRecord.cpp

install:: all
	$(call installlib,$(DESTDIR)$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))
	$(call installprogram,$(MIGRATE),$(DESTDIR)$(install_bindir))
	$(call installdata,$(top_srcdir)/../man/man1/icestormmigrate.1,$(DESTDIR)$(install_mandir))

include .depend
