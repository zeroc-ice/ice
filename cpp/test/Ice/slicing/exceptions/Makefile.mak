# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..\..

CLIENT		= client.exe
SERVER		= server.exe
SERVERAMD	= serveramd.exe

TARGETS		= $(CLIENT) $(SERVER) $(SERVERAMD)

SLICE_OBJS	= .\ClientPrivate.obj \
		  .\ServerPrivate.obj \
		  .\ServerPrivateAMD.obj \
		  .\Test.obj \
		  .\TestAMD.obj

COBJS		= .\Test.obj \
		  .\Client.obj \
		  .\ClientPrivate.obj \
		  .\AllTests.obj

SOBJS		= .\Test.obj \
		  .\ServerPrivate.obj \
		  .\TestI.obj \
		  .\Server.obj

SAMDOBJS	= .\TestAMD.obj \
		  .\ServerPrivateAMD.obj \
		  .\TestAMDI.obj \
		  .\ServerAMD.obj

OBJS		= $(COBJS) \
		  $(SOBJS) \
		  $(SAMDOBJS)

!include $(top_srcdir)/config/Make.rules.mak

ICECPPFLAGS	= -I. $(ICECPPFLAGS)

CPPFLAGS	= -I. -I../../../include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(GENERATE_PDB)" == "yes"
CPDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
SAPDBFLAGS       = /pdb:$(SERVERAMD:.exe=.pdb)
!endif

$(CLIENT): $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(SETARGV) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SERVER): $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SETARGV) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SERVERAMD): $(SAMDOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SAPDBFLAGS) $(SETARGV) $(SAMDOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q Test.cpp Test.h
	del /q TestAMD.cpp TestAMD.h
	del /q ClientPrivate.cpp ClientPrivate.h
	del /q ServerPrivate.cpp ServerPrivate.h
	del /q ServerPrivateAMD.cpp ServerPrivateAMD.h
