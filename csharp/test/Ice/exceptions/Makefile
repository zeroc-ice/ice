# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

TARGETS		= client.exe server.exe collocated.exe serveramd.exe

C_SRCS		= AllTests.cs Client.cs EmptyI.cs ObjectFactoryI.cs ServantLocatorI.cs ../../TestCommon/TestApp.cs
S_SRCS		= EmptyI.cs ObjectFactoryI.cs ServantLocatorI.cs Server.cs ThrowerI.cs
COL_SRCS	= AllTests.cs Collocated.cs EmptyI.cs ObjectFactoryI.cs ServantLocatorI.cs ThrowerI.cs \
	          ../../TestCommon/TestApp.cs
SAMD_SRCS	= EmptyI.cs ObjectFactoryI.cs ServantLocatorI.cs Server.cs ThrowerAMDI.cs

SLICE_SRCS	= $(SDIR)/Test.ice
SLICE_AMD_SRCS  = $(SDIR)/TestAMD.ice

SDIR		= .

GDIR		= generated

include $(top_srcdir)/config/Make.rules.cs

MCSFLAGS	:= $(MCSFLAGS) -target:exe

SLICE2CSFLAGS	:= $(SLICE2CSFLAGS) -I. -I$(slicedir)

client.exe: $(C_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,Ice) $(subst /,$(DSEP),$^)

server.exe: $(S_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,Ice) $(subst /,$(DSEP),$^)

collocated.exe: $(COL_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,Ice) $(subst /,$(DSEP),$^)

serveramd.exe: $(SAMD_SRCS) $(GEN_AMD_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,Ice) $(subst /,$(DSEP),$^)
