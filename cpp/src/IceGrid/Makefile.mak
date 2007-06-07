# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

LIB_OBJS	= Admin.obj \
		  Query.obj \
		  Locator.obj \
		  Exception.obj \
		  Descriptor.obj \
		  Observer.obj \
		  Session.obj \
		  Registry.obj \
		  UserAccountMapper.obj

ADMIN_OBJS	= Grammar.obj \
		  Scanner.obj \
		  Parser.obj \
		  DescriptorParser.obj \
		  DescriptorBuilder.obj \
		  DescriptorHelper.obj \
		  FileParser.obj \
		  FileParserI.obj \
		  Util.obj \
		  Internal.obj \
		  Client.obj

COMMON_OBJS	= Internal.obj \
		  DescriptorParser.obj \
		  DescriptorBuilder.obj \
		  FileCache.obj \
		  TraceLevels.obj \
		  PlatformInfo.obj

NODE_OBJS	= NodeI.obj \
		  ServerI.obj \
		  ServerAdapterI.obj \
		  Activator.obj \
		  NodeSessionManager.obj

REGISTRY_OBJS	= RegistryI.obj \
		  InternalRegistryI.obj \
		  StringApplicationInfoDict.obj \
		  IdentityObjectInfoDict.obj \
		  StringAdapterInfoDict.obj \
		  Database.obj \
		  Allocatable.obj \
		  AdapterCache.obj \
		  ObjectCache.obj \
		  AllocatableObjectCache.obj \
		  ServerCache.obj \
		  NodeCache.obj \
		  ReplicaCache.obj \
		  LocatorI.obj \
		  LocatorRegistryI.obj \
		  AdminI.obj \
		  Util.obj \
		  DescriptorHelper.obj \
		  NodeSessionI.obj \
		  ReplicaSessionI.obj \
		  ReapThread.obj \
		  SessionI.obj \
		  AdminSessionI.obj \
		  SessionServantLocatorI.obj \
		  Topics.obj \
		  QueryI.obj \
		  WaitQueue.obj \
		  FileUserAccountMapperI.obj \
		  ReplicaSessionManager.obj \
		  WellKnownObjectsManager.obj

NODE_SVR_OBJS	= $(COMMON_OBJS) \
		  $(NODE_OBJS) \
		  $(REGISTRY_OBJS) \
		  IceGridNode.obj

REGISTRY_SVR_OBJS = \
		  $(COMMON_OBJS) \
		  $(REGISTRY_OBJS) \
		  IceGridRegistry.obj

!ifdef BUILD_UTILS
SRCS            = $(ADMIN_OBJS:.obj=.cpp) \
		  $(COMMON_OBJS:.obj=.cpp) \
		  $(NODE_OBJS:.obj=.cpp) \
		  $(REGISTRY_OBJS:.obj=.cpp) \
		  IceGridNode.cpp \
		  IceGridRegistry.cpp
!else
SRCS		= $(LIB_OBJS:.obj=.cpp)
!endif

HDIR		= $(includedir)\IceGrid
SDIR		= $(slicedir)\IceGrid

SLICE2FREEZECMD = $(SLICE2FREEZE) --ice --include-dir IceGrid $(ICECPPFLAGS)

!include $(top_srcdir)\config\Make.rules.mak

LINKWITH 	= $(LIBS) glacier2$(LIBSUFFIX).lib
ALINKWITH 	= $(LINKWITH) icegrid$(LIBSUFFIX).lib icexml$(LIBSUFFIX).lib icepatch2$(LIBSUFFIX).lib
NLINKWITH	= $(ALINKWITH) icestorm$(LIBSUFFIX).lib freeze$(LIBSUFFIX).lib icebox$(LIBSUFFIX).lib \
		  icessl$(LIBSUFFIX).lib icestormservice$(LIBSUFFIX).lib $(OPENSSL_LIBS)
!if "$(CPP_COMPILER)" != "BCC2006"
NLINKWITH	= $(NLINKWITH) pdh.lib ws2_32.lib
!endif

