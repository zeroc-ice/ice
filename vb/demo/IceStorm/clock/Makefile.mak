# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

TARGETS		= publisher.exe subscriber.exe

C_SRCS		= Publisher.vb
S_SRCS		= Subscriber.vb

GEN_SRCS	= $(GDIR)\Clock.vb

SLICE_SRCS	= $(SDIR)/Clock.ice

SDIR		= .

GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak.vb

VBCFLAGS	= $(VBCFLAGS) -target:exe

publisher.exe: $(C_SRCS) $(GEN_SRCS)
	$(VBC) $(VBCFLAGS) -out:$@ -r:$(csbindir)\icecs.dll -r:$(csbindir)\icestormcs.dll $(C_SRCS) $(GEN_SRCS)

subscriber.exe: $(S_SRCS) $(GEN_SRCS)
	$(VBC) $(VBCFLAGS) -out:$@ -r:$(csbindir)\icecs.dll -r:$(csbindir)\icestormcs.dll $(S_SRCS) $(GEN_SRCS)

!include .depend
