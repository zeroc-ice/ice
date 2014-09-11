# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

ADMIN		= $(top_srcdir)\bin\icegridadmin.exe
NODE_SERVER	= $(top_srcdir)\bin\icegridnode.exe
REGISTRY_SERVER	= $(top_srcdir)\bin\icegridregistry.exe

TARGETS         = $(ADMIN) $(NODE_SERVER) $(REGISTRY_SERVER)

SLICE_OBJS      = Internal.obj

ADMIN_OBJS	= Client.obj \
		  DescriptorBuilder.obj \
		  DescriptorHelper.obj \
		  DescriptorParser.obj \
		  FileParserI.obj \
		  Grammar.obj \
		  Parser.obj \
		  Scanner.obj \
		  Util.obj \
                  $(SLICE_OBJS)

COMMON_OBJS	= AdminRouter.obj \
                  DescriptorBuilder.obj \
		  DescriptorParser.obj \
		  FileCache.obj \
		  PlatformInfo.obj \
		  SessionManager.obj \
		  TraceLevels.obj \
                  $(SLICE_OBJS)

NODE_OBJS	= Activator.obj \
		  NodeAdminRouter.obj \
                  NodeI.obj \
		  NodeSessionManager.obj \
		  ServerAdapterI.obj \
		  ServerI.obj

REGISTRY_OBJS	= AdapterCache.obj \
		  AdminCallbackRouter.obj \
		  AdminI.obj \
		  AdminSessionI.obj \
		  Allocatable.obj \
		  AllocatableObjectCache.obj \
		  Database.obj \
		  DescriptorHelper.obj \
		  FileUserAccountMapperI.obj \
		  IdentityObjectInfoDict.obj \
		  InternalRegistryI.obj \
		  LocatorI.obj \
		  LocatorRegistryI.obj \
		  NodeCache.obj \
		  NodeSessionI.obj \
		  ObjectCache.obj \
		  PluginFacadeI.obj \
		  QueryI.obj \
		  ReapThread.obj \
		  RegistryAdminRouter.obj \
                  RegistryI.obj \
		  ReplicaCache.obj \
		  ReplicaSessionI.obj \
		  ReplicaSessionManager.obj \
		  SerialsDict.obj \
		  ServerCache.obj \
		  SessionI.obj \
		  SessionServantManager.obj \
		  StringAdapterInfoDict.obj \
		  StringApplicationInfoDict.obj \
		  Topics.obj \
		  Util.obj \
		  WellKnownObjectsManager.obj

NODE_SVR_OBJS	= $(COMMON_OBJS) \
		  $(NODE_OBJS) \
		  $(REGISTRY_OBJS) \
		  IceGridNode.obj

REGISTRY_SVR_OBJS = \
		  $(COMMON_OBJS) \
		  $(REGISTRY_OBJS) \
		  IceGridRegistry.obj

SRCS            = $(ADMIN_OBJS:.obj=.cpp) \
		  $(COMMON_OBJS:.obj=.cpp) \
		  $(NODE_OBJS:.obj=.cpp) \
		  $(REGISTRY_OBJS:.obj=.cpp) \
		  IceGridNode.cpp \
		  IceGridRegistry.cpp

HDIR		= $(headerdir)\IceGrid
SDIR		= $(slicedir)\IceGrid

SLICE2FREEZECMD = $(SLICE2FREEZE) -I.. --ice --include-dir IceGrid $(ICECPPFLAGS)

!include $(top_srcdir)\config\Make.rules.mak

LINKWITH 	= $(LIBS) glacier2$(LIBSUFFIX).lib
ALINKWITH 	= $(LINKWITH) icegrid$(LIBSUFFIX).lib icexml$(LIBSUFFIX).lib icepatch2$(LIBSUFFIX).lib \
		  icebox$(LIBSUFFIX).lib
NLINKWITH	= $(ALINKWITH) freeze$(LIBSUFFIX).lib icestorm$(LIBSUFFIX).lib icebox$(LIBSUFFIX).lib \
		  icessl$(LIBSUFFIX).lib icestormservice$(LIBSUFFIX).lib  libeay32.lib pdh.lib ws2_32.lib

SLICE2CPPFLAGS	= --checksum --ice --include-dir IceGrid $(SLICE2CPPFLAGS)
CPPFLAGS	= -I. -I.. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN -Zm200

