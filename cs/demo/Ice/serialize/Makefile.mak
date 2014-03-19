# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

TARGETS		= client.exe server.exe

C_SRCS		= Client.cs
S_SRCS		= GreetI.cs Server.cs

GEN_SRCS	= $(GDIR)\Greet.cs

SDIR		= .

GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak.cs

MCSFLAGS	= $(MCSFLAGS) -target:exe

client.exe: $(C_SRCS) $(GEN_SRCS) MyGreeting.dll
	$(MCS) $(MCSFLAGS) -out:$@ -r:"$(refdir)\Ice.dll" -r:MyGreeting.dll $(C_SRCS) $(GEN_SRCS)

server.exe: $(S_SRCS) $(GEN_SRCS) MyGreeting.dll
	$(MCS) $(MCSFLAGS) -out:$@ -r:"$(refdir)\Ice.dll" -r:MyGreeting.dll $(S_SRCS) $(GEN_SRCS)

MyGreeting.dll: MyGreeting.cs
	$(MCS) $(MCSFLAGS) -target:library -out:MyGreeting.dll MyGreeting.cs

clean::
	del /q MyGreeting.dll

!include .depend.mak
