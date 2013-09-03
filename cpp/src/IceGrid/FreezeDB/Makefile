# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

LIBFILENAME     = $(call mklibfilename,IceGridFreezeDB,$(VERSION))
SONAME          = $(call mksoname,IceGridFreezeDB,$(SOVERSION))  
LIBNAME		= $(call mklibname,IceGridFreezeDB)

TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))

OBJS   		= StringApplicationInfoDict.o \
		  IdentityObjectInfoDict.o \
		  StringAdapterInfoDict.o \
		  SerialsDict.o \
		  FreezeDB.o

DB_OBJS		= FreezeTypes.o

%.o: $(top_srcdir)/src/IceDB/%.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

SRCS		= $(OBJS:.o=.cpp)

HDIR		= $(headerdir)/IceGrid
SDIR		= $(slicedir)/IceGrid

SLICE2FREEZECMD = $(SLICE2FREEZE) --ice --include-dir IceGrid/FreezeDB $(ICECPPFLAGS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I../.. $(CPPFLAGS)

LINKWITH        := $(DB_RPATH_LINK) -lIceGrid -lGlacier2 -lFreeze -lIceDB -lIce -lIceUtil 

$(libdir)/$(LIBFILENAME): $(OBJS) $(DB_OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS) $(DB_OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@


# The slice2freeze rules are structured like this to avoid issues with
# parallel make.
../../IceGrid/FreezeDB/StringApplicationInfoDict.h: StringApplicationInfoDict.cpp
StringApplicationInfoDict.cpp: $(SDIR)/Admin.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f StringApplicationInfoDict.h StringApplicationInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringApplicationInfoDict,string,IceGrid::ApplicationInfo \
	StringApplicationInfoDict $(SDIR)/Admin.ice

../../IceGrid/FreezeDB/IdentityObjectInfoDict.h: IdentityObjectInfoDict.cpp
IdentityObjectInfoDict.cpp: $(slicedir)/Ice/Identity.ice $(SDIR)/Admin.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::IdentityObjectInfoDict,Ice::Identity,IceGrid::ObjectInfo \
	--dict-index IceGrid::IdentityObjectInfoDict,type \
	IdentityObjectInfoDict $(slicedir)/Ice/Identity.ice $(SDIR)/Admin.ice

../../IceGrid/FreezeDB/StringAdapterInfoDict.h: StringAdapterInfoDict.cpp
StringAdapterInfoDict.cpp: $(SDIR)/Admin.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f StringAdapterInfoDict.h StringAdapterInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringAdapterInfoDict,string,IceGrid::AdapterInfo \
	--dict-index IceGrid::StringAdapterInfoDict,replicaGroupId StringAdapterInfoDict $(SDIR)/Admin.ice

../../IceGrid/FreezeDB/SerialsDict.h: SerialsDict.cpp
SerialsDict.cpp: $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f SerialsDict.h SerialsDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::SerialsDict,string,long SerialsDict

clean::
	-rm -f StringApplicationInfoDict.h StringApplicationInfoDict.cpp
	-rm -f IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp
	-rm -f StringAdapterInfoDict.h StringAdapterInfoDict.cpp
	-rm -f SerialsDict.h SerialsDict.cpp

install:: all
	$(call installlib,$(DESTDIR)$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))

include .depend