!if "$(GENERATE_PDB)" == "yes"
APDBFLAGS       = /pdb:$(ADMIN:.exe=.pdb)
RPDBFLAGS       = /pdb:$(REGISTRY_SERVER:.exe=.pdb)
NPDBFLAGS       = /pdb:$(NODE_SERVER:.exe=.pdb)
!endif

ARES_FILE       = IceGridAdmin.res
RRES_FILE       = IceGridRegistry.res
NRES_FILE       = IceGridNode.res

$(ADMIN): $(ADMIN_OBJS) IceGridAdmin.res
	$(LINK) $(LD_EXEFLAGS) $(APDBFLAGS) $(ADMIN_OBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(ALINKWITH) $(ARES_FILE)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) &&\
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(REGISTRY_SERVER): $(REGISTRY_SVR_OBJS) IceGridRegistry.res
	$(LINK) $(LD_EXEFLAGS) $(RPDBFLAGS) $(REGISTRY_SVR_OBJS) $(SETARGV) $(PREOUT)$@ \
		$(PRELIBS)$(NLINKWITH) $(RRES_FILE)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(NODE_SERVER): $(NODE_SVR_OBJS) IceGridNode.res
	$(LINK) $(LD_EXEFLAGS) $(NPDBFLAGS) $(NODE_SVR_OBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(NLINKWITH) $(NRES_FILE)
	@if exist $@.manifest \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

Scanner.cpp : Scanner.l
	flex Scanner.l
	del /q $@
	echo #include "IceUtil/ScannerConfig.h" >> Scanner.cpp
	type lex.yy.c >> Scanner.cpp
	del /q lex.yy.c

Grammar.cpp Grammar.h: Grammar.y
	del /q Grammar.h Grammar.cpp
	bison -dvt Grammar.y
	move Grammar.tab.c Grammar.cpp
	move Grammar.tab.h Grammar.h
	del /q Grammar.output

StringApplicationInfoDict.h StringApplicationInfoDict.cpp: $(SDIR)\Admin.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	del /q StringApplicationInfoDict.h StringApplicationInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringApplicationInfoDict,string,IceGrid::ApplicationInfo \
	StringApplicationInfoDict $(SDIR)\Admin.ice

IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp: $(slicedir)\Ice\Identity.ice $(SDIR)\Admin.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	del /q IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::IdentityObjectInfoDict,Ice::Identity,IceGrid::ObjectInfo \
	--dict-index IceGrid::IdentityObjectInfoDict,type \
	IdentityObjectInfoDict $(slicedir)\Ice\Identity.ice $(SDIR)\Admin.ice

StringAdapterInfoDict.h StringAdapterInfoDict.cpp: $(SDIR)\Admin.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	del /q StringAdapterInfoDict.h StringAdapterInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringAdapterInfoDict,string,IceGrid::AdapterInfo \
	--dict-index IceGrid::StringAdapterInfoDict,replicaGroupId StringAdapterInfoDict $(SDIR)\Admin.ice

SerialsDict.h SerialsDict.cpp: $(SLICE2FREEZE) $(SLICEPARSERLIB)
	del /q SerialsDict.h SerialsDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::SerialsDict,string,long SerialsDict

clean::
	-del /q Internal.cpp Internal.h
	-del /q StringApplicationInfoDict.h StringApplicationInfoDict.cpp
	-del /q StringAdapterInfoDict.h StringAdapterInfoDict.cpp
	-del /q IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp
	-del /q SerialsDict.h SerialsDict.cpp
	-del /q $(ADMIN:.exe=.*)
	-del /q $(NODE_SERVER:.exe=.*)
	-del /q $(REGISTRY_SERVER:.exe=.*)
	-del /q IceGridAdmin.res IceGridNode.res IceGridRegistry.res

install:: all
	copy $(ADMIN) "$(install_bindir)"
	copy $(NODE_SERVER) "$(install_bindir)"
	copy $(REGISTRY_SERVER) "$(install_bindir)"


!if "$(GENERATE_PDB)" == "yes"

install:: all
	copy $(ADMIN:.exe=.pdb) "$(install_bindir)"
	copy $(NODE_SERVER:.exe=.pdb) "$(install_bindir)"
	copy $(REGISTRY_SERVER:.exe=.pdb) "$(install_bindir)"

!endif

!include .depend.mak
