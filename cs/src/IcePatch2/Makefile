# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

PKG		= icepatch2cs
LIBNAME		= $(PKG).dll
TARGETS		= $(bindir)/$(LIBNAME)

SRCS		= AssemblyInfo.cs

SLICE_SRCS	= $(SDIR)/FileInfo.ice \
		  $(SDIR)/FileServer.ice

SDIR		= $(slicedir)/IcePatch2
GDIR		= generated

include $(top_srcdir)/config/Make.rules.cs

MCSFLAGS	:= $(MCSFLAGS) -target:library -out:$(TARGETS) -unsafe
MCSFLAGS	:= $(MCSFLAGS) -keyfile:$(top_srcdir)/config/IcecsKey.snk

SLICE2CSFLAGS	:= $(SLICE2CSFLAGS) -I$(slicedir) --ice

$(TARGETS):: $(SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) $(call ref,icecs) $(subst /,$(DSEP),$^)

install:: all
	$(call installassembly,$(TARGETS),$(PKG))

include .depend
