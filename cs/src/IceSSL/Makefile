# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

PKG		= IceSSL
LIBNAME		= $(PKG).dll
TARGETS		= $(bindir)/$(LIBNAME)
POLICY_TARGET	= $(POLICY).dll

SRCS		= AcceptorI.cs \
		  AssemblyInfo.cs \
		  ConnectorI.cs \
		  EndpointI.cs \
		  Instance.cs \
		  Plugin.cs \
		  PluginI.cs \
		  RFC2253.cs \
		  TransceiverI.cs \
		  TrustManager.cs \
		  Util.cs

SLICE_SRCS	= $(SDIR)/Endpoint.ice

SDIR		= $(slicedir)/IceSSL
GDIR		= generated

include $(top_srcdir)/config/Make.rules.cs

MCSFLAGS	:= $(MCSFLAGS) -target:library -out:$(TARGETS) -warnaserror-
MCSFLAGS	:= $(MCSFLAGS) -keyfile:$(KEYFILE)
MCSFLAGS	:= $(MCSFLAGS) /doc:$(bindir)/$(PKG).xml /nowarn:1591

SLICE2CSFLAGS   := $(SLICE2CSFLAGS) --ice -I$(slicedir)

$(TARGETS):: $(SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) $(call ref,Ice) $(subst /,$(DSEP),$^)

install:: all
	(cd $(bindir); $(call installassembly,$(LIBNAME),$(PKG)); $(call installpolicy,$(POLICY)); \
	 $(call installmdb,$(LIBNAME).mdb); \
	 $(call installdata,$(PKG).xml,$(install_bindir)))

ifeq ($(GACINSTALL),yes)
install:: all
	$(call installdata,../../lib/pkgconfig/$(PKG).pc,$(install_pkgconfigdir))
endif

include .depend
