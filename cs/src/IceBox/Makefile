# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

PKG		= IceBox
LIBNAME		= $(PKG).dll
ICEBOXNET	= $(bindir)/iceboxnet.exe
TARGETS		= $(ICEBOXNET) $(bindir)/$(LIBNAME)
POLICY_TARGET	= $(POLICY).dll

L_SRCS		= AssemblyInfoLib.cs
I_SRCS		= AssemblyInfoExe.cs Server.cs ServiceManagerI.cs

SLICE_SRCS	= $(SDIR)/IceBox.ice

SDIR		= $(slicedir)/IceBox
GDIR		= generated

include $(top_srcdir)/config/Make.rules.cs

EXE_MCSFLAGS	:= $(MCSFLAGS) -target:exe

LIB_MCSFLAGS	:= $(MCSFLAGS) -target:library -out:$(bindir)/$(LIBNAME)
LIB_MCSFLAGS	:= $(LIB_MCSFLAGS) -keyfile:$(KEYFILE)
LIB_MCSFLAGS	:= $(LIB_MCSFLAGS) /doc:$(bindir)/$(PKG).xml /nowarn:1591

SLICE2CSFLAGS	:= $(SLICE2CSFLAGS) --checksum --ice -I. -I$(slicedir)

$(ICEBOXNET): $(I_SRCS) $(bindir)/$(LIBNAME)
	$(MCS) $(EXE_MCSFLAGS) -out:$@ $(call ref,$(PKG)) $(call ref,Ice) $(I_SRCS)

$(bindir)/$(LIBNAME): $(L_SRCS) $(GEN_SRCS)
	$(MCS) $(LIB_MCSFLAGS) $(call ref,Glacier2) $(call ref,Ice) $(subst /,$(DSEP),$^)

install:: all
	(cd $(bindir); $(call installassembly,$(LIBNAME),$(PKG)); $(call installpolicy,$(POLICY)); \
	 $(call installmdb,$(LIBNAME).mdb); \
	 $(call installdata,$(PKG).xml,$(install_bindir)))

install:: all
	$(call installprogram,$(ICEBOXNET),$(install_bindir))
	$(call installdata,$(ICEBOXNET:.exe=.exe.config),$(install_bindir))
	$(call installmdb,$(ICEBOXNET).mdb)

ifeq ($(GACINSTALL),yes)
install:: all
	$(call installdata,../../lib/pkgconfig/$(PKG).pc,$(install_pkgconfigdir))
endif

include .depend