!ifdef BUILD_UTILS

SLICE2CPPFLAGS	= --checksum --ice --include-dir IceGrid $(SLICE2CPPFLAGS)
CPPFLAGS	= -I. -I.. -Idummyinclude $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
!if "$(CPP_COMPILER)" != "BCC2006"
CPPFLAGS 	= $(CPPFLAGS) -Zm200
!endif

!else

SLICE2CPPFLAGS	= --checksum --ice --include-dir IceGrid --dll-export ICE_GRID_API $(SLICE2CPPFLAGS)
CPPFLAGS        = -I.. -DICE_GRID_API_EXPORTS $(CPPFLAGS)

!endif

!if "$(CPP_COMPILER)" != "BCC2006" && "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
APDBFLAGS       = /pdb:$(ADMIN:.exe=.pdb)
RPDBFLAGS       = /pdb:$(REGISTRY_SERVER:.exe=.pdb)
NPDBFLAGS       = /pdb:$(NODE_SERVER:.exe=.pdb)
!endif

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(LIB_OBJS)
	$(LINK) $(LD_DLLFLAGS) $(PDBFLAGS) $(LIB_OBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

$(ADMIN): $(ADMIN_OBJS)
	$(LINK) $(LD_EXEFLAGS) $(APDBFLAGS) $(ADMIN_OBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(ALINKWITH)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) &&\
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(REGISTRY_SERVER): $(REGISTRY_SVR_OBJS)
	$(LINK) $(LD_EXEFLAGS) $(RPDBFLAGS) $(REGISTRY_SVR_OBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(NLINKWITH)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(NODE_SERVER): $(NODE_SVR_OBJS)
	$(LINK) $(LD_EXEFLAGS) $(NPDBFLAGS) $(NODE_SVR_OBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(NLINKWITH)
	@if exist $@.manifest \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

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
	del /q Locator.cpp $(HDIR)\Locator.h
	del /q Query.cpp $(HDIR)\Query.h
	del /q Session.cpp $(HDIR)\Session.h
	del /q Observer.cpp $(HDIR)\Observer.h
	del /q Descriptor.cpp $(HDIR)\Descriptor.h
	del /q UserAccountMapper.cpp $(HDIR)\UserAccountMapper.h
	del /q Registry.cpp $(HDIR)\Registry.h
	del /q Internal.cpp Internal.h
	del /q $(DLLNAME:.dll=.*)
	del /q $(ADMIN:.exe=.*)
	del /q $(NODE_SERVER:.exe=.*)
	del /q $(REGISTRY_SERVER:.exe=.*)

clean::
	del /q Grammar.cpp Grammar.h
	del /q Scanner.cpp

install:: all
	copy $(LIBNAME) $(install_libdir)
	copy $(DLLNAME) $(install_bindir)
	copy $(ADMIN) $(install_bindir)
	copy $(NODE_SERVER) $(install_bindir)
	copy $(REGISTRY_SERVER) $(install_bindir)

!if "$(OPTIMIZE)" != "yes"

!if "$(CPP_COMPILER)" == "BCC2006"

install:: all
	copy $(DLLNAME:.dll=.tds) $(install_bindir)
	copy $(ADMIN:.exe=.tds) $(install_bindir)
	copy $(NODE_SERVER:.exe=.tds) $(install_bindir)
	copy $(REGISTRY_SERVER:.exe=.tds) $(install_bindir)

!else

install:: all
	copy $(DLLNAME:.dll=.pdb) $(install_bindir)
	copy $(ADMIN:.exe=.pdb) $(install_bindir)
	copy $(NODE_SERVER:.exe=.pdb) $(install_bindir)
	copy $(REGISTRY_SERVER:.exe=.pdb) $(install_bindir)

!endif

!endif

!else

install:: all

$(EVERYTHING)::
	@$(MAKE) -nologo /f Makefile.mak BUILD_UTILS=1 $@

!endif

!include .depend
