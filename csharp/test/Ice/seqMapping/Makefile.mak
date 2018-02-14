# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

TARGETS		= client.exe server.exe collocated.exe serveramd.exe Serializable.dll

C_SRCS		= AllTests.cs Client.cs Twoways.cs TwowaysAMI.cs Custom.cs ..\..\TestCommon\TestApp.cs
S_SRCS		= MyClassI.cs Server.cs Custom.cs
COL_SRCS	= AllTests.cs Collocated.cs MyClassI.cs Twoways.cs TwowaysAMI.cs Custom.cs ..\..\TestCommon\TestApp.cs
SAMD_SRCS	= MyClassAMDI.cs ServerAMD.cs Custom.cs

GEN_SRCS	= $(GDIR)\Test.cs
GEN_AMD_SRCS	= $(GDIR)\TestAMD.cs

SDIR		= .

GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak.cs

MCSFLAGS	= $(MCSFLAGS) -target:exe

client.exe: $(C_SRCS) $(GEN_SRCS) Serializable.dll
	$(MCS) $(MCSFLAGS) -out:$@ -r:"$(refdir)\Ice.dll" -r:Serializable.dll $(C_SRCS) $(GEN_SRCS)

server.exe: $(S_SRCS) $(GEN_SRCS) Serializable.dll
	$(MCS) $(MCSFLAGS) -out:$@ -r:"$(refdir)\Ice.dll" -r:Serializable.dll $(S_SRCS) $(GEN_SRCS)

collocated.exe: $(COL_SRCS) $(GEN_SRCS) Serializable.dll
	$(MCS) $(MCSFLAGS) -out:$@ -r:"$(refdir)\Ice.dll" -r:Serializable.dll $(COL_SRCS) $(GEN_SRCS)

serveramd.exe: $(SAMD_SRCS) $(GEN_AMD_SRCS) Serializable.dll
	$(MCS) $(MCSFLAGS) -out:$@ -r:"$(refdir)\Ice.dll" -r:Serializable.dll $(SAMD_SRCS) $(GEN_AMD_SRCS)

Serializable.dll: Serializable.cs
	$(MCS) $(MCSFLAGS) -target:library -out:Serializable.dll /keyfile:"$(KEYFILE)" Serializable.cs
