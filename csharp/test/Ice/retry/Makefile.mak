# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

TARGETS		= client.exe server.exe collocated.exe

C_SRCS		= AllTests.cs Client.cs ..\..\TestCommon\TestApp.cs Instrumentation.cs SystemFailure.cs
S_SRCS		= RetryI.cs Server.cs SystemFailure.cs
COL_SRCS	= AllTests.cs Collocated.cs ..\..\TestCommon\TestApp.cs Instrumentation.cs SystemFailure.cs RetryI.cs

GEN_SRCS	= $(GDIR)\Test.cs

SDIR		= .

GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak.cs

MCSFLAGS	= $(MCSFLAGS) -target:exe

client.exe: $(C_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:"$(refdir)\Ice.dll" $(C_SRCS) $(GEN_SRCS)

server.exe: $(S_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:"$(refdir)\Ice.dll" $(S_SRCS) $(GEN_SRCS)

collocated.exe: $(COL_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:"$(refdir)\Ice.dll" $(COL_SRCS) $(GEN_SRCS)
