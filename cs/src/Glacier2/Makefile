# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

PKG		= glacier2cs
LIBNAME		= $(PKG).dll
TARGETS		= $(bindir)/$(LIBNAME)

SRCS		= AssemblyInfo.cs

SLICE_SRCS	= $(SDIR)/PermissionsVerifier.ice \
		  $(SDIR)/PermissionsVerifierF.ice \
		  $(SDIR)/Router.ice \
		  $(SDIR)/RouterF.ice \
		  $(SDIR)/Session.ice \
		  $(SDIR)/SessionF.ice

SDIR		= $(slicedir)/Glacier2
GDIR		= generated

include $(top_srcdir)/config/Make.rules.cs

MCSFLAGS	:= $(MCSFLAGS) -target:library -out:$(TARGETS) -unsafe

SLICE2CSFLAGS	:= $(SLICE2CSFLAGS) -I$(slicedir)

$(TARGETS):: $(SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) $(call ref,icecs) $^

ifeq ($(NOGAC),)

install:: all
	$(GACUTIL) -i $(bindir)/$(LIBNAME)

endif

include .depend
