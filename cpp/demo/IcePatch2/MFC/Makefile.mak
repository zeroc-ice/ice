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

OBJS		= PatchClient.obj \
		  PatchClientDlg.obj \
		  stdafx.obj
		
SRCS		= $(OBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS) -D_AFXDLL -DVC_EXTRALEAN
LINKWITH	= icepatch2$(LIBSUFFIX).lib $(LIBS)

!if "$(BORLAND_HOME)" == "" & "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
!endif

$(CLIENT): $(OBJS) $(COBJS) PatchClient.res
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) /subsystem:windows $(OBJS) $(COBJS) PatchClient.res \
	  $(PREOUT)$@ $(PRELIBS)$(LINKWITH)

PatchClient.res: PatchClient.rc
	rc.exe PatchClient.rc

clean::
	del /q PatchClient.res

!include .depend
