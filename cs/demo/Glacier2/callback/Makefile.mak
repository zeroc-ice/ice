# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

TARGETS		= client.exe server.exe sessionserver.exe
TARGETS_CONFIG	= $(TARGETS:.exe=.exe.config)

C_SRCS		= CallbackReceiverI.cs Client.cs
S_SRCS		= CallbackI.cs Server.cs
SS_SRCS		= SessionI.cs SessionManagerI.cs SessionServer.cs

GEN_SRCS	= $(GDIR)\Callback.cs

SDIR		= .

GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak.cs

MCSFLAGS	= $(MCSFLAGS) -target:exe

SLICE2CSFLAGS	= $(SLICE2CSFLAGS) --ice -I. -I$(slicedir)

client.exe: $(C_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:$(refdir)\Ice.dll -r:$(refdir)\Glacier2.dll $(C_SRCS) $(GEN_SRCS)

server.exe: $(S_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:$(refdir)\Ice.dll $(S_SRCS) $(GEN_SRCS)

sessionserver.exe: $(SS_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:$(refdir)\Ice.dll -r:$(refdir)\Glacier2.dll $(SS_SRCS)

!include .depend
