# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

PKG		= iceboxcs
LIBNAME		= $(PKG).dll
TARGETS		= $(bindir)/iceboxnet.exe $(bindir)/$(LIBNAME)

L_SRCS		= AssemblyInfo.cs
I_SRCS		= Server.cs ServiceManagerI.cs

SLICE_SRCS	= $(SDIR)/IceBox.ice

SDIR		= $(slicedir)/IceBox
GDIR		= generated

include $(top_srcdir)/config/Make.rules.cs

MCSFLAGS	:= $(MCSFLAGS) -target:exe

SLICE2CSFLAGS	:= $(SLICE2CSFLAGS) --checksum --ice -I. -I$(slicedir)

$(bindir)/iceboxnet.exe: $(I_SRCS) $(bindir)/$(LIBNAME)
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,$(PKG)) $(call ref,icecs) $(I_SRCS)

$(bindir)/$(LIBNAME): $(L_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -target:library -out:$(bindir)/$(LIBNAME) -unsafe $(call ref,glacier2cs) $(call ref,icecs) $^

ifeq ($(NOGAC),)

install:: all
	$(GACUTIL) -i $(bindir)/$(LIBNAME)

else

install:: all
	$(call installlibrary,$(bindir)/iceboxcs.dll,$(install_bindir))

endif

install:: all
	$(call installprogram,$(bindir)/iceboxnet.exe,$(install_bindir))

include .depend
