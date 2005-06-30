# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

LIBTARGETS	= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))
TARGETS         = $(LIBTARGETS) $(NODE_SERVER) $(REGISTRY_SERVER) $(ADMIN)

LIB_OBJS	= Admin.o \
		  Query.o \
	 	  Exception.o

ADMIN_OBJS	= Grammar.o \
		  Scanner.o \
		  Parser.o \
                  DescriptorVisitor.o \
                  DescriptorParser.o \
                  DescriptorUtil.o \
		  Client.o

COMMON_OBJS	= Internal.o \
		  TraceLevels.o

NODE_OBJS	= NodeI.o \
	          ServerFactory.o \
		  ServerI.o \
		  ServerAdapterI.o \
	          Activator.o \
		  WaitQueue.o \
                  DescriptorParser.o

REGISTRY_OBJS   = Registry.o \
	 	  ObjectRegistryI.o \
		  AdapterRegistryI.o \
		  ServerRegistryI.o \
		  NodeRegistryI.o \
	          ApplicationRegistryI.o \
		  StringObjectProxyDict.o \
		  StringObjectProxySeqDict.o \
		  StringServerDescriptorDict.o \
                  StringStringSeqDict.o \
		  IdentityObjectDescDict.o \
		  LocatorI.o \
		  LocatorRegistryI.o \
		  AdapterFactory.o \
		  AdapterI.o \
		  AdminI.o \
		  DescriptorVisitor.o \
                  DescriptorUtil.o \
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
		  Internal.ice

HDIR		= $(includedir)/IcePack
LOCAL_HDIR	= ../IcePack
SDIR		= $(slicedir)/IcePack
LOCAL_SDIR	= ../IcePack

SLICE2FREEZECMD = $(SLICE2FREEZE) --ice --include-dir IcePack $(ICECPPFLAGS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= $(CPPFLAGS) -I.. -DICE_PACK_API_EXPORTS $(READLINE_FLAGS)
ICECPPFLAGS	:= $(ICECPPFLAGS) -I..
SLICE2CPPFLAGS 	:= --checksum --ice --include-dir IcePack --dll-export ICE_PACK_API $(SLICE2CPPFLAGS)
LINKWITH        := -lIce -lIceUtil

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
	$(CXX) $(LDFLAGS) -o $@ $(ADMIN_OBJS) -lIceXML -lIcePack $(LIBS) $(READLINE_LIBS) $(EXPAT_LIBS)

$(REGISTRY_SERVER): $(REGISTRY_SVR_OBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(REGISTRY_SVR_OBJS) -lIcePack -lFreeze -lIceXML $(LIBS) $(DB_LIBS)

$(NODE_SERVER): $(NODE_SVR_OBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(NODE_SVR_OBJS) -lIcePack -lFreeze -lIceXML $(LIBS) $(DB_LIBS)

$(LOCAL_HDIR)/%.h %.cpp: $(SDIR)/%.ice $(SLICE2CPP)
	rm -f $(HDIR)/$(*F).h $(*F).cpp
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)/$(*F).ice

$(LOCAL_HDIR)/StringObjectProxyDict.h StringObjectProxyDict.cpp: $(SLICE2FREEZE)
	rm -f StringObjectProxyDict.h StringObjectProxyDict.cpp
	$(SLICE2FREEZECMD) --dict IcePack::StringObjectProxyDict,string,Object* \
	StringObjectProxyDict ../../slice/Ice/BuiltinSequences.ice

clean::
	rm -f StringObjectProxyDict.h StringObjectProxyDict.cpp

$(LOCAL_HDIR)/StringStringSeqDict.h StringStringSeqDict.cpp: $(SLICE2FREEZE)
	rm -f StringStringSeqDict.h StringStringSeqDict.cpp
	$(SLICE2FREEZECMD) --dict IcePack::StringStringSeqDict,string,Ice::StringSeq \
	StringStringSeqDict ../../slice/Ice/BuiltinSequences.ice

clean::
	rm -f StringStringSeqDict.h StringStringSeqDict.cpp

$(LOCAL_HDIR)/StringServerDescriptorDict.h StringServerDescriptorDict.cpp: $(SLICE2FREEZE)
	rm -f StringServerDescriptorDict.h StringServerDescriptorDict.cpp
	$(SLICE2FREEZECMD) --dict IcePack::StringServerDescriptorDict,string,IcePacK::ServerDescriptor \
	StringServerDescriptorDict ../../slice/IcePack/Admin.ice

clean::
	rm -f StringServerDescriptorDict.h StringServerDescriptorDict.cpp

$(LOCAL_HDIR)/IdentityObjectDescDict.h IdentityObjectDescDict.cpp: $(SLICE2FREEZE)
	rm -f IdentityObjectDescDict.h IdentityObjectDescDict.cpp
	$(SLICE2FREEZECMD) --dict IcePack::IdentityObjectDescDict,Ice::Identity,IcePack::ObjectDescriptor \
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
#../IcePack/Grammar.y: Grammar.h

clean::
#	rm -f Grammar.cpp Grammar.h
#	rm -f Scanner.cpp

install:: all
	$(call installlib,$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))
	$(call installprogram,$(ADMIN),$(install_bindir))
	$(call installprogram,$(NODE_SERVER),$(install_bindir))
	$(call installprogram,$(REGISTRY_SERVER),$(install_bindir))

include .depend
