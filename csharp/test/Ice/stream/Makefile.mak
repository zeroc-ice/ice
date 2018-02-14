# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

TARGETS		= client.exe Serializable.dll

C_SRCS		= Client.cs AllTests.cs ..\..\TestCommon\TestApp.cs

GEN_SRCS	= $(GDIR)\Test.cs

SDIR		= .

GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak.cs

MCSFLAGS	= $(MCSFLAGS) -target:exe

SLICE2CSFLAGS	= $(SLICE2CSFLAGS) --stream -I. -I"$(slicedir)"

client.exe: $(C_SRCS) $(GEN_SRCS) Serializable.dll
	$(MCS) $(MCSFLAGS) -out:$@ -r:"$(refdir)\Ice.dll" -r:Serializable.dll $(C_SRCS) $(GEN_SRCS)

Serializable.dll: Serializable.cs
	$(MCS) $(MCSFLAGS) -target:library -out:Serializable.dll /keyfile:"$(KEYFILE)" Serializable.cs
