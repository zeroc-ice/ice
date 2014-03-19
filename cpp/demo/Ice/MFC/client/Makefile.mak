# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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

CPPFLAGS	= -I. $(CPPFLAGS) -D_AFXDLL -DVC_EXTRALEAN -D_UNICODE

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
!endif

$(CLIENT): $(OBJS) $(COBJS) HelloClient.res
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) /entry:wWinMainCRTStartup /subsystem:windows $(OBJS) $(COBJS) HelloClient.res \
	  $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q Hello.cpp Hello.h
	del /q HelloClient.res

!include .depend.mak
