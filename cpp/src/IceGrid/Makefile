# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

ADMIN		= $(top_srcdir)/bin/icegridadmin
NODE_SERVER	= $(top_srcdir)/bin/icegridnode
REGISTRY_SERVER	= $(top_srcdir)/bin/icegridregistry

TARGETS         = $(NODE_SERVER) $(REGISTRY_SERVER) $(ADMIN)

ADMIN_OBJS	= Grammar.o \
		  Scanner.o \
		  Parser.o \
                  DescriptorParser.o \
                  DescriptorBuilder.o \
		  DescriptorHelper.o \
                  FileParserI.o \
		  Util.o \
		  Internal.o \
		  Client.o

COMMON_OBJS	= Internal.o \
                  DescriptorParser.o \
                  DescriptorBuilder.o \
		  TraceLevels.o \
		  FileCache.o \
		  PlatformInfo.o

NODE_OBJS	= NodeI.o \
                  NodeServerAdminRouter.o \
		  ServerI.o \
		  ServerAdapterI.o \
	          Activator.o \
		  NodeSessionManager.o

REGISTRY_OBJS   = AdminCallbackRouter.o \
                  RegistryI.o \
                  RegistryServerAdminRouter.o \
	  	  InternalRegistryI.o \
		  StringApplicationInfoDict.o \
		  IdentityObjectInfoDict.o \
		  StringAdapterInfoDict.o \
		  Database.o \
	          Allocatable.o \
		  AdapterCache.o \
		  ObjectCache.o \
		  AllocatableObjectCache.o \
		  ServerCache.o \
		  NodeCache.o \
		  ReplicaCache.o \
		  LocatorI.o \
		  LocatorRegistryI.o \
		  AdminI.o \
		  Util.o \
		  DescriptorHelper.o \
		  NodeSessionI.o \
		  ReplicaSessionI.o \
		  ReapThread.o \
		  SessionI.o \
		  AdminSessionI.o \
		  SessionServantManager.o \
		  Topics.o \
		  QueryI.o \
		  FileUserAccountMapperI.o \
		  ReplicaSessionManager.o \
		  WellKnownObjectsManager.o

NODE_SVR_OBJS 	= $(COMMON_OBJS) \
		  $(NODE_OBJS) \
		  $(REGISTRY_OBJS) \
		  IceGridNode.o

REGISTRY_SVR_OBJS = \
		  $(COMMON_OBJS) \
		  $(REGISTRY_OBJS) \
		  IceGridRegistry.o

SRCS		= $(ADMIN_OBJS:.o=.cpp) \
		  $(COMMON_OBJS:.o=.cpp) \
		  $(NODE_OBJS:.o=.cpp) \
		  $(REGISTRY_OBJS:.o=.cpp) \
		  IceGridNode.cpp \
		  IceGridRegistry.cpp

SLICE_SRCS	= ../IceGrid/Internal.ice

HDIR		= $(headerdir)/IceGrid
SDIR		= $(slicedir)/IceGrid
LOCAL_HDIR	= ../IceGrid
LOCAL_SDIR	= ../IceGrid

SLICE2FREEZECMD = $(SLICE2FREEZE) --ice --include-dir IceGrid $(ICECPPFLAGS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= $(CPPFLAGS) -I.. $(OPENSSL_FLAGS) $(READLINE_FLAGS)
ICECPPFLAGS	:= $(ICECPPFLAGS) -I..
SLICE2CPPFLAGS 	:= --checksum --ice --include-dir IceGrid $(SLICE2CPPFLAGS)
LINKWITH        := -lGlacier2 $(BZIP2_RPATH_LINK) -lIce -lIceUtil

$(ADMIN): $(ADMIN_OBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(ADMIN_OBJS) -lGlacier2 $(EXPAT_RPATH_LINK) -lIceXML -lIceGrid -lIcePatch2 -lIceBox \
	$(LIBS) $(READLINE_LIBS) $(OPENSSL_RPATH_LINK)

$(REGISTRY_SERVER): $(REGISTRY_SVR_OBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(REGISTRY_SVR_OBJS) -lIceGrid -lIceStormService -lGlacier2 -lIcePatch2 \
	$(DB_RPATH_LINK) -lFreeze -lIceBox $(EXPAT_RPATH_LINK) -lIceXML -lIceSSL $(OPENSSL_LIBS) $(LIBS)

$(NODE_SERVER): $(NODE_SVR_OBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(NODE_SVR_OBJS) -lIceGrid -lIceStormService -lIceBox -lGlacier2 -lIcePatch2 \
	$(DB_RPATH_LINK) -lFreeze $(EXPAT_RPATH_LINK) -lIceXML -lIceSSL $(OPENSSL_LIBS) $(LIBS)

$(LOCAL_HDIR)/StringApplicationInfoDict.h StringApplicationInfoDict.cpp: $(LOCAL_SDIR)/Internal.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f StringApplicationInfoDict.h StringApplicationInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringApplicationInfoDict,string,IceGrid::ApplicationInfo \
	StringApplicationInfoDict $(LOCAL_SDIR)/Internal.ice

clean::
	-rm -f StringApplicationInfoDict.h StringApplicationInfoDict.cpp

$(LOCAL_HDIR)/IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp: $(slicedir)/Ice/Identity.ice $(LOCAL_SDIR)/Internal.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::IdentityObjectInfoDict,Ice::Identity,IceGrid::ObjectInfo \
	--dict-index IceGrid::IdentityObjectInfoDict,type \
	IdentityObjectInfoDict $(slicedir)/Ice/Identity.ice $(LOCAL_SDIR)/Internal.ice

clean::
	rm -f IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp

$(LOCAL_HDIR)/StringAdapterInfoDict.h StringAdapterInfoDict.cpp: $(SDIR)/Admin.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f StringAdapterInfoDict.h StringAdapterInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringAdapterInfoDict,string,IceGrid::AdapterInfo \
	--dict-index IceGrid::StringAdapterInfoDict,replicaGroupId StringAdapterInfoDict $(SDIR)/Admin.ice

clean::
	rm -f StringAdapterInfoDict.h StringAdapterInfoDict.cpp

# Needed for make -jn to work.
#../IceGrid/Grammar.y: Grammar.h

install:: all
	$(call installprogram,$(ADMIN),$(install_bindir))
	$(call installprogram,$(NODE_SERVER),$(install_bindir))
	$(call installprogram,$(REGISTRY_SERVER),$(install_bindir))

include .depend
