# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME  = $(call mklibfilename,IceStormDB,$(VERSION))
SONAME       = $(call mksoname,IceStormDB,$(SOVERSION))  
LIBNAME	= $(call mklibname,IceStormDB)

SVCLIBFILENAME  = $(call mklibfilename,IceStormService,$(VERSION))
SVCSONAME       = $(call mksoname,IceStormService,$(SOVERSION))  
SVCLIBNAME	= $(call mklibname,IceStormService)

ADMIN		= $(top_srcdir)/bin/icestormadmin

LIBTARGETS   = $(call mklibtargets,$(libdir)/$(SVCLIBFILENAME),$(libdir)/$(SVCSONAME),$(libdir)/$(SVCLIBNAME))
SVCLIBTARGETS   = $(call mklibtargets,$(libdir)/$(SVCLIBFILENAME),$(libdir)/$(SVCSONAME),$(libdir)/$(SVCLIBNAME))
TARGETS         = $(LIBTARGETS) $(SVCLIBTARGETS) $(ADMIN)

SERVICE_OBJS	= NodeI.o \
		  Observers.o \
		  Instance.o \
		  TraceLevels.o \
		  Subscriber.o \
		  TopicI.o \
		  TopicManagerI.o \
		  TransientTopicI.o \
		  TransientTopicManagerI.o \
		  Service.o

OBJS		= IceStorm.o \
		  LLURecord.o \
		  Election.o \
		  SubscriberRecord.o \
		  IceStormInternal.o

AOBJS		= Admin.o \
		  Grammar.o \
		  Scanner.o \
		  Parser.o \
		  LLURecord.o \
		  Election.o \
		  SubscriberRecord.o \
		  IceStormInternal.o

SRCS		= $(SERVICE_OBJS:.o=.cpp) \
		  $(OBJS:.o=.cpp) \
		  $(AOBJS:.o=.cpp) \

SLICE_SRCS	= ../IceStorm/Election.ice \
		  ../IceStorm/IceStormInternal.ice \
		  ../IceStorm/LLURecord.ice \
		  ../IceStorm/SubscriberRecord.ice

HDIR		= $(headerdir)/IceStorm
SDIR		= $(slicedir)/IceStorm

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= $(CPPFLAGS) -I.. $(READLINE_FLAGS)
ICECPPFLAGS	:= $(ICECPPFLAGS) -I..
SLICE2CPPFLAGS	:= --ice --include-dir IceStorm $(SLICE2CPPFLAGS)
LINKWITH	:= $(BZIP2_RPATH_LINK) -lIce -lIceUtil
SVCLINKWITH	:= $(BZIP2_RPATH_LINK) -lIceGrid -lGlacier2 -lIceStormDB -lIceDB -lIceBox -lIce -lIceUtil

$(libdir)/$(SVCLIBFILENAME): $(SERVICE_OBJS) $(libdir)/$(LIBNAME)
	rm -f $@
	$(call mkshlib,$@,$(SVCSONAME),$(SERVICE_OBJS),$(SVCLINKWITH))

$(libdir)/$(SVCSONAME): $(libdir)/$(SVCLIBFILENAME)
	rm -f $@
	ln -s $(SVCLIBFILENAME) $@

$(libdir)/$(SVCLIBNAME): $(libdir)/$(SVCSONAME)
	rm -f $@
	ln -s $(SVCSONAME) $@

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

# Needed for make -jn to work.
../IceStorm/Grammar.y: Grammar.h

SUBDIRS = FreezeDB
ifneq ($(QT_HOME),)
   SUBDIRS += SqlDB
endif

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    if test -d $$subdir ; \
	    then \
		echo "making $@ in $$subdir"; \
		( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	    fi; \
	done

install:: all
	$(call installlib,$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))
	$(call installlib,$(install_libdir),$(libdir),$(SVCLIBFILENAME),$(SVCSONAME),$(SVCLIBNAME))
	$(call installprogram,$(ADMIN),$(install_bindir))

include .depend
