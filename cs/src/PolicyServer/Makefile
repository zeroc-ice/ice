# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

POLICYSERVER	= $(bindir)/policyserver.exe
TARGETS		= $(POLICYSERVER)
SRCS		= AssemblyInfo.cs PolicyServer.cs

include $(top_srcdir)/config/Make.rules.cs

MCSFLAGS	:= $(MCSFLAGS) -target:exe


$(POLICYSERVER): $(SRCS) $(assembliesdir)/$(LIBNAME)
	$(MCS) $(MCSFLAGS) -out:$@ $(SRCS)

install:: all
	$(call installprogram,$(POLICYSERVER),$(DESTDIR)$(install_bindir))
	$(call installmdb,$(POLICYSERVER).mdb)
