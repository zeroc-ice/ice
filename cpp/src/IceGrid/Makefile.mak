# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

# Set appropriately if building IceGrid to use SQL database
#QTSQL_HOME     = C:\Qt\4.5.2

ADMIN		= $(top_srcdir)\bin\icegridadmin.exe
NODE_SERVER	= $(top_srcdir)\bin\icegridnode.exe
REGISTRY_SERVER	= $(top_srcdir)\bin\icegridregistry.exe

TARGETS         = $(ADMIN) $(NODE_SERVER) $(REGISTRY_SERVER)

ADMIN_OBJS	= Grammar.obj \
		  Scanner.obj \
		  Parser.obj \
		  DescriptorParser.obj \
		  DescriptorBuilder.obj \
		  DescriptorHelper.obj \
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
                  NodeServerAdminRouter.obj \
		  ServerI.obj \
		  ServerAdapterI.obj \
		  Activator.obj \
		  NodeSessionManager.obj

REGISTRY_OBJS	= AdminCallbackRouter.obj \
                  RegistryI.obj \
                  RegistryServerAdminRouter.obj \
		  InternalRegistryI.obj \
		  Database.obj \
		  DatabaseCache.obj \
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
		  SessionServantManager.obj \
		  Topics.obj \
		  QueryI.obj \
		  FileUserAccountMapperI.obj \
		  ReplicaSessionManager.obj \
		  WellKnownObjectsManager.obj

!if "$(QTSQL_HOME)" != ""
ICESQL_DIR	= $(top_srcdir)\src\IceSQL

{$(ICESQL_DIR)\}.cpp.obj::
    $(CXX) /c $(CPPFLAGS) $(CXXFLAGS) $<

REGISTRY_OBJS	= $(REGISTRY_OBJS) \
		  SqlStringApplicationInfoDict.obj \
		  SqlIdentityObjectInfoDict.obj \
		  SqlStringAdapterInfoDict.obj

ICESQL_OBJS	= SqlTypes.obj
!else
REGISTRY_OBJS	= $(REGISTRY_OBJS) \
		  StringApplicationInfoDict.obj \
		  IdentityObjectInfoDict.obj \
		  StringAdapterInfoDict.obj
!endif


NODE_SVR_OBJS	= $(COMMON_OBJS) \
		  $(NODE_OBJS) \
		  $(REGISTRY_OBJS) \
		  IceGridNode.obj

!if "$(QTSQL_HOME)" != ""
NODE_SVR_OBJS	= $(NODE_SVR_OBJS) \
		  $(ICESQL_OBJS)
!endif

REGISTRY_SVR_OBJS = \
		  $(COMMON_OBJS) \
		  $(REGISTRY_OBJS) \
		  IceGridRegistry.obj

!if "$(QTSQL_HOME)" != ""
REGISTRY_SVR_OBJS = $(REGISTRY_SVR_OBJS) \
		    $(ICESQL_OBJS)
!endif

SRCS            = $(ADMIN_OBJS:.obj=.cpp) \
		  $(COMMON_OBJS:.obj=.cpp) \
		  $(NODE_OBJS:.obj=.cpp) \
		  $(REGISTRY_OBJS:.obj=.cpp) \
		  IceGridNode.cpp \
		  IceGridRegistry.cpp

!if "$(QTSQL_HOME)" != ""
SRCS		= $(SRCS) \
		  $(ICESQL_DIR)\SqlTypes.cpp
!endif

HDIR		= $(headerdir)\IceGrid
SDIR		= $(slicedir)\IceGrid
LOCAL_HDIR	= ..\IceGrid
LOCAL_SDIR	= ..\IceGrid

SLICE2FREEZECMD = $(SLICE2FREEZE) --ice --include-dir IceGrid $(ICECPPFLAGS)

!include $(top_srcdir)\config\Make.rules.mak

!if "$(QTSQL_HOME)" != ""
DBLINKWITH	= $(QTSQL_LIBS)
CPPFLAGS	= $(QTSQL_FLAGS) $(CPPFLAGS)
!else
DBLINKWITH	= freeze$(LIBSUFFIX).lib
!endif

LINKWITH 	= $(LIBS) glacier2$(LIBSUFFIX).lib
ALINKWITH 	= $(LINKWITH) icegrid$(LIBSUFFIX).lib icexml$(LIBSUFFIX).lib icepatch2$(LIBSUFFIX).lib \
		  icebox$(LIBSUFFIX).lib
NLINKWITH	= $(ALINKWITH) $(DBLINKWITH) icestorm$(LIBSUFFIX).lib icebox$(LIBSUFFIX).lib \
		  icessl$(LIBSUFFIX).lib icestormservice$(LIBSUFFIX).lib $(OPENSSL_LIBS) pdh.lib ws2_32.lib

SLICE2CPPFLAGS	= --checksum --ice --include-dir IceGrid $(SLICE2CPPFLAGS)
CPPFLAGS	= -I. -I.. -Idummyinclude $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN -Zm200

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

!if "$(QTSQL_HOME)" != ""
$(LOCAL_HDIR)\StringApplicationInfoDict.h:
	type dummyinclude\unistd.h > StringApplicationInfoDict.h

$(LOCAL_HDIR)\IdentityObjectInfoDict.h:
	type dummyinclude\unistd.h > IdentityObjectInfoDict.h

$(LOCAL_HDIR)\StringAdapterInfoDict.h:
	type dummyinclude\unistd.h > StringAdapterInfoDict.h
!else
StringApplicationInfoDict.h StringApplicationInfoDict.cpp: $(LOCAL_SDIR)\Internal.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	del /q StringApplicationInfoDict.h StringApplicationInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringApplicationInfoDict,string,IceGrid::ApplicationInfo \
	StringApplicationInfoDict $(LOCAL_SDIR)\Internal.ice
 
IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp: $(slicedir)\Ice\Identity.ice $(LOCAL_SDIR)\Internal.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	del /q IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::IdentityObjectInfoDict,Ice::Identity,IceGrid::ObjectInfo \
	--dict-index IceGrid::IdentityObjectInfoDict,type \
	IdentityObjectInfoDict $(slicedir)\Ice\Identity.ice $(LOCAL_SDIR)\Internal.ice

StringAdapterInfoDict.h StringAdapterInfoDict.cpp: $(SDIR)\Admin.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	del /q StringAdapterInfoDict.h StringAdapterInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringAdapterInfoDict,string,IceGrid::AdapterInfo \
	--dict-index IceGrid::StringAdapterInfoDict,replicaGroupId StringAdapterInfoDict $(SDIR)\Admin.ice
!endif

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

clean::
	-del /q StringApplicationInfoDict.h StringApplicationInfoDict.cpp
	-del /q StringAdapterInfoDict.h StringAdapterInfoDict.cpp
	-del /q IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp
	-del /q Internal.cpp Internal.h
	-del /q $(ADMIN:.exe=.*)
	-del /q $(NODE_SERVER:.exe=.*)
	-del /q $(REGISTRY_SERVER:.exe=.*)
	-del /q IceGridAdmin.res IceGridNode.res IceGridRegistry.res

install:: all
	copy $(ADMIN) $(install_bindir)
	copy $(NODE_SERVER) $(install_bindir)
	copy $(REGISTRY_SERVER) $(install_bindir)


!if "$(GENERATE_PDB)" == "yes"

install:: all
	copy $(ADMIN:.exe=.pdb) $(install_bindir)
	copy $(NODE_SERVER:.exe=.pdb) $(install_bindir)
	copy $(REGISTRY_SERVER:.exe=.pdb) $(install_bindir)

!endif

!include .depend
