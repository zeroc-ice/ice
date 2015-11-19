# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

ADMIN		= $(top_srcdir)\bin\icegridadmin.exe
DB		= $(top_srcdir)\bin\icegriddb.exe

NODE_SERVER_D	= $(top_srcdir)\bin\icegridnoded.exe
NODE_SERVER_R	= $(top_srcdir)\bin\icegridnode.exe

NODE_SERVER	= $(top_srcdir)\bin\icegridnode$(LIBSUFFIX).exe

REGISTRY_SERVER_D = $(top_srcdir)\bin\icegridregistryd.exe
REGISTRY_SERVER_R = $(top_srcdir)\bin\icegridregistry.exe

REGISTRY_SERVER	= $(top_srcdir)\bin\icegridregistry$(LIBSUFFIX).exe

TARGETS         = $(ADMIN) $(NODE_SERVER) $(REGISTRY_SERVER) $(DB)

CSLICE_OBJS      = .\Internal.obj \
		  .\IceLocatorDiscovery.obj

BISON_FLEX_OBJS = .\Grammar.obj \
                  .\Scanner.obj

ADMIN_OBJS	= .\Client.obj \
		  .\DescriptorBuilder.obj \
		  .\DescriptorHelper.obj \
		  .\DescriptorParser.obj \
		  .\FileParserI.obj \
		  .\Parser.obj \
		  .\Util.obj \
		  $(CSLICE_OBJS) \
                  $(BISON_FLEX_OBJS)

COMMON_OBJS	= .\AdminRouter.obj \
		  .\DescriptorBuilder.obj \
		  .\DescriptorParser.obj \
		  .\FileCache.obj \
		  .\PlatformInfo.obj \
		  .\SessionManager.obj \
		  .\TraceLevels.obj \
		  $(CSLICE_OBJS)

NODE_OBJS	= .\Activator.obj \
		  .\NodeAdminRouter.obj \
		  .\NodeI.obj \
		  .\NodeSessionManager.obj \
		  .\ServerAdapterI.obj \
		  .\ServerI.obj

REGISTRY_OBJS	= .\AdapterCache.obj \
		  .\AdminCallbackRouter.obj \
		  .\AdminI.obj \
		  .\AdminSessionI.obj \
		  .\Allocatable.obj \
		  .\AllocatableObjectCache.obj \
		  .\Database.obj \
		  .\DescriptorHelper.obj \
		  .\FileUserAccountMapperI.obj \
		  .\InternalRegistryI.obj \
		  .\LocatorI.obj \
		  .\LocatorRegistryI.obj \
		  .\NodeCache.obj \
		  .\NodeSessionI.obj \
		  .\ObjectCache.obj \
		  .\PluginFacadeI.obj \
		  .\QueryI.obj \
		  .\ReapThread.obj \
		  .\RegistryAdminRouter.obj \
		  .\RegistryI.obj \
		  .\ReplicaCache.obj \
		  .\ReplicaSessionI.obj \
		  .\ReplicaSessionManager.obj \
		  .\ServerCache.obj \
		  .\SessionI.obj \
		  .\SessionServantManager.obj \
		  .\Topics.obj \
		  .\Util.obj \
		  .\WellKnownObjectsManager.obj

NODE_SVR_OBJS	= $(COMMON_OBJS) \
		  $(NODE_OBJS) \
		  $(REGISTRY_OBJS) \
		  .\IceGridNode.obj

REGISTRY_SVR_OBJS = \
		  $(COMMON_OBJS) \
		  $(REGISTRY_OBJS) \
		  .\IceGridRegistry.obj

DSLICE_OBJS	= .\DBTypes.obj

DB_OBJS		= .\IceGridDB.obj \
		  $(DSLICE_OBJS)

OBJS            = $(ADMIN_OBJS) \
		  $(DB_OBJS) \
		  $(NODE_SVR_OBJS) \
		  $(REGISTRY_SVR_OBJS)

SLICE_OBJS	= $(CSLICE_OBJS) \
		  $(DSLICE_OBJS)

HDIR		= $(headerdir)\IceGrid
SDIR		= $(slicedir)\IceGrid

!include $(top_srcdir)\config\Make.rules.mak

$(OBJS)		: $(LMDB_NUPKG)

