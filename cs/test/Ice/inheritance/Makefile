#**********************************************************************
#
# Copyright (c) 2003
# ZeroC, Inc.
# Billerica, MA, USA
#
# All Rights Reserved.
#
# Ice is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by
# the Free Software Foundation.
#
# **********************************************************************

top_srcdir	= ../../..

TARGETS		= client.exe server.exe collocated.exe

C_SRCS		= AllTests.cs Client.cs
S_SRCS		= CAI.cs CBI.cs CCI.cs CDI.cs IAI.cs IB1I.cs IB2I.cs ICI.cs InitialI.cs Server.cs
COL_SRCS	= AllTests.cs CAI.cs CBI.cs CCI.cs CDI.cs Collocated.cs IAI.cs IB1I.cs IB2I.cs ICI.cs InitialI.cs

SLICE_SRCS	= $(SDIR)/Test.ice

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
