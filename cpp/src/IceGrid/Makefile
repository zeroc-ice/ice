# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

ADMIN		= $(bindir)/icegridadmin
DB		= $(bindir)/icegriddb
NODE_SERVER	= $(bindir)/icegridnode
REGISTRY_SERVER	= $(bindir)/icegridregistry

TARGETS         = $(NODE_SERVER) $(REGISTRY_SERVER) $(ADMIN) $(DB)

CSLICE_OBJS	= Internal.o \
		  IceLocatorDiscovery.o

ADMIN_OBJS	= Client.o \
		  DescriptorBuilder.o \
		  DescriptorHelper.o \
		  DescriptorParser.o \
		  FileParserI.o \
		  Grammar.o \
		  Parser.o \
		  Scanner.o \
		  Util.o \
		  $(CSLICE_OBJS)

COMMON_OBJS	= AdminRouter.o \
		  DescriptorBuilder.o \
		  DescriptorParser.o \
		  FileCache.o \
		  PlatformInfo.o \
		  SessionManager.o \
		  TraceLevels.o \
		  $(CSLICE_OBJS)

NODE_OBJS	= Activator.o \
		  NodeAdminRouter.o \
		  NodeI.o \
		  NodeSessionManager.o \
		  ServerAdapterI.o \
		  ServerI.o

REGISTRY_OBJS   = AdminCallbackRouter.o \
		  AdapterCache.o \
		  AdminI.o \
		  AdminSessionI.o \
		  Allocatable.o \
		  AllocatableObjectCache.o \
		  Database.o \
		  DescriptorHelper.o \
		  FileUserAccountMapperI.o \
		  InternalRegistryI.o \
		  LocatorI.o \
		  LocatorRegistryI.o \
		  NodeCache.o \
		  NodeSessionI.o \
		  ObjectCache.o \
		  PluginFacadeI.o \
		  QueryI.o \
		  ReapThread.o \
		  RegistryAdminRouter.o \
		  RegistryI.o \
		  ReplicaCache.o \
		  ReplicaSessionI.o \
		  ReplicaSessionManager.o \
		  ServerCache.o \
		  SessionI.o \
		  SessionServantManager.o \
		  Topics.o \
		  Util.o \
		  WellKnownObjectsManager.o

NODE_SVR_OBJS 	= $(COMMON_OBJS) \
		  $(NODE_OBJS) \
		  $(REGISTRY_OBJS) \
		  IceGridNode.o

REGISTRY_SVR_OBJS = \
		  $(COMMON_OBJS) \
		  $(REGISTRY_OBJS) \
		  IceGridRegistry.o

DSLICE_OBJS	= DBTypes.o

DB_OBJS		= IceGridDB.o \
		  $(DSLICE_OBJS)

OBJS		= $(ADMIN_OBJS) \
		  $(COMMON_OBJS) \
		  $(NODE_OBJS) \
		  $(NODE_SVR_OBJS) \
		  $(REGISTRY_SVR_OBJS) \
		  $(DB_OBJS)

SLICE_OBJS	= $(CSLICE_OBJS) \
		  $(DSLICE_OBJS)

SDIR		= $(slicedir)/IceGrid

RPATH_DIR	= $(LOADER_PATH)/../$(libsubdir)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= $(CPPFLAGS) -I.. $(READLINE_FLAGS)
ICECPPFLAGS	:= $(ICECPPFLAGS) -I..
SLICE2CPPFLAGS 	:= --checksum --ice --include-dir IceGrid $(SLICE2CPPFLAGS)
SLICE2FREEZECMD	:= $(SLICE2FREEZE) --ice --include-dir IceGrid $(ICECPPFLAGS)

$(ADMIN): $(ADMIN_OBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(ADMIN_OBJS) -lGlacier2 $(EXPAT_RPATH_LINK) -lIceXML -lIceGrid -lIcePatch2 -lIceBox \
	$(LIBS) $(READLINE_LIBS) $(OPENSSL_RPATH_LINK)

$(DB): $(DB_OBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(DB_OBJS) -lIceGrid -lGlacier2 -lIcePatch2 -lIceDB $(LIBS)

$(REGISTRY_SERVER): $(REGISTRY_SVR_OBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(REGISTRY_SVR_OBJS) -lIceGrid -lIceStorm -lIceStormService -lGlacier2 -lIcePatch2 \
	$(RPATH_LINK) -lIceDB -lIceBox $(EXPAT_RPATH_LINK) -lIceXML -lIceSSL $(OPENSSL_RPATH_LINK) $(LIBS)

$(NODE_SERVER): $(NODE_SVR_OBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(NODE_SVR_OBJS) -lIceGrid -lIceStorm -lIceStormService -lIceBox -lGlacier2 \
	-lIceDB -lIcePatch2 $(EXPAT_RPATH_LINK) -lIceXML -lIceSSL $(OPENSSL_RPATH_LINK) $(LIBS)

../IceLocatorDiscovery/%.h %.cpp: $(slicedir)/IceLocatorDiscovery/%.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	rm -f $(*F).h $(*F).cpp
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(slicedir)/IceLocatorDiscovery/$(*F).ice
	@touch $(*F).cpp
	@mkdir -p .depend
	@$(SLICE2CPP) $(SLICE2CPPFLAGS) --depend $(slicedir)/IceLocatorDiscovery/$(*F).ice > .depend/$(*F).ice.d

install:: all
	$(call installprogram,$(ADMIN),$(DESTDIR)$(install_bindir))
	$(call installprogram,$(DB),$(DESTDIR)$(install_bindir))
	$(call installdata,$(top_srcdir)/../man/man1/icegridadmin.1,$(DESTDIR)$(install_mandir))
	$(call installprogram,$(NODE_SERVER),$(DESTDIR)$(install_bindir))
	$(call installdata,$(top_srcdir)/../man/man1/icegridnode.1,$(DESTDIR)$(install_mandir))
	$(call installprogram,$(REGISTRY_SERVER),$(DESTDIR)$(install_bindir))
	$(call installdata,$(top_srcdir)/../man/man1/icegridregistry.1,$(DESTDIR)$(install_mandir))
