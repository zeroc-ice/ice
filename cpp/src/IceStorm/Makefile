# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
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
LIBTARGETS   	= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)$(cpp11libdirsuffix)/$(LIBNAME))
TARGETS         = $(LIBTARGETS) $(ADMIN) $(DB)

LSLICE_OBJS	= Election.o \
		  IceStormInternal.o \
		  Instrumentation.o \
		  LinkRecord.o \
		  LLURecord.o \
		  SubscriberRecord.o

LIB_OBJS	= Instance.o \
		  InstrumentationI.o \
		  NodeI.o \
		  Observers.o \
		  Service.o \
		  Subscriber.o \
		  TopicI.o \
		  TopicManagerI.o \
		  TraceLevels.o \
		  TransientTopicI.o \
		  TransientTopicManagerI.o \
		  Util.o \
		  $(SLICE_OBJS)

AOBJS		= Admin.o \
		  Grammar.o \
		  Parser.o \
		  Scanner.o \
		  $(SLICE_OBJS)

DBSLICE_OBJS	= DBTypes.o

DBOBJS		= IceStormDB.o \
		  $(DBSLICE_OBJS)

SLICE_OBJS	= $(LSLICE_OBJS) \
		  $(DBSLICE_OBJS)

OBJS		= $(LIB_OBJS) \
		  $(AOBJS) \
		  $(DBOBJS)

RPATH_DIR	= $(LOADER_PATH)/../$(libsubdir)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= $(CPPFLAGS) -I.. $(READLINE_FLAGS)
ICECPPFLAGS	:= $(ICECPPFLAGS) -I..
SLICE2CPPFLAGS	:= --ice --include-dir IceStorm $(SLICE2CPPFLAGS)
LINKWITH	:= $(BZIP2_RPATH_LINK) -lIceStorm -lIceGrid -lGlacier2 -lIceBox -lIce -lIceUtil -lIceDB

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

$(DB): $(DBOBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(DBOBJS) -lIceStorm -lIceDB -lIcePatch2 $(LIBS)

# Needed for make -jn to work.
../IceStorm/Grammar.y: Grammar.h

clean::
	-rm -f LinkRecord.h LinkRecord.cpp

install:: all
	$(call installlib,$(DESTDIR)$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))
	$(call installprogram,$(ADMIN),$(DESTDIR)$(install_bindir))
	$(call installprogram,$(DB),$(DESTDIR)$(install_bindir))
	$(call installdata,$(top_srcdir)/../man/man1/icestormadmin.1,$(DESTDIR)$(install_mandir))
