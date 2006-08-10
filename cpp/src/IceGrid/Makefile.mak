# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\icegrid$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\icegrid$(SOVERSION)$(LIBSUFFIX).dll

ADMIN		= $(top_srcdir)\bin\icegridadmin.exe
NODE_SERVER	= $(top_srcdir)\bin\icegridnode.exe
REGISTRY_SERVER	= $(top_srcdir)\bin\icegridregistry.exe

!ifdef BUILD_UTILS

TARGETS         = $(ADMIN) $(NODE_SERVER) $(REGISTRY_SERVER)

!else

TARGETS         = $(LIBNAME) $(DLLNAME)

!endif

LIB_OBJS	= Admin.o \
		  Query.o \
		  Exception.o \
		  Descriptor.o \
		  Observer.o \
		  Session.o \
		  Registry.o \
		  UserAccountMapper.o

ADMIN_OBJS	= Grammar.o \
		  Scanner.o \
		  Parser.o \
		  DescriptorParser.o \
		  DescriptorBuilder.o \
		  DescriptorHelper.o \
		  FileParser.o \
		  FileParserI.o \
		  Util.o \
		  Internal.o \
		  Client.o

COMMON_OBJS	= Internal.o \
		  DescriptorParser.o \
		  DescriptorBuilder.o \
		  TraceLevels.o

NODE_OBJS	= NodeI.o \
		  ServerI.o \
		  ServerAdapterI.o \
		  Activator.o \
		  PlatformInfo.o \
		  NodeSessionManager.o

REGISTRY_OBJS	= RegistryI.o \
		  InternalRegistryI.o \
		  StringApplicationInfoDict.o \
		  IdentityObjectInfoDict.o \
		  StringAdapterInfoDict.o \
		  Database.o \
		  Allocatable.o \
		  AdapterCache.o \
		  ObjectCache.o \
		  AllocatableObjectCache.o \
		  ServerCache.o \
		  NodeCache.o \
		  ReplicaCache.o \
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
		  SessionServantLocatorI.o \
		  Topics.o \
		  QueryI.o \
		  WaitQueue.o \
		  FileUserAccountMapperI.o \
		  ReplicaSessionManager.o

NODE_SVR_OBJS	= $(COMMON_OBJS) \
		  $(NODE_OBJS) \
		  $(REGISTRY_OBJS) \
		  IceGridNode.o

REGISTRY_SVR_OBJS = \
		  $(COMMON_OBJS) \
		  $(REGISTRY_OBJS) \
		  IceGridRegistry.o
		    
SRCS		= $(LIB_OBJS:.o=.cpp) \
		  $(ADMIN_OBJS:.o=.cpp) \
		  $(COMMON_OBJS:.o=.cpp) \
		  $(NODE_OBJS:.o=.cpp) \
		  $(REGISTRY_OBJS:.o=.cpp) \
		  IceGridNode.cpp \
		  IceGridRegistry.cpp

HDIR		= $(includedir)\IceGrid
SDIR		= $(slicedir)\IceGrid

SLICE2FREEZECMD = $(SLICE2FREEZE) --ice --include-dir IceGrid $(ICECPPFLAGS)

!include $(top_srcdir)\config\Make.rules.mak

SLICE2CPPFLAGS	= --checksum --ice --include-dir IceGrid --dll-export ICE_GRID_API $(SLICE2CPPFLAGS)
LINKWITH 	= $(LIBS) glacier2$(LIBSUFFIX).lib
ALINKWITH 	= $(LINKWITH) icegrid$(LIBSUFFIX).lib icexml$(LIBSUFFIX).lib icepatch2$(LIBSUFFIX).lib
NLINKWITH	= $(ALINKWITH) icestorm$(LIBSUFFIX).lib freeze$(LIBSUFFIX).lib icebox$(LIBSUFFIX).lib \
		  icessl$(LIBSUFFIX).lib icestormservice$(LIBSUFFIX).lib $(OPENSSL_LIBS)

!ifdef BUILD_UTILS

CPPFLAGS	= -I. -I.. -Idummyinclude $(CPPFLAGS)

!else

CPPFLAGS        = -I.. -DICE_GRID_API_EXPORTS $(CPPFLAGS)

!endif

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(LIB_OBJS)
	del /q $@
	$(LINK) $(LD_DLLFLAGS) $(LIB_OBJS), $(DLLNAME),, $(LINKWITH)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)

