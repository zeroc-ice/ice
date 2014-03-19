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

LIBTARGETS   	= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))
TARGETS         = $(LIBTARGETS) $(ADMIN)

OBJS		= NodeI.o \
		  Observers.o \
		  Instance.o \
		  TraceLevels.o \
		  Subscriber.o \
		  TopicI.o \
		  TopicManagerI.o \
		  TransientTopicI.o \
		  TransientTopicManagerI.o \
		  Service.o \
		  LLURecord.o \
		  Election.o \
		  SubscriberRecord.o \
		  IceStormInternal.o \
		  Instrumentation.o \
		  InstrumentationI.o \
		  Util.o

AOBJS		= Admin.o \
		  Grammar.o \
		  Scanner.o \
		  Parser.o \
		  LLURecord.o \
		  Election.o \
		  SubscriberRecord.o \
		  IceStormInternal.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(AOBJS:.o=.cpp) \

SLICE_SRCS	= ../IceStorm/Instrumentation.ice \
		  ../IceStorm/Election.ice \
		  ../IceStorm/IceStormInternal.ice \
		  ../IceStorm/LLURecord.ice \
		  ../IceStorm/SubscriberRecord.ice

HDIR		= $(headerdir)/IceStorm
SDIR		= $(slicedir)/IceStorm

ifeq ($(CPP11),yes)
RPATH_DIR	= @loader_path/../../lib/c++11
else
RPATH_DIR	= @loader_path/../lib
endif

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= $(CPPFLAGS) -I.. -DICESTORM_SERVICE_API_EXPORTS $(READLINE_FLAGS)
ICECPPFLAGS	:= $(ICECPPFLAGS) -I..
SLICE2CPPFLAGS	:= --ice --include-dir IceStorm --dll-export ICE_STORM_SERVICE_API $(SLICE2CPPFLAGS)
LINKWITH	:= $(BZIP2_RPATH_LINK) -lIceStorm -lIceGrid -lGlacier2 -lIceDB -lIceBox -lIce -lIceUtil

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

install:: all
	$(call installlib,$(DESTDIR)$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))
	$(call installprogram,$(ADMIN),$(DESTDIR)$(install_bindir))
	$(call installdata,$(top_srcdir)/../man/man1/icestormadmin.1,$(DESTDIR)$(install_mandir))

SUBDIRS = FreezeDB

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    if test -d $$subdir ; \
	    then \
		echo "making $@ in $$subdir"; \
		( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	    fi; \
	done

include .depend
