# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..\..

SERVER		= server.exe

TARGETS		= $(SERVER)

SLICE_OBJS	= Hello.obj

OBJS		= $(SLICE_OBJS) \
		  HelloI.obj \
		  HelloServer.obj \
		  HelloServerDlg.obj \
		  LogI.obj \
		  stdafx.obj

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS) -D_AFXDLL -DVC_EXTRALEAN -D_UNICODE

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
!endif

$(SERVER): $(OBJS) HelloServer.res
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) /entry:wWinMainCRTStartup /subsystem:windows $(OBJS) HelloServer.res \
	  $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q Hello.cpp Hello.h
	del /q HelloServer.res
