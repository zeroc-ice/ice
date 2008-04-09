# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../../..

TARGETS		= client.exe

C_SRCS		= AllTests.cs Client.cs

SLICE_SRCS	= $(SDIR)/Test.ice $(SDIR)/Forward.ice
SLICE_C_SRCS	= $(SDIR)/ClientPrivate.ice

SDIR		= .

GDIR		= generated

include $(top_srcdir)/config/Make.rules.cs

MCSFLAGS	:= $(MCSFLAGS) -target:exe

SLICE2SLFLAGS	:= $(SLICE2SLFLAGS) -I.

client.exe: $(C_SRCS) $(GEN_SRCS) $(CGEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,IceSL) $(subst /,$(DSEP),$^)

include .depend
