# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,IceGrid,$(VERSION))
SONAME          = $(call mksoname,IceGrid,$(SOVERSION))  
LIBNAME		= $(call mklibname,IceGrid)

PERFLIBFILENAME = $(call mklibfilename,IcePerf,$(VERSION))
PERFSONAME      = $(call mksoname,IcePerf,$(SOVERSION))  
PERFLIBNAME	= $(call mklibname,IcePerf)

ADMIN		= $(top_srcdir)/bin/icegridadmin
OBSERVER	= $(top_srcdir)/bin/icegridobserver
NODE_SERVER	= $(top_srcdir)/bin/icegridnode
REGISTRY_SERVER	= $(top_srcdir)/bin/icegridregistry

LIBTARGETS	= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))
PERFLIBTARGETS	= $(call mklibtargets,$(libdir)/$(PERFLIBFILENAME),$(libdir)/$(PERFSONAME),$(libdir)/$(PERFLIBNAME))
TARGETS         = $(PERFLIBTARGETS) $(LIBTARGETS) $(NODE_SERVER) $(REGISTRY_SERVER) $(ADMIN) $(OBSERVER)

LIB_OBJS	= Admin.o \
		  Query.o \
	 	  Exception.o \
		  Descriptor.o \
		  Observer.o

PERFLIB_OBJS	= PerfTypes.o \
                  PerfLinux.o

ADMIN_OBJS	= Grammar.o \
		  Scanner.o \
		  Parser.o \
                  DescriptorParser.o \
                  DescriptorBuilder.o \
		  DescriptorHelper.o \
		  Util.o \
		  Internal.o \
		  Client.o

OBSERVER_OBJS	= IceGridObserver.o

COMMON_OBJS	= Internal.o \
		  TraceLevels.o

NODE_OBJS	= NodeI.o \
		  ServerI.o \
		  ServerAdapterI.o \
	          Activator.o \
		  WaitQueue.o \
                  DescriptorParser.o \
                  DescriptorBuilder.o \
		  PlatformInfo.o

REGISTRY_OBJS   = RegistryI.o \
		  StringObjectProxiesDict.o \
		  StringApplicationDescriptorDict.o \
		  IdentityObjectInfoDict.o \
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
		  $(PERFLIB_OBJS:.o=.cpp) \
		  $(ADMIN_OBJS:.o=.cpp) \
		  $(COMMON_OBJS:.o=.cpp) \
		  $(NODE_OBJS:.o=.cpp) \
		  $(REGISTRY_OBJS:.o=.cpp) \
	 	  $(OBSERVER_OBJS:.o=.cpp) \
		  IceGridNode.cpp \
		  IceGridRegistry.cpp

SLICE_SRCS	= $(SDIR)/PerfTypes.ice \
		  $(SDIR)/Admin.ice \
                  $(SDIR)/Exception.ice \
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
LINKWITH        := -lGlacier2 -lIce -lIceUtil

$(libdir)/$(LIBFILENAME): $(LIB_OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(LIB_OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

$(libdir)/$(PERFLIBFILENAME): $(PERFLIB_OBJS)
	rm -f $@
	$(call mkshlib,$@,$(PERFSONAME),$(PERFLIB_OBJS),$(LINKWITH))

$(libdir)/$(PERFSONAME): $(libdir)/$(PERFLIBFILENAME)
	rm -f $@
	ln -s $(PERFLIBFILENAME) $@

$(libdir)/$(PERFLIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(PERFSONAME) $@

$(ADMIN): $(ADMIN_OBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(ADMIN_OBJS) -lIcePatch2 -lGlacier2 -lIceXML -lIceGrid $(LIBS) $(READLINE_LIBS) \
	$(EXPAT_LIBS)

$(OBSERVER): $(OBSERVER_OBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBSERVER_OBJS) -lGlacier2 -lIceGrid $(LIBS) $(READLINE_LIBS) \
	$(EXPAT_LIBS)

$(REGISTRY_SERVER): $(REGISTRY_SVR_OBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(REGISTRY_SVR_OBJS) -lIceGrid -lIceStormService -lGlacier2 -lIcePatch2 -lFreeze \
	-lIceBox -lIceXML $(LIBS) $(DB_LIBS)

$(NODE_SERVER): $(NODE_SVR_OBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(NODE_SVR_OBJS) -lIceGrid -lIcePatch2 -lIceStormService -lGlacier2 -lFreeze -lIceXML \
	-lIceBox $(LIBS) $(DB_LIBS)

$(LOCAL_HDIR)/%.h %.cpp: $(SDIR)/%.ice $(SLICE2CPP)
	rm -f $(HDIR)/$(*F).h $(*F).cpp
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)/$(*F).ice

$(LOCAL_HDIR)/StringObjectProxiesDict.h StringObjectProxiesDict.cpp: $(SLICE2FREEZE)
	rm -f StringObjectProxiesDict.h StringObjectProxiesDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringObjectProxiesDict,string,IceGrid::StringObjectProxyDict \
	StringObjectProxiesDict $(LOCAL_SDIR)/Internal.ice

clean::
	rm -f StringObjectProxiesDict.h StringObjectProxiesDict.cpp

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
