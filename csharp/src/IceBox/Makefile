# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

PKG		= IceBox
LIBNAME		= $(PKG).dll
ICEBOXNET	= $(bindir)/iceboxnet.exe
TARGETS		= $(ICEBOXNET) $(assembliesdir)/$(LIBNAME)
POLICY_TARGET	= $(POLICY).dll

L_SRCS		= AssemblyInfo.cs
I_SRCS		= AssemblyInfoExe.cs Server.cs ServiceManagerI.cs

SLICE_SRCS	= $(SDIR)/IceBox.ice

SDIR		= $(slicedir)/IceBox
GDIR		= generated

include $(top_srcdir)/config/Make.rules.cs

EXE_MCSFLAGS	:= $(MCSFLAGS) -target:exe

LIB_MCSFLAGS	:= $(MCSFLAGS) -target:library -out:$(assembliesdir)/$(LIBNAME)
LIB_MCSFLAGS	:= $(LIB_MCSFLAGS) -keyfile:$(KEYFILE)
LIB_MCSFLAGS	:= $(LIB_MCSFLAGS) /doc:$(assembliesdir)/$(PKG).xml /nowarn:1591

SLICE2CSFLAGS	:= $(SLICE2CSFLAGS) --checksum --ice -I. -I$(slicedir)

$(ICEBOXNET): $(I_SRCS) $(assembliesdir)/$(LIBNAME)
	$(MCS) $(EXE_MCSFLAGS) -out:$@ $(call ref,$(PKG)) $(call ref,Ice) $(I_SRCS)

$(assembliesdir)/$(LIBNAME): $(L_SRCS) $(GEN_SRCS)
	$(MCS) $(LIB_MCSFLAGS) $(call ref,Ice) $(subst /,$(DSEP),$^)

install:: all
	(cd $(assembliesdir); $(call installassembly,$(LIBNAME),$(PKG)); $(call installpolicy,$(POLICY)); \
	 $(call installmdb,$(LIBNAME).mdb); \
	 $(call installdata,$(PKG).xml,$(DESTDIR)$(install_assembliesdir)))

install:: all
	$(call installprogram,$(ICEBOXNET),$(DESTDIR)$(install_bindir))
	$(call installdata,$(top_srcdir)/../man/man1/iceboxnet.1,$(DESTDIR)$(install_mandir))
	$(call installmdb,$(ICEBOXNET).mdb)

ifeq ($(GACINSTALL),yes)
install:: all
	$(call installdata,../../lib/pkgconfig/$(PKG).pc,$(DESTDIR)$(install_pkgconfigdir))
endif

clean::
	-rm -f $(assembliesdir)/$(PKG).xml
