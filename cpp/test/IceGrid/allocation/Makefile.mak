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
VERIFIER        = verifier.exe

TARGETS		= $(CLIENT) $(SERVER) $(VERIFIER)


SLICE_OBJS	= .\Test.obj

COBJS		= $(SLICE_OBJS) \
		  .\Client.obj \
		  .\AllTests.obj

SOBJS		= $(SLICE_OBJS) \
		  .\TestI.obj \
		  .\Server.obj

VOBJS		= .\PermissionsVerifier.obj

OBJS		= $(COBJS) \
		  $(SOBJS) \
		  $(VOBJS)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
LINKWITH	= $(LIBS)  

!if "$(GENERATE_PDB)" == "yes"
CPDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
VPDBFLAGS        = /pdb:$(VERIFIER:.exe=.pdb)
!endif

$(CLIENT): $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(LDEXEFLAGS) $(CPDBFLAGS) $(SETARGV) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SERVER): $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SETARGV) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(VERIFIER): $(VOBJS)
	$(LINK) $(LD_EXEFLAGS) $(VPDBFLAGS) $(SETARGV) $(VOBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q Test.cpp Test.h
	del /q build.txt
	if exist db\node rmdir /s /q db\node 
	if exist db\registry rmdir /s /q db\registry 
	if exist db\node-1 rmdir /s /q db\node-1
	if exist db\node-2 rmdir /s /q db\node-2
	if exist db\replica-1 rmdir /s /q db\replica-1

!if "$(OPTIMIZE)" == "yes"

all::
	@echo release > build.txt

!else

all::
	@echo debug > build.txt

!endif