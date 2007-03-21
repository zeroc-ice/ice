# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

CLIENT		= client.exe
MFCCLIENT	= mfcclient.exe

!ifdef BUILD_MFC

TARGETS		= $(MFCCLIENT)

!else

TARGETS		= $(CLIENT)

!endif

OBJS		= Chat.obj \
		  PingThread.obj \
		  Router.obj \
		  Session.obj

COBJS		= Client.obj

MOBJS		= ChatClient.obj \
		  ChatClientDlg.obj \
		  ChatConfigDlg.obj \
		  LogI.obj \
		  stdafx.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(COBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

SLICE2CPPEFLAGS = -I. --ice $(SLICE2CPPEFLAGS)

!ifdef BUILD_MFC
CPPFLAGS	= -I. $(CPPFLAGS) -D_AFXDLL
!else
CPPFLAGS	= -I. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN -WX
!endif

!if "$(OPTIMIZE_SPEED)" != "yes" & "$(OPTIMIZE_SIZE)" != "yes"
CPDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
MPDBFLAGS        = /pdb:$(MFCCLIENT:.exe=.pdb)
!endif

$(CLIENT): $(OBJS) $(COBJS)
	$(LINK) $(LDFLAGS) $(CPDBFLAGS) $(OBJS) $(COBJS) /out:$@ $(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(MFCCLIENT): $(OBJS) $(MOBJS) ChatClient.res
	$(LINK) $(LDFLAGS) $(MPDBFLAGS) /subsystem:windows $(OBJS) $(MOBJS) ChatClient.res /out:$@ $(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

ChatClient.res: ChatClient.rc
	rc.exe ChatClient.rc

!ifndef BUILD_MFC

clean::
	del /q Chat.cpp Chat.h
	del /q Router.cpp Router.h
	del /q Session.cpp Session.h

$(EVERYTHING)::
	$(MAKE) -nologo /f Makefile.mak BUILD_MFC=1 $@

!endif

!include .depend
