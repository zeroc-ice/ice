# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

CLIENT		= client.exe

TARGETS		= $(CLIENT)

OBJS		= Client.obj

SRCS		= $(OBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../include $(CPPFLAGS)

!if "$(BORLAND_HOME)" == "" & "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
!endif

$(CLIENT): $(OBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(BASELIBS)

!include .depend
