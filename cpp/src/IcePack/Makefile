# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,IcePack,$(VERSION))
SONAME          = $(call mksoname,IcePack,$(SOVERSION))  
LIBNAME		= $(call mklibname,IcePack)

ADMIN		= $(top_srcdir)/bin/icepackadmin
NODE_SERVER	= $(top_srcdir)/bin/icepacknode
REGISTRY_SERVER	= $(top_srcdir)/bin/icepackregistry

TARGETS		= $(libdir)/$(LIBFILENAME) $(libdir)/$(SONAME) $(libdir)/$(LIBNAME) \
                  $(NODE_SERVER) $(REGISTRY_SERVER) $(ADMIN)

LIB_OBJS	= Admin.o \
		  Query.o \
	 	  Exception.o

ADMIN_OBJS	= Grammar.o \
		  Scanner.o \
		  Parser.o \
		  Client.o

COMMON_OBJS	= Internal.o \
		  TraceLevels.o \
		  ComponentBuilder.o

NODE_OBJS	= NodeInfo.o \
		  NodeI.o \
		  ServiceBuilder.o \
		  ServerBuilder.o \
		  ServerI.o \
		  ServerAdapterI.o \
		  ServerFactory.o \
		  ServerDeployerI.o \
	          Activator.o \
		  ActivatorI.o \
		  WaitQueue.o

REGISTRY_OBJS   = Registry.o \
	 	  ObjectRegistryI.o \
		  AdapterRegistryI.o \
		  ServerRegistryI.o \
		  NodeRegistryI.o \
		  StringObjectProxyDict.o \
		  StringObjectProxySeqDict.o \
		  IdentityObjectDescDict.o \
		  LocatorI.o \
		  LocatorRegistryI.o \
		  ApplicationBuilder.o \
		  AdapterFactory.o \
		  AdapterI.o \
		  AdminI.o \
		  QueryI.o

NODE_SVR_OBJS 	= $(COMMON_OBJS) \
		  $(NODE_OBJS) \
		  $(REGISTRY_OBJS) \
		  IcePackNode.o

REGISTRY_SVR_OBJS = \
		  $(COMMON_OBJS) \
		  $(REGISTRY_OBJS) \
		  IcePackRegistry.o

SRCS		= $(LIB_OBJS:.o=.cpp) \
		  $(ADMIN_OBJS:.o=.cpp) \
		  $(COMMON_OBJS:.o=.cpp) \
		  $(NODE_OBJS:.o=.cpp) \
		  $(REGISTRY_OBJS:.o=.cpp) \
		  IcePackNode.cpp \
		  IcePackRegistry.cpp

SLICE_SRCS	= $(SDIR)/Admin.ice \
                  $(SDIR)/Exception.ice \
                  $(SDIR)/Query.ice \
		  Internal.ice \
		  Activator.ice

HDIR		= $(includedir)/IcePack
LOCAL_HDIR	= ../IcePack
SDIR		= $(slicedir)/IcePack
LOCAL_SDIR	= ../IcePack

SLICE2FREEZECMD = $(SLICE2FREEZE) --ice --include-dir IcePack $(ICECPPFLAGS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DICE_PACK_API_EXPORTS $(READLINE_FLAGS)
ICECPPFLAGS	:= -I.. $(ICECPPFLAGS)
SLICE2CPPFLAGS 	:= --ice --include-dir IcePack --dll-export ICE_PACK_API $(SLICE2CPPFLAGS)
LINKWITH        := -lIceBox -lIce -lIceUtil

$(libdir)/$(LIBFILENAME): $(LIB_OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(LIB_OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

$(ADMIN): $(ADMIN_OBJS) $(libdir)/$(LIBNAME)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(ADMIN_OBJS) -lIcePack -lIceBox $(LIBS) $(READLINE_LIBS)

$(REGISTRY_SERVER): $(REGISTRY_SVR_OBJS) $(libdir)/$(LIBNAME)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(REGISTRY_SVR_OBJS) -lIcePack -lIceBox -lFreeze -lIceXML $(LIBS) $(DB_LIBS)

$(NODE_SERVER): $(NODE_SVR_OBJS) $(libdir)/$(LIBNAME)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(NODE_SVR_OBJS) -lIcePack -lIceBox -lFreeze -lIceXML $(LIBS) $(DB_LIBS)

$(LOCAL_HDIR)/%.h %.cpp: $(SDIR)/%.ice $(SLICE2CPP)
	rm -f $(HDIR)/$(*F).h $(*F).cpp
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)/$(*F).ice

$(LOCAL_HDIR)/StringObjectProxyDict.h StringObjectProxyDict.cpp: $(SLICE2FREEZE)
	rm -f StringObjectProxyDict.h StringObjectProxyDict.cpp
	$(SLICE2FREEZECMD) --dict IcePack::StringObjectProxyDict,string,Object* \
	StringObjectProxyDict ../../slice/Ice/BuiltinSequences.ice

clean::
	rm -f StringObjectProxyDict.h StringObjectProxyDict.cpp

$(LOCAL_HDIR)/IdentityObjectDescDict.h IdentityObjectDescDict.cpp: $(SLICE2FREEZE)
	rm -f IdentityObjectDescDict.h IdentityObjectDescDict.cpp
	$(SLICE2FREEZECMD) --dict IcePack::IdentityObjectDescDict,Ice::Identity,IcePack::ObjectDescription \
	IdentityObjectDescDict ../../slice/Ice/Identity.ice $(LOCAL_SDIR)/Internal.ice

clean::
	rm -f IdentityObjectDescDict.h IdentityObjectDescDict.cpp

$(LOCAL_HDIR)/StringObjectProxySeqDict.h StringObjectProxySeqDict.cpp: $(SLICE2FREEZE)
	rm -f StringObjectProxySeqDict.h StringObjectProxySeqDict.cpp
	$(SLICE2FREEZECMD) --dict IcePack::StringObjectProxySeqDict,string,Ice::ObjectProxySeq \
	StringObjectProxySeqDict $(slicedir)/Ice/BuiltinSequences.ice

clean::
	rm -f StringObjectProxySeqDict.h StringObjectProxySeqDict.cpp

# Needed for make -jn to work.
../IcePack/Grammar.y: Grammar.h

clean::
	rm -f Grammar.cpp Grammar.h
	rm -f Scanner.cpp

install:: all
	$(INSTALL_LIBRARY) $(libdir)/$(LIBFILENAME) $(install_libdir)
	rm -f $(install_libdir)/$(SONAME)
	ln -s $(LIBFILENAME) $(install_libdir)/$(SONAME)
	rm -f $(install_libdir)/$(LIBNAME)
	ln -s $(SONAME) $(install_libdir)/$(LIBNAME)
	$(INSTALL_PROGRAM) $(ADMIN) $(install_bindir)
	$(INSTALL_PROGRAM) $(NODE_SERVER) $(install_bindir)
	$(INSTALL_PROGRAM) $(REGISTRY_SERVER) $(install_bindir)

include .depend
