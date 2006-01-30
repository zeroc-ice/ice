# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,IceStorm,$(VERSION))
SONAME          = $(call mksoname,IceStorm,$(SOVERSION))  
LIBNAME		= $(call mklibname,IceStorm)

SVCLIBFILENAME  = $(call mklibfilename,IceStormService,$(VERSION))
SVCSONAME       = $(call mksoname,IceStormService,$(SOVERSION))  
SVCLIBNAME	= $(call mklibname,IceStormService)

ADMIN		= $(top_srcdir)/bin/icestormadmin

LIBTARGETS	= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))
SVCLIBTARGETS   = $(call mklibtargets,$(libdir)/$(SVCLIBFILENAME),$(libdir)/$(SVCSONAME),$(libdir)/$(SVCLIBNAME))
TARGETS         = $(LIBTARGETS) $(SVCLIBTARGETS) $(ADMIN)

OBJS		= IceStorm.o

SERVICE_OBJS	= IceStorm.o \
		  TraceLevels.o \
		  Flusher.o \
		  Subscriber.o \
		  OnewaySubscriber.o \
		  OnewayBatchSubscriber.o \
		  LinkSubscriber.o \
		  SubscriberFactory.o \
		  TopicI.o \
		  TopicManagerI.o \
                  PersistentTopicMap.o \
		  LinkRecord.o \
		  IceStormInternal.o \
		  Service.o \
		  QueuedProxy.o \
		  OnewayProxy.o \
		  TwowayProxy.o \
		  LinkProxy.o

AOBJS		= Admin.o \
		  Grammar.o \
		  Scanner.o \
		  WeightedGraph.o \
		  Parser.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(SERVICE_OBJS:.o=.cpp) \
		  $(SERVER_OBJS:.o=.cpp) \
		  $(AOBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/IceStorm.ice \
		  IceStormInternal.ice \
		  LinkRecord.ice

HDIR		= $(includedir)/IceStorm
SDIR		= $(slicedir)/IceStorm

SLICE2FREEZECMD = $(SLICE2FREEZE) --ice --include-dir IceStorm $(ICECPPFLAGS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= $(CPPFLAGS) -I.. $(READLINE_FLAGS) 
ICECPPFLAGS	:= $(ICECPPFLAGS) -I..
SLICE2CPPFLAGS	:= --checksum --ice --dll-export ICE_STORM_API --include-dir IceStorm $(SLICE2CPPFLAGS)
LINKWITH	:= $(BZIP2_RPATH_LINK) -lIce -lIceUtil
SVCLINKWITH	:= $(DB_RPATH_LINK) -lFreeze -lIceBox $(BZIP2_RPATH_LINK) -lIce -lIceUtil

IceStorm.o: IceStorm.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -DICE_STORM_API_EXPORTS $<

%.o: %.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -DICE_STORM_SERVICE_API_EXPORTS $<

$(libdir)/$(LIBFILENAME): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

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
	$(CXX) $(LDFLAGS) -o $@ $(AOBJS) -lIceStorm $(EXPAT_RPATH_LINK) -lIceXML $(READLINE_LIBS) $(LIBS)

PersistentTopicMap.h PersistentTopicMap.cpp: ../IceStorm/LinkRecord.ice $(slicedir)/Ice/Identity.ice $(SLICE2FREEZE)
	rm -f PersistentTopicMap.h PersistentTopicMap.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::PersistentTopicMap,string,IceStorm::LinkRecordDict PersistentTopicMap \
	../IceStorm/LinkRecord.ice

clean::
	rm -f PersistentTopicMap.h PersistentTopicMap.cpp

# Needed for make -jn to work.
../IceStorm/Grammar.y: Grammar.h

clean::
	-rm -f Grammar.cpp Grammar.h
	-rm -f Scanner.cpp

install:: all
	$(call installlib,$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))
	$(call installlib,$(install_libdir),$(libdir),$(SVCLIBFILENAME),$(SVCSONAME),$(SVCLIBNAME))
	$(call installprogram,$(ADMIN),$(install_bindir))

include .depend
