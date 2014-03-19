# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

!if "$(WINRT)" != "yes"
NAME_PREFIX	= 
EXT		= .exe
!else
NAME_PREFIX	= Ice_plugin_
EXT		= .dll
!endif

CLIENT		= $(NAME_PREFIX)client
LIBNAME		= TestPlugin$(LIBSUFFIX).lib
DLLNAME		= TestPlugin$(SOVERSION)$(LIBSUFFIX).dll
PLUGINDIR	= plugins

TARGETS		= $(CLIENT)$(EXT) $(PLUGINDIR)\$(LIBNAME) $(PLUGINDIR)\$(DLLNAME)

COBJS		= Client.obj

POBJS		= Plugin.obj

SRCS		= $(COBJS:.obj=.cpp) \
		  $(POBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(GENERATE_PDB)" == "yes"
CPDBFLAGS        = /pdb:$(CLIENT).pdb
PPDBFLAGS       = /pdb:$(DLLNAME:.dll=.pdb)
!endif

!if "$(WINRT)" != "yes"
LD_TESTFLAGS	= $(LD_EXEFLAGS) $(SETARGV)
!else
LD_TESTFLAGS	= $(LD_DLLFLAGS) /export:dllMain
!endif

LINKWITH        = $(LIBS)

$(PLUGINDIR)\$(LIBNAME): $(PLUGINDIR)\$(DLLNAME)
	    
$(PLUGINDIR)\$(DLLNAME): $(POBJS)
	$(LINK) $(BASE):0x22000000 $(LD_DLLFLAGS) $(PDBFLAGS) $(POBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH)
	@if exist $(PLUGINDIR)\$(DLLNAME:.dll=.lib) move $(PLUGINDIR)\$(DLLNAME:.dll=.lib) $(PLUGINDIR)\$(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(PLUGINDIR)\$(DLLNAME:.dll=.exp) del /q $(PLUGINDIR)\$(DLLNAME:.dll=.exp)

$(CLIENT)$(EXT): $(COBJS)
	$(LINK) $(LD_TESTFLAGS) $(CPDBFLAGS) $(SETARGV) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q $(PLUGINDIR)\*.ilk $(PLUGINDIR)\*.pdb

!include .depend.mak
