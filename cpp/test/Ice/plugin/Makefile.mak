# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

!if "$(WINRT)" != "yes"
NAME_PREFIX	=
EXT		= .exe
OBJDIR		= .
PLUGINDIR	= plugins
!else
NAME_PREFIX	= Ice_plugin_
EXT		= .dll
OBJDIR		= winrt
PLUGINDIR	= plugins\winrt
!endif

CLIENT		= $(NAME_PREFIX)client
LIBNAME		= TestPlugin$(LIBSUFFIX).lib
DLLNAME		= TestPlugin$(SOVERSION)$(LIBSUFFIX).dll

TARGETS		= $(CLIENT)$(EXT) $(PLUGINDIR)\$(LIBNAME) $(PLUGINDIR)\$(DLLNAME)

COBJS		= $(OBJDIR)\Client.obj

POBJS		= $(OBJDIR)\Plugin.obj

OBJS		= $(COBJS) \
		  $(POBJS)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(GENERATE_PDB)" == "yes"
CPDBFLAGS        = /pdb:$(CLIENT).pdb
PPDBFLAGS       = /pdb:$(DLLNAME:.dll=.pdb)
!endif

LINKWITH        = $(LIBS)

$(PLUGINDIR)\$(LIBNAME): $(PLUGINDIR)\$(DLLNAME)

$(PLUGINDIR)\$(DLLNAME): $(POBJS)
	$(LINK) $(LD_DLLFLAGS) $(PDBFLAGS) $(POBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH)
	@if exist $(PLUGINDIR)\$(DLLNAME:.dll=.lib) move $(PLUGINDIR)\$(DLLNAME:.dll=.lib) $(PLUGINDIR)\$(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(PLUGINDIR)\$(DLLNAME:.dll=.exp) del /q $(PLUGINDIR)\$(DLLNAME:.dll=.exp)

$(CLIENT)$(EXT): $(COBJS)
	$(LINK) $(LD_TESTFLAGS) $(CPDBFLAGS) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q $(PLUGINDIR)\*.ilk $(PLUGINDIR)\*.pdb
