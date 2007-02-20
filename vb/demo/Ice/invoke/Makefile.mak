# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

TARGETS		= client.exe server.exe

C_SRCS		= Client.vb
S_SRCS		= PrinterI.vb Server.vb

GEN_SRCS	= $(GDIR)\Printer.vb

SLICE_SRCS	= $(SDIR)/Printer.ice

SDIR		= .

GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak.vb

VBCFLAGS	= $(VBCFLAGS) -target:exe

SLICE2VBFLAGS	= --stream $(SLICE2VBFLAGS)

client.exe: $(C_SRCS) $(GEN_SRCS)
	$(VBC) $(VBCFLAGS) -out:$@ -r:$(csbindir)\icecs.dll $(C_SRCS) $(GEN_SRCS)

server.exe: $(S_SRCS) $(GEN_SRCS)
	$(VBC) $(VBCFLAGS) -out:$@ -r:$(csbindir)\icecs.dll $(S_SRCS) $(GEN_SRCS)

!include .depend
