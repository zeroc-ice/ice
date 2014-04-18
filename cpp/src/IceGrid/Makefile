# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

ADMIN		= $(bindir)/icegridadmin
NODE_SERVER	= $(bindir)/icegridnode
REGISTRY_SERVER	= $(bindir)/icegridregistry

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
		  PlatformInfo.o \
		  SessionManager.o

NODE_OBJS	= NodeI.o \
                  NodeServerAdminRouter.o \
		  ServerI.o \
		  ServerAdapterI.o \
	          Activator.o \
		  NodeSessionManager.o

REGISTRY_OBJS   = AdminCallbackRouter.o \
                  RegistryI.o \
		  PluginFacadeI.o \
                  RegistryServerAdminRouter.o \
	  	  InternalRegistryI.o \
	          Allocatable.o \
		  AdapterCache.o \
		  ObjectCache.o \
		  AllocatableObjectCache.o \
		  ServerCache.o \
		  NodeCache.o \
		  ReplicaCache.o \
		  Database.o \
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
		  WellKnownObjectsManager.o \
		  StringApplicationInfoDict.o \
		  IdentityObjectInfoDict.o \
		  StringAdapterInfoDict.o \
		  SerialsDict.o

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

ifeq ($(CPP11),yes)
RPATH_DIR	= @loader_path/../../lib/c++11
else
RPATH_DIR	= @loader_path/../lib
endif

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= $(CPPFLAGS) -I.. $(OPENSSL_FLAGS) $(READLINE_FLAGS)
ICECPPFLAGS	:= $(ICECPPFLAGS) -I..
SLICE2CPPFLAGS 	:= --checksum --ice --include-dir IceGrid $(SLICE2CPPFLAGS)
SLICE2FREEZECMD	:= $(SLICE2FREEZE) --ice --include-dir IceGrid $(ICECPPFLAGS)

$(ADMIN): $(ADMIN_OBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(ADMIN_OBJS) -lGlacier2 $(EXPAT_RPATH_LINK) -lIceXML -lIceGrid -lIcePatch2 -lIceBox \
	$(LIBS) $(READLINE_LIBS) $(OPENSSL_RPATH_LINK)

$(REGISTRY_SERVER): $(REGISTRY_SVR_OBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(REGISTRY_SVR_OBJS) -lIceGrid -lIceStorm -lIceStormService -lGlacier2 -lIcePatch2 \
	-lFreeze -lIceBox $(EXPAT_RPATH_LINK) -lIceXML -lIceSSL $(OPENSSL_LIBS) $(LIBS)

$(NODE_SERVER): $(NODE_SVR_OBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(NODE_SVR_OBJS) -lIceGrid -lIceStorm -lIceStormService -lIceBox -lGlacier2 \
	-lFreeze -lIcePatch2 $(EXPAT_RPATH_LINK) -lIceXML -lIceSSL $(OPENSSL_LIBS) $(LIBS)

# The slice2freeze rules are structured like this to avoid issues with
# parallel make.
../IceGrid/StringApplicationInfoDict.h: StringApplicationInfoDict.cpp
StringApplicationInfoDict.cpp: $(SDIR)/Admin.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f StringApplicationInfoDict.h StringApplicationInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringApplicationInfoDict,string,IceGrid::ApplicationInfo \
	StringApplicationInfoDict $(SDIR)/Admin.ice

../IceGrid/IdentityObjectInfoDict.h: IdentityObjectInfoDict.cpp
IdentityObjectInfoDict.cpp: $(slicedir)/Ice/Identity.ice $(SDIR)/Admin.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::IdentityObjectInfoDict,Ice::Identity,IceGrid::ObjectInfo \
	--dict-index IceGrid::IdentityObjectInfoDict,type \
	IdentityObjectInfoDict $(slicedir)/Ice/Identity.ice $(SDIR)/Admin.ice

../IceGrid/StringAdapterInfoDict.h: StringAdapterInfoDict.cpp
StringAdapterInfoDict.cpp: $(SDIR)/Admin.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f StringAdapterInfoDict.h StringAdapterInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringAdapterInfoDict,string,IceGrid::AdapterInfo \
	--dict-index IceGrid::StringAdapterInfoDict,replicaGroupId StringAdapterInfoDict $(SDIR)/Admin.ice

../IceGrid/SerialsDict.h: SerialsDict.cpp
SerialsDict.cpp: $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f SerialsDict.h SerialsDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::SerialsDict,string,long SerialsDict

clean::
	-rm -f StringApplicationInfoDict.h StringApplicationInfoDict.cpp
	-rm -f IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp
	-rm -f StringAdapterInfoDict.h StringAdapterInfoDict.cpp
	-rm -f SerialsDict.h SerialsDict.cpp

install:: all
	$(call installprogram,$(ADMIN),$(DESTDIR)$(install_bindir))
	$(call installdata,$(top_srcdir)/../man/man1/icegridadmin.1,$(DESTDIR)$(install_mandir))
	$(call installprogram,$(NODE_SERVER),$(DESTDIR)$(install_bindir))
	$(call installdata,$(top_srcdir)/../man/man1/icegridnode.1,$(DESTDIR)$(install_mandir))
	$(call installprogram,$(REGISTRY_SERVER),$(DESTDIR)$(install_bindir))
	$(call installdata,$(top_srcdir)/../man/man1/icegridregistry.1,$(DESTDIR)$(install_mandir))


include .depend
