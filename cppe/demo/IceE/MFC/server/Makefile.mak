# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..\..

SERVER		= server.exe

TARGETS		= $(SERVER)

OBJS		= Hello.obj \
		  HelloI.obj \
		  HelloServer.obj \
		  HelloServerDlg.obj \
		  LogI.obj \
		  stdafx.obj
		
SRCS		= $(OBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= $(MFC_CPPFLAGS) -I. $(CPPFLAGS) -DVC_EXTRALEAN
!if "$(SMART_DEVICE)" == "" | "$(STATICLIBS)" != "yes"
CPPFLAGS        = $(CPPFLAGS) -D_AFXDLL
!endif

!if "$(OPTIMIZE_SPEED)" != "yes" & "$(OPTIMIZE_SIZE)" != "yes"
PDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
!endif

!if "$(SMART_DEVICE)" == ""

RESFILE         = HelloServer.res
HelloServer.res: HelloServer.rc
        $(RC) HelloServer.rc

!else

RESFILE         = HelloServerCE.res
HelloServerCE.res: HelloServerCE.rc
	$(RC) HelloServerCE.rc

!endif


$(SERVER): $(OBJS) $(COBJS) $(RESFILE)
	$(LINK) $(LDFLAGS) $(MFC_LDFLAGS) $(PDBFLAGS) $(OBJS) $(COBJS) $(RESFILE) /out:$@ $(LIBS)

clean::
	del /q Hello.cpp Hello.h
	del /q HelloServer.res HelloServerCE.res

!include .depend
