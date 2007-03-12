# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

TARGETS		= $(bindir)\generatevb.exe

SRCS		= Generate.vb

!include $(top_srcdir)\config\Make.rules.mak.vb

VBCFLAGS	= $(VBCFLAGS) -target:exe

$(TARGETS): $(SRCS)
	$(VBC) $(VBCFLAGS) -out:$@ $(SRCS)

!include .depend
