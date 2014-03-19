# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

TARGETS		= publisher.exe subscriber.exe

C_SRCS		= Publisher.vb
S_SRCS		= Subscriber.vb

GEN_SRCS	= $(GDIR)\Clock.cs

SLICE_SRCS	= $(SDIR)/Clock.ice

SLICE_ASSEMBLY  = clockSlice.dll

SDIR		= .

GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak.vb

VBCFLAGS	= $(VBCFLAGS) -target:exe

publisher.exe: $(C_SRCS) $(SLICE_ASSEMBLY)
	$(VBC) $(VBCFLAGS) -out:$@ -r:"$(csbindir)\Ice.dll" -r:"$(csbindir)\IceStorm.dll" -r:$(SLICE_ASSEMBLY) $(C_SRCS)

subscriber.exe: $(S_SRCS) $(SLICE_ASSEMBLY)
	$(VBC) $(VBCFLAGS) -out:$@ -r:"$(csbindir)\Ice.dll" -r:"$(csbindir)\IceStorm.dll" -r:$(SLICE_ASSEMBLY) $(S_SRCS)

!include .depend.mak
