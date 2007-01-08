# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

TARGETS		= publisher.exe consumer.exe server.exe

P_SRCS		= Publisher.cs
C_SRCS		= Consumer.cs
S_SRCS		= QueueI.cs Server.cs

SLICE_SRCS	= $(SDIR)/Queue.ice

SDIR		= .

GDIR		= generated

include $(top_srcdir)/config/Make.rules.cs

MCSFLAGS	:= $(MCSFLAGS) -target:exe
SLICE2CSFLAGS	:= -I$(slicedir) $(SLICE2CSFLAGS)

publisher.exe: $(P_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,icecs) $(subst /,$(DSEP),$^)

consumer.exe: $(C_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,icecs) $(subst /,$(DSEP),$^)

server.exe: $(S_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ $(call ref,icecs) $(subst /,$(DSEP),$^)

include .depend
