# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

TARGETS		= client.exe server.exe

C_SRCS		= Client.cs
S_SRCS		= GreetI.cs Server.cs

SLICE_SRCS	= $(SDIR)/Greet.ice

SDIR		= .

GDIR		= generated

include $(top_srcdir)/config/Make.rules.cs

MCSFLAGS	:= $(MCSFLAGS) -target:exe

client.exe: $(C_SRCS) $(GEN_SRCS) MyGreeting.dll
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,Ice) -r:MyGreeting.dll $(C_SRCS) $(GEN_SRCS)

server.exe: $(S_SRCS) $(GEN_SRCS) MyGreeting.dll
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,Ice) -r:MyGreeting.dll $(S_SRCS) $(GEN_SRCS)

MyGreeting.dll: MyGreeting.cs
	$(MCS) $(MCSFLAGS) -target:library -out:MyGreeting.dll MyGreeting.cs

clean::
	rm -f MyGreeting.dll

include .depend
