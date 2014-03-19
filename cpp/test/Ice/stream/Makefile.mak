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
NAME_PREFIX	= Ice_stream_
EXT		= .dll
!endif

CLIENT		= $(NAME_PREFIX)client

TARGETS		= $(CLIENT)$(EXT)

COBJS		= Test.obj \
		  Client.obj

SRCS		= $(COBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

SLICE2CPPFLAGS	= --stream $(SLICE2CPPFLAGS) 
CPPFLAGS	= -I. -I../../include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(WINRT)" != "yes"
LD_TESTFLAGS	= $(LD_EXEFLAGS) $(SETARGV)
!else
LD_TESTFLAGS	= $(LD_DLLFLAGS) /export:dllMain
!endif

!if "$(GENERATE_PDB)" == "yes"
CPDBFLAGS        = /pdb:$(CLIENT).pdb
!endif

$(CLIENT)$(EXT): $(COBJS)
	$(LINK) $(LD_TESTFLAGS) $(CPDBFLAGS) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q Test.cpp Test.h

!include .depend.mak
