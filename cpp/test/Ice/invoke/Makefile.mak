# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
NAME_PREFIX	= Ice_invoke_
EXT		= .dll
OBJDIR		= winrt
!endif

CLIENT		= $(NAME_PREFIX)client
SERVER		= $(NAME_PREFIX)server

TARGETS		= $(CLIENT)$(EXT) $(SERVER)$(EXT)

SLICE_OBJS	= $(OBJDIR)\Test.obj

COBJS		= $(SLICE_OBJS) \
		  $(OBJDIR)\Client.obj \
		  $(OBJDIR)\AllTests.obj

SOBJS		= $(SLICE_OBJS) \
		  $(OBJDIR)\BlobjectI.obj \
		  $(OBJDIR)\Server.obj

OBJS		= $(COBJS) \
		  $(SOBJS)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(GENERATE_PDB)" == "yes"
CPDBFLAGS        = /pdb:$(CLIENT).pdb
SPDBFLAGS        = /pdb:$(SERVER).pdb
!endif

$(CLIENT)$(EXT): $(COBJS)
	$(LINK) $(LD_TESTFLAGS) $(CPDBFLAGS) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SERVER)$(EXT): $(SOBJS)
	$(LINK) $(LD_TESTFLAGS) $(SPDBFLAGS) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q Test.cpp Test.h
