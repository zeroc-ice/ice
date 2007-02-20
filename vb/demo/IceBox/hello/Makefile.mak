# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

TARGETS		= client.exe helloservice.dll

C_SRCS		= Client.vb
S_SRCS		= HelloI.vb HelloServiceI.vb

GEN_SRCS	= $(GDIR)\Hello.vb

SLICE_SRCS	= $(SDIR)/Hello.ice

SDIR		= .

GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak.vb

VBCFLAGS	= $(VBCFLAGS) -target:exe -rootnamespace:HelloIceBox

client.exe: $(C_SRCS) $(GEN_SRCS)
	$(VBC) $(VBCFLAGS) -out:$@ -r:$(csbindir)\icecs.dll $(C_SRCS) $(GEN_SRCS)

helloservice.dll: $(S_SRCS) $(GEN_SRCS)
	$(VBC) $(VBCFLAGS) -target:library -out:$@ -r:$(csbindir)\iceboxcs.dll -r:$(csbindir)\icecs.dll \
		$(S_SRCS) $(GEN_SRCS)

!include .depend
