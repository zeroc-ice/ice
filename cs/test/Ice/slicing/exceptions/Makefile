# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../../..

TARGETS		= client.exe server.exe serveramd.exe

C_SRCS		= AllTests.cs Client.cs ../../../TestCommon/TestApp.cs
S_SRCS		= Server.cs TestI.cs
SAMD_SRCS	= ServerAMD.cs TestAMDI.cs

GEN_SRCS	= $(GDIR)/Test.cs
GEN_C_SRCS	= $(GDIR)/ClientPrivate.cs
GEN_S_SRCS	= $(GDIR)/ServerPrivate.cs
GEN_AMD_SRCS	= $(GDIR)/TestAMD.cs
GEN_SAMD_SRCS	= $(GDIR)/ServerPrivateAMD.cs

include $(top_srcdir)/config/Make.rules.cs

MCSFLAGS	:= $(MCSFLAGS) -target:exe

SLICE2CSFLAGS	:= $(SLICE2CSFLAGS) -I.

client.exe: $(C_SRCS) $(GEN_SRCS) $(GEN_C_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,Ice) $^

server.exe: $(S_SRCS) $(GEN_SRCS) $(GEN_S_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,Ice) $^

serveramd.exe: $(SAMD_SRCS) $(GEN_AMD_SRCS) $(GEN_SAMD_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,Ice) $^
