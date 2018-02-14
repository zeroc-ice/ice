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
NAME_PREFIX	= Ice_hash_
EXT		= .dll
OBJDIR		= winrt
!endif

CLIENT		= $(NAME_PREFIX)client

TARGETS		= $(CLIENT)$(EXT)

SLICE_OBJS	= $(OBJDIR)\Test.obj

OBJS		= $(SLICE_OBJS) \
		  $(OBJDIR)\Client.obj

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN


!if "$(GENERATE_PDB)" == "yes"
CPDBFLAGS        = /pdb:$(CLIENT).pdb
!endif

$(CLIENT)$(EXT): $(OBJS)
	$(LINK) $(LD_TESTFLAGS) $(CPDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q Test.cpp Test.h
