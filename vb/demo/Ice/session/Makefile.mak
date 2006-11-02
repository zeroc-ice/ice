# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

TARGETS		= client.exe server.exe

C_SRCS		= Client.vb
S_SRCS		= HelloI.vb ReapThread.vb Server.vb SessionFactoryI.vb SessionI.vb

GEN_SRCS	= $(GDIR)\Session.vb

SDIR		= .

GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak

VBCFLAGS	= $(VBCFLAGS) -target:exe -rootnamespace:SessionDemo

SLICE2VBFLAGS	= $(SLICE2VBFLAGS) --ice -I. -I$(slicedir)

client.exe: $(C_SRCS) $(GEN_SRCS)
	$(VBC) $(VBCFLAGS) -out:$@ -r:$(csbindir)\icecs.dll $(C_SRCS) $(GEN_SRCS)

server.exe: $(S_SRCS) $(GEN_SRCS)
	$(VBC) $(VBCFLAGS) -out:$@ -r:$(csbindir)\icecs.dll $(S_SRCS) $(GEN_SRCS)

!include .depend
