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
NODE_SERVER	= $(top_srcdir)/bin/icegridnode
REGISTRY_SERVER	= $(top_srcdir)/bin/icegridregistry

LIBTARGETS	= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))
PERFLIBTARGETS	= $(call mklibtargets,$(libdir)/$(PERFLIBFILENAME),$(libdir)/$(PERFSONAME),$(libdir)/$(PERFLIBNAME))
TARGETS         = $(PERFLIBTARGETS) $(LIBTARGETS) $(NODE_SERVER) $(REGISTRY_SERVER) $(ADMIN)

LIB_OBJS	= Admin.o \
		  Query.o \
	 	  Exception.o

PERFLIB_OBJS	= PerfTypes.o \
                  PerfLinux.o

ADMIN_OBJS	= Grammar.o \
		  Scanner.o \
		  Parser.o \
                  DescriptorParser.o \
		  DescriptorHelper.o \
		  Util.o \
		  Internal.o \
		  Client.o

COMMON_OBJS	= Internal.o \
		  TraceLevels.o

NODE_OBJS	= NodeI.o \
		  ServerI.o \
		  ServerAdapterI.o \
	          Activator.o \
		  WaitQueue.o \
                  DescriptorParser.o

REGISTRY_OBJS   = RegistryI.o \
		  StringObjectProxyDict.o \
		  StringApplicationDescriptorDict.o \
		  IdentityObjectDescDict.o \
		  Database.o \
		  LocatorI.o \
		  LocatorRegistryI.o \
		  AdminI.o \
		  Util.o \
		  DescriptorHelper.o \
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
		  IceGridNode.cpp \
		  IceGridRegistry.cpp

SLICE_SRCS	= $(SDIR)/PerfTypes.ice \
		  $(SDIR)/Admin.ice \
                  $(SDIR)/Exception.ice \
                  $(SDIR)/Query.ice \
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
	$(CXX) $(LDFLAGS) -o $@ $(ADMIN_OBJS) -lIcePatch2 -lIceXML -lIceGrid $(LIBS) $(READLINE_LIBS) $(EXPAT_LIBS)

$(REGISTRY_SERVER): $(REGISTRY_SVR_OBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(REGISTRY_SVR_OBJS) -lIceGrid -lIcePatch2 -lFreeze -lIceXML $(LIBS) $(DB_LIBS)

$(NODE_SERVER): $(NODE_SVR_OBJS) $(LIBTARGETS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(NODE_SVR_OBJS) -lIceGrid -lIcePatch2 -lFreeze -lIceXML $(LIBS) $(DB_LIBS)

$(LOCAL_HDIR)/%.h %.cpp: $(SDIR)/%.ice $(SLICE2CPP)
	rm -f $(HDIR)/$(*F).h $(*F).cpp
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)/$(*F).ice

$(LOCAL_HDIR)/StringObjectProxyDict.h StringObjectProxyDict.cpp: $(SLICE2FREEZE)
	rm -f StringObjectProxyDict.h StringObjectProxyDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringObjectProxyDict,string,Object* \
	StringObjectProxyDict ../../slice/Ice/BuiltinSequences.ice

clean::
	rm -f StringObjectProxyDict.h StringObjectProxyDict.cpp

$(LOCAL_HDIR)/StringStringSeqDict.h StringStringSeqDict.cpp: $(SLICE2FREEZE)
	rm -f StringStringSeqDict.h StringStringSeqDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringStringSeqDict,string,Ice::StringSeq \
	StringStringSeqDict ../../slice/Ice/BuiltinSequences.ice

clean::
	rm -f StringStringSeqDict.h StringStringSeqDict.cpp

$(LOCAL_HDIR)/StringServerDescriptorDict.h StringServerDescriptorDict.cpp: $(SLICE2FREEZE)
	rm -f StringServerDescriptorDict.h StringServerDescriptorDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringServerDescriptorDict,string,IceGrid::ServerDescriptor \
	StringServerDescriptorDict ../../slice/IceGrid/Admin.ice

clean::
	rm -f StringServerDescriptorDict.h StringServerDescriptorDict.cpp

$(LOCAL_HDIR)/StringApplicationDescriptorDict.h StringApplicationDescriptorDict.cpp: $(SLICE2FREEZE)
	rm -f StringApplicationDescriptorDict.h StringApplicationDescriptorDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringApplicationDescriptorDict,string,IceGrid::ApplicationDescriptor \
	StringApplicationDescriptorDict ../../slice/IceGrid/Admin.ice

clean::
	rm -f StringApplicationDescriptorDict.h StringApplicationDescriptorDict.cpp

$(LOCAL_HDIR)/IdentityObjectDescDict.h IdentityObjectDescDict.cpp: $(SLICE2FREEZE)
	rm -f IdentityObjectDescDict.h IdentityObjectDescDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::IdentityObjectDescDict,Ice::Identity,IceGrid::ObjectDescriptor \
	--dict-index IceGrid::IdentityObjectDescDict,type \
	IdentityObjectDescDict ../../slice/Ice/Identity.ice $(LOCAL_SDIR)/Internal.ice

clean::
	rm -f IdentityObjectDescDict.h IdentityObjectDescDict.cpp

$(LOCAL_HDIR)/StringObjectProxySeqDict.h StringObjectProxySeqDict.cpp: $(SLICE2FREEZE)
	rm -f StringObjectProxySeqDict.h StringObjectProxySeqDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringObjectProxySeqDict,string,Ice::ObjectProxySeq \
	StringObjectProxySeqDict $(slicedir)/Ice/BuiltinSequences.ice

clean::
	rm -f StringObjectProxySeqDict.h StringObjectProxySeqDict.cpp

# Needed for make -jn to work.
#../IceGrid/Grammar.y: Grammar.h

clean::
#	rm -f Grammar.cpp Grammar.h
#	rm -f Scanner.cpp

install:: all
	$(call installlib,$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))
	$(INSTALL_PROGRAM) $(ADMIN) $(install_bindir)
	$(INSTALL_PROGRAM) $(NODE_SERVER) $(install_bindir)
	$(INSTALL_PROGRAM) $(REGISTRY_SERVER) $(install_bindir)

include .depend
