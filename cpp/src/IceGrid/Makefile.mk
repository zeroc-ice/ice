# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

local_node_srcs		= Activator.cpp \
			  NodeAdminRouter.cpp \
			  NodeI.cpp \
			  NodeSessionManager.cpp \
			  ServerAdapterI.cpp \
			  ServerI.cpp

local_registry_srcs 	= Internal.ice \
		     	  AdminRouter.cpp \
			  DescriptorBuilder.cpp \
			  DescriptorParser.cpp \
			  FileCache.cpp \
			  PlatformInfo.cpp \
			  SessionManager.cpp \
			  TraceLevels.cpp \
			  IceLocatorDiscovery.cpp \
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
			  Topics.cpp \
			  Util.cpp \
			  WellKnownObjectsManager.cpp

local_admin_srcs	= Internal.ice \
		     	  Client.cpp \
		     	  DescriptorBuilder.cpp \
		     	  DescriptorHelper.cpp \
		     	  DescriptorParser.cpp \
		     	  FileParserI.cpp \
		     	  Grammar.cpp \
		     	  Parser.cpp \
		     	  Scanner.cpp \
		     	  Util.cpp

$(project)_programs 		= icegridnode icegridregistry icegridadmin icegriddb
$(project)_sliceflags		:= -Isrc --include-dir IceGrid
$(project)_generated_includedir := $(project)/generated/IceGrid
$(project)_dependencies 	:= IceGrid Glacier2 Ice IceUtil
$(project)_targetdir		:= $(bindir)

icegridnode_sources	 	:= $(addprefix $(currentdir)/,$(local_node_srcs) $(local_registry_srcs) IceGridNode.cpp)
icegridnode_dependencies 	:= IceBox IceStormService IceStorm IceXML IceSSL IcePatch2 IceDB

icegridregistry_sources	 	:= $(addprefix $(currentdir)/,$(local_registry_srcs) IceGridRegistry.cpp)
icegridregistry_dependencies 	:= IceBox IceStormService IceStorm IceXML IceSSL IcePatch2 IceDB $(local_dependencies)

icegridadmin_dependencies 	:= IcePatch2 IceBox IceXML
icegridadmin_sources	 	:= $(slicedir)/IceLocatorDiscovery/IceLocatorDiscovery.ice \
				   $(addprefix $(currentdir)/,$(local_admin_srcs))

icegriddb_dependencies 		:= IcePatch2 IceDB
icegriddb_sources	 	:= $(addprefix $(currentdir)/,IceGridDB.cpp DBTypes.ice)

projects += $(project)
