# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

TARGETS		= client.exe server.exe LoggerPlugin.dll HelloPlugin.dll

C_SRCS		= Client.cs
S_SRCS		= Server.cs
L_SRCS		= LoggerI.cs LoggerPluginFactoryI.cs
H_SRCS		= HelloI.cs HelloPluginI.cs HelloPluginFactoryI.cs

GEN_SRCS	= $(GDIR)\Hello.cs

SDIR		= .

GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak.cs

MCSFLAGS	= $(MCSFLAGS) -target:exe

client.exe: $(C_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:"$(refdir)\Ice.dll" $(C_SRCS) $(GEN_SRCS)

server.exe: $(S_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:"$(refdir)\Ice.dll" $(S_SRCS)

LoggerPlugin.dll: $(L_SRCS)
	$(MCS) $(MCSFLAGS) -target:library -out:$@ -r:"$(refdir)\Ice.dll" $(L_SRCS)

HelloPlugin.dll: $(H_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -target:library -out:$@ -r:"$(refdir)\Ice.dll" $(H_SRCS) $(GEN_SRCS)

!include .depend.mak
