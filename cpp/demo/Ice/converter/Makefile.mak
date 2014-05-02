# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

CLIENT1		= client1.exe
CLIENT2		= client2.exe
SERVER		= server.exe

TARGETS		= $(CLIENT1) $(CLIENT2) $(SERVER)

OBJS		= Greet.obj

C1OBJS		= Client.obj \
		  StringConverterI.obj \
		  ClientWithConverter.obj

C2OBJS		= Client.obj \
		  ClientWithoutConverter.obj
		  
SOBJS		= GreetI.obj \
		  Server.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(COBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(GENERATE_PDB)" == "yes"
C1PDBFLAGS        = /pdb:$(CLIENT1:.exe=.pdb)
C2PDBFLAGS        = /pdb:$(CLIENT2:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
!endif

$(CLIENT1): $(OBJS) $(C1OBJS)
	$(LINK) $(LD_EXEFLAGS) $(C1PDBFLAGS) $(SETARGV) $(OBJS) $(C1OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(CLIENT2): $(OBJS) $(C2OBJS)
	$(LINK) $(LD_EXEFLAGS) $(C2PDBFLAGS) $(SETARGV) $(OBJS) $(C2OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SERVER): $(OBJS) $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SETARGV) $(OBJS) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q Greet.cpp Greet.h

!include .depend.mak
