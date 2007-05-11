# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

CPPFLAGS        = $(MFC_CPPFLAGS) -I. $(CPPFLAGS) -DICEE_PURE_CLIENT -DVC_EXTRALEAN
!if "$(STATICLIBS)" != "yes"
CPPFLAGS	= $(CPPFLAGS) -D_AFXDLL
!endif

!if "$(OPTIMIZE_SPEED)" != "yes" && "$(OPTIMIZE_SIZE)" != "yes"
PDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
!endif

!if "$(EMBEDDED_DEVICE)" == ""

RESFILE         = HelloClient.res
HelloClient.res: HelloClient.rc
        $(RC) $(RCFLAGS) HelloClient.rc

!else

RESFILE         = HelloClientCE.res
HelloClientCE.res: HelloClientCE.rc
        $(RC) $(RCFLAGS) HelloClientCE.rc

!endif

$(CLIENT): $(OBJS) $(COBJS) $(RESFILE)
	$(LINK) $(LDFLAGS) $(MFC_LDFLAGS) $(PDBFLAGS) $(OBJS) $(COBJS) $(RESFILE) /out:$@ $(MFC_MINLIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest

clean::
	del /q Hello.cpp Hello.h
	del /q HelloClient.res HelloClientCE.res

!include .depend
