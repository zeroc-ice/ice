# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

TARGETS		= client.exe server.exe collocated.exe serveramd.exe

C_SRCS		= AllTests.cs Client.cs Twoways.cs TwowaysAMI.cs
S_SRCS		= CheckedCastLocator.cs MyDerivedClassI.cs Server.cs TestCheckedCastI.cs
COL_SRCS	= AllTests.cs CheckedCastLocator.cs Collocated.cs MyDerivedClassI.cs TestCheckedCastI.cs \
		  Twoways.cs TwowaysAMI.cs
SAMD_SRCS	= CheckedCastLocator.cs MyDerivedClassAMDI.cs Server.cs TestCheckedCastAMDI.cs

SLICE_SRCS	= $(SDIR)/Test.ice
SLICE_AMD_SRCS	= $(SDIR)/TestAMD.ice

SDIR		= .

GDIR		= generated

include $(top_srcdir)/config/Make.rules

MCSFLAGS	:= $(MCSFLAGS) -target:exe

SLICE2CSFLAGS	:= $(SLICE2CSFLAGS) --ice -I. -I$(slicedir)

client.exe: $(C_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:$(libdir)/icecs.dll $^

server.exe: $(S_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:$(libdir)/icecs.dll $^

collocated.exe: $(COL_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:$(libdir)/icecs.dll $^

serveramd.exe: $(SAMD_SRCS) $(GEN_AMD_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:$(libdir)/icecs.dll $^

include .depend
