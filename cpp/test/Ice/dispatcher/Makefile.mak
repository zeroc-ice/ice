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
NAME_PREFIX	= Ice_dispatcher_
EXT		= .dll
OBJDIR		= winrt
!endif

CLIENT		= $(NAME_PREFIX)client
SERVER		= $(NAME_PREFIX)server
COLLOCATED	= $(NAME_PREFIX)collocated

TARGETS		= $(CLIENT)$(EXT) $(SERVER)$(EXT) $(COLLOCATED)$(EXT)

SLICE_OBJS  	= $(OBJDIR)\Test.obj

COBJS		= $(SLICE_OBJS) \
		  $(OBJDIR)\Client.obj \
		  $(OBJDIR)\AllTests.obj \
		  $(OBJDIR)\Dispatcher.obj

SOBJS		= $(SLICE_OBJS) \
		  $(OBJDIR)\TestI.obj \
		  $(OBJDIR)\Server.obj \
		  $(OBJDIR)\Dispatcher.obj

COLOBJS		= $(SLICE_OBJS) \
		  $(OBJDIR)\TestI.obj \
	  	  $(OBJDIR)\Collocated.obj \
		  $(OBJDIR)\Dispatcher.obj \
		  $(OBJDIR)\AllTests.obj

OBJS		= $(COBJS) \
		  $(SOBJS) \
		  $(COLOBJS)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(GENERATE_PDB)" == "yes"
CPDBFLAGS        = /pdb:$(CLIENT).pdb
SPDBFLAGS        = /pdb:$(SERVER).pdb
COLPDBFLAGS      = /pdb:$(COLLOCATED).pdb
!endif

$(CLIENT)$(EXT): $(COBJS)
	$(LINK) $(LD_TESTFLAGS) $(CPDBFLAGS) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SERVER)$(EXT): $(SOBJS)
	$(LINK) $(LD_TESTFLAGS) $(SPDBFLAGS) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(COLLOCATED)$(EXT): $(COLOBJS)
	$(LINK) $(LD_TESTFLAGS) $(COLPDBFLAGS) $(COLOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q Test.cpp Test.h
