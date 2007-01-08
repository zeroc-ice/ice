# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

PKG		= icegridcs
LIBNAME		= $(PKG).dll
TARGETS		= $(bindir)/$(LIBNAME)

SRCS		= AssemblyInfo.cs

SLICE_SRCS	= $(SDIR)/Admin.ice \
		  $(SDIR)/Observer.ice \
		  $(SDIR)/Descriptor.ice \
		  $(SDIR)/Exception.ice \
		  $(SDIR)/FileParser.ice \
		  $(SDIR)/Locator.ice \
		  $(SDIR)/Query.ice \
		  $(SDIR)/Registry.ice \
		  $(SDIR)/Session.ice \
		  $(SDIR)/UserAccountMapper.ice

SDIR		= $(slicedir)/IceGrid
GDIR		= generated

include $(top_srcdir)/config/Make.rules.cs

MCSFLAGS	:= $(MCSFLAGS) -target:library -out:$(TARGETS) -unsafe
MCSFLAGS	:= $(MCSFLAGS) -keyfile:$(top_srcdir)/config/IcecsKey.snk

SLICE2CSFLAGS	:= $(SLICE2CSFLAGS) --ice -I$(slicedir)

$(TARGETS):: $(SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) $(call ref,glacier2cs) $(call ref,icecs) $(subst /,$(DSEP),$^)

ifeq ($(NOGAC),)

install:: all
	$(GACUTIL) -i $(bindir)/$(LIBNAME)

else

install:: all
	$(call installlibrary,$(bindir)/icegridcs.dll,$(install_bindir))

endif

include .depend
