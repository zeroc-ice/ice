#**********************************************************************
#
# Copyright (c) 2003
# ZeroC, Inc.
# Billerica, MA, USA
#
# All Rights Reserved.
#
# Ice is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by
# the Free Software Foundation.
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

TARGETS		= $(libdir)/$(LIBFILENAME) $(libdir)/$(SONAME) $(libdir)/$(LIBNAME) \
                  $(libdir)/$(SVCLIBFILENAME) $(libdir)/$(SVCSONAME) $(libdir)/$(SVCLIBNAME) \
                  $(ADMIN)

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

CPPFLAGS	:= -I.. $(CPPFLAGS) -DICE_STORM_API_EXPORTS $(READLINE_FLAGS) 
ICECPPFLAGS	:= -I.. $(ICECPPFLAGS)
SLICE2CPPFLAGS	:= --ice --dll-export ICE_STORM_API --include-dir IceStorm $(SLICE2CPPFLAGS)
LINKWITH	:= -lIce -lIceUtil
SVCLINKWITH	:= -lFreeze -lIce -lIceUtil

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


$(ADMIN): $(AOBJS) $(libdir)/$(LIBNAME)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(AOBJS) -lIceStorm -lIceXML $(READLINE_LIBS) $(LIBS)



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
	$(INSTALL_LIBRARY) $(libdir)/$(LIBFILENAME) $(install_libdir)
	rm -f $(install_libdir)/$(SONAME)
	ln -s $(LIBFILENAME) $(install_libdir)/$(SONAME)
	rm -f $(install_libdir)/$(LIBNAME)
	ln -s $(SONAME) $(install_libdir)/$(LIBNAME)
	$(INSTALL_LIBRARY) $(libdir)/$(SVCLIBFILENAME) $(install_libdir)
	rm -f $(install_libdir)/$(SVCSONAME)
	ln -s $(SVCLIBFILENAME) $(install_libdir)/$(SVCSONAME)
	rm -f $(install_libdir)/$(SVCLIBNAME)
	ln -s $(SVCSONAME) $(install_libdir)/$(SVCLIBNAME)
	$(INSTALL_PROGRAM) $(ADMIN) $(install_bindir)

include .depend
