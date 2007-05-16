# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

TARGETS		= client.exe server.exe collocated.exe

C_SRCS		= AllTests.cs BI.cs CI.cs DI.cs II.cs JI.cs HI.cs Client.cs
S_SRCS		= InitialI.cs BI.cs CI.cs DI.cs II.cs JI.cs HI.cs Server.cs UnexpectedObjectExceptionTestI.cs
COL_SRCS	= AllTests.cs BI.cs CI.cs DI.cs II.cs JI.cs HI.cs Collocated.cs InitialI.cs \
                  UnexpectedObjectExceptionTestI.cs

GEN_SRCS	= $(GDIR)\Test.cs

SDIR		= .

GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak.cs

MCSFLAGS	= $(MCSFLAGS) -target:exe

SLICE2CSFLAGS	= $(SLICE2CSFLAGS) -I. --stream

client.exe: $(C_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:$(bindir)\icecs.dll $(C_SRCS) $(GEN_SRCS)

server.exe: $(S_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:$(bindir)\icecs.dll $(S_SRCS) $(GEN_SRCS)

collocated.exe: $(COL_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:$(bindir)\icecs.dll $(COL_SRCS) $(GEN_SRCS)

!include .depend
