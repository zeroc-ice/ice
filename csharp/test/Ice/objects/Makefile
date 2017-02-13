# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

TARGETS		= client.exe server.exe collocated.exe

C_SRCS		= AllTests.cs BI.cs CI.cs DI.cs EI.cs FI.cs II.cs JI.cs HI.cs Client.cs ../../TestCommon/TestApp.cs
S_SRCS		= InitialI.cs BI.cs CI.cs DI.cs EI.cs FI.cs II.cs JI.cs HI.cs Server.cs \
		  UnexpectedObjectExceptionTestI.cs
COL_SRCS	= AllTests.cs BI.cs CI.cs DI.cs EI.cs FI.cs II.cs JI.cs HI.cs Collocated.cs InitialI.cs \
		  UnexpectedObjectExceptionTestI.cs ../../TestCommon/TestApp.cs

SLICE_SRCS	= $(SDIR)/Test.ice

SDIR		= .

GDIR		= generated

include $(top_srcdir)/config/Make.rules.cs

MCSFLAGS	:= $(MCSFLAGS) -target:exe

SLICE2CSFLAGS	:= $(SLICE2CSFLAGS) -I. --stream

client.exe: $(C_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,Ice) $(subst /,$(DSEP),$^)

server.exe: $(S_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,Ice) $(subst /,$(DSEP),$^)

collocated.exe: $(COL_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,Ice) $(subst /,$(DSEP),$^)
