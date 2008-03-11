# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

PKG		= IceBox
LIBNAME		= $(bindir)/$(PKG).dll
ICEBOXNET	= $(bindir)/iceboxnet.exe
TARGETS		= $(ICEBOXNET) $(LIBNAME)

L_SRCS		= AssemblyInfo.cs
I_SRCS		= Server.cs ServiceManagerI.cs

SLICE_SRCS	= $(SDIR)/IceBox.ice

SDIR		= $(slicedir)/IceBox
GDIR		= generated

include $(top_srcdir)/config/Make.rules.cs

EXE_MCSFLAGS	:= $(MCSFLAGS) -target:exe

LIB_MCSFLAGS	:= $(MCSFLAGS) -target:library -out:$(LIBNAME) -unsafe
LIB_MCSFLAGS	:= $(LIB_MCSFLAGS) -keyfile:$(KEYFILE)

SLICE2CSFLAGS	:= $(SLICE2CSFLAGS) --checksum --ice -I. -I$(slicedir)

$(ICEBOXNET): $(I_SRCS) $(LIBNAME)
	$(MCS) $(EXE_MCSFLAGS) -out:$@ $(call ref,$(PKG)) $(call ref,Ice) $(I_SRCS)

$(LIBNAME): $(L_SRCS) $(GEN_SRCS)
	$(MCS) $(LIB_MCSFLAGS) $(call ref,Glacier2) $(call ref,Ice) $(subst /,$(DSEP),$^)

install:: all
	$(call installassembly,$(LIBNAME),$(PKG))

install:: all
	$(call installprogram,$(ICEBOXNET),$(install_bindir))

include .depend
