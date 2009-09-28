# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

# Set appropriately if building IceGrid to use SQL database
#QTSQL_HOME	= /opt/qt

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
	          Allocatable.o \
		  AdapterCache.o \
		  ObjectCache.o \
		  AllocatableObjectCache.o \
		  ServerCache.o \
		  NodeCache.o \
		  ReplicaCache.o \
		  Database.o \
		  DatabaseCache.o \
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

ifneq ($(QTSQL_HOME),)
ICESQL_DIR	= $(top_srcdir)/src/IceSQL

%.o: $(ICESQL_DIR)/%.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

REGISTRY_OBJS  += SqlStringApplicationInfoDict.o \
		  SqlIdentityObjectInfoDict.o \
		  SqlStringAdapterInfoDict.o

ICESQL_OBJS	= SqlTypes.o
else
REGISTRY_OBJS  += StringApplicationInfoDict.o \
		  IdentityObjectInfoDict.o \
		  StringAdapterInfoDict.o
endif

NODE_SVR_OBJS 	= $(COMMON_OBJS) \
		  $(NODE_OBJS) \
		  $(REGISTRY_OBJS) \
		  IceGridNode.o

ifneq ($(QTSQL_HOME),)
NODE_SVR_OBJS  += $(ICESQL_OBJS)
endif

REGISTRY_SVR_OBJS = \
		  $(COMMON_OBJS) \
		  $(REGISTRY_OBJS) \
		  IceGridRegistry.o

ifneq ($(QTSQL_HOME),)
REGISTRY_SVR_OBJS += \
		  $(ICESQL_OBJS)
endif

SRCS		= $(ADMIN_OBJS:.o=.cpp) \
		  $(COMMON_OBJS:.o=.cpp) \
		  $(NODE_OBJS:.o=.cpp) \
		  $(REGISTRY_OBJS:.o=.cpp) \
		  IceGridNode.cpp \
		  IceGridRegistry.cpp

ifneq ($(QTSQL_HOME),)
SRCS	       += $(patsubst %.o,$(ICESQL_DIR)/%.cpp,$(ICESQL_DIR))
endif

SLICE_SRCS	= ../IceGrid/Internal.ice

HDIR		= $(headerdir)/IceGrid
SDIR		= $(slicedir)/IceGrid
LOCAL_HDIR	= ../IceGrid
LOCAL_SDIR	= ../IceGrid

SLICE2FREEZECMD = $(SLICE2FREEZE) --ice --include-dir IceGrid $(ICECPPFLAGS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= $(CPPFLAGS) -I.. $(OPENSSL_FLAGS) $(READLINE_FLAGS) $(QTSQL_FLAGS)
ICECPPFLAGS	:= $(ICECPPFLAGS) -I..
SLICE2CPPFLAGS 	:= --checksum --ice --include-dir IceGrid $(SLICE2CPPFLAGS)

ifneq ($(QTSQL_HOME),)
DBLINKWITH      += $(QTSQL_RPATH_LINK) $(QTSQL_LIBS)
else
DBLINKWITH      += $(DB_RPATH_LINK) -lFreeze
endif

$(ADMIN): $(ADMIN_OBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(ADMIN_OBJS) -lGlacier2 $(EXPAT_RPATH_LINK) -lIceXML -lIceGrid -lIcePatch2 -lIceBox \
	$(LIBS) $(READLINE_LIBS) $(OPENSSL_RPATH_LINK)

$(REGISTRY_SERVER): $(REGISTRY_SVR_OBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(REGISTRY_SVR_OBJS) -lIceGrid -lIceStormService -lGlacier2 -lIcePatch2 \
	$(DBLINKWITH) -lIceBox $(EXPAT_RPATH_LINK) -lIceXML -lIceSSL $(OPENSSL_LIBS) $(LIBS)

$(NODE_SERVER): $(NODE_SVR_OBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(NODE_SVR_OBJS) -lIceGrid -lIceStormService -lIceBox -lGlacier2 -lIcePatch2 \
	$(DBLINKWITH) $(EXPAT_RPATH_LINK) -lIceXML -lIceSSL $(OPENSSL_LIBS) $(LIBS)

ifneq ($(QTSQL_HOME),)
# Required because of inclusion in .depend
$(LOCAL_HDIR)/StringApplicationInfoDict.h:
	touch $@

$(LOCAL_HDIR)/IdentityObjectInfoDict.h:
	touch $@

$(LOCAL_HDIR)/StringAdapterInfoDict.h:
	touch $@
else
$(LOCAL_HDIR)/StringApplicationInfoDict.h StringApplicationInfoDict.cpp: $(LOCAL_SDIR)/Internal.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f StringApplicationInfoDict.h StringApplicationInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringApplicationInfoDict,string,IceGrid::ApplicationInfo \
	StringApplicationInfoDict $(LOCAL_SDIR)/Internal.ice

$(LOCAL_HDIR)/IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp: $(slicedir)/Ice/Identity.ice $(LOCAL_SDIR)/Internal.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::IdentityObjectInfoDict,Ice::Identity,IceGrid::ObjectInfo \
	--dict-index IceGrid::IdentityObjectInfoDict,type \
	IdentityObjectInfoDict $(slicedir)/Ice/Identity.ice $(LOCAL_SDIR)/Internal.ice

$(LOCAL_HDIR)/StringAdapterInfoDict.h StringAdapterInfoDict.cpp: $(SDIR)/Admin.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f StringAdapterInfoDict.h StringAdapterInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringAdapterInfoDict,string,IceGrid::AdapterInfo \
	--dict-index IceGrid::StringAdapterInfoDict,replicaGroupId StringAdapterInfoDict $(SDIR)/Admin.ice
endif

clean::
	-rm -f StringApplicationInfoDict.h StringApplicationInfoDict.cpp
	rm -f IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp
	rm -f StringAdapterInfoDict.h StringAdapterInfoDict.cpp

# Needed for make -jn to work.
#../IceGrid/Grammar.y: Grammar.h

install:: all
	$(call installprogram,$(ADMIN),$(install_bindir))
	$(call installprogram,$(NODE_SERVER),$(install_bindir))
	$(call installprogram,$(REGISTRY_SERVER),$(install_bindir))

include .depend
