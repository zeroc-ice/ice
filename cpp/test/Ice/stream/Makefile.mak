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

COBJS		= Test.obj \
		  Client.obj

SRCS		= $(COBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

SLICE2CPPFLAGS	= --stream $(SLICE2CPPFLAGS)
CPPFLAGS	= -I. -I../../include $(CPPFLAGS)

!if "$(BORLAND_HOME)" == "" & "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
!endif

$(CLIENT): $(COBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)

clean::
	del /q Test.cpp Test.h

!include .depend
