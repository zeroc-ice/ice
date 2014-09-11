# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

TARGETS		= publisher.exe subscriber.exe

C_SRCS		= Publisher.cs
S_SRCS		= Subscriber.cs

SLICE_SRCS	= $(SDIR)/Clock.ice

SDIR		= .

GDIR		= generated

include $(top_srcdir)/config/Make.rules.cs

MCSFLAGS	:= $(MCSFLAGS) -target:exe

publisher.exe: $(C_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,Ice) $(call ref,IceStorm) $(subst /,$(DSEP),$^)

subscriber.exe: $(S_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,Ice) $(call ref,IceStorm) $(subst /,$(DSEP),$^)

clean::
	-rm -rf db/*

include .depend
