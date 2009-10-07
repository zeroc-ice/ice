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

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= $(CPPFLAGS) -I.. $(OPENSSL_FLAGS) $(READLINE_FLAGS)
ICECPPFLAGS	:= $(ICECPPFLAGS) -I..
SLICE2CPPFLAGS 	:= --checksum --ice --include-dir IceGrid $(SLICE2CPPFLAGS)

$(ADMIN): $(ADMIN_OBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(ADMIN_OBJS) -lGlacier2 $(EXPAT_RPATH_LINK) -lIceXML -lIceGrid -lIcePatch2 -lIceBox \
	$(LIBS) $(READLINE_LIBS) $(OPENSSL_RPATH_LINK)

$(REGISTRY_SERVER): $(REGISTRY_SVR_OBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(REGISTRY_SVR_OBJS) -lIceGrid -lIceStormService -lIceStormDB -lGlacier2 -lIcePatch2 \
	-lIceDB -lIceBox $(EXPAT_RPATH_LINK) -lIceXML -lIceSSL $(OPENSSL_LIBS) $(LIBS)

$(NODE_SERVER): $(NODE_SVR_OBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(NODE_SVR_OBJS) -lIceGrid -lIceStormService -lIceStormDB -lIceBox -lGlacier2 \
	-lIcePatch2 -lIceDB $(EXPAT_RPATH_LINK) -lIceXML -lIceSSL $(OPENSSL_LIBS) $(LIBS)


SUBDIRS = FreezeDB
ifneq ($(QT_HOME),)
   SUBDIRS += SqlDB
endif

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    if test -d $$subdir ; \
	    then \
		echo "making $@ in $$subdir"; \
		( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	    fi; \
	done

# Needed for make -jn to work.
#../IceGrid/Grammar.y: Grammar.h

install:: all
	$(call installprogram,$(ADMIN),$(install_bindir))
	$(call installprogram,$(NODE_SERVER),$(install_bindir))
	$(call installprogram,$(REGISTRY_SERVER),$(install_bindir))

include .depend
