# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..\..

CLIENT		= client.exe

TARGETS		= $(CLIENT)

OBJS		= Hello.obj \
		  HelloClient.obj \
		  HelloClientDlg.obj \
		  stdafx.obj
		
SRCS		= $(OBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS) -D_AFXDLL

!if "$(BORLAND_HOME)" == "" & "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
!endif

$(CLIENT): $(OBJS) $(COBJS) HelloClient.res
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) /subsystem:windows $(OBJS) $(COBJS) HelloClient.res \
	  $(PREOUT)$@ $(PRELIBS)$(LIBS)

Hello.cpp Hello.h: Hello.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) Hello.ice

HelloClient.res: HelloClient.rc
	rc.exe HelloClient.rc

clean::
	del /q Hello.cpp Hello.h
	del /q HelloClient.res

!include .depend
