# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..\..

TARGETS		= client.exe server.exe serveramd.exe

C_SRCS		= AllTests.cs Client.cs ..\..\..\TestCommon\TestApp.cs
S_SRCS		= Server.cs TestI.cs
SAMD_SRCS	= ServerAMD.cs TestAMDI.cs

GEN_SRCS	= $(GDIR)\Test.cs
CGEN_SRCS	= $(GDIR)\ClientPrivate.cs
SGEN_SRCS	= $(GDIR)\ServerPrivate.cs
GEN_AMD_SRCS	= $(GDIR)\TestAMD.cs
SAMD_GEN_SRCS	= $(GDIR)\ServerPrivateAMD.cs

SDIR		= .

GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak.cs

MCSFLAGS	= $(MCSFLAGS) -target:exe

SLICE2CSFLAGS	= $(SLICE2CSFLAGS) -I.

client.exe: $(C_SRCS) $(GEN_SRCS) $(CGEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:"$(refdir)\Ice.dll" $(C_SRCS) $(GEN_SRCS) $(CGEN_SRCS)

server.exe: $(S_SRCS) $(GEN_SRCS) $(SGEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:"$(refdir)\Ice.dll" $(S_SRCS) $(GEN_SRCS) $(SGEN_SRCS)

serveramd.exe: $(SAMD_SRCS) $(GEN_AMD_SRCS) $(SAMD_GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:"$(refdir)\Ice.dll" $(SAMD_SRCS) $(GEN_AMD_SRCS) $(SAMD_GEN_SRCS)