$(ADMIN): $(ADMIN_OBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(ADMIN_OBJS), $@,, $(ALINKWITH)

$(REGISTRY_SERVER): $(REGISTRY_SVR_OBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(REGISTRY_SVR_OBJS), $@,, $(NLINKWITH)

$(NODE_SERVER): $(NODE_SVR_OBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(NODE_SVR_OBJS), $@,, $(NLINKWITH)

StringApplicationInfoDict.h StringApplicationInfoDict.cpp: $(SLICE2FREEZE)
	del /q StringApplicationInfoDict.h StringApplicationInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringApplicationInfoDict,string,IceGrid::ApplicationInfo \
	StringApplicationInfoDict Internal.ice

IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp: $(SLICE2FREEZE)
	del /q IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::IdentityObjectInfoDict,Ice::Identity,IceGrid::ObjectInfo \
	--dict-index IceGrid::IdentityObjectInfoDict,type \
	IdentityObjectInfoDict ..\..\slice\Ice\Identity.ice Internal.ice

StringAdapterInfoDict.h StringAdapterInfoDict.cpp: $(SLICE2FREEZE)
	del /q StringAdapterInfoDict.h StringAdapterInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringAdapterInfoDict,string,IceGrid::AdapterInfo \
	--dict-index IceGrid::StringAdapterInfoDict,replicaGroupId StringAdapterInfoDict $(SDIR)\Admin.ice

Admin.cpp $(HDIR)\Admin.h: $(SDIR)\Admin.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\Admin.ice
	move Admin.h $(HDIR)

Exception.cpp $(HDIR)\Exception.h: $(SDIR)\Exception.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\Exception.ice
	move Exception.h $(HDIR)

FileParser.cpp $(HDIR)\FileParser.h: $(SDIR)\FileParser.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\FileParser.ice
	move FileParser.h $(HDIR)

Query.cpp $(HDIR)\Query.h: $(SDIR)\Query.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\Query.ice
	move Query.h $(HDIR)

Session.cpp $(HDIR)\Session.h: $(SDIR)\Session.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\Session.ice
	move Session.h $(HDIR)

Observer.cpp $(HDIR)\Observer.h: $(SDIR)\Observer.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\Observer.ice
	move Observer.h $(HDIR)

Descriptor.cpp $(HDIR)\Descriptor.h: $(SDIR)\Descriptor.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\Descriptor.ice
	move Descriptor.h $(HDIR)

UserAccountMapper.cpp $(HDIR)\UserAccountMapper.h: $(SDIR)\UserAccountMapper.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\UserAccountMapper.ice
	move UserAccountMapper.h $(HDIR)

Registry.cpp $(HDIR)\Registry.h: $(SDIR)\Registry.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\Registry.ice
	move Registry.h $(HDIR)

Internal.cpp Internal.h: Internal.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) Internal.ice

Scanner.cpp : Scanner.l
	flex Scanner.l
	del /q $@
	echo #include "IceUtil/Config.h" > Scanner.cpp
	type lex.yy.c >> Scanner.cpp
	del /q lex.yy.c

Grammar.cpp Grammar.h: Grammar.y
	del /q Grammar.h Grammar.cpp
	bison -dvt Grammar.y
	move Grammar.tab.c Grammar.cpp
	move Grammar.tab.h Grammar.h
	del /q Grammar.output

!ifdef BUILD_UTILS

clean::
	del /q StringApplicationInfoDict.h StringApplicationInfoDict.cpp
	del /q StringAdapterInfoDict.h StringAdapterInfoDict.cpp
	del /q IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp

clean::
	del /q Admin.cpp $(HDIR)\Admin.h
	del /q Exception.cpp $(HDIR)\Exception.h
	del /q FileParser.cpp $(HDIR)\FileParser.h
	del /q Query.cpp $(HDIR)\Query.h
	del /q Session.cpp $(HDIR)\Session.h
	del /q Observer.cpp $(HDIR)\Observer.h
	del /q Descriptor.cpp $(HDIR)\Descriptor.h
	del /q UserAccountMapper.cpp $(HDIR)\UserAccountMapper.h
	del /q Registry.cpp $(HDIR)\Registry.h
	del /q Internal.cpp Internal.h

clean::
	del /q Grammar.cpp Grammar.h
	del /q Scanner.cpp

install:: all
	copy $(LIBNAME) $(install_libdir)
	copy $(DLLNAME) $(install_bindir)
	copy $(ADMIN) $(install_bindir)
	copy $(NODE_SERVER) $(install_bindir)
	copy $(REGISTRY_SERVER) $(install_bindir)

!else

install:: all

$(EVERYTHING)::
	$(MAKE) /f Makefile.mak BUILD_UTILS=1 $@

!endif

!include .depend
