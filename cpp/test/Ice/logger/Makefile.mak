# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

CLIENT1		= client1.exe
CLIENT2		= client2.exe
CLIENT3		= client3.exe
CLIENT4		= client4.exe
CLIENT5		= client5.exe

TARGETS		= $(CLIENT1) $(CLIENT2) $(CLIENT3) $(CLIENT4) $(CLIENT5)

C1OBJS		= .\Client1.obj
C2OBJS		= .\Client2.obj
C3OBJS		= .\Client3.obj
C4OBJS		= .\Client4.obj
C5OBJS		= .\Client5.obj

OBJS		= $(C1OBJS) \
		  $(C2OBJS) \
		  $(C3OBJS) \
		  $(C4OBJS) \
		  $(C5OBJS)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../include $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(GENERATE_PDB)" == "yes"
C1PDBFLAGS        = /pdb:$(CLIENT1:.exe=.pdb)
C2PDBFLAGS        = /pdb:$(CLIENT2:.exe=.pdb)
C3PDBFLAGS        = /pdb:$(CLIENT3:.exe=.pdb)
C4PDBFLAGS        = /pdb:$(CLIENT4:.exe=.pdb)
C5PDBFLAGS        = /pdb:$(CLIENT5:.exe=.pdb)
!endif

$(CLIENT1): $(C1OBJS)
	$(LINK) $(LD_EXEFLAGS) $(C1PDBFLAGS) $(SETARGV) $(C1OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(CLIENT2): $(C2OBJS)
	$(LINK) $(LD_EXEFLAGS) $(C2PDBFLAGS) $(SETARGV) $(C2OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(CLIENT3): $(C3OBJS)
	$(LINK) $(LD_EXEFLAGS) $(C3PDBFLAGS) $(SETARGV) $(C3OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(CLIENT4): $(C4OBJS)
	$(LINK) $(LD_EXEFLAGS) $(C4PDBFLAGS) $(SETARGV) $(C4OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(CLIENT5): $(C5OBJS)
	$(LINK) $(LD_EXEFLAGS) $(C5PDBFLAGS) $(SETARGV) $(C5OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest
