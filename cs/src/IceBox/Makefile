# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

PKG		= iceboxcs
LIBNAME		= $(bindir)/$(PKG).dll
ICEBOXNET	= $(bindir)/iceboxnet.exe
ICEBOXADMINNET	= $(bindir)/iceboxadminnet.exe
TARGETS		= $(ICEBOXNET) $(ICEBOXADMINNET) $(LIBNAME)

L_SRCS		= AssemblyInfo.cs
I_SRCS		= Server.cs ServiceManagerI.cs
A_SRCS		= Admin.cs

SLICE_SRCS	= $(SDIR)/IceBox.ice

SDIR		= $(slicedir)/IceBox
GDIR		= generated

include $(top_srcdir)/config/Make.rules.cs

MCSFLAGS	:= $(MCSFLAGS) -target:exe

LIB_MCSFLAGS	:= -target:library -out:$(LIBNAME) -unsafe
LIB_MCSFLAGS	:= $(LIB_MCSFLAGS) -keyfile:$(top_srcdir)/config/IcecsKey.snk

SLICE2CSFLAGS	:= $(SLICE2CSFLAGS) --checksum --ice -I. -I$(slicedir)

$(ICEBOXNET): $(I_SRCS) $(LIBNAME)
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,$(PKG)) $(call ref,icecs) $(I_SRCS)

$(ICEBOXADMINNET): $(A_SRCS) $(LIBNAME)
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,$(PKG)) $(call ref,icecs) $(A_SRCS)

$(LIBNAME): $(L_SRCS) $(GEN_SRCS)
	$(MCS) $(LIB_MCSFLAGS) $(call ref,glacier2cs) $(call ref,icecs) $(subst /,$(DSEP),$^)

ifeq ($(NOGAC),)

install:: all
	$(GACUTIL) -i $(LIBNAME)

else

install:: all
	$(call installlibrary,$(LIBNAME),$(install_bindir))

endif

install:: all
	$(call installprogram,$(ICEBOXNET),$(install_bindir))
	$(call installprogram,$(ICEBOXADMINNET),$(install_bindir))

include .depend
