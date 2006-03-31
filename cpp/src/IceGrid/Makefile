# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,IceGrid,$(VERSION))
SONAME          = $(call mksoname,IceGrid,$(SOVERSION))  
LIBNAME		= $(call mklibname,IceGrid)

ADMIN		= $(top_srcdir)/bin/icegridadmin
NODE_SERVER	= $(top_srcdir)/bin/icegridnode
REGISTRY_SERVER	= $(top_srcdir)/bin/icegridregistry

LIBTARGETS	= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))
TARGETS         = $(LIBTARGETS) $(NODE_SERVER) $(REGISTRY_SERVER) $(ADMIN)

LIB_OBJS	= Admin.o \
		  Query.o \
	 	  Exception.o \
		  Descriptor.o \
		  Observer.o

ADMIN_OBJS	= Grammar.o \
		  Scanner.o \
		  Parser.o \
                  DescriptorParser.o \
                  DescriptorBuilder.o \
		  DescriptorHelper.o \
                  FileParser.o \
                  FileParserI.o \
		  Util.o \
		  Internal.o \
		  Client.o

COMMON_OBJS	= Internal.o \
                  DescriptorParser.o \
                  DescriptorBuilder.o \
		  TraceLevels.o

NODE_OBJS	= NodeI.o \
		  ServerI.o \
		  ServerAdapterI.o \
	          Activator.o \
		  WaitQueue.o \
		  PlatformInfo.o

REGISTRY_OBJS   = RegistryI.o \
		  StringApplicationDescriptorDict.o \
		  IdentityObjectInfoDict.o \
		  StringAdapterInfoDict.o \
		  Database.o \
		  AdapterCache.o \
		  ObjectCache.o \
		  ServerCache.o \
		  NodeCache.o \
		  LocatorI.o \
		  LocatorRegistryI.o \
		  AdminI.o \
		  Util.o \
		  DescriptorHelper.o \
		  NodeSessionI.o \
		  ReapThread.o \
		  ObserverSessionI.o \
		  SessionManagerI.o \
		  Topics.o \
		  QueryI.o

NODE_SVR_OBJS 	= $(COMMON_OBJS) \
		  $(NODE_OBJS) \
		  $(REGISTRY_OBJS) \
		  IceGridNode.o

REGISTRY_SVR_OBJS = \
		  $(COMMON_OBJS) \
		  $(REGISTRY_OBJS) \
		  IceGridRegistry.o

SRCS		= $(LIB_OBJS:.o=.cpp) \
		  $(ADMIN_OBJS:.o=.cpp) \
		  $(COMMON_OBJS:.o=.cpp) \
		  $(NODE_OBJS:.o=.cpp) \
		  $(REGISTRY_OBJS:.o=.cpp) \
		  IceGridNode.cpp \
		  IceGridRegistry.cpp

SLICE_SRCS	= $(SDIR)/Admin.ice \
                  $(SDIR)/Exception.ice \
                  $(SDIR)/FileParser.ice \
                  $(SDIR)/Query.ice \
		  $(SDIR)/Observer.ice \
		  $(SDIR)/Descriptor.ice \
		  Internal.ice

HDIR		= $(includedir)/IceGrid
LOCAL_HDIR	= ../IceGrid
SDIR		= $(slicedir)/IceGrid
LOCAL_SDIR	= ../IceGrid

SLICE2FREEZECMD = $(SLICE2FREEZE) --ice --include-dir IceGrid $(ICECPPFLAGS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= $(CPPFLAGS) -I.. -DICE_GRID_API_EXPORTS $(READLINE_FLAGS)
ICECPPFLAGS	:= $(ICECPPFLAGS) -I..
SLICE2CPPFLAGS 	:= --checksum --ice --include-dir IceGrid --dll-export ICE_GRID_API $(SLICE2CPPFLAGS)
LINKWITH        := -lGlacier2 $(BZIP2_RPATH_LINK) -lIce -lIceUtil

$(libdir)/$(LIBFILENAME): $(LIB_OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(LIB_OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

$(ADMIN): $(ADMIN_OBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(ADMIN_OBJS) -lGlacier2 $(EXPAT_RPATH_LINK) -lIceXML -lIceGrid $(LIBS) $(READLINE_LIBS)

$(REGISTRY_SERVER): $(REGISTRY_SVR_OBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(REGISTRY_SVR_OBJS) -lIceGrid -lIceStormService -lGlacier2 $(DB_RPATH_LINK) -lFreeze \
	-lIceBox $(EXPAT_RPATH_LINK) -lIceXML $(LIBS)

$(NODE_SERVER): $(NODE_SVR_OBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(NODE_SVR_OBJS) -lIceGrid -lIceStormService -lIceBox -lGlacier2 $(OPENSSL_RPATH_LINK) -lIcePatch2 $(DB_RPATH_LINK) -lFreeze $(EXPAT_RPATH_LINK) -lIceXML \
	$(LIBS)

$(LOCAL_HDIR)/%.h %.cpp: $(SDIR)/%.ice $(SLICE2CPP)
	rm -f $(HDIR)/$(*F).h $(*F).cpp
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)/$(*F).ice

$(LOCAL_HDIR)/StringApplicationDescriptorDict.h StringApplicationDescriptorDict.cpp: $(SLICE2FREEZE)
	rm -f StringApplicationDescriptorDict.h StringApplicationDescriptorDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringApplicationDescriptorDict,string,IceGrid::ApplicationDescriptor \
	StringApplicationDescriptorDict ../../slice/IceGrid/Admin.ice

clean::
	rm -f StringApplicationDescriptorDict.h StringApplicationDescriptorDict.cpp

$(LOCAL_HDIR)/IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp: $(SLICE2FREEZE)
	rm -f IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::IdentityObjectInfoDict,Ice::Identity,IceGrid::ObjectInfo \
	--dict-index IceGrid::IdentityObjectInfoDict,type \
	IdentityObjectInfoDict ../../slice/Ice/Identity.ice $(LOCAL_SDIR)/Internal.ice

clean::
	rm -f IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp

$(LOCAL_HDIR)/StringAdapterInfoDict.h StringAdapterInfoDict.cpp: $(SLICE2FREEZE)
	rm -f StringAdapterInfoDict.h StringAdapterInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringAdapterInfoDict,string,IceGrid::AdapterInfo \
	--dict-index IceGrid::StringAdapterInfoDict,replicaGroupId StringAdapterInfoDict $(SDIR)/Admin.ice

clean::
	rm -f StringAdapterInfoDict.h StringAdapterInfoDict.cpp

# Needed for make -jn to work.
#../IceGrid/Grammar.y: Grammar.h

clean::
#	rm -f Grammar.cpp Grammar.h
#	rm -f Scanner.cpp

install:: all
	$(call installlib,$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))
	$(call installprogram,$(ADMIN),$(install_bindir))
	$(call installprogram,$(NODE_SERVER),$(install_bindir))
	$(call installprogram,$(REGISTRY_SERVER),$(install_bindir))

include .depend
