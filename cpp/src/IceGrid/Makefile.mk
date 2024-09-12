#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

local_node_srcs         = Activator.cpp \
                          NodeAdminRouter.cpp \
                          NodeI.cpp \
                          NodeSessionManager.cpp \
                          ServerAdapterI.cpp \
                          ServerI.cpp

local_registry_srcs     = Internal.ice \
                          AdminRouter.cpp \
                          DescriptorBuilder.cpp \
                          DescriptorParser.cpp \
                          FileCache.cpp \
                          PlatformInfo.cpp \
                          SessionManager.cpp \
                          TraceLevels.cpp \
                          AdminCallbackRouter.cpp \
                          AdapterCache.cpp \
                          AdminI.cpp \
                          AdminSessionI.cpp \
                          Allocatable.cpp \
                          AllocatableObjectCache.cpp \
                          Database.cpp \
                          DescriptorHelper.cpp \
                          FileUserAccountMapperI.cpp \
                          InternalRegistryI.cpp \
                          LocatorI.cpp \
                          LocatorRegistryI.cpp \
                          NodeCache.cpp \
                          NodeSessionI.cpp \
                          ObjectCache.cpp \
                          PluginFacadeI.cpp \
                          QueryI.cpp \
                          ReapThread.cpp \
                          RegistryAdminRouter.cpp \
                          RegistryI.cpp \
                          ReplicaCache.cpp \
                          ReplicaSessionI.cpp \
                          ReplicaSessionManager.cpp \
                          ServerCache.cpp \
                          SessionI.cpp \
                          SessionServantManager.cpp \
                          SynchronizationException.cpp \
                          Topics.cpp \
                          Util.cpp \
                          WellKnownObjectsManager.cpp

local_admin_srcs        = Internal.ice \
                          Client.cpp \
                          DescriptorBuilder.cpp \
                          DescriptorHelper.cpp \
                          DescriptorParser.cpp \
                          FileParserI.cpp \
                          Grammar.cpp \
                          Parser.cpp \
                          Scanner.cpp \
                          Util.cpp

$(project)_programs             = icegridnode icegridregistry icegridadmin
$(project)_dependencies         := IceGrid Glacier2 Ice
$(project)_targetdir            := $(bindir)

# This is not necessary for the icegridadmin sources. However, we want to build all objects with the same flags to
# reuse common object files in the different programs.
$(project)_cppflags             := $(if $(lmdb_includedir),-I$(lmdb_includedir))

icegridnode_sources             := $(addprefix $(currentdir)/,$(local_node_srcs) $(local_registry_srcs) IceGridNode.cpp) \
                                   $(slicedir)/IceLocatorDiscovery/IceLocatorDiscovery.ice
icegridnode_dependencies        := IceBox IceStormService IceStorm IceXML IceDB

icegridregistry_sources         := $(addprefix $(currentdir)/,$(local_registry_srcs) IceGridRegistry.cpp) \
                                   $(slicedir)/IceLocatorDiscovery/IceLocatorDiscovery.ice
icegridregistry_dependencies    := IceBox IceStormService IceStorm IceXML IceDB $(local_dependencies)

icegridadmin_sources            := $(addprefix $(currentdir)/,$(local_admin_srcs))
icegridadmin_dependencies       := IceBox IceXML IceLocatorDiscovery

projects += $(project)