LINKWITH 	= $(LIBS)
ALINKWITH 	= $(LIBS)
DLINKWITH 	= $(LIBS)
NLINKWITH	= $(LIBS) advapi32.lib pdh.lib ws2_32.lib

SLICE2CPPFLAGS	= --checksum --ice --include-dir IceGrid $(SLICE2CPPFLAGS)
CPPFLAGS	= -I. -I.. $(LMDB_CPPFLAGS) $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN -Zm200

!if "$(GENERATE_PDB)" == "yes"
APDBFLAGS       = /pdb:$(ADMIN:.exe=.pdb)
DPDBFLAGS       = /pdb:$(DB:.exe=.pdb)
RPDBFLAGS       = /pdb:$(REGISTRY_SERVER:.exe=.pdb)
NPDBFLAGS       = /pdb:$(NODE_SERVER:.exe=.pdb)
!endif

ARES_FILE       = IceGridAdmin.res
DRES_FILE       = IceGridDB.res
RRES_FILE       = IceGridRegistry.res
NRES_FILE       = IceGridNode.res

$(ADMIN): $(ADMIN_OBJS) IceGridAdmin.res
	$(LINK) $(LD_EXEFLAGS) $(APDBFLAGS) $(ADMIN_OBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(ALINKWITH) $(ARES_FILE)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) &&\
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest
	@if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

$(DB): $(DB_OBJS) IceGridDB.res
	$(LINK) $(LD_EXEFLAGS) $(DPDBFLAGS) $(DB_OBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(DLINKWITH) $(DRES_FILE)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) &&\
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest
	@if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

$(REGISTRY_SERVER): $(REGISTRY_SVR_OBJS) IceGridRegistry.res
	$(LINK) $(LD_EXEFLAGS) $(RPDBFLAGS) $(REGISTRY_SVR_OBJS) $(SETARGV) $(PREOUT)$@ \
		$(PRELIBS)$(NLINKWITH) $(RRES_FILE)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest
	@if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

$(NODE_SERVER): $(NODE_SVR_OBJS) IceGridNode.res
	$(LINK) $(LD_EXEFLAGS) $(NPDBFLAGS) $(NODE_SVR_OBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(NLINKWITH) $(NRES_FILE)
	@if exist $@.manifest \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest
	@if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

IceLocatorDiscovery.h IceLocatorDiscovery.cpp: $(slicedir)\IceLocatorDiscovery\IceLocatorDiscovery.ice "$(SLICE2CPP)" "$(SLICEPARSERLIB)"
	del /q $(*F).h $(*F).cpp
	"$(SLICE2CPP)" $(SLICE2CPPFLAGS) $(slicedir)\IceLocatorDiscovery\IceLocatorDiscovery.ice

{$(slicedir)\IceLocatorDiscovery}.ice{$(SLICE_DEPEND_DIR)\}.d:
	@echo Generating dependencies for $<
	@"$(SLICE2CPP)" $(SLICE2CPPFLAGS) --depend $< | cscript /NoLogo $(top_srcdir)\..\config\makedepend-slice.vbs $(*F).ice

clean::
	-del /q Internal.cpp Internal.h
	-del /q $(ADMIN:.exe=.*)
	-del /q $(DB:.exe=.*)
	-del /q $(NODE_SERVER_D:.exe=.*) $(NODE_SERVER_R:.exe=.*)
	-del /q $(REGISTRY_SERVER_D:.exe=.*) $(REGISTRY_SERVER_R:.exe=.*)
	-del /q IceGridAdmin.res IceGridNode.res IceGridRegistry.res

install:: all
	copy $(ADMIN) "$(install_bindir)"
	copy $(DB) "$(install_bindir)"
	copy $(NODE_SERVER) "$(install_bindir)"
	copy $(REGISTRY_SERVER) "$(install_bindir)"


!if "$(GENERATE_PDB)" == "yes"

install:: all
	copy $(ADMIN:.exe=.pdb) "$(install_bindir)"
	copy $(DB:.exe=.pdb) "$(install_bindir)"
	copy $(NODE_SERVER:.exe=.pdb) "$(install_bindir)"
	copy $(REGISTRY_SERVER:.exe=.pdb) "$(install_bindir)"

!endif
