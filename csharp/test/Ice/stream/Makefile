# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

TARGETS		= client.exe

C_SRCS		= Client.cs AllTests.cs ../../TestCommon/TestApp.cs

SLICE_SRCS	= $(SDIR)/Test.ice

SDIR		= .

GDIR		= generated

include $(top_srcdir)/config/Make.rules.cs

MCSFLAGS	:= $(MCSFLAGS) -target:exe

SLICE2CSFLAGS	:= $(SLICE2CSFLAGS) --stream -I. -I$(slicedir)

client.exe: $(C_SRCS) $(GEN_SRCS) Serializable.dll
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,Ice) -r:Serializable.dll $(C_SRCS) $(GEN_SRCS)

Serializable.dll: Serializable.cs
	$(MCS) $(MCSFLAGS) -target:library -out:Serializable.dll /keyfile:$(KEYFILE) Serializable.cs

clean::
	rm -f Serializable.dll
