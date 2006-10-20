# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

TARGETS		= $(bindir)\generatecs.exe

SRCS		= Generate.cs

!include $(top_srcdir)\config\Make.rules.mak

MCSFLAGS	= $(MCSFLAGS) -target:exe

$(TARGETS): $(SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ $(SRCS)

install::
	copy $(TARGETS) $(install_bindir)

!include .depend
