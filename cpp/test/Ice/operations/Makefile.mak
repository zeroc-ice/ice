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
!else
NAME_PREFIX	= Ice_operations_
EXT		= .dll
OBJDIR		= winrt
!endif

CLIENT		= $(NAME_PREFIX)client
SERVER		= $(NAME_PREFIX)server
SERVERAMD	= $(NAME_PREFIX)serveramd
COLLOCATED	= $(NAME_PREFIX)collocated

TARGETS		= $(CLIENT)$(EXT) $(SERVER)$(EXT) $(SERVERAMD)$(EXT) $(COLLOCATED)$(EXT)

SLICE_OBJS	= $(OBJDIR)\Test.obj $(OBJDIR)\TestAMD.obj

COBJS		= $(OBJDIR)\Test.obj \
		  $(OBJDIR)\Client.obj \
		  $(OBJDIR)\AllTests.obj \
		  $(OBJDIR)\Twoways.obj \
		  $(OBJDIR)\Oneways.obj \
		  $(OBJDIR)\TwowaysAMI.obj \
		  $(OBJDIR)\OnewaysAMI.obj \
		  $(OBJDIR)\BatchOneways.obj \
		  $(OBJDIR)\BatchOnewaysAMI.obj

SOBJS		= $(OBJDIR)\Test.obj \
		  $(OBJDIR)\TestI.obj \
		  $(OBJDIR)\Server.obj

SAMDOBJS	= $(OBJDIR)\TestAMD.obj \
		  $(OBJDIR)\TestAMDI.obj \
		  $(OBJDIR)\ServerAMD.obj

COLOBJS		= $(OBJDIR)\Test.obj \
		  $(OBJDIR)\TestI.obj \
		  $(OBJDIR)\Collocated.obj \
		  $(OBJDIR)\AllTests.obj \
		  $(OBJDIR)\Twoways.obj \
		  $(OBJDIR)\Oneways.obj \
		  $(OBJDIR)\TwowaysAMI.obj \
		  $(OBJDIR)\OnewaysAMI.obj \
		  $(OBJDIR)\BatchOneways.obj \
		  $(OBJDIR)\BatchOnewaysAMI.obj

OBJS		= $(COBJS) \
		  $(SOBJS) \
		  $(SAMDOBJS) \
		  $(COLOBJS)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN -Zm200 -bigobj

!if "$(GENERATE_PDB)" == "yes"
CPDBFLAGS        = /pdb:$(CLIENT).pdb
SPDBFLAGS        = /pdb:$(SERVER).pdb
SAPDBFLAGS       = /pdb:$(SERVERAMD).pdb
COPDBFLAGS       = /pdb:$(COLLOCATED).pdb
!endif

$(CLIENT)$(EXT): $(COBJS)
	$(LINK) $(LD_TESTFLAGS) $(CPDBFLAGS) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SERVER)$(EXT): $(SOBJS)
	$(LINK) $(LD_TESTFLAGS) $(SPDBFLAGS) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SERVERAMD)$(EXT): $(SAMDOBJS)
	$(LINK) $(LD_TESTFLAGS) $(SAPDBFLAGS) $(SAMDOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(COLLOCATED)$(EXT): $(COLOBJS)
	$(LINK) $(LD_TESTFLAGS) $(COPDBFLAGS) $(COLOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q Test.cpp Test.h
	del /q TestAMD.cpp TestAMD.h
