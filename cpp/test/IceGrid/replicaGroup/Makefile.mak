# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

CLIENT		= client.exe
SERVER		= server.exe

LIBNAME		= testservice$(LIBSUFFIX).lib
DLLNAME		= testservice$(LIBSUFFIX).dll

PLUGINLIBNAME	= registryplugin$(SOVERSION)$(LIBSUFFIX).lib
PLUGINDLLNAME	= registryplugin$(SOVERSION)$(LIBSUFFIX).dll

TARGETS		= $(CLIENT) $(SERVER) $(LIBNAME) $(DLLNAME) $(PLUGINLIBNAME) $(PLUGINDLLNAME)

SLICE_OBJS	= .\Test.obj

COBJS		= $(SLICE_OBJS) \
		  .\Client.obj \
		  .\AllTests.obj

SOBJS		= $(SLICE_OBJS) \
		  .\TestI.obj \
		  .\Server.obj

SERVICE_OBJS	= $(SLICE_OBJS) \
		  .\TestI.obj \
		  .\Service.obj

OBJS		= $(COBJS) \
		  $(SOBJS) \
		  $(SERVICE_OBJS)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
LINKWITH	= $(LIBS) 

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
CPDBFLAGS       = /pdb:$(CLIENT:.exe=.pdb)
SPDBFLAGS       = /pdb:$(SERVER:.exe=.pdb)
!endif

$(LIBNAME) : $(DLLNAME)

$(DLLNAME): $(SERVICE_OBJS)
	$(LINK) $(LD_DLLFLAGS) $(PDBFLAGS) $(SETARGV) $(SERVICE_OBJS) $(PREOUT)$(DLLNAME) $(PRELIBS)$(LINKWITH)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

$(PLUGINLIBNAME) : $(PLUGINDLLNAME)

$(PLUGINDLLNAME): RegistryPlugin.obj
	$(LINK) $(LD_DLLFLAGS) $(PDBFLAGS) $(SETARGV) RegistryPlugin.obj $(PREOUT)$(PLUGINDLLNAME) $(PRELIBS)$(LINKWITH) \
	   
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(PLUGINDLLNAME:.dll=.exp) del /q $(PLUGINDLLNAME:.dll=.exp)

$(CLIENT): $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(SETARGV) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH) \
	   
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SERVER): $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SETARGV) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q Test.cpp Test.h

!if "$(OPTIMIZE)" == "yes"

all::
	@echo release > build.txt

!else

all::
	@echo debug > build.txt

!endif

clean::
	del /q build.txt
	if exist db\node rmdir /s /q db\node 
	if exist db\registry rmdir /s /q db\registry 
	if exist db\replica-1 rmdir /s /q db\replica-1
