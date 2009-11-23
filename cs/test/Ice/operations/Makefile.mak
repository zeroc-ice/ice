# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

TARGETS		= client.exe server.exe collocated.exe serveramd.exe

C_SRCS		= AllTests.cs Client.cs Twoways.cs TwowaysAMI.cs TwowaysNewAMI.cs BatchOneways.cs Oneways.cs \
		  OnewaysAMI.cs OnewaysNewAMI.cs
S_SRCS		= MyDerivedClassI.cs Server.cs StateChangerI.cs
COL_SRCS	= AllTests.cs Collocated.cs MyDerivedClassI.cs Twoways.cs TwowaysAMI.cs TwowaysNewAMI.cs \
		  BatchOneways.cs Oneways.cs OnewaysAMI.cs OnewaysNewAMI.cs
SAMD_SRCS	= MyDerivedClassAMDI.cs Server.cs StateChangerI.cs

GEN_SRCS	= $(GDIR)\Test.cs $(GDIR)\StateChanger.cs
GEN_AMD_SRCS	= $(GDIR)\TestAMD.cs $(GDIR)\StateChanger.cs

SDIR		= .

GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak.cs

MCSFLAGS	= $(MCSFLAGS) -target:exe

SLICE2CSFLAGS	= $(SLICE2CSFLAGS) --ice -I. -I$(slicedir)

client.exe: $(C_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:$(refdir)\Ice.dll $(C_SRCS) $(GEN_SRCS)

server.exe: $(S_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:$(refdir)\Ice.dll $(S_SRCS) $(GEN_SRCS)

collocated.exe: $(COL_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:$(refdir)\Ice.dll $(COL_SRCS) $(GEN_SRCS)

serveramd.exe: $(SAMD_SRCS) $(GEN_AMD_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:$(refdir)\Ice.dll $(SAMD_SRCS) $(GEN_AMD_SRCS)

!include .depend
