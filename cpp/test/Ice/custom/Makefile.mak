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
SERVER		= server.exe
SERVERAMD	= serveramd.exe
COLLOCATED	= collocated.exe

TARGETS		= $(CLIENT) $(SERVER) $(SERVERAMD) $(COLLOCATED)

COBJS           = Test.obj \
		  Wstring.obj \
		  Client.obj \
		  AllTests.obj \
		  MyByteSeq.obj \
		  StringConverterI.obj 

SOBJS           = Test.obj \
		  Wstring.obj \
		  TestI.obj \
		  WstringI.obj \
		  Server.obj \
		  MyByteSeq.obj \
		  StringConverterI.obj

SAMDOBJS        = TestAMD.obj \
		  TestAMDI.obj \
		  WstringAMD.obj \
		  WstringAMDI.obj \
		  ServerAMD.obj \
		  MyByteSeq.obj \
		  StringConverterI.obj

COLOBJS         = Test.obj \
		  Wstring.obj \
		  TestI.obj \
		  WstringI.obj \
		  Collocated.obj \
		  AllTests.obj \
		  MyByteSeq.obj \
		  StringConverterI.obj

SRCS		= $(COBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp) \
		  $(SAMDOBJS:.obj=.cpp) \
		  $(COLOBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

SLICE2CPPFLAGS	= --stream $(SLICE2CPPFLAGS) -DWIN32_LEAN_AND_MEAN
CPPFLAGS	= -I. -I../../include $(CPPFLAGS)
!if "$(BORLAND_HOME)" == ""
CPPFLAGS	= $(CPPFLAGS) -Zm200
!endif

!if "$(BORLAND_HOME)" == "" & "$(OPTIMIZE)" != "yes"
CPDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
SAPDBFLAGS       = /pdb:$(SERVERAMD:.exe=.pdb)
COPDBFLAGS       = /pdb:$(COLLOCATED:.exe=.pdb)
!endif

$(CLIENT): $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)

$(SERVER): $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)

$(SERVERAMD): $(SAMDOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SAPDBFLAGS) $(SAMDOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)

$(COLLOCATED): $(COLOBJS)
	$(LINK) $(LD_EXEFLAGS) $(COPDBFLAGS) $(COLOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)

clean::
	del /q Test.cpp Test.h
	del /q TestAMD.cpp TestAMD.h
	del /q Wstring.cpp Wstring.h
	del /q WstringAMD.cpp WstringAMD.h

!include .depend
