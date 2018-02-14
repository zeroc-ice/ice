# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

CLIENT		= client.exe

TARGETS		= $(CLIENT)

OBJS		= .\TestBase.obj \
		  .\CreateTest.obj \
		  .\AliveTest.obj \
		  .\StartTest.obj \
		  .\SleepTest.obj \
		  .\RecMutexTest.obj \
		  .\MutexTest.obj \
		  .\MonitorMutexTest.obj \
		  .\MonitorRecMutexTest.obj \
		  .\CountDownLatchTest.obj \
		  .\TestSuite.obj \
		  .\Client.obj

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
!endif

$(CLIENT): $(OBJS)
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) $(SETARGV) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(BASELIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest
