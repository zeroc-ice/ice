# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../../..

TARGETS		= client.exe server.exe

C_SRCS		= AllTests.cs Client.cs
S_SRCS		= Server.cs TestI.cs

SLICE_SRCS	= $(SDIR)/Test.ice
SLICE_S_SRCS	= ServerPrivate.ice

SDIR		= .

GDIR		= generated

include $(top_srcdir)/config/Make.rules

MCSFLAGS	:= $(MCSFLAGS) -target:exe

SLICE2CSFLAGS	:= $(SLICE2CSFLAGS) -I.

client.exe: $(C_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:$(LIBS) $^

server.exe: $(S_SRCS) $(GEN_SRCS) $(SGEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:$(LIBS) $^

install:: all
	$(INSTALL) $(libdir)/$(LIBNAME) $(install_libdir)

include .depend
