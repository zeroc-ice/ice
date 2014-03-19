# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

TARGETS		= client.exe server.exe

C_SRCS		= Client.vb ClientPrinterI.vb DerivedPrinterI.vb ObjectFactory.vb PrinterI.vb
S_SRCS		= ClientPrinterI.vb DerivedPrinterI.vb InitialI.vb ObjectFactory.vb PrinterI.vb Server.vb

GEN_SRCS	= $(GDIR)\Value.cs

SLICE_SRCS	= $(SDIR)/Value.ice

SLICE_ASSEMBLY  = valueSlice.dll

SDIR		= .

GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak.vb

VBCFLAGS	= $(VBCFLAGS) -target:exe

client.exe: $(C_SRCS) $(SLICE_ASSEMBLY)
	$(VBC) $(VBCFLAGS) -out:$@ -r:"$(csbindir)\Ice.dll" -r:$(SLICE_ASSEMBLY) $(C_SRCS)

server.exe: $(S_SRCS) $(SLICE_ASSEMBLY)
	$(VBC) $(VBCFLAGS) -out:$@ -r:"$(csbindir)\Ice.dll" -r:$(SLICE_ASSEMBLY) $(S_SRCS)

!include .depend.mak
