# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

TARGETS		= client.exe server.exe sessionserver.exe
TARGETS_CONFIG	= $(TARGETS:.exe=.exe.config)

C_SRCS		= CallbackReceiverI.vb Client.vb
S_SRCS		= CallbackI.vb Server.vb
SS_SRCS		= SessionI.vb SessionManagerI.vb SessionServer.vb

GEN_SRCS	= $(GDIR)\Callback.cs

SLICE_SRCS	= $(SDIR)/Callback.ice

SLICE_ASSEMBLY  = glacier2CallbackSlice.dll

SDIR		= .

GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak.vb

VBCFLAGS	= $(VBCFLAGS) -target:exe -rootnamespace:Glacier2Demo

client.exe: $(C_SRCS) $(SLICE_ASSEMBLY)
	$(VBC) $(VBCFLAGS) -out:$@ -r:$(csbindir)\Ice.dll -r:$(SLICE_ASSEMBLY) -r:$(csbindir)\Glacier2.dll $(C_SRCS)

server.exe: $(S_SRCS) $(SLICE_ASSEMBLY)
	$(VBC) $(VBCFLAGS) -out:$@ -r:$(csbindir)\Ice.dll -r:$(SLICE_ASSEMBLY) $(S_SRCS)

sessionserver.exe: $(SS_SRCS)
	$(VBC) $(VBCFLAGS) -out:$@ -r:$(csbindir)\Ice.dll -r:$(csbindir)\Glacier2.dll $(SS_SRCS)

!include .depend
