# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

TARGETS		= client.exe server.exe collocated.exe

C_SRCS		= AllTests.cs Client.cs EmptyI.cs ObjectFactoryI.cs ServantLocatorI.cs
S_SRCS		= EmptyI.cs ObjectFactoryI.cs ServantLocatorI.cs Server.cs ThrowerI.cs
COL_SRCS	= AllTests.cs Collocated.cs EmptyI.cs ObjectFactoryI.cs ServantLocatorI.cs ThrowerI.cs

SLICE_SRCS	= $(SDIR)/Test.ice \
		  $(SDIR)/TestAMD.ice

SDIR		= .

GDIR		= generated

include $(top_srcdir)/config/Make.rules

MCSFLAGS	:= $(MCSFLAGS) -target:exe

SLICE2CSFLAGS	:= $(SLICE2CSFLAGS) -I.

client.exe: $(C_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:$(LIBS) $^

server.exe: $(S_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:$(LIBS) $^

collocated.exe: $(COL_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:$(LIBS) $^

install:: all
	$(INSTALL) $(libdir)/$(LIBNAME) $(install_libdir)

include .depend
