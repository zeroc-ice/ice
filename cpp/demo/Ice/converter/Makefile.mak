# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

MAXWARN         = yes

CLIENT1		= client1.exe
CLIENT2		= client2.exe
SERVER		= server.exe

TARGETS		= $(CLIENT1) $(CLIENT2) $(SERVER)

SLICE_OBJS	= .\Greet.obj

C1OBJS		= $(SLICE_OBJS) \
		  .\Client.obj \
		  .\ClientWithConverter.obj \
		  .\StringConverterI.obj


C2OBJS		= $(SLICE_OBJS) \
		  .\Client.obj \
		  .\ClientWithoutConverter.obj

SOBJS		= $(SLICE_OBJS) \
		  .\GreetI.obj \
		  .\Server.obj

OBJS		= $(C1OBJS) \
		  $(C2OBJS) \
		  $(SOBJS)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(GENERATE_PDB)" == "yes"
C1PDBFLAGS        = /pdb:$(CLIENT1:.exe=.pdb)
C2PDBFLAGS        = /pdb:$(CLIENT2:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
!endif

$(CLIENT1): $(C1OBJS)
	$(LINK) $(LD_EXEFLAGS) $(C1PDBFLAGS) $(SETARGV) $(C1OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(CLIENT2): $(C2OBJS)
	$(LINK) $(LD_EXEFLAGS) $(C2PDBFLAGS) $(SETARGV) $(C2OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SERVER): $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SETARGV) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q Greet.cpp Greet.h
