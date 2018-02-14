# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

TARGETS		= client.exe server.exe collocated.exe

C_SRCS		= AllTests.cs Client.cs EmptyI.cs ../../TestCommon/TestApp.cs
S_SRCS		= AI.cs BI.cs CI.cs DI.cs EI.cs EmptyI.cs FI.cs GI.cs HI.cs Server.cs
COL_SRCS	= AI.cs AllTests.cs BI.cs CI.cs Collocated.cs DI.cs EI.cs EmptyI.cs FI.cs GI.cs HI.cs \
		  ../../TestCommon/TestApp.cs

SLICE_SRCS	= $(SDIR)/Test.ice

SDIR		= .

GDIR		= generated

include $(top_srcdir)/config/Make.rules.cs

MCSFLAGS	:= $(MCSFLAGS) -target:exe

SLICE2CSFLAGS	:= $(SLICE2CSFLAGS) -I.

client.exe: $(C_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,Ice) $(subst /,$(DSEP),$^)

server.exe: $(S_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,Ice) $(subst /,$(DSEP),$^)

collocated.exe: $(COL_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,Ice) $(subst /,$(DSEP),$^)
