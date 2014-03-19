# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

TARGETS		= client.exe helloservice.dll

C_SRCS		= Client.vb
S_SRCS		= HelloI.vb HelloServiceI.vb

GEN_SRCS	= $(GDIR)\Hello.cs

SLICE_SRCS	= $(SDIR)/Hello.ice

SLICE_ASSEMBLY  = helloIceBoxSlice.dll

SDIR		= .

GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak.vb

VBCFLAGS	= $(VBCFLAGS) -target:exe -rootnamespace:HelloIceBox

client.exe: $(C_SRCS) $(SLICE_ASSEMBLY)
	$(VBC) $(VBCFLAGS) -out:$@ -r:"$(csbindir)\Ice.dll" -r:$(SLICE_ASSEMBLY) $(C_SRCS)

helloservice.dll: $(S_SRCS) $(SLICE_ASSEMBLY)
	$(VBC) $(VBCFLAGS) -target:library -out:$@ -r:"$(csbindir)\IceBox.dll" -r:"$(csbindir)\Ice.dll" \
		-r:$(SLICE_ASSEMBLY) $(S_SRCS)

!include .depend.mak
