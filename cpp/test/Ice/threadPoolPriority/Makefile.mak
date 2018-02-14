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
SERVER		= server.exe
SERVERCUSTOM	= servercustom.exe

TARGETS		= $(CLIENT) $(SERVER) $(SERVERCUSTOM)

SLICE_OBJS	= .\Test.obj

COBJS		= $(SLICE_OBJS) \
		  .\Client.obj \
		  .\AllTests.obj

SOBJS		= $(SLICE_OBJS) \
		  .\TestI.obj \
		  .\Server.obj

SCOBJS		= $(SLICE_OBJS) \
		  .\TestI.obj \
		  .\ServerCustomThreadPool.obj

OBJS		= $(COBJS) \
		  $(SOBJS) \
		  $(SCOBJS)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(GENERATE_PDB)" == "yes"
CPDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
SCPDBFLAGS       = /pdb:$(SERVERCUSTOM:.exe=.pdb)
!endif

$(CLIENT): $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(SETARGV) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SERVER): $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SETARGV) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SERVERCUSTOM): $(SCOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SCPDBFLAGS) $(SETARGV) $(SCOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q Test.cpp Test.h
