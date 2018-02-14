# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

TARGETS		= client.exe server.exe serveramd.exe

C_SRCS		= AllTests.cs Client.cs \
	          ../../TestCommon/TestApp.cs
S_SRCS		= TestI.cs Server.cs
SAMD_SRCS	= TestAMDI.cs ServerAMD.cs

SLICE_SRCS	= $(SDIR)/Test.ice
SLICE_AMD_SRCS	= $(SDIR)/TestAMD.ice

SDIR		= .

GDIR		= generated

include $(top_srcdir)/config/Make.rules.cs

MCSFLAGS	:= $(MCSFLAGS) -target:exe

SLICE2CSFLAGS	:= $(SLICE2CSFLAGS) -I. -I$(slicedir) --stream

client.exe: $(C_SRCS) $(GEN_SRCS) Serializable.dll
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,Ice) -r:Serializable.dll $(C_SRCS) $(GEN_SRCS)

server.exe: $(S_SRCS) $(GEN_SRCS) Serializable.dll
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,Ice) -r:Serializable.dll $(S_SRCS) $(GEN_SRCS)

serveramd.exe: $(SAMD_SRCS) $(GEN_AMD_SRCS) Serializable.dll
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,Ice) -r:Serializable.dll $(SAMD_SRCS) $(GEN_AMD_SRCS)

Serializable.dll:
	$(MCS) $(MCSFLAGS) -target:library -out:Serializable.dll /keyfile:$(KEYFILE) SerializableClass.cs

clean::
	rm -f Serializable.dll
