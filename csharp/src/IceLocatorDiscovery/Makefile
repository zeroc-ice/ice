# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

PKG		= IceLocatorDiscovery
LIBNAME		= $(PKG).dll
TARGETS		= $(assembliesdir)/$(LIBNAME)
POLICY_TARGET	= $(POLICY).dll

SRCS		= AssemblyInfo.cs \
		  PluginI.cs

SLICE_SRCS	= $(SDIR)/IceLocatorDiscovery.ice

SDIR		= $(slicedir)/IceLocatorDiscovery
GDIR		= generated

include $(top_srcdir)/config/Make.rules.cs

MCSFLAGS	:= $(MCSFLAGS) -target:library -out:$(TARGETS) -warnaserror-
MCSFLAGS	:= $(MCSFLAGS) -keyfile:$(KEYFILE)
MCSFLAGS	:= $(MCSFLAGS) /doc:$(assembliesdir)/$(PKG).xml /nowarn:1591

SLICE2CSFLAGS   := $(SLICE2CSFLAGS) --ice -I$(slicedir)

$(TARGETS):: $(SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) $(call ref,Ice) $(subst /,$(DSEP),$^)

install:: all
	(cd $(assembliesdir); $(call installassembly,$(LIBNAME),$(PKG)); $(call installpolicy,$(POLICY)); \
	 $(call installmdb,$(LIBNAME).mdb); \
	 $(call installdata,$(PKG).xml,$(DESTDIR)$(install_assembliesdir)))

ifeq ($(GACINSTALL),yes)
install:: all
	$(call installdata,../../lib/pkgconfig/$(PKG).pc,$(DESTDIR)$(install_pkgconfigdir))
endif

clean::
	-rm -f $(assembliesdir)/$(PKG).xml
