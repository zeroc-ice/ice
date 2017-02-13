# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

PKG			= Glacier2
LIBNAME		= $(PKG).dll
TARGETS		= $(assembliesdir)/$(LIBNAME)
POLICY_TARGET	= $(POLICY).dll

SRCS		= Application.cs \
		  AssemblyInfo.cs \
		  SessionCallback.cs \
		  SessionFactoryHelper.cs \
		  SessionHelper.cs

SLICE_SRCS	= $(SDIR)/Metrics.ice \
		  $(SDIR)/PermissionsVerifier.ice \
		  $(SDIR)/Router.ice \
		  $(SDIR)/Session.ice \
		  $(SDIR)/SSLInfo.ice

SDIR		= $(slicedir)/Glacier2
GDIR		= generated

include $(top_srcdir)/config/Make.rules.cs

MCSFLAGS	:= $(MCSFLAGS) -target:library -out:$(TARGETS) -warnaserror-
MCSFLAGS	:= $(MCSFLAGS) -keyfile:$(KEYFILE)
MCSFLAGS	:= $(MCSFLAGS) /doc:$(assembliesdir)/$(PKG).xml /nowarn:1591

SLICE2CSFLAGS	:= $(SLICE2CSFLAGS) -I$(slicedir) --ice

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
